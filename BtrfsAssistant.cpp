#include "BtrfsAssistant.h"
#include "FileBrowser.h"
#include "Settings.h"
#include "System.h"
#include "config.h"
#include "ui_btrfs-assistant.h"

#include <QDebug>
#include <QInputDialog>
#include <QTimer>

/**
 * @brief A simple wrapper to QMessageBox for creating consistent error messages
 * @param errorText - A QString containing the text to display to the user
 */
static void displayError(const QString &errorText) { QMessageBox::critical(0, "Error", errorText); }

/**
 * @brief Selects all rows in @p listWidget that match an item in @p items
 * @param items - A QStringList which contain the strings to select in @p listWidget
 * @param listWidget - A pointer to a QListWidget where the selections will be made
 */
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

BtrfsAssistant::BtrfsAssistant(BtrfsMaintenance *btrfsMaintenance, Btrfs *btrfs, Snapper *snapper, QWidget *parent)
    : QMainWindow(parent), m_ui(new Ui::BtrfsAssistant), m_btrfs(btrfs), m_snapper(snapper), m_btrfsMaint(btrfsMaintenance) {
    m_ui->setupUi(this);

    // Ensure the application is running as root
    if (!System::checkRootUid()) {
        displayError(tr("The application must be run as the superuser(root)"));
        exit(1);
    }

    m_hasSnapper = snapper != nullptr;
    m_hasBtrfsmaintenance = btrfsMaintenance != nullptr;

    m_sourceModel = new SubvolumeModel(this);
    m_sourceModel->load(m_btrfs->volumes());
    m_subvolumeModel = new SubvolumeFilterModel(this);
    m_subvolumeModel->setSourceModel(m_sourceModel);
    connect(m_ui->checkBox_subvolIncludeSnapshots, &QCheckBox::toggled, m_subvolumeModel, &SubvolumeFilterModel::setIncludeSnapshots);
    connect(m_ui->checkBox_subvolIncludeContainer, &QCheckBox::toggled, m_subvolumeModel, &SubvolumeFilterModel::setIncludeContainer);

    // timers for filesystem operations
    m_balanceTimer = new QTimer(this);
    m_scrubTimer = new QTimer(this);
    connect(m_balanceTimer, &QTimer::timeout, this, &BtrfsAssistant::btrfsBalanceStatusUpdateUI);
    connect(m_scrubTimer, &QTimer::timeout, this, &BtrfsAssistant::btrfsScrubStatusUpdateUI);

    setup();
    this->setWindowTitle(QCoreApplication::applicationName());
}

BtrfsAssistant::~BtrfsAssistant() { delete m_ui; }

void BtrfsAssistant::btrfsBalanceStatusUpdateUI() {
    QString uuid = m_ui->comboBox_btrfsDevice->currentText();
    QString balanceStatus = m_btrfs->balanceStatus(m_btrfs->mountRoot(uuid));

    // if balance is running currently, make sure you can stop it and we monitor progress
    if (!balanceStatus.contains("No balance found")) {
        m_ui->pushButton_btrfsBalance->setText("Stop");
        // update status to current balance operation status
        m_ui->label_btrfsBalanceStatus->setText(balanceStatus);
        // keep updating UI if it isn't already doing so
        if (m_balanceTimer->timerId() == -1) {
            m_balanceTimer->start();
        }
    } else {
        // update status to reflect no balance running and stop timer
        m_ui->label_btrfsBalanceStatus->setText("No balance running.");
        m_ui->pushButton_btrfsBalance->setText("Start");
        m_balanceTimer->stop();
    }
}

void BtrfsAssistant::btrfsScrubStatusUpdateUI() {
    QString uuid = m_ui->comboBox_btrfsDevice->currentText();
    QString scrubStatus = m_btrfs->scrubStatus(m_btrfs->mountRoot(uuid));

    // update status to current scrub operation status
    m_ui->label_btrfsScrubStatus->setText(scrubStatus);
    // if scrub is running currently, make sure you can stop it and we monitor progress
    if (scrubStatus.contains("ETA:")) {
        m_ui->pushButton_btrfsScrub->setText("Stop");
        if (m_scrubTimer->timerId() == -1) {
            m_scrubTimer->start();
        }
    } else {
        m_scrubTimer->stop();
        m_ui->pushButton_btrfsScrub->setText("Start");
    }
}

void BtrfsAssistant::loadSnapperUI() {
    // If snapper isn't installed, no need to continue
    if (!m_hasSnapper)
        return;

    // Load the list of valid configs
    m_ui->comboBox_snapperConfigs->clear();
    m_ui->comboBox_snapperSubvols->clear();
    m_ui->comboBox_snapperConfigSettings->clear();

    // Load the configs in the snapper new subtab and the snapper settings tabs
    const QStringList configs = m_snapper->configs();
    for (const QString &config : configs) {
        m_ui->comboBox_snapperConfigs->addItem(config);
        m_ui->comboBox_snapperConfigSettings->addItem(config);
    }

    // Load the subvols in the snapper restore subtab
    const QStringList subvols = m_snapper->subvolKeys();
    for (const QString &subvol : subvols) {
        m_ui->comboBox_snapperSubvols->addItem(subvol);
    }
}

