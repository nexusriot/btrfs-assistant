#include "btrfs-assistant.h"
#include "config.h"
#include "ui_btrfs-assistant.h"

#include "System.h"

#include <QDebug>

/*
 *
 * static free utility functions
 *
 */

// A simple wrapper to QMessageBox for creating consistent error messages
static void displayError(const QString &errorText) { QMessageBox::critical(0, "Error", errorText); }

// Converts a double to a human readable string for displaying data storage amounts
static const QString toHumanReadable(double number) {
    int i = 0;
    const QVector<QString> units = {"B", "kiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    while (number > 1024) {
        number /= 1024;
        i++;
    }
    return QString::number(number) + " " + units[i];
}

// Selects all rows in @p listWidget that match an item in @p items
static void setListWidgetSelections(const QStringList &items, QListWidget *listWidget) {
    QAbstractItemModel *model = listWidget->model();
    QItemSelectionModel *selectModel = listWidget->selectionModel();
    for (int i = 0; i < model->rowCount(); i++) {
        QModelIndex index = model->index(i, 0);
        if (items.contains(model->data(index).toString())) {
            selectModel->select(index, QItemSelectionModel::Select);
        }
    }
}

/*
 *
 * BtrfsAssistant functions
 *
 */

BtrfsAssistant::BtrfsAssistant(QWidget *parent) : QMainWindow(parent), ui(new Ui::BtrfsAssistant) {
    ui->setupUi(this);

    m_btrfs = new Btrfs();
    setup();
    this->setWindowTitle(tr("Btrfs Assistant"));
}

BtrfsAssistant::~BtrfsAssistant() { delete ui; }

// setup various items first time program runs
bool BtrfsAssistant::setup() {
    settings = new QSettings("/etc/btrfs-assistant.conf", QSettings::NativeFormat);

    // Save the state of snapper and btrfsmaintenance being installed since we have to check them so often
    QString snapperPath = settings->value("snapper", "/usr/bin/snapper").toString();
    hasSnapper = QFile::exists(snapperPath);

    QString btrfsMaintenanceConfig = settings->value("bm_config", "/etc/default/btrfsmaintenance").toString();
    hasBtrfsmaintenance = QFile::exists(btrfsMaintenanceConfig);

    // If snapper isn't installed, hide the snapper-related elements of the UI
    if (hasSnapper) {
        ui->groupBox_snapper_config_edit->hide();
        m_snapper = new Snapper(m_btrfs);
    } else {
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->tab_snapper_general), false);
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->tab_snapper_settings), false);
    }

    // Populate the UI
    refreshInterface();

    refreshBtrfsUi();
    loadSnapperUI();
    if (m_snapper->configs().contains("root")) {
        ui->comboBox_snapper_configs->setCurrentText("root");
    }
    populateSnapperGrid();
    populateSnapperConfigSettings();
    ui->pushButton_restore_snapshot->setEnabled(false);

    if (hasBtrfsmaintenance) {
        m_btrfsMaint = new BtrfsMaintenance(btrfsMaintenanceConfig,
                                            settings->value("bm_refresh_service", "btrfsmaintenance-refresh.service").toString());
        populateBmTab();
    } else {
        // Hide the btrfs maintenance tab
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->tab_btrfsmaintenance), false);
    }

    return true;
}

void BtrfsAssistant::refreshBtrfsUi() {
    ui->comboBox_btrfsdevice->clear();
    const QStringList uuidList = Btrfs::listFilesystems();
    for (const QString &uuid : uuidList) {
        ui->comboBox_btrfsdevice->addItem(uuid);
    }
    populateBtrfsUi(ui->comboBox_btrfsdevice->currentText());
    reloadSubvolList(ui->comboBox_btrfsdevice->currentText());
}

// Updates the checkboxes and comboboxes with values from the system
void BtrfsAssistant::refreshInterface() {
    const auto enabledUnits = System::findEnabledUnits();

    // Loop through the checkboxes
    const QList<QCheckBox *> checkboxes =
        ui->scrollArea_bm->findChildren<QCheckBox *>() + ui->groupBox_snapperUnits->findChildren<QCheckBox *>();
    for (QCheckBox *checkbox : checkboxes) {
        if (checkbox->property("actionType") == "service") {
            checkbox->setChecked(enabledUnits.contains(checkbox->property("actionData").toString()));
        }
    }
}

