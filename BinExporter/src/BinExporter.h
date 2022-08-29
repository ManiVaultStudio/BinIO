#pragma once

#include <WriterPlugin.h>

#include "PointData.h"

#include <QDialog>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox> 

using namespace hdps::plugin;

struct DataContent {
    DataContent() : dataVals{}, numDimensions(0), numPoints(0), isDerived(false), onlyIndices(false), derivedFrom(""), sourceNumDimensions(0), sourceNumPoints(0) {};
    std::vector<float> dataVals;
    unsigned int numDimensions;
    unsigned int numPoints;

    bool isDerived;
    bool onlyIndices;
    QString derivedFrom;
    unsigned int sourceNumDimensions;
    unsigned int sourceNumPoints;
};

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
    BinExporterDialog(QWidget* parent) :
        QDialog(parent), writeButton(tr("Write file"))
    {
        setWindowTitle(tr("Binary Exporter"));

        QLabel* indicesLabel = new QLabel("Save only indices");

        writeButton.setDefault(true);

        connect(&writeButton, &QPushButton::pressed, this, &BinExporterDialog::closeDialogAction);
        connect(this, &BinExporterDialog::closeDialog, this, &QDialog::accept);

        QHBoxLayout *layout = new QHBoxLayout();
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
    QPushButton writeButton;
};

// =============================================================================
// View
// =============================================================================

class BinExporter : public WriterPlugin
{
    Q_OBJECT
public:
    BinExporter(const PluginFactory* factory);
    ~BinExporter(void) override;

    void init() override;

    void writeData() Q_DECL_OVERRIDE;

private:
    /*! Get data set contents from core
     *
     * \param dataSetName Data set name to request from core
    */
    DataContent retrieveDataSetContent(hdps::Dataset<Points> dataSet);

    /*! Write vector contents to disk
     * Stores content in little endian binary form.
     * Overrides existing files with at the given path.
     *
     * \param vec Data to write to disk
     * \param writePath Target path
    */
    template<typename T>
    void writeVecToBinary(std::vector<T> vec, QString writePath);

    void writeInfoTextForBinary(QString writePath, DataContent& dataContent);

    bool _onlyIdices;

};


// =============================================================================
// Factory
// =============================================================================

class BinExporterFactory : public WriterPluginFactory
{
    Q_INTERFACES(hdps::plugin::WriterPluginFactory hdps::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.tudelft.BinExporter"
            FILE  "BinExporter.json")

public:
    BinExporterFactory(void) {}
    ~BinExporterFactory(void) override {}

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    WriterPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};