void BtrfsAssistant::populateBmTab() {
    // Populate the frequency values from maintenance configuration
    m_ui->comboBox_bmBalanceFreq->clear();
    m_ui->comboBox_bmBalanceFreq->insertItems(0, m_bmFreqValues);
    m_ui->comboBox_bmBalanceFreq->setCurrentText(m_btrfsMaint->value("BTRFS_BALANCE_PERIOD"));
    m_ui->comboBox_bmScrubFreq->clear();
    m_ui->comboBox_bmScrubFreq->insertItems(0, m_bmFreqValues);
    m_ui->comboBox_bmScrubFreq->setCurrentText(m_btrfsMaint->value("BTRFS_SCRUB_PERIOD"));
    m_ui->comboBox_bmDefragFreq->clear();
    m_ui->comboBox_bmDefragFreq->insertItems(0, m_bmFreqValues);
    m_ui->comboBox_bmDefragFreq->setCurrentText(m_btrfsMaint->value("BTRFS_DEFRAG_PERIOD"));

    // Populate the balance section
    const QStringList balanceMounts = m_btrfsMaint->value("BTRFS_BALANCE_MOUNTPOINTS").trimmed().split(":");
    const QStringList mountpoints = Btrfs::listMountpoints();
    m_ui->listWidget_bmBalance->clear();
    m_ui->listWidget_bmBalance->insertItems(0, mountpoints);
    if (balanceMounts.contains("auto")) {
        m_ui->checkBox_bmBalance->setChecked(true);
        m_ui->listWidget_bmBalance->setDisabled(true);
    } else {
        m_ui->checkBox_bmBalance->setChecked(false);
        setListWidgetSelections(balanceMounts, m_ui->listWidget_bmBalance);
    }

    // Populate the scrub section
    const QStringList scrubMounts = m_btrfsMaint->value("BTRFS_SCRUB_MOUNTPOINTS").trimmed().split(":");
    m_ui->listWidget_bmScrub->clear();
    m_ui->listWidget_bmScrub->insertItems(0, mountpoints);
    if (scrubMounts.contains("auto")) {
        m_ui->checkBox_bmScrub->setChecked(true);
        m_ui->listWidget_bmScrub->setDisabled(true);
    } else {
        m_ui->checkBox_bmScrub->setChecked(false);
        setListWidgetSelections(scrubMounts, m_ui->listWidget_bmScrub);
    }

    // Populate the defrag section
    const QStringList defragMounts = m_btrfsMaint->value("BTRFS_DEFRAG_PATHS").trimmed().split(":");

    // In the case of defrag we need to include any nested subvols listed in the config
    QStringList combinedMountpoints = defragMounts + mountpoints;

    // Remove empty and duplicate entries
    combinedMountpoints.removeAll(QString());
    combinedMountpoints.removeDuplicates();

    m_ui->listWidget_bmDefrag->clear();
    m_ui->listWidget_bmDefrag->insertItems(0, combinedMountpoints);
    if (defragMounts.contains("auto")) {
        m_ui->checkBox_bmDefrag->setChecked(true);
        m_ui->listWidget_bmDefrag->setDisabled(true);
    } else {
        m_ui->checkBox_bmDefrag->setChecked(false);
        setListWidgetSelections(defragMounts, m_ui->listWidget_bmDefrag);
    }
}

void BtrfsAssistant::populateBtrfsUi(const QString &uuid) {

    setEnableQuotaButtonStatus();

    BtrfsMeta btrfsVolume = m_btrfs->btrfsVolume(uuid);

    if (!btrfsVolume.populated) {
        return;
    }

    // For the tools section
    int dataPercent = ((double)btrfsVolume.dataUsed / btrfsVolume.dataSize) * 100;
    m_ui->progressBar_btrfsdata->setValue(dataPercent);
    m_ui->progressBar_btrfsmeta->setValue(((double)btrfsVolume.metaUsed / btrfsVolume.metaSize) * 100);
    m_ui->progressBar_btrfssys->setValue(((double)btrfsVolume.sysUsed / btrfsVolume.sysSize) * 100);

    // The information section
    m_ui->label_btrfsAllocatedValue->setText(System::toHumanReadable(btrfsVolume.allocatedSize));
    m_ui->label_btrfsUsedValue->setText(System::toHumanReadable(btrfsVolume.usedSize));
    m_ui->label_btrfsSizeValue->setText(System::toHumanReadable(btrfsVolume.totalSize));
    m_ui->label_btrfsFreeValue->setText(System::toHumanReadable(btrfsVolume.freeSize));
    float freePercent = (double)btrfsVolume.allocatedSize / btrfsVolume.totalSize;
    if (freePercent < 0.70) {
        m_ui->label_btrfsMessage->setText(tr("You have lots of free space, did you overbuy?"));
    } else if (freePercent > 0.95) {
        m_ui->label_btrfsMessage->setText(tr("Situation critical!  Time to delete some data or buy more disk"));
    } else {
        m_ui->label_btrfsMessage->setText(tr("Your disk space is well utilized"));
    }

    // filesystems operation section
    btrfsBalanceStatusUpdateUI();
    btrfsScrubStatusUpdateUI();
}