// Populates the UI for the BTRFS tab
void BtrfsAssistant::populateBtrfsUi(const QString &uuid) {

    BtrfsMeta btrfsVolume = m_btrfs->btrfsVolume(uuid);

    if (!btrfsVolume.populated) {
        return;
    }

    // For the tools section
    int dataPercent = ((double)btrfsVolume.dataUsed / btrfsVolume.dataSize) * 100;
    ui->progressBar_btrfsdata->setValue(dataPercent);
    ui->progressBar_btrfsmeta->setValue(((double)btrfsVolume.metaUsed / btrfsVolume.metaSize) * 100);
    ui->progressBar_btrfssys->setValue(((double)btrfsVolume.sysUsed / btrfsVolume.sysSize) * 100);

    // The information section
    ui->label_btrfsallocated->setText(toHumanReadable(btrfsVolume.allocatedSize));
    ui->label_btrfsused->setText(toHumanReadable(btrfsVolume.usedSize));
    ui->label_btrfssize->setText(toHumanReadable(btrfsVolume.totalSize));
    ui->label_btrfsfree->setText(toHumanReadable(btrfsVolume.freeSize));
    float freePercent = (double)btrfsVolume.allocatedSize / btrfsVolume.totalSize;
    if (freePercent < 0.70) {
        ui->label_btrfsmessage->setText(tr("You have lots of free space, did you overbuy?"));
    } else if (freePercent > 0.95) {
        ui->label_btrfsmessage->setText(tr("Situation critical!  Time to delete some data or buy more disk"));
    } else {
        ui->label_btrfsmessage->setText(tr("Your disk space is well utilized"));
    }
}

void BtrfsAssistant::on_pushButton_load_clicked() {
    m_btrfs->reloadVolumes();
    refreshBtrfsUi();

    ui->pushButton_load->clearFocus();
}

void BtrfsAssistant::on_pushButton_loadsubvol_clicked() {
    QString uuid = ui->comboBox_btrfsdevice->currentText();

    if (uuid.isEmpty()) {
        displayError(tr("No device selected") + "\n" + tr("Please Select a device first"));
        return;
    }

    reloadSubvolList(uuid);

    ui->pushButton_loadsubvol->clearFocus();
}

// Reloads the list of subvolumes on the BTRFS Details tab
void BtrfsAssistant::reloadSubvolList(const QString &uuid) {
    m_btrfs->reloadSubvols(uuid);
    populateSubvolList(uuid);
}

// Populate the btrfsmaintenance tab using the settings loaded from the config file
void BtrfsAssistant::populateBmTab() {
    ui->comboBox_bmBalanceFreq->clear();
    ui->comboBox_bmBalanceFreq->insertItems(0, bmFreqValues);
    ui->comboBox_bmBalanceFreq->setCurrentText(m_btrfsMaint->value("BTRFS_BALANCE_PERIOD"));
    ui->comboBox_bmScrubFreq->clear();
    ui->comboBox_bmScrubFreq->insertItems(0, bmFreqValues);
    ui->comboBox_bmScrubFreq->setCurrentText(m_btrfsMaint->value("BTRFS_SCRUB_PERIOD"));
    ui->comboBox_bmDefragFreq->clear();
    ui->comboBox_bmDefragFreq->insertItems(0, bmFreqValues);
    ui->comboBox_bmDefragFreq->setCurrentText(m_btrfsMaint->value("BTRFS_DEFRAG_PERIOD"));

    // Populate the balance section
    const QStringList balanceMounts = m_btrfsMaint->value("BTRFS_BALANCE_MOUNTPOINTS").trimmed().split(":");
    const QStringList mountpoints = Btrfs::listMountpoints();
    ui->listWidget_bmBalance->clear();
    ui->listWidget_bmBalance->insertItems(0, mountpoints);
    if (balanceMounts.contains("auto")) {
        ui->checkBox_bmBalance->setChecked(true);
        ui->listWidget_bmBalance->setDisabled(true);
    } else {
        ui->checkBox_bmBalance->setChecked(false);
        setListWidgetSelections(balanceMounts, ui->listWidget_bmBalance);
    }

    // Populate the scrub section
    const QStringList scrubMounts = m_btrfsMaint->value("BTRFS_SCRUB_MOUNTPOINTS").trimmed().split(":");
    ui->listWidget_bmScrub->clear();
    ui->listWidget_bmScrub->insertItems(0, mountpoints);
    if (scrubMounts.contains("auto")) {
        ui->checkBox_bmScrub->setChecked(true);
        ui->listWidget_bmScrub->setDisabled(true);
    } else {
        ui->checkBox_bmScrub->setChecked(false);
        setListWidgetSelections(scrubMounts, ui->listWidget_bmScrub);
    }

    // Populate the defrag section
    const QStringList defragMounts = m_btrfsMaint->value("BTRFS_DEFRAG_PATHS").trimmed().split(":");

    // In the case of defrag we need to include any nested subvols listed in the config
    QStringList combinedMountpoints = defragMounts + mountpoints;

    // Remove empty and duplicate entries
    combinedMountpoints.removeAll(QString());
    combinedMountpoints.removeDuplicates();

    ui->listWidget_bmDefrag->clear();
    ui->listWidget_bmDefrag->insertItems(0, combinedMountpoints);
    if (defragMounts.contains("auto")) {
        ui->checkBox_bmDefrag->setChecked(true);
        ui->listWidget_bmDefrag->setDisabled(true);
    } else {
        ui->checkBox_bmDefrag->setChecked(false);
        setListWidgetSelections(defragMounts, ui->listWidget_bmDefrag);
    }
}

