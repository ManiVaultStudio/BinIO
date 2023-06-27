#pragma once

#include <actions/DatasetPickerAction.h>

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

class BinLoader;

enum BinaryDataType
{
    FLOAT, UBYTE
};

class BinLoadingInputDialog : public QDialog
{
    Q_OBJECT

public:
    BinLoadingInputDialog(QWidget* parent, BinLoader& binLoader, QString fileName);

    /** Get preferred size */
    QSize sizeHint() const override {
        return QSize(400, 50);
    }

    /** Get minimum size hint*/
    QSize minimumSizeHint() const override {
        return sizeHint();
    }

    /** Get the GUI name of the loaded dataset */
    QString getDatasetName() const {
        return _datasetNameAction.getString();
    }

    /** Get the binary data type */
    BinaryDataType getDataType() const {
        if (_dataTypeAction.getCurrentIndex() == 0) // Float32
            return BinaryDataType::FLOAT;
        // else if (_dataTypeAction.getCurrentIndex() == 1) // Unsigned Byte (Uint8)
        return BinaryDataType::UBYTE;
    }

    /** Get the number of dimensions */
    std::int32_t getNumberOfDimensions() const {
        return _numberOfDimensionsAction.getValue();
    }

    /** Get the desired storage type */
    QString getStoreAs() const {
        return _storeAsAction.getCurrentText();
    }

    /** Get whether the dataset will be marked as derived */
    bool getIsDerived() const {
        return _isDerivedAction.isChecked();
    }

    /** Get smart pointer to dataset (if any) */
    hdps::Dataset<hdps::DatasetImpl> getSourceDataset() {
        return _datasetPickerAction.getCurrentDataset();
    }

protected:
    hdps::gui::StringAction            _datasetNameAction;             /** Dataset name action */
    hdps::gui::OptionAction            _dataTypeAction;                /** Data type action */
    hdps::gui::IntegralAction          _numberOfDimensionsAction;      /** Number of dimensions action */
    hdps::gui::OptionAction            _storeAsAction;                 /** Store as action */
    hdps::gui::ToggleAction            _isDerivedAction;               /** Mark dataset as derived action */
    hdps::gui::DatasetPickerAction     _datasetPickerAction;           /** Dataset picker action for picking source datasets */
    hdps::gui::TriggerAction           _loadAction;                    /** Load action */
    hdps::gui::GroupAction             _groupAction;                   /** Group action */
};

// =============================================================================
// View
// =============================================================================

class BinLoader : public LoaderPlugin
{
    Q_OBJECT
public:
    BinLoader(const PluginFactory* factory) : LoaderPlugin(factory) { }
    ~BinLoader(void) override;

    void init() override;

    void loadData() Q_DECL_OVERRIDE;
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

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    LoaderPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};