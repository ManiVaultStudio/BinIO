#pragma once

#include <LoaderPlugin.h>

#include <QDialog>
#include <QCheckBox>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>

using namespace hdps::plugin;

// =============================================================================
// Loading input box
// =============================================================================

enum BinaryDataType
{
    FLOAT, UBYTE
};

class BinLoadingInputDialog : public QDialog
{
    Q_OBJECT
public:
    BinLoadingInputDialog(QWidget* parent, QString fileName, QStringList dataSet_guids, QStringList dataSet_gui_names) :
        QDialog(parent), _dataset_guids(dataSet_guids)
    {
        setWindowTitle(tr("Binary Loader"));

        dataNameInput = new QLineEdit();
        dataNameInput->setText(fileName);

        dataTypeInput = new QComboBox();
        dataTypeInput->addItem("Float");
        dataTypeInput->addItem("Unsigned Byte");

        numDimsInput = new QSpinBox();
        numDimsInput->setMinimum(1);
        numDimsInput->setMaximum(INT_MAX);
        numDimsInput->setValue(1);
        numDimsLabel = new QLabel(tr("Number of dimensions:"));
        numDimsLabel->setBuddy(numDimsInput);

        isDerived = new QCheckBox("Derived?");
        dataSetsLabel = new QLabel(tr("Data sets:"));

        // Add gui names of data sets that can be derived from to combo box
        dataSetsBox = new QComboBox();
        dataSetsBox->addItem("");
        for (auto& dataSetName : dataSet_gui_names)
        {
            dataSetsBox->addItem(dataSetName);
        }

        dataSetsBox->setEnabled(false);

        loadButton = new QPushButton(tr("Load file"));
        loadButton->setDefault(true);

        connect(loadButton, &QPushButton::pressed, this, &BinLoadingInputDialog::closeDialogAction);
        connect(this, &BinLoadingInputDialog::closeDialog, this, &QDialog::accept);
        connect(isDerived, &QCheckBox::stateChanged, dataSetsBox, &QLabel::setEnabled);

        QGridLayout *layout = new QGridLayout;
        layout->addWidget(new QLabel(tr("Name:")), 0, 0);

        layout->addWidget(dataNameInput, 0, 1);
        layout->addWidget(numDimsLabel, 0, 2);
        layout->addWidget(numDimsInput, 0, 3);
        layout->addWidget(dataTypeInput, 0, 4);

        layout->addWidget(isDerived, 1, 1);
        layout->addWidget(dataSetsLabel, 1, 2);
        layout->addWidget(dataSetsBox, 1, 3);

        layout->addWidget(loadButton, 1, 5);

        setLayout(layout);
    }

signals:
    void closeDialog(unsigned int numDimensions, BinaryDataType dataType, QString dataSetName, bool isDerived, QString sourceName);

public slots:
    void closeDialogAction()
    {
        BinaryDataType dataType;
        if (dataTypeInput->currentText() == "Float")
            dataType = BinaryDataType::FLOAT;
        else if (dataTypeInput->currentText() == "Unsigned Byte")
            dataType = BinaryDataType::UBYTE;

        // leave source name empty if data is not derived
        if (isDerived->isChecked())
            emit closeDialog(numDimsInput->value(), dataType, dataNameInput->text(), isDerived->isChecked(), _dataset_guids[dataSetsBox->currentIndex()-1]);
        else
            emit closeDialog(numDimsInput->value(), dataType, dataNameInput->text(), isDerived->isChecked(), "");
    }

private:
    QLineEdit* dataNameInput;
    QComboBox* dataTypeInput;
    QLabel* numDimsLabel;
    QSpinBox* numDimsInput;
    QCheckBox* isDerived;
    QComboBox* dataSetsBox;
    QLabel* dataSetsLabel;

    QStringList _dataset_guids;

    QPushButton* loadButton;
};

// =============================================================================
// View
// =============================================================================

class BinLoader : public QObject, public LoaderPlugin
{
    Q_OBJECT
public:
    BinLoader(const PluginFactory* factory) : LoaderPlugin(factory) { }
    ~BinLoader(void) override;

    void init() override;

    void loadData() Q_DECL_OVERRIDE;

public slots:
    void dialogClosed(unsigned int numDimensions, BinaryDataType dataType, QString dataSetName, bool isDerived, QString sourceName);

private:
    unsigned int _numDimensions;
    BinaryDataType _dataType;
    QString _dataSetName;
    bool _isDerived;
    QString _parent_guid;
};


// =============================================================================
// Factory
// =============================================================================

class BinLoaderFactory : public LoaderPluginFactory
{
    Q_INTERFACES(hdps::plugin::LoaderPluginFactory hdps::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.tudelft.BinLoader"
                      FILE  "BinLoader.json")

public:
    BinLoaderFactory(void) {}
    ~BinLoaderFactory(void) override {}

    /** Returns the plugin icon */
    QIcon getIcon() const override;

    LoaderPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};