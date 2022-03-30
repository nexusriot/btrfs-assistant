#ifndef BTRFSASSISTANT_H
#define BTRFSASSISTANT_H

#include <QInputDialog>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QSignalMapper>
#include <QTranslator>

#include "Btrfs.h"
#include "BtrfsMaintenance.h"
#include "Snapper.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class BtrfsAssistant;
}
QT_END_NAMESPACE

/**
 * @brief The BtrfsAssistant class handles the custom application logic that ties all the various btrfs and snapper systems together.
 */
class BtrfsAssistant : public QMainWindow {
    Q_OBJECT

  public:
    /**
     * @brief Constructor that initializes application with various services and ensures application running as root.
     * @param btrfsMaintenance
     * @param btrfs
     * @param snapper
     * @param parent
     */
    explicit BtrfsAssistant(BtrfsMaintenance *btrfsMaintenance, Btrfs *btrfs, Snapper *snapper, QWidget *parent = 0);
    ~BtrfsAssistant();

  private:
    /**
     * @brief Btrfs maintenance frequency values
     */
    const QStringList m_bmFreqValues = {"none", "daily", "weekly", "monthly"};
    Btrfs *m_btrfs;
    BtrfsMaintenance *m_btrfsMaint;
    QSet<QCheckBox *> m_changedCheckBoxes;
    QHash<QString, QCheckBox *> m_configCheckBoxes;
    bool m_hasSnapper = false;
    bool m_hasBtrfsmaintenance = false;
    Snapper *m_snapper;
    Ui::BtrfsAssistant *m_ui;

    /**
     * @brief Timer used to periodically update UI on balance progress
     */
    QTimer *m_balanceTimer;
    /**
     * @brief Timer used to periodically update UI on scrub progress
     */
    QTimer *m_scrubTimer;

    /**
     * @brief Toggle the UI elements in Snapper tab depending on restore mode checkbox state.
     * @param enable
     */
    void enableRestoreMode(bool enable);
    /**
     * @brief Load snapper subvolumes into combobox on snapper tab.
     */
    void loadSnapperRestoreMode();
    /**
     * @brief Checks if snapper is installed and load snapper UI elements.
     */
    void loadSnapperUI();
    /**
     * @brief Sets up Btrfs Maintenance tab
     */
    void populateBmTab();
    /**
     * @brief Populate the Btrfs tab with the selected device's information.
     * @param uuid
     */
    void populateBtrfsUi(const QString &uuid);
    /**
     * @brief Populates the main grid on the Snapper tab
     */
    void populateSnapperGrid();
    /**
     * @brief Populates a selected config on the Snapper Settings tab
     */
    void populateSnapperConfigSettings();
    /**
     * @brief Populates the Btrfs Subvolumes tab with the selected devices information.
     * @param uuid
     */
    void refreshSubvolListUi(const QString &uuid);
    /**
     * @brief Refresh the Btrfs tab UI.
     */
    void refreshBtrfsUi();
    /**
     * @brief Update the UI on Snapper Settings based on systems enabled units.
     */
    void refreshSnapperServices();
    /**
     * @brief Restore a selected snapshot to the selected subvolume.
     * @param uuid
     * @param subvolume
     */
    void restoreSnapshot(const QString &uuid, const QString &subvolume);
    /**
     * @brief Initial application setup.
     * @return
     */
    bool setup();

    void setupConfigBoxes();
    /**
     * @brief Enables or disables the timeline spinboxes to match the timeline checkbox
     * @param enable
     */
    void snapperTimelineEnable(bool enable);
    /**
     * @brief Update system service states depending on checkbox states in UI.
     * @param checkboxList
     */
    void updateServices(QList<QCheckBox *>);
    /**
     * @brief Method used to fetch and update the btrfs balance status
     */
    void btrfsBalanceStatusUpdateUI();
    /**
     * @brief Method used to fetch and update the btrfs balance status
     */
    void btrfsScrubStatusUpdateUI();
    /**
     * @brief Method used to fetch and update the btrfs balance status
     */
    void btrfsDefragStatusUpdateUI();

  private slots:

    /**
     * @brief Toggle balance list selection depending on checkbox state.
     * @param checked
     */
    void on_checkBox_bmBalance_clicked(bool checked);
    /**
     * @brief Toggle defrag list selection depending on checkbox state.
     * @param checked
     */
    void on_checkBox_bmDefrag_clicked(bool checked);
    /**
     * @brief Toggle scrub list selection depending on checkbox state.
     * @param checked
     */
    void on_checkBox_bmScrub_clicked(bool checked);
    /**
     * @brief Refresh subvolume list when toggling include snapshots checkbox state.
     */
    void on_checkBox_includesnapshots_clicked();
    /**
     * @brief Toggle timeline snapshot inputs based on enable timeline snapshots checkbox state.
     * @param checked
     */
    void on_checkBox_snapper_enabletimeline_clicked(bool checked);
    /**
     * @brief Update Snapper tab UI depending on restore mode checkbox state.
     * @param checked
     */
    void on_checkBox_snapper_restore_clicked(bool checked);
    /**
     * @brief Btrfs balance button handler
     */
    void on_pushButton_btrfsBalance_clicked();
    /**
     * @brief When a change is detected on the dropdown of btrfs devices, repopulate the UI based on the new selection
     */
    void on_comboBox_btrfsdevice_activated(int);
    /**
     * @brief Btrfs scrub button handler
     */
    void on_pushButton_btrfsScrub_clicked();
    /**
     * @brief Repopulate the grid when a different config is selected
     */
    void on_comboBox_snapper_configs_activated(int);
    /**
     * @brief When a new config is selected repopulate the UI
     */
    void on_comboBox_snapper_config_settings_activated(int);
    /**
     * @brief Apply btrfs maintenance system settings based on the UI state.
     */
    void on_pushButton_bmApply_clicked();
    /**
     * @brief Delete a subvolume after checking for a variety of errors
     */
    void on_pushButton_deletesubvol_clicked();
    /**
     * @brief Refreshes Btrfs data button handler
     */
    void on_pushButton_load_clicked();
    /**
     * @brief Refreshes subvolume data button handler
     */
    void on_pushButton_loadsubvol_clicked();
    /**
     * @brief Restore snapshot button handler
     */
    void on_pushButton_restore_snapshot_clicked();
    /**
     * @brief Snapper browse snapshot button handler
     */
    void on_pushButton_snapperBrowse_clicked();
    /**
     * @brief Snapper new snapshot button handler
     */
    void on_pushButton_snapper_create_clicked();
    /**
     * @brief Snapper delete snapshot button handler
     */
    void on_pushButton_snapper_delete_clicked();
    /**
     * @brief Snapper new config button handler
     * Use snapper to delete a config when the delete config button is pressed
     */
    void on_pushButton_snapper_delete_config_clicked();
    /**
     * @brief Snapper new config button handler
     * Switches the snapper config between edit config and new config mode
     */
    void on_pushButton_snapper_new_config_clicked();
    /**
     * @brief Snapper save config button handler
     * Uses snapper to create a new config or save an existing config when the save button is pressed
     */
    void on_pushButton_snapper_save_config_clicked();
    /**
     * @brief Snapper Settings apply systemd changes button handler
     */
    void on_pushButton_SnapperUnitsApply_clicked();
};
#endif // BTRFSASSISTANT_H
