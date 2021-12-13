#include "BinLoader.h"

#include "PointData.h"
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

    // Get unique identifier and gui names from all point data sets in the core
    auto dataSets = _core->requestAllDataSets(QVector<hdps::DataType> {PointType});

    QStringList dataset_guids;
    QStringList dataset_gui_names;

    for (auto& dataSet : dataSets)
    {
        dataset_gui_names.append(dataSet->getGuiName());
        dataset_guids.append(dataSet->getGuid());
    }

    BinLoadingInputDialog inputDialog(nullptr, QFileInfo(fileName).baseName(), dataset_guids, dataset_gui_names);
    inputDialog.setModal(true);

    // dialogClosed will set all necessary info for adding the data set to the core
    connect(&inputDialog, &BinLoadingInputDialog::closeDialog, this, &BinLoader::dialogClosed);

    // open dialog and wait for user input
    int ok = inputDialog.exec();

    // convert binary data to float vector
    std::vector<float> data;
    if (ok == QDialog::Accepted) {
        if (_dataType == BinaryDataType::FLOAT)
        {
            for (int i = 0; i < contents.size() / 4; i++)
            {
                float f = ((float*) contents.data())[i];

                data.push_back(f);
            }
        }
        else if (_dataType == BinaryDataType::UBYTE)
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
    if (ok && !_dataSetName.isEmpty()) {

        Dataset<Points> point_data;
        if (_isDerived & (_parent_guid != ""))
        {
            auto parent_dataset = _core->requestDataset<Points>(_parent_guid);
            point_data = _core->createDerivedData(_dataSetName, parent_dataset, parent_dataset);
        }   
        else
            point_data = _core->addDataset("Points", _dataSetName);

        point_data->setData(data.data(), data.size() / _numDimensions, _numDimensions);

        qDebug() << "Number of dimensions: " << point_data->getNumDimensions();

        _core->notifyDataAdded(point_data);

        qDebug() << "BIN file loaded. Num data points: " << point_data->getNumPoints();
    }
}

void BinLoader::dialogClosed(unsigned int numDimensions, BinaryDataType dataType, QString dataSetName, bool isDerived, QString source_guid)
{
    _numDimensions = numDimensions;
    _dataType = dataType;
    _dataSetName = dataSetName;
    _isDerived = isDerived;
    _parent_guid = source_guid;
    qDebug() << _numDimensions << _dataType << _dataSetName;
    if (isDerived)
        qDebug() << "Derived from " << _core->requestDataset(_parent_guid)->getGuiName();
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
