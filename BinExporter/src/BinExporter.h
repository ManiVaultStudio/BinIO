#pragma once

#include <WriterPlugin.h>

#include <PointData/PointData.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <fstream>

using namespace mv::plugin;
using namespace mv::gui;

// =============================================================================
// Helper
// =============================================================================

struct DataContent {
    DataContent() : dataVals{}, numDimensions(0), numPoints(0), isDerived(false), onlyIndices(false), derivedFrom(""), sourceNumDimensions(0), sourceNumPoints(0) {};
    std::vector<float> dataVals;
    std::uint64_t numDimensions;
    std::uint64_t numPoints;

    bool isDerived;
    bool onlyIndices;
    QString derivedFrom;
    std::uint64_t sourceNumDimensions;
    std::uint64_t sourceNumPoints;
};

/*! Write vector contents to disk
 * Stores content in little endian binary form.
 * Overrides existing files with at the given path.
 *
 * \param vec Data to write to disk
 * \param writePath Target path
*/
template<typename T>
void writeVecToBinary(const QString& writePath, const std::vector<T>& vec) {
    std::ofstream fout(writePath.toStdString(), std::ofstream::out | std::ofstream::binary);
    fout.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(T));
    fout.close();
}

void writeInfoTextForBinary(const QString& writePath, const DataContent& dataContent);

// =============================================================================
// Loading input box
// =============================================================================

enum BinaryDataType
{
    FLOAT, UBYTE
};

class BinExporterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BinExporterDialog(QWidget* parent) :
        QDialog(parent), writeButton(tr("Write file"))
    {
        setWindowTitle(tr("Binary Exporter"));

        auto* indicesLabel = new QLabel("Save only indices");

        writeButton.setDefault(true);

        connect(&writeButton, &QPushButton::pressed, this, &BinExporterDialog::closeDialogAction);
        connect(this, &BinExporterDialog::closeDialog, this, &QDialog::accept);

        auto*layout = new QHBoxLayout();
        layout->addWidget(indicesLabel);
        layout->addWidget(&saveIndices);
        layout->addWidget(&writeButton);
        setLayout(layout);
    }

signals:
    void closeDialog(bool onlyIndices);

public slots:
    // Pass selected data set name from BinExporterDialog to BinExporter (dialogClosed)
    void closeDialogAction() {
        emit closeDialog(saveIndices.isChecked());
    }

private:
    QCheckBox       saveIndices;
    QPushButton     writeButton;
};

// =============================================================================
// View
// =============================================================================

class BinExporter : public WriterPlugin
{
    Q_OBJECT
public:
    explicit BinExporter(const PluginFactory* factory);
    ~BinExporter(void) override = default;

    void init() override;

    void writeData() Q_DECL_OVERRIDE;

private:
    /*! Get data set contents from core
     *
     * \param dataset Data set to request from core
    */
    DataContent retrieveDataSetContent(const mv::Dataset<Points>& dataset) const;

private:
    bool _onlyIdices;   // save indices, e.g. of a selection instead of data values

};


// =============================================================================
// Factory
// =============================================================================

class BinExporterFactory : public WriterPluginFactory
{
    Q_INTERFACES(mv::plugin::WriterPluginFactory mv::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "studio.manivault.BinExporter"
                          FILE  "PluginInfo.json")

public:
    BinExporterFactory();
    ~BinExporterFactory(void) override = default;

    WriterPlugin* produce() override;

    mv::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};