void BtrfsAssistant::populateSnapperConfigSettings() {
    QString name = m_ui->comboBox_snapperConfigSettings->currentText();
    if (name.isEmpty()) {
        return;
    }

    // Retrieve settings for selected config
    const QMap<QString, QString> config = m_snapper->config(name);

    if (config.isEmpty()) {
        return;
    }

    // Populate UI elements with the values from the snapper config settings.
    m_ui->label_snapperConfigNameValue->setText(name);
    const QStringList keys = config.keys();
    for (const QString &key : keys) {
        if (key == "SUBVOLUME") {
            m_ui->label_snapperBackupPathValue->setText(config[key]);
        } else if (key == "TIMELINE_CREATE") {
            m_ui->checkBox_snapperEnableTimeline->setChecked(config[key].toStdString() == "yes");
        } else if (key == "TIMELINE_LIMIT_HOURLY") {
            m_ui->spinBox_snapperHourly->setValue(config[key].toInt());
        } else if (key == "TIMELINE_LIMIT_DAILY") {
            m_ui->spinBox_snapperDaily->setValue(config[key].toInt());
        } else if (key == "TIMELINE_LIMIT_WEEKLY") {
            m_ui->spinBox_snapperWeekly->setValue(config[key].toInt());
        } else if (key == "TIMELINE_LIMIT_MONTHLY") {
            m_ui->spinBox_snapperMonthly->setValue(config[key].toInt());
        } else if (key == "TIMELINE_LIMIT_YEARLY") {
            m_ui->spinBox_snapperYearly->setValue(config[key].toInt());
        } else if (key == "NUMBER_LIMIT") {
            m_ui->spinBox_snapperNumber->setValue(config[key].toInt());
        }
    }

    snapperTimelineEnable(m_ui->checkBox_snapperEnableTimeline->isChecked());
}

void BtrfsAssistant::populateSnapperGrid() {
    // We need to the locale for displaying the date/time
    const QLocale locale = QLocale::system();

    const QString config = m_ui->comboBox_snapperConfigs->currentText();

    // Clear the table and set the headers
    m_ui->tableWidget_snapperNew->clear();
    m_ui->tableWidget_snapperNew->setColumnCount(4);
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Number", "The number associated with a snapshot")));
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Date/Time")));
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Type")));
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Description")));
    m_ui->tableWidget_snapperNew->sortByColumn(0, Qt::DescendingOrder);

    // Disabling sorting while populating the grid is required or the grid won't repopulate properly
    m_ui->tableWidget_snapperNew->setSortingEnabled(false);


    // Make sure there is something to populate
    QVector<SnapperSnapshots> snapshots = m_snapper->snapshots(config);
    if (snapshots.isEmpty()) {
        return;
    }

    // Populate the table
    m_ui->tableWidget_snapperNew->setRowCount(snapshots.size());
    for (int i = 0; i < snapshots.size(); i++) {
        // Ensure proper sorting of numbers and dates
        QTableWidgetItem *number = new QTableWidgetItem(snapshots.at(i).number);
        number->setData(Qt::DisplayRole, snapshots.at(i).number);
        QTableWidgetItem *snapTime = new QTableWidgetItem(locale.toString(snapshots.at(i).time, QLocale::ShortFormat));
        snapTime->setData(Qt::DisplayRole, snapshots.at(i).time);

        m_ui->tableWidget_snapperNew->setItem(i, 0, number);
        m_ui->tableWidget_snapperNew->setItem(i, 1, snapTime);
        m_ui->tableWidget_snapperNew->setItem(i, 2, new QTableWidgetItem(snapshots.at(i).type));
        m_ui->tableWidget_snapperNew->setItem(i, 3, new QTableWidgetItem(snapshots.at(i).desc));
    }

    // Re-enable sorting and resize the colums to make everything fit
    m_ui->tableWidget_snapperNew->setSortingEnabled(true);
    m_ui->tableWidget_snapperNew->resizeColumnsToContents();
}

void BtrfsAssistant::populateSnapperRestoreGrid() {
    // We need to the locale for displaying the date/time
    const QLocale locale = QLocale::system();

    // Get the name of the subvolume to list in the grid
    const QString config = m_ui->comboBox_snapperSubvols->currentText();

    // Clear the table and set the headers
    m_ui->tableWidget_snapperRestore->clear();
    m_ui->tableWidget_snapperRestore->setColumnCount(5);
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem(0,
                                                              new QTableWidgetItem(tr("Number", "The number associated with a snapshot")));
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Subvolume")));
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Date/Time")));
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Type")));
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Description")));
    m_ui->tableWidget_snapperRestore->sortByColumn(0, Qt::DescendingOrder);

    // Disabling sorting while populating the grid is required or the grid won't repopulate properly - This must be Qt bug, right?
    m_ui->tableWidget_snapperRestore->setSortingEnabled(false);

    QVector<SnapperSubvolume> subvols = m_snapper->subvols(config);
    // Make sure there is something to populate
    if (subvols.isEmpty())
        return;

    // Populate the table
    m_ui->tableWidget_snapperRestore->setRowCount(subvols.count());
    for (int i = 0; i < subvols.count(); i++) {
        // Ensure proper sorting of numbers and dates
        QTableWidgetItem *number = new QTableWidgetItem(subvols.at(i).snapshotNum);
        number->setData(Qt::DisplayRole, subvols.at(i).snapshotNum);
        QTableWidgetItem *snapTime = new QTableWidgetItem(locale.toString(subvols.at(i).time, QLocale::ShortFormat));
        snapTime->setData(Qt::DisplayRole, subvols.at(i).time);

        m_ui->tableWidget_snapperRestore->setItem(i, 0, number);
        m_ui->tableWidget_snapperRestore->setItem(i, 1, new QTableWidgetItem(subvols.at(i).subvol));
        m_ui->tableWidget_snapperRestore->setItem(i, 2, snapTime);
        m_ui->tableWidget_snapperRestore->setItem(i, 3, new QTableWidgetItem(subvols.at(i).type));
        m_ui->tableWidget_snapperRestore->setItem(i, 4, new QTableWidgetItem(subvols.at(i).desc));
    }

    // Re-enable sorting and resize the colums to make everything fit
    m_ui->tableWidget_snapperRestore->setSortingEnabled(true);
    m_ui->tableWidget_snapperRestore->resizeColumnsToContents();
}