// Populates the UI for the BTRFS details tab
void BtrfsAssistant::populateSubvolList(const QString &uuid) {
    ui->listWidget_subvols->clear();

    QMapIterator<int, Subvolume> i(m_btrfs->listSubvolumes(uuid));

    bool includeSnaps = ui->checkBox_includesnapshots->isChecked();

    while (i.hasNext()) {
        i.next();
        if (includeSnaps || !(Btrfs::isTimeshift(i.value().subvolName) || Btrfs::isSnapper(i.value().subvolName)))
            ui->listWidget_subvols->addItem(i.value().subvolName);
    }
    ui->listWidget_subvols->sortItems();
}

void BtrfsAssistant::on_checkBox_includesnapshots_clicked() { populateSubvolList(ui->comboBox_btrfsdevice->currentText()); }

void BtrfsAssistant::on_checkBox_bmBalance_clicked(bool checked) { ui->listWidget_bmBalance->setDisabled(checked); }

void BtrfsAssistant::on_checkBox_bmScrub_clicked(bool checked) { ui->listWidget_bmScrub->setDisabled(checked); }

void BtrfsAssistant::on_checkBox_bmDefrag_clicked(bool checked) { ui->listWidget_bmDefrag->setDisabled(checked); }

void BtrfsAssistant::updateServices(QList<QCheckBox *> checkboxList) {
    QStringList enabledUnits = System::findEnabledUnits();

    for (auto checkbox : checkboxList) {
        QString service = checkbox->property("actionData").toString().trimmed();
        if (service != "" && enabledUnits.contains(service) != checkbox->isChecked()) {
            if (checkbox->isChecked()) {
                System::enableService(service, true);
            } else {
                System::enableService(service, false);
            }
        }
    }
}

void BtrfsAssistant::on_pushButton_bmApply_clicked() {

    // First, update the services per the checkboxes
    updateServices(ui->scrollArea_bm->findChildren<QCheckBox *>());

    // Read and set the Btrfs maintenance settings
    m_btrfsMaint->setValue("BTRFS_BALANCE_PERIOD", ui->comboBox_bmBalanceFreq->currentText());
    m_btrfsMaint->setValue("BTRFS_SCRUB_PERIOD", ui->comboBox_bmScrubFreq->currentText());
    m_btrfsMaint->setValue("BTRFS_DEFRAG_PERIOD", ui->comboBox_bmDefragFreq->currentText());

    if (ui->checkBox_bmBalance->isChecked()) {
        m_btrfsMaint->setValue("BTRFS_BALANCE_MOUNTPOINTS", "auto");
    } else {
        const QList<QListWidgetItem *> balanceItems = ui->listWidget_bmBalance->selectedItems();
        QStringList balancePaths;
        for (const QListWidgetItem *item : balanceItems) {
            balancePaths.append(item->text());
        }
        m_btrfsMaint->setValue("BTRFS_BALANCE_MOUNTPOINTS", balancePaths.join(":"));
    }

    if (ui->checkBox_bmScrub->isChecked()) {
        m_btrfsMaint->setValue("BTRFS_SCRUB_MOUNTPOINTS", "auto");
    } else {
        const QList<QListWidgetItem *> scrubItems = ui->listWidget_bmScrub->selectedItems();
        QStringList scrubPaths;
        for (const QListWidgetItem *item : scrubItems) {
            scrubPaths.append(item->text());
        }
        m_btrfsMaint->setValue("BTRFS_SCRUB_MOUNTPOINTS", scrubPaths.join(":"));
    }

    if (ui->checkBox_bmDefrag->isChecked()) {
        m_btrfsMaint->setValue("BTRFS_DEFRAG_PATHS", "auto");
    } else {
        const QList<QListWidgetItem *> defragItems = ui->listWidget_bmDefrag->selectedItems();
        QStringList defragPaths;
        for (const QListWidgetItem *item : defragItems) {
            defragPaths.append(item->text());
        }
        m_btrfsMaint->setValue("BTRFS_DEFRAG_PATHS", defragPaths.join(":"));
    }

    // Force Btrfs Maintenance to reload the config file
    m_btrfsMaint->refresh();

    QMessageBox::information(0, tr("Btrfs Assistant"), tr("Changes applied"));

    ui->pushButton_bmApply->clearFocus();
}

