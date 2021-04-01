#pragma once

#include <WriterPlugin.h>

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
	BinExporterDialog(QWidget* parent, std::vector<QString> dataSetNames) :
		QDialog(parent), writeButton(tr("Write file"))
	{
		setWindowTitle(tr("Binary Exporter"));

		for (QString& dataSetName : dataSetNames)
			dataSetsBox.addItem(dataSetName);

		if (dataSetNames.size() == 0)
			dataSetsBox.setEnabled(false);

		QLabel* dataSetsLabel = new QLabel("DataSets");
		QLabel* indicesLabel = new QLabel("Save only indices");

		writeButton.setDefault(true);

		connect(&writeButton, &QPushButton::pressed, this, &BinExporterDialog::closeDialogAction);
		connect(this, &BinExporterDialog::closeDialog, this, &QDialog::accept);

		QHBoxLayout *layout = new QHBoxLayout();
		layout->addWidget(dataSetsLabel);
		layout->addWidget(&dataSetsBox);
		layout->addWidget(indicesLabel);
		layout->addWidget(&saveIndices);
		layout->addWidget(&writeButton);
		setLayout(layout);
	}

signals:
	void closeDialog(QString dataSetName, bool onlyIndices);

public slots:
	// Pass selected data set name from BinExporterDialog to BinExporter (dialogClosed)
	void closeDialogAction() {
		emit closeDialog(dataSetsBox.currentText(), saveIndices.isChecked());
	}

private:
	QComboBox dataSetsBox;
	QCheckBox saveIndices;
	QPushButton writeButton;
};

// =============================================================================
// View
// =============================================================================

class BinExporter : public QObject, public WriterPlugin
{
	Q_OBJECT
public:
	BinExporter() : WriterPlugin("BIN Exporter"), _dataSetName("") { }
	~BinExporter(void) override;

	void init() override;

	void writeData() Q_DECL_OVERRIDE;

public slots:
	void dialogClosed(QString dataSetName, bool onlyIdices);

private:
	/*! Get data set contents from core
	 *
	 * \param dataSetName Data set name to request from core
	*/
	DataContent retrieveDataSetContent(QString dataSetName);

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

	QString _dataSetName;
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

	WriterPlugin* produce() override;
};