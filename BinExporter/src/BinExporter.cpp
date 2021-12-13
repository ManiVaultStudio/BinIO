#include "BinExporter.h"

#include "Set.h"

#include <QtCore>
#include <QtDebug>
#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>

#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>

Q_PLUGIN_METADATA(IID "nl.tudelft.BinExporter")

using namespace hdps;

BinExporter::BinExporter(const PluginFactory* factory) :
    WriterPlugin(factory)
{
}

BinExporter::~BinExporter(void)
{
}

void BinExporter::init()
{
}

void BinExporter::writeData()
{
    // Let the user select one of those data sets
    BinExporterDialog inputDialog(nullptr);
    
    inputDialog.setModal(true);

    connect(&inputDialog, &BinExporterDialog::closeDialog, this, [this](bool onlyIdices) {
        _onlyIdices = onlyIdices;
    });

    int ok = inputDialog.exec();

    if ((ok == QDialog::Accepted)) {

        // Let the user chose the save path
        QSettings settings(QLatin1String{ "HDPS" }, QLatin1String{ "Plugins/" } +getKind());
        const QLatin1String directoryPathKey("directoryPath");
        const auto directoryPath = settings.value(directoryPathKey).toString() + "/";
        auto& inputDataset = _input;        // why does getInputDataset<Points>() throw an error? 
        QString fileName = QFileDialog::getSaveFileName(
            nullptr, tr("Save data set"), directoryPath + inputDataset->getGuiName() + ".bin", tr("Binary file (*.bin);;All Files (*)"));

        // Only continue when the dialog has not been not canceled and the file name is non-empty.
        if (fileName.isNull() || fileName.isEmpty())
        {
            qDebug() << "BinExporter: No data written to disk - File name empty";
            return;
        }
        else
        {
            // store the directory name
            settings.setValue(directoryPathKey, QFileInfo(fileName).absolutePath());

            // get data from core
            auto& inputDataset = _input;        // why does getInputDataset<Points>() throw an error? 
            DataContent dataContent = retrieveDataSetContent(inputDataset);
            writeVecToBinary(dataContent.dataVals, fileName);
            writeInfoTextForBinary(fileName, dataContent);
            qDebug() << "BinExporter: Data written to disk - File name: " << fileName;
            return;
        }
    }
    else
    {
        qDebug() << "BinExporter: No data written to disk - No data set selected";
        return;
    }
}

DataContent BinExporter::retrieveDataSetContent(hdps::Dataset<Points> dataSet) {
    DataContent dataContent;
    std::vector<float> dataFromSet;

    // Get number of enabled dimensions
    unsigned int numDimensions = dataSet->getNumDimensions();

    if (_onlyIdices)
    {
        std::transform(dataSet->indices.begin(), dataSet->indices.end(), std::back_inserter(dataFromSet), [](int x) { return (float)x; });
        dataContent.onlyIndices = true;
    }
    else
    {
        // Get indices of selected points
        std::vector<unsigned int> pointIDsGlobal = dataSet->indices;
        // If points represent all data set, select them all
        if (dataSet->isFull()) {
            std::vector<unsigned int> all(dataSet->getNumPoints());
            std::iota(std::begin(all), std::end(all), 0);

            pointIDsGlobal = all;
        }

        // For all selected points, retrieve values from each dimension
        dataFromSet.reserve(pointIDsGlobal.size() * numDimensions);

        dataSet->visitFromBeginToEnd([&dataFromSet, &pointIDsGlobal, &numDimensions](auto beginOfData, auto endOfData)
        {
            for (const auto& pointId : pointIDsGlobal)
            {
                for (unsigned int dimensionId = 0; dimensionId < numDimensions; dimensionId++)
                {
                    const auto index = pointId * numDimensions + dimensionId;
                    dataFromSet.push_back(beginOfData[index]);
                }
            }
        });
    }

    // Data content for writing to disk
    dataContent.dataVals = dataFromSet;
    dataContent.numDimensions = numDimensions;
    dataContent.numPoints = dataSet->getNumPoints();

    if (dataSet->isDerivedData())
    {
        dataContent.isDerived = true;

        auto sourceData = dataSet->getSourceDataset<Points>();

        dataContent.derivedFrom = sourceData->getGuiName();
        dataContent.sourceNumDimensions = sourceData->getNumDimensions();
        dataContent.sourceNumPoints = sourceData->getNumPoints();
    }

    return dataContent;
}

template<typename T>
void BinExporter::writeVecToBinary(std::vector<T> vec, QString writePath) {
    std::ofstream fout(writePath.toStdString(), std::ofstream::out | std::ofstream::binary);
    fout.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(T));
    fout.close();
}


void BinExporter::writeInfoTextForBinary(QString writePath, DataContent& dataContent) {
    std::string infoText;
    std::string fileName = QFileInfo(writePath).fileName().toStdString();

    infoText += fileName + "\n";
    infoText += "Num dimensions: " + std::to_string(dataContent.numDimensions) + "\n";
    infoText += "Num data points: " + std::to_string(dataContent.numPoints) + "\n";
    infoText += "Data type: float \n";			// currently hard=coded	

    if (dataContent.isDerived)
    {
        infoText += "Derived: true \n";
        infoText += "Source data: " + dataContent.derivedFrom.toStdString() + "\n";
        infoText += "Num dimensions (source): " + std::to_string(dataContent.sourceNumDimensions) + "\n";
        infoText += "Num data points (source): " + std::to_string(dataContent.sourceNumPoints) + "\n";
    }

    if (dataContent.onlyIndices)
    {
        infoText += "Contains only indices (e.g. of a selection) \n";
    }

    std::ofstream fout(writePath.section(".", 0, 0).toStdString() + ".txt");
    fout << infoText;
    fout.close();
}

QIcon BinExporterFactory::getIcon() const
{
    return Application::getIconFont("FontAwesome").getIcon("database");
}

// =============================================================================
// Factory
// =============================================================================

WriterPlugin* BinExporterFactory::produce()
{
    return new BinExporter(this);
}

DataTypes BinExporterFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