void BtrfsAssistant::on_pushButton_SnapperUnitsApply_clicked() {

    updateServices(ui->groupBox_snapperUnits->findChildren<QCheckBox *>());

    QMessageBox::information(0, tr("Btrfs Assistant"), tr("Changes applied successfully"));

    ui->pushButton_SnapperUnitsApply->clearFocus();
}

// Delete a subvolume after checking for a variety of errors
void BtrfsAssistant::on_pushButton_deletesubvol_clicked() {
    QString subvol = ui->listWidget_subvols->currentItem()->text();
    QString uuid = ui->comboBox_btrfsdevice->currentText();

    // Make sure the everything is good in the UI
    if (subvol.isEmpty() || uuid.isEmpty()) {
        displayError(tr("Nothing to delete!"));
        ui->pushButton_deletesubvol->clearFocus();
        return;
    }

    // get the subvolid, if it isn't found abort
    int subvolid = m_btrfs->subvolId(uuid, subvol);
    if (subvolid == 0) {
        displayError(tr("Failed to delete subvolume!") + "\n\n" + tr("subvolid missing from map"));
        ui->pushButton_deletesubvol->clearFocus();
        return;
    }

    // ensure the subvol isn't mounted, btrfs will delete a mounted subvol but we probably shouldn't
    if (Btrfs::isMounted(uuid, subvolid)) {
        displayError(tr("You cannot delete a mounted subvolume") + "\n\n" + tr("Please unmount the subvolume before continuing"));
        ui->pushButton_deletesubvol->clearFocus();
        return;
    }

    Result result;

    // Check to see if the subvolume is a snapper snapshot
    if (Btrfs::isSnapper(subvol) && hasSnapper) {
        QMessageBox::information(0, tr("Snapshot Delete"),
                                 tr("That subvolume is a snapper shapshot") + "\n\n" + tr("Please use the snapper tab to remove it"));
        return;
    } else {
        // Everything looks good so far, now we put up a confirmation box
        if (QMessageBox::question(0, tr("Confirm"), tr("Are you sure you want to delete ") + subvol) != QMessageBox::Yes) {
            return;
        }
    }

    bool success = m_btrfs->deleteSubvol(uuid, subvolid);

    if (success) {
        reloadSubvolList(uuid);
    } else {
        displayError(tr("Failed to delete subvolume " + subvol.toUtf8()));
    }

    ui->pushButton_deletesubvol->clearFocus();
}

// When a change is detected on the dropdown of btrfs devices, repopulate the UI based on the new selection
void BtrfsAssistant::on_comboBox_btrfsdevice_activated(int) {
    QString device = ui->comboBox_btrfsdevice->currentText();
    if (!device.isEmpty()) {
        populateBtrfsUi(device);
        reloadSubvolList(device);
    }
    ui->comboBox_btrfsdevice->clearFocus();
}

