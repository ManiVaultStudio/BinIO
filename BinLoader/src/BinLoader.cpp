#include "BinLoader.h"

#include <PointData.h>

#include "Set.h"

#include <QtCore>
#include <QtDebug>

#include <vector>
#include <QInputDialog>

#include <fstream>
#include <iterator>
#include <vector>

Q_PLUGIN_METADATA(IID "nl.tudelft.BinLoader")

using namespace hdps;

// =============================================================================
// View
// =============================================================================

BinLoader::~BinLoader(void)
{

}

void BinLoader::init()
{

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

    // convert binary data to float vector
    std::vector<float> data;
    if (ok == QDialog::Accepted) {
        if (inputDialog.getDataType() == BinaryDataType::FLOAT)
        {
            for (int i = 0; i < contents.size() / 4; i++)
            {
                float f = ((float*) contents.data())[i];

                data.push_back(f);
            }
        }
        else if (inputDialog.getDataType() == BinaryDataType::UBYTE)
        {
            for (int i = 0; i < contents.size(); i++)
            {
                unsigned char c = (unsigned char) contents[i];

                float f = (float)(c);
                data.push_back(f);
            }
        }
    }

    // add data to the core
    if (ok && !inputDialog.getDatasetName().isEmpty()) {

        Dataset<Points> point_data;

        auto sourceDataset = inputDialog.getSourceDataset();

        if (sourceDataset.isValid())
            point_data = _core->createDerivedData(inputDialog.getDatasetName(), sourceDataset);
        else
            point_data = _core->addDataset("Points", inputDialog.getDatasetName());

        point_data->setData(data.data(), data.size() / inputDialog.getNumberOfDimensions(), inputDialog.getNumberOfDimensions());

        qDebug() << "Number of dimensions: " << point_data->getNumDimensions();

        _core->notifyDataAdded(point_data);

        qDebug() << "BIN file loaded. Num data points: " << point_data->getNumPoints();
    }
}

QIcon BinLoaderFactory::getIcon() const
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
    _dataTypeAction(this, "Data type", { "Float", "Unsigned byte" }),
    _numberOfDimensionsAction(this, "Number of dimensions", 1, 1000000, 1, 1),
    _isDerivedAction(this, "Mark as derived", false, false),
    _datasetPickerAction(this, "Source dataset"),
    _loadAction(this, "Load"),
    _groupAction(this)
{
    setWindowTitle(tr("Binary Loader"));

    _datasetNameAction.setMayReset(false);
    _dataTypeAction.setMayReset(false);
    _numberOfDimensionsAction.setMayReset(false);
    _isDerivedAction.setMayReset(false);
    _datasetPickerAction.setMayReset(false);
    _loadAction.setMayReset(false);

    _numberOfDimensionsAction.setDefaultWidgetFlags(IntegralAction::WidgetFlag::SpinBox);

    // Load some settings
    _dataTypeAction.setCurrentIndex(binLoader.getSetting("DataType").toInt());
    _numberOfDimensionsAction.setValue(binLoader.getSetting("NumberOfDimensions").toInt());

    _groupAction << _datasetNameAction;
    _groupAction << _dataTypeAction;
    _groupAction << _numberOfDimensionsAction;
    _groupAction << _isDerivedAction;
    _groupAction << _datasetPickerAction;
    _groupAction << _loadAction;

    auto layout = new QVBoxLayout();

    layout->setMargin(0);
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

        accept();
    });
}
