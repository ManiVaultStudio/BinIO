#include "BinLoader.h"

#include <PointData/PointData.h>

#include "Set.h"

#include <QtCore>
#include <QtDebug>

#include <vector>
#include <QInputDialog>

#include <fstream>
#include <iterator>
#include <vector>
#include <type_traits>

Q_PLUGIN_METADATA(IID "nl.tudelft.BinLoader")

using namespace hdps;
using namespace hdps::gui;

// =============================================================================
// View
// =============================================================================

BinLoader::~BinLoader(void)
{

}

void BinLoader::init()
{

}

namespace {


template <typename T, typename S>
void readDataAndAddToCore(hdps::Dataset<Points>& point_data, int32_t numDims, const std::vector<char>& contents)
{

    // convert binary data to float vector
    std::vector<S> data;
    auto add_to_data = [&data](auto val) ->void {
        auto c = static_cast<S>(val);
        data.push_back(c);
    };

    if constexpr (std::is_same_v<T, float>) {
        for (int i = 0; i < contents.size() / 4; i++)
        {
            float f = ((float*)contents.data())[i];
            add_to_data(f);
        }
    }
    else if constexpr (std::is_same_v<T, unsigned char>)
    {
        for (int i = 0; i < contents.size(); i++)
        {
            T c = static_cast<T>(contents[i]);
            add_to_data(c);
        }
    }
    else
    {
        qWarning() << "BinLoader.cpp::readDataAndAddToCore: No data loaded. Template typename not implemented.";
    }

    // add data to the core
    point_data->setData(std::move(data), numDims);
    events().notifyDatasetChanged(point_data);

    qDebug() << "Number of dimensions: " << point_data->getNumDimensions();
    qDebug() << "BIN file loaded. Num data points: " << point_data->getNumPoints();

}

// Recursively searches for the data element type that is specified by the selectedDataElementType parameter. 
template <typename T, unsigned N = 0>
void recursiveReadDataAndAddToCore(const QString& selectedDataElementType, hdps::Dataset<Points>& point_data, int32_t numDims, const std::vector<char>& contents)
{
    const QLatin1String nthDataElementTypeName(std::get<N>(PointData::getElementTypeNames()));

    if (selectedDataElementType == nthDataElementTypeName)
    {
        readDataAndAddToCore<T, PointData::ElementTypeAt<N>>(point_data, numDims, contents);
    }
    else
    {
        recursiveReadDataAndAddToCore<T, N + 1>(selectedDataElementType, point_data, numDims, contents);
    }
}

template <>
void recursiveReadDataAndAddToCore<float, PointData::getNumberOfSupportedElementTypes()>(const QString&, hdps::Dataset<Points>&, int32_t, const std::vector<char>&)
{
    // This specialization does nothing, intensionally! 
}

template <>
void recursiveReadDataAndAddToCore<unsigned char, PointData::getNumberOfSupportedElementTypes()>(const QString&, hdps::Dataset<Points>&, int32_t, const std::vector<char>&)
{
    // This specialization does nothing, intensionally! 
}

}

void BinLoader::loadData()
{
    const QString fileName = AskForFileName(tr("BIN Files (*.bin)"));

    // Don't try to load a file if the dialog was cancelled or the file name is empty
    if (fileName.isNull() || fileName.isEmpty())
        return;

    qDebug() << "Loading BIN file: " << fileName;

    // read in binary data
    std::vector<char> contents;
    std::ifstream in(fileName.toStdString(), std::ios::in | std::ios::binary);
    if (in)
    {
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
    }
    else
    {
        throw DataLoadException(fileName, "File was not found at location.");
    }

    BinLoadingInputDialog inputDialog(nullptr, *this, QFileInfo(fileName).baseName());
    inputDialog.setModal(true);

    // open dialog and wait for user input
    int ok = inputDialog.exec();

    if (ok == QDialog::Accepted && !inputDialog.getDatasetName().isEmpty()) {
    
        auto sourceDataset = inputDialog.getSourceDataset();
        auto numDims = inputDialog.getNumberOfDimensions();
        auto storeAs = inputDialog.getStoreAs();

        Dataset<Points> point_data;

        if (sourceDataset.isValid())
            point_data = _core->createDerivedDataset<Points>(inputDialog.getDatasetName(), sourceDataset);
        else
            point_data = _core->addDataset<Points>("Points", inputDialog.getDatasetName());

        events().notifyDatasetAdded(point_data);

        if (inputDialog.getDataType() == BinaryDataType::FLOAT)
        {
            recursiveReadDataAndAddToCore<float>(storeAs, point_data, numDims, contents);
        }
        else if (inputDialog.getDataType() == BinaryDataType::UBYTE)
        {
            recursiveReadDataAndAddToCore<unsigned char>(storeAs, point_data, numDims, contents);
        }
    }

}