// Restores a snapper snapshot after extensive error checking
void BtrfsAssistant::restoreSnapshot(const QString &uuid, const QString &subvolume) {
    if (!Btrfs::isSnapper(subvolume)) {
        displayError(tr("This is not a snapshot that can be restored by this application"));
        return;
    }

    // Ensure the list of subvolumes is not out-of-date
    m_btrfs->reloadSubvols(uuid);

    const int subvolId = m_btrfs->subvolId(uuid, subvolume);
    int targetId = m_btrfs->subvolTopParent(uuid, subvolId);
    const QMap<int, Subvolume> subvols = m_btrfs->listSubvolumes(uuid);
    QString targetSubvol = subvols[targetId].subvolName;

    // Get the subvolid of the target and do some additional error checking
    if (targetId == 0 || targetSubvol.isEmpty()) {
        displayError(tr("Target not found"));
        return;
    }

    // Handle a special case where the snapshot is of the root of the Btrfs partition
    if (targetSubvol == ".snapshots") {
        targetId = 5;
        targetSubvol = "";
    }

    // We are out of errors to check for, time to ask for confirmation
    if (QMessageBox::question(0, tr("Confirm"),
                              tr("Are you sure you want to restore ") + subvolume + tr(" to ", "as in from/to") + targetSubvol) !=
        QMessageBox::Yes)
        return;

    // Ensure the root of the partition is mounted and get the mountpoint
    QString mountpoint = Btrfs::mountRoot(uuid);

    // Make sure we have a trailing /
    if (mountpoint.right(1) != "/")
        mountpoint += "/";

    // We are out of excuses, time to do the restore....carefully
    QString targetBackup = "restore_backup_" + targetSubvol + "_" + QTime::currentTime().toString("HHmmsszzz");

    QDir dirWorker;

    // Find the children before we start
    const QStringList children = m_btrfs->children(targetId, uuid);

    // Rename the target
    if (!Btrfs::renameSubvolume(QDir::cleanPath(mountpoint + targetSubvol), QDir::cleanPath(mountpoint + targetBackup))) {
        displayError(tr("Failed to make a backup of target subvolume"));
        return;
    }

    // We moved the snapshot so we need to change the location
    const QString newSubvolume = targetBackup + subvolume.right(subvolume.length() - targetSubvol.length());

    // Place a snapshot of the source where the target was
    System::runCmd("btrfs subvolume snapshot " + mountpoint + newSubvolume + " " + mountpoint + targetSubvol, false);

    // Make sure it worked
    if (!dirWorker.exists(mountpoint + targetSubvol)) {
        // That failed, try to put the old one back
        Btrfs::renameSubvolume(QDir::cleanPath(mountpoint + targetBackup), QDir::cleanPath(mountpoint + targetSubvol));
        displayError(tr("Failed to restore subvolume!") + "\n\n" +
                     tr("Snapshot restore failed.  Please verify the status of your system before rebooting"));
        return;
    }

    // The restore was successful, now we need to move any child subvolumes into the target
    QString childSubvolPath;
    for (const QString &childSubvol : children) {
        childSubvolPath = childSubvol.right(childSubvol.length() - (targetSubvol.length() + 1));

        // rename snapshot
        QString sourcePath = QDir::cleanPath(mountpoint + targetBackup + QDir::separator() + childSubvolPath);
        QString destinationPath = QDir::cleanPath(mountpoint + childSubvol);
        if (!Btrfs::renameSubvolume(sourcePath, destinationPath)) {
            // If this fails, not much can be done except let the user know
            displayError(tr("The restore was successful but the migration of the nested subvolumes failed") + "\n\n" +
                         tr("Please migrate the those subvolumes manually"));
            return;
        }
    }

    // If we get here I guess it worked
    QMessageBox::information(0, tr("Snapshot Restore"),
                             tr("Snapshot restoration complete.") + "\n\n" + tr("A copy of the original subvolume has been saved as ") +
                                 targetBackup + "\n\n" + tr("Please reboot immediately"));
}

// Loads the snapper configs and snapshots
void BtrfsAssistant::loadSnapperUI() {
    // If snapper isn't installed, no need to continue
    if (!hasSnapper)
        return;

    // Load the list of valid configs
    ui->comboBox_snapper_configs->clear();
    ui->comboBox_snapper_config_settings->clear();

    const QStringList configs = m_snapper->configs();
    for (const QString &config : configs) {
        ui->comboBox_snapper_configs->addItem(config);
        ui->comboBox_snapper_config_settings->addItem(config);
    }
}

// Populates the main grid on the Snapper tab
void BtrfsAssistant::populateSnapperGrid() {
    if (ui->checkBox_snapper_restore->isChecked()) {
        QString config = ui->comboBox_snapper_configs->currentText();

        // Clear the table and set the headers
        ui->tableWidget_snapper->clear();
        ui->tableWidget_snapper->setColumnCount(3);
        ui->tableWidget_snapper->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Subvolume")));
        ui->tableWidget_snapper->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Date/Time")));
        ui->tableWidget_snapper->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Description")));

        QVector<SnapperSubvolume> subvols = m_snapper->subvols(config);
        // Make sure there is something to populate
        if (subvols.isEmpty())
            return;

        // Populate the table
        ui->tableWidget_snapper->setRowCount(subvols.size());
        for (int i = 0; i < subvols.size(); i++) {
            QTableWidgetItem *subvol = new QTableWidgetItem(subvols.at(i).subvol);
            ui->tableWidget_snapper->setItem(i, 0, subvol);
            ui->tableWidget_snapper->setItem(i, 1, new QTableWidgetItem(subvols.at(i).time));
            ui->tableWidget_snapper->setItem(i, 2, new QTableWidgetItem(subvols.at(i).desc));
        }
    } else {
        QString config = ui->comboBox_snapper_configs->currentText();

        // Clear the table and set the headers
        ui->tableWidget_snapper->clear();
        ui->tableWidget_snapper->setColumnCount(3);
        ui->tableWidget_snapper->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Number", "The number associated with a snapshot")));
        ui->tableWidget_snapper->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Date/Time")));
        ui->tableWidget_snapper->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Description")));

        // Make sure there is something to populate
        QVector<SnapperSnapshots> snapshots = m_snapper->snapshots(config);
        if (snapshots.isEmpty()) {
            return;
        }

        // Populate the table
        ui->tableWidget_snapper->setRowCount(snapshots.size());
        for (int i = 0; i < snapshots.size(); i++) {
            QTableWidgetItem *number = new QTableWidgetItem(snapshots.at(i).number);
            number->setData(Qt::DisplayRole, snapshots.at(i).number);
            ui->tableWidget_snapper->setItem(i, 0, number);
            ui->tableWidget_snapper->setItem(i, 1, new QTableWidgetItem(snapshots.at(i).time));
            ui->tableWidget_snapper->setItem(i, 2, new QTableWidgetItem(snapshots.at(i).desc));
        }
    }

    // Resize the colums to make everything fit
    ui->tableWidget_snapper->resizeColumnsToContents();
    ui->tableWidget_snapper->sortItems(0, Qt::DescendingOrder);
}