void BtrfsAssistant::refreshBtrfsUi() {

    // Repopulate device selection combo box with detected btrfs filesystems.
    m_ui->comboBox_btrfsDevice->clear();
    const QStringList uuidList = Btrfs::listFilesystems();
    for (const QString &uuid : uuidList) {
        m_ui->comboBox_btrfsDevice->addItem(uuid);
    }

    // Repopulate data using the first detected btrfs filesystem.
    populateBtrfsUi(m_ui->comboBox_btrfsDevice->currentText());
    refreshSubvolListUi();
}

void BtrfsAssistant::refreshSnapperServices() {
    const auto enabledUnits = System::findEnabledUnits();

    // Loop through the checkboxes and change state to match
    const QList<QCheckBox *> checkboxes =
        m_ui->scrollArea_bm->findChildren<QCheckBox *>() + m_ui->groupBox_snapperUnits->findChildren<QCheckBox *>();
    for (QCheckBox *checkbox : checkboxes) {
        if (checkbox->property("actionType") == "service") {
            checkbox->setChecked(enabledUnits.contains(checkbox->property("actionData").toString()));
        }
    }
}

void BtrfsAssistant::refreshSubvolListUi() {

    bool showQuota = false;

    for (const QString &uuid : m_btrfs->listFilesystems()) {
        // Check to see if the size related colums should be hidden
        if (Btrfs::isQuotaEnabled(Btrfs::mountRoot(uuid))) {
            showQuota = true;
        }
    }

    // Update table sizes and columns based on subvolumes
    m_ui->tableView_subvols->verticalHeader()->hide();
    m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::Id);
    m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::ParentId);

    // Hide quota data if no columns supported it
    if (!showQuota) {
        m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::Size);
        m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::ExclusiveSize);
    } else {
        m_ui->tableView_subvols->showColumn(SubvolumeModel::Column::Size);
        m_ui->tableView_subvols->showColumn(SubvolumeModel::Column::ExclusiveSize);
    }

    // If there is only a single filesystem then hide the Uuid column
    if (m_ui->comboBox_btrfsDevice->count() == 1) {
        m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::Uuid);
    }
}

void BtrfsAssistant::restoreSnapshot(const QString &uuid, const QString &subvolume) {
    if (!Btrfs::isSnapper(subvolume)) {
        displayError(tr("This is not a snapshot that can be restored by this application"));
        return;
    }

    // Ensure the list of subvolumes is not out-of-date
    m_btrfs->loadSubvols(uuid);

    const int subvolId = m_btrfs->subvolId(uuid, subvolume);
    const QString snapshotSubvol = Snapper::findSnapshotSubvolume(subvolume);
    if (snapshotSubvol.isEmpty()) {
        displayError(tr("Snapshot subvolume not found"));
        return;
    }

    // Check the map for the target subvolume
    const QString targetSubvol = m_snapper->findTargetSubvol(snapshotSubvol, uuid);
    const int targetId = m_btrfs->subvolId(uuid, targetSubvol);

    if (targetId == 0 || targetSubvol.isEmpty()) {
        displayError(tr("Target not found"));
        return;
    }

    // We are out of errors to check for, time to ask for confirmation
    if (QMessageBox::question(0, tr("Confirm"),
                              tr("Are you sure you want to restore ") + subvolume + tr(" to ", "as in from/to") + targetSubvol) !=
        QMessageBox::Yes)
        return;

    // Everything checks out, time to do the restore
    RestoreResult restoreResult = m_snapper->restoreSubvol(uuid, subvolId, targetId);

    // Report the outcome to the end user
    if (restoreResult.success) {
        QMessageBox::information(0, tr("Snapshot Restore"),
                                 tr("Snapshot restoration complete.") + "\n\n" + tr("A copy of the original subvolume has been saved as ") +
                                     restoreResult.backupSubvolName + "\n\n" + tr("Please reboot immediately"));
    } else {
        displayError(restoreResult.failureMessage);
    }
}

void BtrfsAssistant::setup() {

    // If snapper isn't installed, hide the snapper-related elements of the UI
    if (m_hasSnapper) {
        m_ui->groupBox_snapperConfigEdit->hide();
    } else {
        m_ui->tabWidget->setTabVisible(m_ui->tabWidget->indexOf(m_ui->tab_snapper_general), false);
        m_ui->tabWidget->setTabVisible(m_ui->tabWidget->indexOf(m_ui->tab_snapper_settings), false);
    }

    // Connect the subvolume view
    m_ui->tableView_subvols->setModel(m_subvolumeModel);
    m_ui->tableView_subvols->sortByColumn(SubvolumeModel::Column::Name, Qt::AscendingOrder);
    m_ui->tableView_subvols->horizontalHeader()->setSectionResizeMode(SubvolumeModel::Column::Name, QHeaderView::Stretch);
    m_ui->tableView_subvols->horizontalHeader()->setSectionResizeMode(SubvolumeModel::Column::Uuid, QHeaderView::ResizeToContents);
    m_ui->tableView_subvols->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Populate the UI
    refreshBtrfsUi();
    if (m_hasSnapper) {
        refreshSnapperServices();
        loadSnapperUI();
        if (m_snapper->configs().contains("root")) {
            m_ui->comboBox_snapperConfigs->setCurrentText("root");
        }
        populateSnapperGrid();
        populateSnapperRestoreGrid();
        populateSnapperConfigSettings();
    }

    // Populate or hide btrfs maintenance tab depending on if system has btrfs maintenance units
    if (m_hasBtrfsmaintenance) {
        populateBmTab();
    } else {
        // Hide the btrfs maintenance tab
        m_ui->tabWidget->setTabVisible(m_ui->tabWidget->indexOf(m_ui->tab_btrfsmaintenance), false);
    }
}

