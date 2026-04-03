#include "BinExporter.h"

#include <actions/PluginTriggerAction.h>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>

#include <numeric>
#include <vector>

Q_PLUGIN_METADATA(IID "studio.manivault.BinExporter")

using namespace mv;
using namespace mv::gui;

BinExporter::BinExporter(const PluginFactory* factory) :
    WriterPlugin(factory),
    _onlyIdices(false)
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

    const int ok = inputDialog.exec();

    if (ok == QDialog::Accepted) {

        // Let the user choose the save path
        const QString registryEntry = "directoryPath";
        const auto directoryPath = QDir(getSetting(registryEntry, "").toString());

        const auto inputDataset = getInputDataset<Points>();
        const QString fileName = QFileDialog::getSaveFileName(
            nullptr, 
            tr("Save data set"), 
            directoryPath.filePath(inputDataset->text() + ".bin"),
            tr("Binary file (*.bin);;All Files (*)"));

        // Only continue when the dialog has not been not canceled and the file name is non-empty.
        if (fileName.isNull() || fileName.isEmpty())
        {
            qDebug() << "BinExporter: No data written to disk - File name empty";
            return;
        }
        else
        {
            // store the directory name
            setSetting(registryEntry, QFileInfo(fileName).absolutePath());

            // get data from core
            const DataContent dataContent = retrieveDataSetContent(inputDataset);
            writeVecToBinary(fileName, dataContent.dataVals);   // writes to .bin file
            writeInfoTextForBinary(fileName, dataContent);      // writes to .txt file
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

DataContent BinExporter::retrieveDataSetContent(const mv::Dataset<Points>& dataset) const {
    DataContent dataContent;
    std::vector<float> dataFromSet;

    // Get number of enabled dimensions
    const std::uint64_t numDimensions = dataset->getNumDimensions();
    const std::uint64_t numPoints = dataset->getNumPoints();

    if (_onlyIdices) // Instead of saving the data values, you might want to save the IDs of a selection
    {
        std::ranges::transform(dataset->indices, std::back_inserter(dataFromSet), [](const int x) { return static_cast<float>(x); });
        dataContent.onlyIndices = true;
    }
    else
    {
        // Get indices of selected points
        std::vector<unsigned int> pointIDsGlobal = dataset->indices;
        // If points represent all data set, select them all
        if (dataset->isFull()) {
            std::vector<unsigned int> all(numPoints);
            std::iota(std::begin(all), std::end(all), 0);

            std::swap(pointIDsGlobal, all);
        }

        // For all selected points, retrieve values from each dimension
        dataFromSet.reserve(pointIDsGlobal.size() * numDimensions);

        dataset->visitFromBeginToEnd([&dataFromSet, &pointIDsGlobal, &numDimensions](auto beginOfData, auto endOfData)
        {
            for (const auto& pointId : pointIDsGlobal)
            {
                for (unsigned int dimensionId = 0; dimensionId < numDimensions; dimensionId++)
                {
                    const auto index = static_cast<std::uint64_t>(pointId) * numDimensions + dimensionId;
                    dataFromSet.push_back(beginOfData[index]);
                }
            }
        });
    }

    // Data content for writing to disk
    dataContent.dataVals = dataFromSet;
    dataContent.numDimensions = numDimensions;
    dataContent.numPoints = numPoints;

    if (dataset->isDerivedData())
    {
        dataContent.isDerived = true;

        auto sourceData = dataset->getSourceDataset<Points>();

        dataContent.derivedFrom = sourceData->text();
        dataContent.sourceNumDimensions = sourceData->getNumDimensions();
        dataContent.sourceNumPoints = sourceData->getNumPoints();
    }

    return dataContent;
}


// =============================================================================
// Factory
// =============================================================================

BinExporterFactory::BinExporterFactory()
{
    setIconByName("database");
}

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

PluginTriggerActions BinExporterFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this](const Dataset<Points>& dataset) -> BinExporter* {
        return dynamic_cast<BinExporter*>(plugins().requestPlugin(getKind(), { dataset }));
    };

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        if (datasets.count() >= 1) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<BinExporterFactory*>(this), this, "BIN Exporter", "Export dataset to binary file", icon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                for (const auto& dataset : datasets)
                    getPluginInstance(dataset);
            });

            pluginTriggerActions << pluginTriggerAction;
        }
    }

    return pluginTriggerActions;
}

// =============================================================================
// Helper
// =============================================================================

void writeInfoTextForBinary(const QString& writePath, const DataContent& dataContent) {
    const std::string fileName = QFileInfo(writePath).fileName().toStdString();

    std::string infoText;
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