// Repopulate the grid when a different config is selected
void BtrfsAssistant::on_comboBox_snapper_configs_activated(int) {
    populateSnapperGrid();
    ui->comboBox_snapper_configs->clearFocus();
}

// When the create config button is clicked, use the inputted data to create the config
void BtrfsAssistant::on_pushButton_snapper_create_clicked() {
    QString config = ui->comboBox_snapper_configs->currentText();

    // If snapper isn't installed, we should bail
    if (!hasSnapper)
        return;

    // This shouldn't be possible but we check anyway
    if (config.isEmpty()) {
        displayError(tr("No config selected for snapshot"));
        return;
    }

    // OK, let's go ahead and take the snapshot
    System::runCmd("snapper -c " + config + " create -d 'Manual Snapshot'", false);

    loadSnapperUI();
    ui->comboBox_snapper_configs->setCurrentText(config);
    populateSnapperGrid();

    ui->pushButton_snapper_create->clearFocus();
}

// When the snapper delete config button is clicked, call snapper to remove the config
void BtrfsAssistant::on_pushButton_snapper_delete_clicked() {
    if (ui->tableWidget_snapper->currentRow() == -1) {
        displayError(tr("Nothing selected!"));
        return;
    }

    // Get all the rows that were selected
    const QList<QTableWidgetItem *> list = ui->tableWidget_snapper->selectedItems();

    QSet<QString> numbers;

    // Get the snapshot numbers for the selected rows
    for (const QTableWidgetItem *item : list) {
        numbers.insert(ui->tableWidget_snapper->item(item->row(), 0)->text());
    }

    // Ask for confirmation
    if (QMessageBox::question(0, tr("Confirm"), tr("Are you sure you want to delete the selected snapshot(s)?")) != QMessageBox::Yes)
        return;

    QString config = ui->comboBox_snapper_configs->currentText();

    // Delete each selected snapshot
    for (const QString &number : qAsConst(numbers)) {
        // This shouldn't be possible but we check anyway
        if (config.isEmpty() || number.isEmpty()) {
            displayError(tr("Cannot delete snapshot"));
            return;
        }

        // Delete the snapshot
        System::runCmd("snapper -c " + config + " delete " + number, false);
    }

    // Reload the UI since something changed
    loadSnapperUI();
    ui->comboBox_snapper_configs->setCurrentText(config);
    populateSnapperGrid();

    ui->pushButton_snapper_delete->clearFocus();
}

// Populates a selected config on the Snapper Settings tab
void BtrfsAssistant::populateSnapperConfigSettings() {
    QString name = ui->comboBox_snapper_config_settings->currentText();
    if (name.isEmpty())
        return;

    QString output = System::runCmd("snapper -c " + name + " get-config | tail -n +3", false).output;

    if (output.isEmpty())
        return;

    ui->label_snapper_config_name->setText(name);
    const QStringList outputList = output.split('\n');
    for (const QString &line : outputList) {
        if (line.isEmpty())
            continue;
        QString key = line.split('|').at(0).trimmed();
        QString value = line.split('|').at(1).trimmed();
        if (key == "SUBVOLUME")
            ui->label_snapper_backup_path->setText(value);
        else if (key == "TIMELINE_CREATE")
            ui->checkBox_snapper_enabletimeline->setChecked(value.toStdString() == "yes");
        else if (key == "TIMELINE_LIMIT_HOURLY")
            ui->spinBox_snapper_hourly->setValue(value.toInt());
        else if (key == "TIMELINE_LIMIT_DAILY")
            ui->spinBox_snapper_daily->setValue(value.toInt());
        else if (key == "TIMELINE_LIMIT_WEEKLY")
            ui->spinBox_snapper_weekly->setValue(value.toInt());
        else if (key == "TIMELINE_LIMIT_MONTHLY")
            ui->spinBox_snapper_monthly->setValue(value.toInt());
        else if (key == "TIMELINE_LIMIT_YEARLY")
            ui->spinBox_snapper_yearly->setValue(value.toInt());
        else if (key == "NUMBER_LIMIT")
            ui->spinBox_snapper_number->setValue(value.toInt());
    }

    snapperTimelineEnable(ui->checkBox_snapper_enabletimeline->isChecked());
}