void BtrfsAssistant::snapperTimelineEnable(bool enable) {
    m_ui->spinBox_snapperHourly->setEnabled(enable);
    m_ui->spinBox_snapperDaily->setEnabled(enable);
    m_ui->spinBox_snapperWeekly->setEnabled(enable);
    m_ui->spinBox_snapperMonthly->setEnabled(enable);
    m_ui->spinBox_snapperYearly->setEnabled(enable);
}

void BtrfsAssistant::updateServices(QList<QCheckBox *> checkboxList) {
    QStringList enabledUnits = System::findEnabledUnits();

    for (auto checkbox : checkboxList) {
        QString service = checkbox->property("actionData").toString().trimmed();
        if (service != "" && enabledUnits.contains(service) != checkbox->isChecked()) {
            System::enableService(service, checkbox->isChecked());
        }
    }
}

void BtrfsAssistant::on_checkBox_bmBalance_clicked(bool checked) { m_ui->listWidget_bmBalance->setDisabled(checked); }

void BtrfsAssistant::on_checkBox_bmDefrag_clicked(bool checked) { m_ui->listWidget_bmDefrag->setDisabled(checked); }

void BtrfsAssistant::on_checkBox_bmScrub_clicked(bool checked) { m_ui->listWidget_bmScrub->setDisabled(checked); }

void BtrfsAssistant::on_checkBox_snapperEnableTimeline_clicked(bool checked) { snapperTimelineEnable(checked); }

void BtrfsAssistant::on_comboBox_btrfsDevice_activated(int index) {
    QString uuid = m_ui->comboBox_btrfsDevice->currentText();
    if (!uuid.isEmpty()) {
        populateBtrfsUi(uuid);
        refreshSubvolListUi();
    }
    m_ui->comboBox_btrfsDevice->clearFocus();
}

void BtrfsAssistant::on_comboBox_snapperConfigSettings_activated(int index) {
    populateSnapperConfigSettings();

    m_ui->comboBox_snapperConfigSettings->clearFocus();
}

void BtrfsAssistant::on_comboBox_snapperConfigs_activated(int index) {
    populateSnapperGrid();
    m_ui->comboBox_snapperConfigs->clearFocus();
}

void BtrfsAssistant::on_comboBox_snapperSubvols_activated(int index) {
    populateSnapperRestoreGrid();
    m_ui->comboBox_snapperSubvols->clearFocus();
}

void BtrfsAssistant::on_pushButton_bmApply_clicked() {
    // Read and set the Btrfs maintenance settings
    m_btrfsMaint->setValue("BTRFS_BALANCE_PERIOD", m_ui->comboBox_bmBalanceFreq->currentText());
    m_btrfsMaint->setValue("BTRFS_SCRUB_PERIOD", m_ui->comboBox_bmScrubFreq->currentText());
    m_btrfsMaint->setValue("BTRFS_DEFRAG_PERIOD", m_ui->comboBox_bmDefragFreq->currentText());

    // Update balance settings
    if (m_ui->checkBox_bmBalance->isChecked()) {
        m_btrfsMaint->setValue("BTRFS_BALANCE_MOUNTPOINTS", "auto");
    } else {
        const QList<QListWidgetItem *> balanceItems = m_ui->listWidget_bmBalance->selectedItems();
        QStringList balancePaths;
        for (const QListWidgetItem *item : balanceItems) {
            balancePaths.append(item->text());
        }
        m_btrfsMaint->setValue("BTRFS_BALANCE_MOUNTPOINTS", balancePaths.join(":"));
    }

    // Update scrub settings
    if (m_ui->checkBox_bmScrub->isChecked()) {
        m_btrfsMaint->setValue("BTRFS_SCRUB_MOUNTPOINTS", "auto");
    } else {
        const QList<QListWidgetItem *> scrubItems = m_ui->listWidget_bmScrub->selectedItems();
        QStringList scrubPaths;
        for (const QListWidgetItem *item : scrubItems) {
            scrubPaths.append(item->text());
        }
        m_btrfsMaint->setValue("BTRFS_SCRUB_MOUNTPOINTS", scrubPaths.join(":"));
    }

    // Update defrag settings
    if (m_ui->checkBox_bmDefrag->isChecked()) {
        m_btrfsMaint->setValue("BTRFS_DEFRAG_PATHS", "auto");
    } else {
        const QList<QListWidgetItem *> defragItems = m_ui->listWidget_bmDefrag->selectedItems();
        QStringList defragPaths;
        for (const QListWidgetItem *item : defragItems) {
            defragPaths.append(item->text());
        }
        m_btrfsMaint->setValue("BTRFS_DEFRAG_PATHS", defragPaths.join(":"));
    }

    // Force Btrfs Maintenance to reload the config file
    m_btrfsMaint->refresh();

    QMessageBox::information(0, tr("Btrfs Assistant"), tr("Changes applied"));

    m_ui->pushButton_bmApply->clearFocus();
}