QIcon BinLoaderFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return Application::getIconFont("FontAwesome").getIcon("database");
}

// =============================================================================
// Factory
// =============================================================================

LoaderPlugin* BinLoaderFactory::produce()
{
    return new BinLoader(this);
}

DataTypes BinLoaderFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

BinLoadingInputDialog::BinLoadingInputDialog(QWidget* parent, BinLoader& binLoader, QString fileName) :
    QDialog(parent),
    _datasetNameAction(this, "Dataset name", fileName),
    _dataTypeAction(this, "Data type", { "Float", "Unsigned Byte" }),
    _numberOfDimensionsAction(this, "Number of dimensions", 1, 1000000, 1),
	_storeAsAction(this, "Store as"),
    _isDerivedAction(this, "Mark as derived", false),
    _datasetPickerAction(this, "Source dataset"),
    _loadAction(this, "Load"),
    _groupAction(this, "Settings")
{
    setWindowTitle(tr("Binary Loader"));

    _numberOfDimensionsAction.setDefaultWidgetFlags(IntegralAction::WidgetFlag::SpinBox);

    QStringList pointDataTypes;
    for (const char* const typeName : PointData::getElementTypeNames())
    {
        pointDataTypes.append(QString::fromLatin1(typeName));
    }
    _storeAsAction.setOptions(pointDataTypes);

    // Load some settings
    _dataTypeAction.setCurrentIndex(binLoader.getSetting("DataType").toInt());
    _numberOfDimensionsAction.setValue(binLoader.getSetting("NumberOfDimensions").toInt());
    _storeAsAction.setCurrentIndex(binLoader.getSetting("StoreAs").toInt());

    _groupAction.addAction(&_datasetNameAction);
    _groupAction.addAction(&_dataTypeAction);
    _groupAction.addAction(&_numberOfDimensionsAction);
    _groupAction.addAction(&_storeAsAction);
    _groupAction.addAction(&_isDerivedAction);
    _groupAction.addAction(&_datasetPickerAction);
    _groupAction.addAction(&_loadAction);

    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_groupAction.createWidget(this));

    setLayout(layout);

    // Update the state of the dataset picker
    const auto updateDatasetPicker = [this]() -> void {
        if (_isDerivedAction.isChecked()) {

            // Get unique identifier and gui names from all point data sets in the core
            auto dataSets = hdps::Application::core()->requestAllDataSets(QVector<hdps::DataType> {PointType});

            // Assign found dataset(s)
            _datasetPickerAction.setDatasets(dataSets);
        }
        else {

            // Assign found dataset(s)
            _datasetPickerAction.setDatasets(hdps::Datasets());
        }

        // Disable dataset picker when not marked as derived
        _datasetPickerAction.setEnabled(_isDerivedAction.isChecked());
    };

    // Populate source datasets once the dataset is marked as derived
    connect(&_isDerivedAction, &ToggleAction::toggled, this, updateDatasetPicker);

    // Update dataset picker at startup
    updateDatasetPicker();

    // Accept when the load action is triggered
    connect(&_loadAction, &TriggerAction::triggered, this, [this, &binLoader]() {

        // Save some settings
        binLoader.setSetting("DataType", _dataTypeAction.getCurrentIndex());
        binLoader.setSetting("NumberOfDimensions", _numberOfDimensionsAction.getValue());
        binLoader.setSetting("StoreAs", _storeAsAction.getCurrentIndex());

        accept();
    });
}