// Enables or disables the timeline spinboxes to match the timeline checkbox
void BtrfsAssistant::snapperTimelineEnable(bool enable) {
    ui->spinBox_snapper_hourly->setEnabled(enable);
    ui->spinBox_snapper_daily->setEnabled(enable);
    ui->spinBox_snapper_weekly->setEnabled(enable);
    ui->spinBox_snapper_monthly->setEnabled(enable);
    ui->spinBox_snapper_yearly->setEnabled(enable);
}

void BtrfsAssistant::on_checkBox_snapper_enabletimeline_clicked(bool checked) { snapperTimelineEnable(checked); }

// When a new config is selected repopulate the UI
void BtrfsAssistant::on_comboBox_snapper_config_settings_activated(int) {
    populateSnapperConfigSettings();

    ui->comboBox_snapper_config_settings->clearFocus();
}

// Uses snapper to create a new config or save an existing config when the save button is pressed
void BtrfsAssistant::on_pushButton_snapper_save_config_clicked() {
    QString name;

    // If the settings box is visible we are changing settings on an existing config
    if (ui->groupBox_snapper_config_settings->isVisible()) {
        name = ui->comboBox_snapper_config_settings->currentText();
        if (name.isEmpty()) {
            displayError(tr("Failed to save changes"));
            ui->pushButton_snapper_save_config->clearFocus();
            return;
        }

        QString command = "snapper -c " + name + " set-config ";
        command += "\"TIMELINE_CREATE=" + QString(ui->checkBox_snapper_enabletimeline->isChecked() ? "yes" : "no") + "\"";
        command += " \"TIMELINE_LIMIT_HOURLY=" + QString::number(ui->spinBox_snapper_hourly->value()) + "\"";
        command += " \"TIMELINE_LIMIT_DAILY=" + QString::number(ui->spinBox_snapper_daily->value()) + "\"";
        command += " \"TIMELINE_LIMIT_WEEKLY=" + QString::number(ui->spinBox_snapper_weekly->value()) + "\"";
        command += " \"TIMELINE_LIMIT_MONTHLY=" + QString::number(ui->spinBox_snapper_monthly->value()) + "\"";
        command += " \"TIMELINE_LIMIT_YEARLY=" + QString::number(ui->spinBox_snapper_yearly->value()) + "\"";
        command += " \"NUMBER_LIMIT=" + QString::number(ui->spinBox_snapper_number->value()) + "\"";

        System::runCmd(command, false);

        QMessageBox::information(0, tr("Snapper"), tr("Changes saved"));
    } else { // This is new config we are creating
        name = ui->lineEdit_snapper_name->text();

        // Remove any whitespace from name
        name = name.simplified().replace(" ", "");

        if (name.isEmpty()) {
            displayError(tr("Please enter a valid name"));
            ui->pushButton_snapper_save_config->clearFocus();
            return;
        }

        if (m_snapper->configs().contains(name)) {
            displayError(tr("That name is already in use!"));
            ui->pushButton_snapper_save_config->clearFocus();
            return;
        }

        // Create the new config
        System::runCmd("snapper -c " + name + " create-config " + ui->comboBox_snapper_path->currentText(), false);

        // Reload the UI
        loadSnapperUI();
        ui->comboBox_snapper_config_settings->setCurrentText(name);
        populateSnapperGrid();
        populateSnapperConfigSettings();

        // Put the ui back in edit mode
        ui->groupBox_snapper_config_display->show();
        ui->groupBox_snapper_config_edit->hide();
        ui->groupBox_snapper_config_settings->show();
    }

    ui->pushButton_snapper_save_config->clearFocus();
}