void BtrfsAssistant::on_pushButton_btrfsBalance_clicked() {
    QString uuid = m_ui->comboBox_btrfsDevice->currentText();

    // Stop or start balance depending on current operation
    if (m_ui->pushButton_btrfsBalance->text().contains("Stop")) {
        m_btrfs->stopBalanceRoot(uuid);
        btrfsBalanceStatusUpdateUI();
    } else {
        m_btrfs->startBalanceRoot(uuid);
        btrfsBalanceStatusUpdateUI();
    }
}

void BtrfsAssistant::on_pushButton_btrfsScrub_clicked() {
    QString uuid = m_ui->comboBox_btrfsDevice->currentText();

    // Stop or start scrub depending on current operation
    if (m_ui->pushButton_btrfsScrub->text().contains("Stop")) {
        m_btrfs->stopScrubRoot(uuid);
        btrfsScrubStatusUpdateUI();
    } else {
        m_btrfs->startScrubRoot(uuid);
        btrfsScrubStatusUpdateUI();
    }
}

void BtrfsAssistant::on_pushButton_subvolDelete_clicked() {
    if (!m_ui->tableView_subvols->selectionModel()->hasSelection()) {
        displayError(tr("Please select a subvolume to delete first!"));
        m_ui->pushButton_subvolDelete->clearFocus();
        return;
    }
    QModelIndexList indexes = m_ui->tableView_subvols->selectionModel()->selection().indexes();
    QString subvol = m_ui->tableView_subvols->model()->data(indexes.at(SubvolumeModel::Column::Name)).toString();
    QString uuid = m_ui->tableView_subvols->model()->data(indexes.at(SubvolumeModel::Column::Uuid)).toString();

    // Make sure the everything is good in the UI
    if (subvol.isEmpty() || uuid.isEmpty()) {
        displayError(tr("Nothing to delete!"));
        m_ui->pushButton_subvolDelete->clearFocus();
        return;
    }

    // get the subvolid, if it isn't found abort
    int subvolid = m_btrfs->subvolId(uuid, subvol);
    if (subvolid == 0) {
        displayError(tr("Failed to delete subvolume!") + "\n\n" + tr("subvolid missing from map"));
        m_ui->pushButton_subvolDelete->clearFocus();
        return;
    }

    // ensure the subvol isn't mounted, btrfs will delete a mounted subvol but we probably shouldn't
    if (Btrfs::isMounted(uuid, subvolid)) {
        displayError(tr("You cannot delete a mounted subvolume") + "\n\n" + tr("Please unmount the subvolume before continuing"));
        m_ui->pushButton_subvolDelete->clearFocus();
        return;
    }

    Result result;

    // Check to see if the subvolume is a snapper snapshot
    if (Btrfs::isSnapper(subvol) && m_hasSnapper) {
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
        m_btrfs->loadSubvols(uuid);
        m_sourceModel->load(m_btrfs->volumes());
        refreshSubvolListUi();
    } else {
        displayError(tr("Failed to delete subvolume " + subvol.toUtf8()));
    }

    m_ui->pushButton_subvolDelete->clearFocus();
}

void BtrfsAssistant::on_pushButton_btrfsRefreshData_clicked() {
    m_btrfs->loadVolumes();
    m_sourceModel->load(m_btrfs->volumes());
    refreshBtrfsUi();

    m_ui->pushButton_btrfsRefreshData->clearFocus();
}

void BtrfsAssistant::on_pushButton_subvolRefresh_clicked() {

    for (const QString &uuid : m_btrfs->listFilesystems()) {
        m_btrfs->loadSubvols(uuid);
    }

    m_sourceModel->load(m_btrfs->volumes());
    refreshSubvolListUi();

    m_ui->pushButton_subvolRefresh->clearFocus();
}

void BtrfsAssistant::on_pushButton_snapperRestore_clicked() {
    if (m_ui->tableWidget_snapperRestore->currentRow() == -1) {
        displayError(tr("Nothing selected!"));
        return;
    }

    QString config = m_ui->comboBox_snapperSubvols->currentText();
    QString subvol = m_ui->tableWidget_snapperRestore->item(m_ui->tableWidget_snapperRestore->currentRow(), 1)->text();

    QVector<SnapperSubvolume> snapperSubvols = m_snapper->subvols(config);

    // This shouldn't be possible but check anyway
    if (snapperSubvols.isEmpty()) {
        displayError(tr("Failed to restore snapshot"));
        return;
    }

    // For a given subvol they all have the same uuid so we can just use the first one
    QString uuid = snapperSubvols.at(0).uuid;

    restoreSnapshot(uuid, subvol);

    m_ui->pushButton_snapperRestore->clearFocus();
}

void BtrfsAssistant::on_pushButton_snapperBrowse_clicked() {
    QString target = m_ui->comboBox_snapperSubvols->currentText();
    if (m_ui->tableWidget_snapperRestore->currentRow() == -1) {
        displayError("You must select snapshot to browse!");
        return;
    }

    QString subvolPath = m_ui->tableWidget_snapperRestore->item(m_ui->tableWidget_snapperRestore->currentRow(), 1)->text();

    QVector<SnapperSubvolume> snapperSubvols = m_snapper->subvols(target);

    // This shouldn't be possible but check anyway
    if (snapperSubvols.isEmpty()) {
        displayError(tr("Failed to find snapshot to browse"));
        return;
    }

    const QString uuid = snapperSubvols.at(0).uuid;

    // We need to mount the root so we can browse from there
    const QString mountpoint = m_btrfs->mountRoot(uuid);

    FileBrowser fb(m_snapper, QDir::cleanPath(mountpoint + QDir::separator() + subvolPath), uuid);
    fb.exec();
}