// Switches the snapper config between edit config and new config mode
void BtrfsAssistant::on_pushButton_snapper_new_config_clicked() {
    if (ui->groupBox_snapper_config_edit->isVisible()) {
        ui->lineEdit_snapper_name->clear();

        // Put the ui back in edit mode
        ui->groupBox_snapper_config_display->show();
        ui->groupBox_snapper_config_edit->hide();
        ui->groupBox_snapper_config_settings->show();

        ui->pushButton_snapper_new_config->setText(tr("New Config"));
        ui->pushButton_snapper_new_config->clearFocus();
    } else {
        // Get a list of btrfs mountpoints that could be backed up
        const QStringList mountpoints = Btrfs::listMountpoints();

        if (mountpoints.isEmpty()) {
            displayError(tr("No btrfs subvolumes found"));
            return;
        }

        // Populate the list of mountpoints after checking that their isn't already a config
        ui->comboBox_snapper_path->clear();
        const QStringList configs = m_snapper->configs();
        for (const QString &mountpoint : mountpoints) {
            if (m_snapper->config(mountpoint).isEmpty()) {
                ui->comboBox_snapper_path->addItem(mountpoint);
            }
        }

        // Put the UI in create config mode
        ui->groupBox_snapper_config_display->hide();
        ui->groupBox_snapper_config_edit->show();
        ui->groupBox_snapper_config_settings->hide();

        ui->pushButton_snapper_new_config->setText(tr("Cancel New Config"));
        ui->pushButton_snapper_new_config->clearFocus();
    }
}

// Use snapper to delete a config when the delete config button is pressed
void BtrfsAssistant::on_pushButton_snapper_delete_config_clicked() {
    QString name = ui->comboBox_snapper_config_settings->currentText();

    if (name.isEmpty()) {
        displayError(tr("No config selected"));
        ui->pushButton_snapper_delete_config->clearFocus();
        return;
    }

    if (name == "root") {
        displayError(tr("You may not don't delete the root config"));
        ui->pushButton_snapper_delete_config->clearFocus();
        return;
    }

    // Ask for confirmation
    if (QMessageBox::question(0, tr("Please Confirm"),
                              tr("Are you sure you want to delete ") + name + "\n\n" + tr("This action cannot be undone")) !=
        QMessageBox::Yes) {
        ui->pushButton_snapper_delete_config->clearFocus();
        return;
    }

    // Delete the config
    System::runCmd("snapper -c " + name + " delete-config", false);

    // Reload the UI with the new list of configs
    loadSnapperUI();
    populateSnapperGrid();
    populateSnapperConfigSettings();

    ui->pushButton_snapper_delete_config->clearFocus();
}

void BtrfsAssistant::on_checkBox_snapper_restore_clicked(bool checked) {
    enableRestoreMode(checked);

    ui->checkBox_snapper_restore->clearFocus();
}

// Puts the snapper tab in restore mode
void BtrfsAssistant::enableRestoreMode(bool enable) {
    ui->pushButton_snapper_create->setEnabled(!enable);
    ui->pushButton_snapper_delete->setEnabled(!enable);
    ui->pushButton_restore_snapshot->setEnabled(enable);

    if (enable) {
        ui->label_snapper_combo->setText(tr("Select Subvolume:"));
        ui->comboBox_snapper_configs->clear();
        ui->tableWidget_snapper->clear();
        loadSnapperRestoreMode();
        populateSnapperGrid();
    } else {
        ui->label_snapper_combo->setText(tr("Select Config:"));
        loadSnapperUI();
        populateSnapperGrid();
    }
}

void BtrfsAssistant::on_pushButton_restore_snapshot_clicked() {
    // First lets double check to ensure we are in restore mode
    if (!ui->checkBox_snapper_restore->isChecked()) {
        displayError(tr("Please enter restore mode before trying to restore a snapshot"));
        return;
    }

    if (ui->tableWidget_snapper->currentRow() == -1) {
        displayError(tr("Nothing selected!"));
        return;
    }

    QString config = ui->comboBox_snapper_configs->currentText();
    QString subvol = ui->tableWidget_snapper->item(ui->tableWidget_snapper->currentRow(), 0)->text();

    QVector<SnapperSubvolume> snapperSubvols = m_snapper->subvols(config);

    // This shouldn't be possible but check anyway
    if (snapperSubvols.isEmpty()) {
        displayError(tr("Failed to restore snapshot"));
        return;
    }

    // For a given subvol they all have the same uuid so we can just use the first one
    QString uuid = snapperSubvols.at(0).uuid;

    restoreSnapshot(uuid, subvol);

    ui->pushButton_restore_snapshot->clearFocus();
}

// When booting off a snapshot this forcibly switches to the correct tab and enables the restore mode
void BtrfsAssistant::switchToSnapperRestore() {
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->tab_snapper_general), true);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tab_snapper_general));
    ui->checkBox_snapper_restore->setChecked(true);
    enableRestoreMode(true);

    return;
}

// Populates the UI for the restore mode of the snapper tab
void BtrfsAssistant::loadSnapperRestoreMode() {
    // Sanity check
    if (!ui->checkBox_snapper_restore->isChecked()) {
        return;
    }

    // Clear the existing info
    ui->comboBox_snapper_configs->clear();

    const QStringList configs = m_snapper->subvolKeys();
    for (const QString &config : configs) {
        ui->comboBox_snapper_configs->addItem(config);
    }
}