void BtrfsAssistant::on_pushButton_snapperCreate_clicked() {
    QString config = m_ui->comboBox_snapperConfigs->currentText();

    // If snapper isn't installed, we should bail
    if (!m_hasSnapper)
        return;

    // This shouldn't be possible but we check anyway
    if (config.isEmpty()) {
        displayError(tr("No config selected for snapshot"));
        return;
    }

    // Ask the user for the description
    bool ok;
    QString snapshotDescription = QInputDialog::getText(this, tr("Enter a description for the snapshot"), tr("Description:"),
                                                        QLineEdit::Normal, "Manual Snapshot", &ok);
    if (!ok) {
        return;
    }

    // OK, let's go ahead and take the snapshot
    const SnapperResult result = m_snapper->createSnapshot(config, snapshotDescription);

    if (result.exitCode != 0) {
        displayError(result.outputList.at(0));
    }

    // Reload the data and refresh the UI
    m_snapper->load();
    loadSnapperUI();
    m_ui->comboBox_snapperConfigs->setCurrentText(config);
    populateSnapperGrid();
    populateSnapperRestoreGrid();

    m_ui->pushButton_snapperCreate->clearFocus();
}

void BtrfsAssistant::on_pushButton_snapperDelete_clicked() {
    if (m_ui->tableWidget_snapperNew->currentRow() == -1) {
        displayError(tr("Nothing selected!"));
        return;
    }

    // Get all the rows that were selected
    const QList<QTableWidgetItem *> list = m_ui->tableWidget_snapperNew->selectedItems();

    QSet<QString> numbers;

    // Get the snapshot numbers for the selected rows
    for (const QTableWidgetItem *item : list) {
        numbers.insert(m_ui->tableWidget_snapperNew->item(item->row(), 0)->text());
    }

    // Ask for confirmation
    if (QMessageBox::question(0, tr("Confirm"), tr("Are you sure you want to delete the selected snapshot(s)?")) != QMessageBox::Yes)
        return;

    QString config = m_ui->comboBox_snapperConfigs->currentText();

    // Delete each selected snapshot
    for (const QString &number : qAsConst(numbers)) {
        // This shouldn't be possible but we check anyway
        if (config.isEmpty() || number.isEmpty()) {
            displayError(tr("Cannot delete snapshot"));
            return;
        }

        // Delete the snapshot
        SnapperResult result = m_snapper->deleteSnapshot(config, number.toInt());
        if (result.exitCode != 0) {
            displayError(result.outputList.at(0));
        }
    }

    // Reload the data and refresh the UI
    m_snapper->load();
    loadSnapperUI();
    m_ui->comboBox_snapperConfigs->setCurrentText(config);
    populateSnapperGrid();
    populateSnapperRestoreGrid();

    m_ui->pushButton_snapperDelete->clearFocus();
}

void BtrfsAssistant::on_pushButton_snapperDeleteConfig_clicked() {
    QString name = m_ui->comboBox_snapperConfigSettings->currentText();

    if (name.isEmpty()) {
        displayError(tr("No config selected"));
        m_ui->pushButton_snapperDeleteConfig->clearFocus();
        return;
    }

    // Ask for confirmation
    if (QMessageBox::question(0, tr("Please Confirm"),
                              tr("Are you sure you want to delete ") + name + "\n\n" + tr("This action cannot be undone")) !=
        QMessageBox::Yes) {
        m_ui->pushButton_snapperDeleteConfig->clearFocus();
        return;
    }

    // Delete the config
    SnapperResult result = m_snapper->deleteConfig(name);

    if (result.exitCode != 0) {
        displayError(result.outputList.at(0));
    }

    // Reload the UI with the new list of configs
    m_snapper->loadConfig(name);
    loadSnapperUI();
    populateSnapperGrid();
    populateSnapperConfigSettings();

    m_ui->pushButton_snapperDeleteConfig->clearFocus();
}

void BtrfsAssistant::on_pushButton_snapperNewConfig_clicked() {
    if (m_ui->groupBox_snapperConfigEdit->isVisible()) {
        m_ui->lineEdit_snapperName->clear();

        // Put the ui back in edit mode
        m_ui->groupBox_snapperConfigDisplay->show();
        m_ui->groupBox_snapperConfigEdit->hide();
        m_ui->groupBox_snapperConfigSettings->show();

        m_ui->pushButton_snapperNewConfig->setText(tr("New Config"));
        m_ui->pushButton_snapperNewConfig->clearFocus();
    } else {
        // Get a list of btrfs mountpoints that could be backed up
        const QStringList mountpoints = Btrfs::listMountpoints();

        if (mountpoints.isEmpty()) {
            displayError(tr("No btrfs subvolumes found"));
            return;
        }

        // Populate the list of mountpoints after checking that their isn't already a config
        m_ui->comboBox_snapperPath->clear();
        const QStringList configs = m_snapper->configs();
        for (const QString &mountpoint : mountpoints) {
            if (m_snapper->config(mountpoint).isEmpty()) {
                m_ui->comboBox_snapperPath->addItem(mountpoint);
            }
        }

        // Put the UI in create config mode
        m_ui->groupBox_snapperConfigDisplay->hide();
        m_ui->groupBox_snapperConfigEdit->show();
        m_ui->groupBox_snapperConfigSettings->hide();

        m_ui->pushButton_snapperNewConfig->setText(tr("Cancel New Config"));
        m_ui->pushButton_snapperNewConfig->clearFocus();
    }
}

void BtrfsAssistant::on_pushButton_snapperSaveConfig_clicked() {
    QString name;

    // If the settings box is visible we are changing settings on an existing config
    if (m_ui->groupBox_snapperConfigSettings->isVisible()) {
        name = m_ui->comboBox_snapperConfigSettings->currentText();
        if (name.isEmpty()) {
            displayError(tr("Failed to save changes"));
            m_ui->pushButton_snapperSaveConfig->clearFocus();
            return;
        }

        QMap<QString, QString> configMap;
        configMap.insert("TIMELINE_CREATE", QString(m_ui->checkBox_snapperEnableTimeline->isChecked() ? "yes" : "no"));
        configMap.insert("TIMELINE_LIMIT_HOURLY", QString::number(m_ui->spinBox_snapperHourly->value()));
        configMap.insert("TIMELINE_LIMIT_DAILY", QString::number(m_ui->spinBox_snapperDaily->value()));
        configMap.insert("TIMELINE_LIMIT_WEEKLY", QString::number(m_ui->spinBox_snapperWeekly->value()));
        configMap.insert("TIMELINE_LIMIT_MONTHLY", QString::number(m_ui->spinBox_snapperMonthly->value()));
        configMap.insert("TIMELINE_LIMIT_YEARLY", QString::number(m_ui->spinBox_snapperYearly->value()));
        configMap.insert("NUMBER_LIMIT", QString::number(m_ui->spinBox_snapperNumber->value()));

        SnapperResult result = m_snapper->setConfig(name, configMap);

        if (result.exitCode != 0) {
            displayError(result.outputList.at(0));
        } else {
            QMessageBox::information(0, tr("Snapper"), tr("Changes saved"));
        }

        loadSnapperUI();
        populateSnapperGrid();
        populateSnapperConfigSettings();
    } else { // This is new config we are creating
        name = m_ui->lineEdit_snapperName->text();

        // Remove any whitespace from name
        name = name.simplified().replace(" ", "");

        if (name.isEmpty()) {
            displayError(tr("Please enter a valid name"));
            m_ui->pushButton_snapperSaveConfig->clearFocus();
            return;
        }

        if (m_snapper->configs().contains(name)) {
            displayError(tr("That name is already in use!"));
            m_ui->pushButton_snapperSaveConfig->clearFocus();
            return;
        }

        // Create the new config
        SnapperResult result = m_snapper->createConfig(name, m_ui->comboBox_snapperPath->currentText());

        if (result.exitCode != 0) {
            displayError(result.outputList.at(0));
        }

        // Reload the UI
        m_snapper->loadConfig(name);
        loadSnapperUI();
        m_ui->comboBox_snapperConfigSettings->setCurrentText(name);
        populateSnapperGrid();
        populateSnapperConfigSettings();

        // Put the ui back in edit mode
        m_ui->groupBox_snapperConfigDisplay->show();
        m_ui->groupBox_snapperConfigEdit->hide();
        m_ui->groupBox_snapperConfigSettings->show();
        m_ui->pushButton_snapperNewConfig->setText(tr("New Config"));
    }

    m_ui->pushButton_snapperSaveConfig->clearFocus();
}

void BtrfsAssistant::on_pushButton_snapperUnitsApply_clicked() {

    updateServices(m_ui->groupBox_snapperUnits->findChildren<QCheckBox *>());

    QMessageBox::information(0, tr("Btrfs Assistant"), tr("Changes applied"));

    m_ui->pushButton_snapperUnitsApply->clearFocus();
}

void BtrfsAssistant::on_pushButton_enableQuota_clicked() {
    if (m_ui->comboBox_btrfsDevice->currentText().isEmpty()) {
        return;
    }
    const QString mountpoint = Btrfs::mountRoot(m_ui->comboBox_btrfsDevice->currentText());

    if (!mountpoint.isEmpty() && m_btrfs->isQuotaEnabled(mountpoint)) {
        Btrfs::setQgroupEnabled(mountpoint, false);
    } else {
        Btrfs::setQgroupEnabled(mountpoint, true);
    }

    setEnableQuotaButtonStatus();
}

void BtrfsAssistant::on_toolButton_snapperNewRefresh_clicked() {
    m_snapper->load();
    populateSnapperGrid();
}

void BtrfsAssistant::on_toolButton_snapperRestoreRefresh_clicked() {
    m_snapper->loadSubvols();
    populateSnapperRestoreGrid();
}

void BtrfsAssistant::setEnableQuotaButtonStatus() {
    if (m_ui->comboBox_btrfsDevice->currentText().isEmpty()) {
        return;
    }
    const QString mountpoint = Btrfs::mountRoot(m_ui->comboBox_btrfsDevice->currentText());

    if (!mountpoint.isEmpty() && m_btrfs->isQuotaEnabled(mountpoint)) {
        m_ui->pushButton_enableQuota->setText(tr("Disable Btrfs Quotas"));
    } else {
        m_ui->pushButton_enableQuota->setText(tr("Enable Btrfs Quotas"));
    }
}
