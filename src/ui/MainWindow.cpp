#include "ui/MainWindow.h"
#include "model/SubvolModel.h"
#include "ui/FileBrowser.h"
#include "ui/RestoreConfirmDialog.h"
#include "ui/SnapshotSubvolumeDialog.h"
#include "ui_MainWindow.h"
#include "util/Btrfs.h"
#include "util/BtrfsMaintenance.h"
#include "util/Snapper.h"
#include "util/System.h"

#include <QDebug>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>

namespace {
enum class SnapperRestoreTableColumn { Number, Subvolume, DateTime, Type, Description };

}

constexpr const char *PARTITION_ROOT_TEXT = "Partition root";

/**
 * @brief cleanTargetSubvol Clean the PARTITION_ROOT_TEXT from the subvol name
 * @param subvol The subvol text to clean
 * @return The cleaned text
 */
static const QString cleanTargetSubvol(const QString subvol) { return subvol == PARTITION_ROOT_TEXT ? QString() : subvol; }

/**
 * @brief Selects all rows in @p listWidget that match an item in @p items
 * @param items - A QStringList which contain the strings to select in @p listWidget
 * @param listWidget - A pointer to a QListWidget where the selections will be made
 */
static void setListWidgetSelections(const QStringList &items, QListWidget *listWidget)
{
    QAbstractItemModel *model = listWidget->model();
    QItemSelectionModel *selectModel = listWidget->selectionModel();
    for (int i = 0; i < model->rowCount(); i++) {
        QModelIndex index = model->index(i, 0);
        if (items.contains(model->data(index).toString())) {
            selectModel->select(index, QItemSelectionModel::Select);
        }
    }
}

MainWindow::MainWindow(Btrfs *btrfs, BtrfsMaintenance *btrfsMaintenance, Snapper *snapper, QWidget *parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow), m_btrfs(btrfs), m_btrfsMaint(btrfsMaintenance), m_snapper(snapper)
{
    m_ui->setupUi(this);
    // Always start on the BTRFS tab, regardless what is the currentIndex in the .ui file
    m_ui->tabWidget_mainWindow->setCurrentWidget(m_ui->tab_btrfs);

    // Ensure the application is running as root
    if (!System::checkRootUid()) {
        displayError(tr("The application must be run as the superuser(root)"));
        exit(1);
    }

    m_hasSnapper = snapper != nullptr;
    m_hasBtrfsmaintenance = btrfsMaintenance != nullptr;

    m_subvolumeModel = new SubvolumeModel(this);
    m_subvolumeModel->load(m_btrfs->filesystems());
    m_subvolumeFilterModel = new SubvolumeFilterModel(this);
    m_subvolumeFilterModel->setSourceModel(m_subvolumeModel);

    connect(m_ui->lineEdit_subvolFilter, &QLineEdit::textChanged, m_subvolumeFilterModel, &SubvolumeFilterModel::setFilterFixedString);
    connect(m_ui->checkBox_subvolIncludeSnapshots, &QCheckBox::toggled, m_subvolumeFilterModel, &SubvolumeFilterModel::setIncludeSnapshots);
    connect(m_ui->checkBox_subvolIncludeContainer, &QCheckBox::toggled, m_subvolumeFilterModel, &SubvolumeFilterModel::setIncludeContainer);

    // timers for filesystem operations
    m_balanceTimer = new QTimer(this);
    m_scrubTimer = new QTimer(this);
    connect(m_balanceTimer, &QTimer::timeout, this, &MainWindow::btrfsBalanceStatusUpdateUI);
    connect(m_scrubTimer, &QTimer::timeout, this, &MainWindow::btrfsScrubStatusUpdateUI);

    setup();
    this->setWindowTitle(QCoreApplication::applicationName());
}

MainWindow::~MainWindow() { delete m_ui; }

void MainWindow::displayError(const QString &errorText) { QMessageBox::critical(this, tr("Error"), errorText); }

void MainWindow::bmRefreshMountpoints()
{
    // Get updated list of mountpoints
    const QStringList mountpoints = Btrfs::listMountpoints();

    // Populate the balance section
    QStringList balanceMounts;
    const QList<QListWidgetItem *> selectedBalanceMounts = m_ui->listWidget_bmBalance->selectedItems();
    for (QListWidgetItem *item : selectedBalanceMounts) {
        balanceMounts << item->text();
    }

    m_ui->listWidget_bmBalance->clear();
    m_ui->listWidget_bmBalance->insertItems(0, mountpoints);

    if (!m_ui->checkBox_bmBalance->isChecked()) {
        setListWidgetSelections(balanceMounts, m_ui->listWidget_bmBalance);
    }

    // Populate the scrub section
    QStringList scrubMounts;
    const QList<QListWidgetItem *> selectedScrubMounts = m_ui->listWidget_bmScrub->selectedItems();
    for (QListWidgetItem *item : selectedScrubMounts) {
        scrubMounts << item->text();
    }
    m_ui->listWidget_bmScrub->clear();
    m_ui->listWidget_bmScrub->insertItems(0, mountpoints);

    if (!m_ui->checkBox_bmScrub->isChecked()) {
        setListWidgetSelections(scrubMounts, m_ui->listWidget_bmScrub);
    }

    // Populate the defrag section
    QStringList defragMounts;
    const QList<QListWidgetItem *> selectedDefragMounts = m_ui->listWidget_bmDefrag->selectedItems();
    for (QListWidgetItem *item : selectedDefragMounts) {
        defragMounts << item->text();
    }

    // In the case of defrag we need to include any nested subvols listed in the config
    QStringList combinedMountpoints = defragMounts + mountpoints;

    // Remove empty and duplicate entries
    combinedMountpoints.removeAll("");
    combinedMountpoints.removeDuplicates();

    m_ui->listWidget_bmDefrag->clear();
    m_ui->listWidget_bmDefrag->insertItems(0, combinedMountpoints);
    if (!m_ui->checkBox_bmDefrag->isChecked()) {
        setListWidgetSelections(defragMounts, m_ui->listWidget_bmDefrag);
    }
}

void MainWindow::btrfsBalanceStatusUpdateUI()
{
    QString uuid = m_ui->comboBox_btrfsDevice->currentText();
    QString balanceStatus = m_btrfs->balanceStatus(Btrfs::findAnyMountpoint(uuid));

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

void MainWindow::btrfsScrubStatusUpdateUI()
{
    QString uuid = m_ui->comboBox_btrfsDevice->currentText();
    QString scrubStatus = m_btrfs->scrubStatus(Btrfs::findAnyMountpoint(uuid));

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

void MainWindow::loadSnapperUI()
{
    // If snapper isn't installed, no need to continue
    if (!m_hasSnapper)
        return;

    // Store current selection to restore after reload
    QString selectedSnapperConfig = m_ui->comboBox_snapperConfigs->currentText();
    QString selectedSnapperSettingsConfig = m_ui->comboBox_snapperConfigSettings->currentText();

    m_ui->comboBox_snapperConfigs->clear();
    m_ui->comboBox_snapperConfigSettings->clear();

    // Load the configs in the snapper new subtab and the snapper settings tabs
    const QStringList configs = m_snapper->configs();
    for (const QString &config : configs) {
        m_ui->comboBox_snapperConfigs->addItem(config);
        m_ui->comboBox_snapperConfigSettings->addItem(config);
    }

    m_ui->comboBox_snapperConfigs->setCurrentText(selectedSnapperConfig);
    m_ui->comboBox_snapperConfigSettings->setCurrentText(selectedSnapperSettingsConfig);

    // Load the subvols in the snapper restore subtab
    for (QString &subvol : m_snapper->subvolKeys()) {
        if (m_ui->comboBox_snapperSubvols->findText(subvol) == -1) {
            if (subvol.isEmpty()) {
                subvol = PARTITION_ROOT_TEXT;
            }
            m_ui->comboBox_snapperSubvols->addItem(subvol);
        }
    }
}

void MainWindow::populateBmTab()
{
    const QStringList frequencyValues = {"none", "daily", "weekly", "monthly"};

    // Populate the frequency values from maintenance configuration
    m_ui->comboBox_bmBalanceFreq->clear();
    m_ui->comboBox_bmBalanceFreq->insertItems(0, frequencyValues);
    m_ui->comboBox_bmBalanceFreq->setCurrentText(m_btrfsMaint->value("BTRFS_BALANCE_PERIOD"));
    m_ui->comboBox_bmScrubFreq->clear();
    m_ui->comboBox_bmScrubFreq->insertItems(0, frequencyValues);
    m_ui->comboBox_bmScrubFreq->setCurrentText(m_btrfsMaint->value("BTRFS_SCRUB_PERIOD"));
    m_ui->comboBox_bmDefragFreq->clear();
    m_ui->comboBox_bmDefragFreq->insertItems(0, frequencyValues);
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
        m_ui->listWidget_bmBalance->setDisabled(false);
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
        m_ui->listWidget_bmScrub->setDisabled(false);
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
        m_ui->listWidget_bmDefrag->setDisabled(false);
        setListWidgetSelections(defragMounts, m_ui->listWidget_bmDefrag);
    }
}

void MainWindow::populateBtrfsUi(const QString &uuid)
{

    setEnableQuotaButtonStatus();

    const BtrfsFilesystem &filesystem = m_btrfs->filesystem(uuid);

    if (!filesystem.isPopulated) {
        return;
    }

    // For the tools section
    int dataPercent = static_cast<int>((double)filesystem.dataUsed / (double)filesystem.dataSize * 100);
    m_ui->progressBar_btrfsdata->setValue(dataPercent);
    m_ui->progressBar_btrfsmeta->setValue(static_cast<int>((double)filesystem.metaUsed / (double)filesystem.metaSize * 100));
    m_ui->progressBar_btrfssys->setValue(static_cast<int>((double)filesystem.sysUsed / (double)filesystem.sysSize * 100));

    // The information section
    m_ui->label_btrfsAllocatedValue->setText(System::toHumanReadable(filesystem.allocatedSize));
    m_ui->label_btrfsUsedValue->setText(System::toHumanReadable(filesystem.usedSize));
    m_ui->label_btrfsSizeValue->setText(System::toHumanReadable(filesystem.totalSize));
    m_ui->label_btrfsFreeValue->setText(System::toHumanReadable(filesystem.freeSize));
    double freePercent = (double)filesystem.allocatedSize / (double)filesystem.totalSize;
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

void MainWindow::populateSnapperConfigSettings()
{
    QString name = m_ui->comboBox_snapperConfigSettings->currentText();
    if (name.isEmpty()) {
        return;
    }

    // Retrieve settings for selected config
    const Snapper::Config &config = m_snapper->config(name);

    if (config.isEmpty()) {
        return;
    }

    // Populate UI elements with the values from the snapper config settings.
    m_ui->label_snapperConfigNameValue->setText(name);
    m_ui->label_snapperBackupPathValue->setText(config.subvolume());
    m_ui->checkBox_snapperEnableTimeline->setChecked(config.isTimelineCreate());
    m_ui->spinBox_snapperHourly->setValue(config.timelineLimitHourly());
    m_ui->spinBox_snapperDaily->setValue(config.timelineLimitDaily());
    m_ui->spinBox_snapperWeekly->setValue(config.timelineLimitWeekly());
    m_ui->spinBox_snapperMonthly->setValue(config.timelineLimitMonthly());
    m_ui->spinBox_snapperYearly->setValue(config.timelineLimitYearly());
    m_ui->spinBox_snapperNumber->setValue(config.numberLimit());

    snapperTimelineEnable(m_ui->checkBox_snapperEnableTimeline->isChecked());
}

void MainWindow::populateSnapperGrid()
{
    // We need to the locale for displaying the date/time
    const QLocale locale = QLocale::system();

    const QString config = m_ui->comboBox_snapperConfigs->currentText();

    // Clear the table and set the headers
    m_ui->tableWidget_snapperNew->clear();
    m_ui->tableWidget_snapperNew->setColumnCount(5);
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Number", "The number associated with a snapshot")));
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Date/Time")));
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Type")));
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Cleanup")));
    m_ui->tableWidget_snapperNew->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Description")));
    m_ui->tableWidget_snapperNew->sortByColumn(0, Qt::DescendingOrder);
    m_ui->tableWidget_snapperNew->setContextMenuPolicy(Qt::CustomContextMenu);

    // Disabling sorting while populating the grid is required or the grid won't repopulate properly
    m_ui->tableWidget_snapperNew->setSortingEnabled(false);

    // Make sure there is something to populate
    QVector<SnapperSnapshot> snapshots = m_snapper->snapshots(config);
    if (snapshots.isEmpty()) {
        return;
    }

    // Populate the table
    m_ui->tableWidget_snapperNew->setRowCount(snapshots.size());
    for (int i = 0; i < snapshots.size(); i++) {
        // Ensure proper sorting of numbers and dates
        QTableWidgetItem *number = new QTableWidgetItem(static_cast<int>(snapshots.at(i).number));
        number->setData(Qt::DisplayRole, snapshots.at(i).number);
        QTableWidgetItem *snapTime = new QTableWidgetItem(locale.toString(snapshots.at(i).time, QLocale::ShortFormat));
        snapTime->setData(Qt::DisplayRole, snapshots.at(i).time);

        m_ui->tableWidget_snapperNew->setItem(i, 0, number);
        m_ui->tableWidget_snapperNew->setItem(i, 1, snapTime);
        m_ui->tableWidget_snapperNew->setItem(i, 2, new QTableWidgetItem(snapshots.at(i).type));
        m_ui->tableWidget_snapperNew->setItem(i, 3, new QTableWidgetItem(snapshots.at(i).cleanup));
        m_ui->tableWidget_snapperNew->setItem(i, 4, new QTableWidgetItem(snapshots.at(i).desc));
    }

    // Re-enable sorting and resize the colums to make everything fit
    m_ui->tableWidget_snapperNew->setSortingEnabled(true);
    m_ui->tableWidget_snapperNew->resizeColumnsToContents();
}

void MainWindow::populateSnapperRestoreGrid()
{
    // We need to the locale for displaying the date/time
    const QLocale locale = QLocale::system();

    // Get the name of the subvolume to list in the grid
    QString config = cleanTargetSubvol(m_ui->comboBox_snapperSubvols->currentText());

    // Clear the table and set the headers
    m_ui->tableWidget_snapperRestore->clear();
    m_ui->tableWidget_snapperRestore->setColumnCount(5);
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem((int)SnapperRestoreTableColumn::Number,
                                                              new QTableWidgetItem(tr("Number", "The number associated with a snapshot")));
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem((int)SnapperRestoreTableColumn::Subvolume,
                                                              new QTableWidgetItem(tr("Subvolume")));
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem((int)SnapperRestoreTableColumn::DateTime,
                                                              new QTableWidgetItem(tr("Date/Time")));
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem((int)SnapperRestoreTableColumn::Type, new QTableWidgetItem(tr("Type")));
    m_ui->tableWidget_snapperRestore->setHorizontalHeaderItem((int)SnapperRestoreTableColumn::Description,
                                                              new QTableWidgetItem(tr("Description")));
    m_ui->tableWidget_snapperRestore->sortByColumn(0, Qt::DescendingOrder);

    // Disabling sorting while populating the grid is required or the grid won't repopulate properly - This must be Qt bug, right?
    m_ui->tableWidget_snapperRestore->setSortingEnabled(false);

    QVector<SnapperSubvolume> subvols = m_snapper->subvols(config);
    // Make sure there is something to populate
    if (subvols.isEmpty()) {
        return;
    }

    // Populate the table
    m_ui->tableWidget_snapperRestore->setRowCount(subvols.count());
    for (int i = 0; i < subvols.count(); i++) {
        const SnapperSubvolume &subvolume = subvols.at(i);
        // Ensure proper sorting of numbers and dates
        QTableWidgetItem *number = new QTableWidgetItem();
        number->setData(Qt::DisplayRole, subvolume.snapshotNum);
        QTableWidgetItem *snapTime = new QTableWidgetItem(locale.toString(subvolume.time, QLocale::ShortFormat));
        snapTime->setData(Qt::DisplayRole, subvolume.time);

        m_ui->tableWidget_snapperRestore->setItem(i, (int)SnapperRestoreTableColumn::Number, number);
        m_ui->tableWidget_snapperRestore->setItem(i, (int)SnapperRestoreTableColumn::Subvolume, new QTableWidgetItem(subvolume.subvol));
        m_ui->tableWidget_snapperRestore->setItem(i, (int)SnapperRestoreTableColumn::DateTime, snapTime);
        m_ui->tableWidget_snapperRestore->setItem(i, (int)SnapperRestoreTableColumn::Type, new QTableWidgetItem(subvolume.type));
        m_ui->tableWidget_snapperRestore->setItem(i, (int)SnapperRestoreTableColumn::Description, new QTableWidgetItem(subvolume.desc));
    }

    // Re-enable sorting and resize the colums to make everything fit
    m_ui->tableWidget_snapperRestore->setSortingEnabled(true);
    m_ui->tableWidget_snapperRestore->resizeColumnsToContents();
}

void MainWindow::refreshBmUi()
{
    // Refresh the mountpoint list widgets
    bmRefreshMountpoints();
}

void MainWindow::refreshBtrfsUi()
{

    // Repopulate device selection combo box with detected btrfs filesystems.
    const QStringList uuidList = Btrfs::listFilesystems();
    for (const QString &uuid : uuidList) {
        if (m_ui->comboBox_btrfsDevice->findText(uuid) == -1) {
            m_ui->comboBox_btrfsDevice->addItem(uuid);
        }
    }

    // Repopulate data using the first detected btrfs filesystem.
    populateBtrfsUi(m_ui->comboBox_btrfsDevice->currentText());
    refreshSubvolListUi();
}

void MainWindow::refreshSnapperServices()
{
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

void MainWindow::refreshSubvolListUi()
{
    bool showQuota = false;

    const auto filesystems = m_btrfs->listFilesystems();
    for (const QString &uuid : filesystems) {
        // Check to see if the size related colums should be hidden
        if (Btrfs::isQuotaEnabled(Btrfs::findAnyMountpoint(uuid))) {
            showQuota = true;
        }
    }

    // Update table sizes and columns based on subvolumes
    m_ui->tableView_subvols->verticalHeader()->hide();
    m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::Id);
    m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::ParentId);
    m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::Uuid);
    m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::ParentUuid);
    m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::ReceivedUuid);
    m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::Generation);

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
        m_ui->tableView_subvols->hideColumn(SubvolumeModel::Column::FilesystemUuid);
    }
}

void MainWindow::restoreSnapshot(const QString &uuid, const QString &subvolume)
{
    if (!Btrfs::isSnapper(subvolume)) {
        displayError(tr("This is not a snapshot that can be restored by this application"));
        return;
    }

    // Ensure the list of subvolumes is not out-of-date
    m_btrfs->loadSubvols(uuid);

    const uint64_t subvolId = m_btrfs->subvolId(uuid, subvolume);
    const SubvolResult subvolResultSnapshot = Snapper::findSnapshotSubvolume(subvolume);
    if (!subvolResultSnapshot.success) {
        displayError(tr("Snapshot subvolume not found"));
        return;
    }

    // Check the map for the target subvolume
    const SubvolResult sr = m_snapper->findTargetSubvol(subvolResultSnapshot.name, uuid);
    const QString targetSubvol = sr.name;
    const uint64_t targetId = m_btrfs->subvolId(uuid, targetSubvol);

    if (targetId == 0 || !sr.success) {
        displayError(tr("Target not found"));
        return;
    }

    if (System::isSubvolidInFstab()) {
        QMessageBox::warning(
            this, tr("Warning subvolid mount detected!"),
            tr("It appears you are currently mounting by subvolid.  Doing a restore in this case may not produce the expected outcome.  "
               "It is highly recommended you switch to mounting by subvolume path before proceeding!"));
    }

    // We are out of errors to check for, time to ask for confirmation
    RestoreConfirmDialog confirmDialog("Confirm", tr("Are you sure you want to restore ") + subvolume + tr(" to ", "as in from/to") +
                                                      targetSubvol + "?");

    if (confirmDialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString backupName = confirmDialog.backupName();

    // Everything checks out, time to do the restore
    RestoreResult restoreResult = m_btrfs->restoreSubvol(uuid, subvolId, targetId, backupName);

    // Report the outcome to the end user
    if (restoreResult.isSuccess) {
        QMessageBox::information(this, tr("Snapshot Restore"),
                                 tr("Snapshot restoration complete.") + "\n\n" + tr("A copy of the original subvolume has been saved as ") +
                                     restoreResult.backupSubvolName + "\n\n" + tr("Please reboot immediately"));
    } else {
        displayError(restoreResult.failureMessage);
    }
}

void MainWindow::setup()
{

    // If snapper isn't installed, hide the snapper-related elements of the UI
    if (m_hasSnapper) {
        m_ui->groupBox_snapperConfigCreate->hide();
    } else {
        m_ui->tabWidget_mainWindow->setTabVisible(m_ui->tabWidget_mainWindow->indexOf(m_ui->tab_snapper_general), false);
        m_ui->tabWidget_mainWindow->setTabVisible(m_ui->tabWidget_mainWindow->indexOf(m_ui->tab_snapper_settings), false);
    }

    // If the system isn't running systemd, hide the systemd-related elements of the UI
    if (!System::hasSystemd()) {
        m_ui->groupBox_snapperUnits->hide();
    }

    // Disable the restore and browse buttons until a selection is made
    m_ui->toolButton_subvolumeBrowse->setEnabled(false);
    m_ui->toolButton_subvolRestoreBackup->setEnabled(false);

    // Connect the subvolume view
    m_ui->tableView_subvols->setModel(m_subvolumeFilterModel);
    connect(m_ui->tableView_subvols->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
            SLOT(subvolsSelectionChanged()));
    m_ui->tableView_subvols->setContextMenuPolicy(Qt::CustomContextMenu);

    m_ui->tableView_subvols->sortByColumn(SubvolumeModel::Column::Name, Qt::AscendingOrder);
    m_ui->tableView_subvols->horizontalHeader()->setSectionResizeMode(SubvolumeModel::Column::Name, QHeaderView::Stretch);
    m_ui->tableView_subvols->horizontalHeader()->setSectionResizeMode(SubvolumeModel::Column::FilesystemUuid,
                                                                      QHeaderView::ResizeToContents);
    m_ui->tableView_subvols->horizontalHeader()->setSectionResizeMode(SubvolumeModel::Column::CreatedAt, QHeaderView::ResizeToContents);
    m_ui->tableView_subvols->horizontalHeader()->setSectionResizeMode(SubvolumeModel::Column::ReadOnly, QHeaderView::ResizeToContents);
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
        m_ui->tabWidget_mainWindow->setTabVisible(m_ui->tabWidget_mainWindow->indexOf(m_ui->tab_btrfsmaintenance), false);
    }
}

void MainWindow::setSnapperSettingsEditModeEnabled(bool enabled)
{

    m_ui->lineEdit_snapperName->clear();
    m_ui->pushButton_snapperNewConfig->setText(tr(enabled ? "New Config" : "Cancel New Config"));
    m_ui->pushButton_snapperNewConfig->clearFocus();

    m_ui->groupBox_snapperConfigCreate->setVisible(!enabled);
    m_ui->groupBox_snapperConfigDisplay->setVisible(enabled);
    m_ui->groupBox_snapperConfigSettings->setVisible(enabled);
}

void MainWindow::snapperTimelineEnable(bool enable)
{
    m_ui->spinBox_snapperHourly->setEnabled(enable);
    m_ui->spinBox_snapperDaily->setEnabled(enable);
    m_ui->spinBox_snapperWeekly->setEnabled(enable);
    m_ui->spinBox_snapperMonthly->setEnabled(enable);
    m_ui->spinBox_snapperYearly->setEnabled(enable);
}

void MainWindow::updateServices(QList<QCheckBox *> checkboxList)
{
    QStringList enabledUnits = System::findEnabledUnits();

    for (auto checkbox : checkboxList) {
        QString service = checkbox->property("actionData").toString().trimmed();
        if (service != "" && enabledUnits.contains(service) != checkbox->isChecked()) {
            System::enableService(service, checkbox->isChecked());
        }
    }
}

void MainWindow::on_checkBox_bmBalance_clicked(bool checked) { m_ui->listWidget_bmBalance->setDisabled(checked); }

void MainWindow::on_checkBox_bmDefrag_clicked(bool checked) { m_ui->listWidget_bmDefrag->setDisabled(checked); }

void MainWindow::on_checkBox_bmScrub_clicked(bool checked) { m_ui->listWidget_bmScrub->setDisabled(checked); }

void MainWindow::on_checkBox_snapperEnableTimeline_clicked(bool checked) { snapperTimelineEnable(checked); }

void MainWindow::on_comboBox_btrfsDevice_activated(int index)
{
    Q_UNUSED(index);

    QString uuid = m_ui->comboBox_btrfsDevice->currentText();
    if (!uuid.isEmpty()) {
        populateBtrfsUi(uuid);
        refreshSubvolListUi();
    }
    m_ui->comboBox_btrfsDevice->clearFocus();
}

void MainWindow::on_comboBox_snapperConfigSettings_activated(int index)
{
    Q_UNUSED(index);

    populateSnapperConfigSettings();

    m_ui->comboBox_snapperConfigSettings->clearFocus();
}

void MainWindow::on_comboBox_snapperConfigs_activated(int index)
{
    Q_UNUSED(index);

    populateSnapperGrid();
    m_ui->comboBox_snapperConfigs->clearFocus();
}

void MainWindow::on_comboBox_snapperSubvols_activated(int index)
{
    Q_UNUSED(index);

    populateSnapperRestoreGrid();
    m_ui->comboBox_snapperSubvols->clearFocus();
}

void MainWindow::on_pushButton_btrfsBalance_clicked()
{
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

void MainWindow::on_pushButton_btrfsRefreshData_clicked()
{
    m_btrfs->loadVolumes();
    m_subvolumeModel->load(m_btrfs->filesystems());
    refreshBtrfsUi();

    m_ui->pushButton_btrfsRefreshData->clearFocus();
}

void MainWindow::on_pushButton_btrfsScrub_clicked()
{
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

void MainWindow::on_pushButton_enableQuota_clicked()
{
    if (m_ui->comboBox_btrfsDevice->currentText().isEmpty()) {
        return;
    }
    const QString mountpoint = Btrfs::findAnyMountpoint(m_ui->comboBox_btrfsDevice->currentText());

    if (!mountpoint.isEmpty() && m_btrfs->isQuotaEnabled(mountpoint)) {
        Btrfs::setQgroupEnabled(mountpoint, false);
    } else {
        Btrfs::setQgroupEnabled(mountpoint, true);
    }

    setEnableQuotaButtonStatus();
}

void MainWindow::on_pushButton_snapperDeleteConfig_clicked()
{
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

void MainWindow::on_pushButton_snapperNewConfig_clicked()
{
    if (m_ui->groupBox_snapperConfigCreate->isVisible()) {
        setSnapperSettingsEditModeEnabled(true);
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
        setSnapperSettingsEditModeEnabled(false);
    }
}

void MainWindow::on_pushButton_snapperSaveConfig_clicked()
{
    QString name;

    // If the settings box is visible we are changing settings on an existing config
    if (m_ui->groupBox_snapperConfigSettings->isVisible()) {
        name = m_ui->comboBox_snapperConfigSettings->currentText();
        if (name.isEmpty()) {
            displayError(tr("Failed to save changes"));
            m_ui->pushButton_snapperSaveConfig->clearFocus();
            return;
        }

        Snapper::Config config;
        config.setTimelineCreate(m_ui->checkBox_snapperEnableTimeline->isChecked());
        config.setTimelineLimitHourly(m_ui->spinBox_snapperHourly->value());
        config.setTimelineLimitDaily(m_ui->spinBox_snapperDaily->value());
        config.setTimelineLimitWeekly(m_ui->spinBox_snapperWeekly->value());
        config.setTimelineLimitMonthly(m_ui->spinBox_snapperMonthly->value());
        config.setTimelineLimitYearly(m_ui->spinBox_snapperYearly->value());
        config.setNumberLimit(m_ui->spinBox_snapperNumber->value());

        SnapperResult result = m_snapper->setConfig(name, config);

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
        setSnapperSettingsEditModeEnabled(true);
    }

    m_ui->pushButton_snapperSaveConfig->clearFocus();
}

void MainWindow::on_pushButton_snapperUnitsApply_clicked()
{

    updateServices(m_ui->groupBox_snapperUnits->findChildren<QCheckBox *>());

    QMessageBox::information(0, tr("Btrfs Assistant"), tr("Changes applied"));

    m_ui->pushButton_snapperUnitsApply->clearFocus();
}

void MainWindow::on_tableView_subvols_customContextMenuRequested(const QPoint &pos)
{
    if (!m_ui->tableView_subvols->selectionModel()->hasSelection()) {
        return;
    }

    QVector<Subvolume> selectedSubvolumes;
    const QModelIndexList selectedIndexes = m_ui->tableView_subvols->selectionModel()->selectedRows(SubvolumeModel::Column::Name);

    for (const QModelIndex &idx : selectedIndexes) {
        QModelIndex sourceIdx = m_subvolumeFilterModel->mapToSource(idx);
        const Subvolume &s = m_subvolumeModel->subvolume(sourceIdx.row());
        selectedSubvolumes.append(s);
    }

    QVector<Subvolume> readOnlySubvols;
    QVector<Subvolume> writeableSubvols;

    for (const Subvolume &s : selectedSubvolumes) {
        if (s.isReadOnly()) {
            readOnlySubvols.append(s);
        } else {
            writeableSubvols.append(s);
        }
    }

    auto setReadOnlyFlag = [this](const QVector<Subvolume> &subvols, bool readOnly) {
        QString msg;
        if (subvols.size() == 1) {
            if (readOnly) {
                msg = tr("Are you sure you want to set read-only flag for %1?").arg(subvols.begin()->subvolName);
            } else {
                msg = tr("Are you sure you want to clear read-only flag for %1?").arg(subvols.begin()->subvolName);
            }
        } else {
            if (readOnly) {
                msg = tr("Are you sure you want to set read-only flag for %1 subvolumes?").arg(subvols.size());
            } else {
                msg = tr("Are you sure you want to clear read-only flag for %1 subvolumes?").arg(subvols.size());
            }
        }
        if (QMessageBox::question(this, tr("Confirm"), msg) == QMessageBox::Yes) {
            QVector<Subvolume> failed;
            for (Subvolume s : subvols) {
                if (m_btrfs->setSubvolumeReadOnly(s, readOnly)) {
                    s.flags = readOnly ? 0x1u : 0;
                    m_subvolumeModel->updateSubvolume(s);
                } else {
                    failed.append(s);
                }
            }

            if (failed.isEmpty()) {
                QMessageBox::information(0, tr("Btrfs Assistant"), tr("Changes applied"));
            } else {
                QStringList names;
                std::transform(failed.begin(), failed.end(), std::back_inserter(names), [](const Subvolume &s) { return s.subvolName; });
                QMessageBox::critical(0, tr("Btrfs Assistant"),
                                      tr("Failed to apply changes to the following subvolumes:") + "\n" + names.join(QChar('\n')));
            }
        }
    };

    QMenu menu;

    if (selectedSubvolumes.size() == 1) {
        const Subvolume &subvol = selectedSubvolumes.first();
        QAction *snapshotAction = menu.addAction(tr("Create &snapshot..."));
        connect(snapshotAction, &QAction::triggered, this, [this, subvol]() {
            SnapshotSubvolumeDialog dialog(this);
            dialog.selectAllTextAndSetFocus();

            if (dialog.exec() == QDialog::Accepted) {
                std::optional<Subvolume> snapshot =
                    m_btrfs->createSnapshot(subvol.filesystemUuid, subvol.id, dialog.destination(), dialog.isReadOnly());
                if (snapshot) {
                    m_subvolumeModel->addSubvolume(*snapshot);
                    QMessageBox::information(0, tr("Btrfs Assistant"), tr("Snapshot created"));
                } else {
                    QMessageBox::critical(0, tr("Btrfs Assistant"), tr("Failed to create the snapshot"));
                }
            }
        });

        QAction *browseAction = menu.addAction(tr("Browse subvolume..."));
        connect(browseAction, &QAction::triggered, this, [this, subvol]() { on_toolButton_subvolumeBrowse_clicked(); });

        if (m_btrfs->isSubvolumeBackup(subvol.subvolName)) {
            QAction *browseAction = menu.addAction(tr("Restore backup..."));
            connect(browseAction, &QAction::triggered, this, [this, subvol]() { on_toolButton_subvolRestoreBackup_clicked(); });
        }

        if (!writeableSubvols.isEmpty()) {
            QAction *readOnlyAction = menu.addAction(tr("Set &read-only flag"));
            connect(readOnlyAction, &QAction::triggered, this,
                    [setReadOnlyFlag, writeableSubvols]() { setReadOnlyFlag(writeableSubvols, true); });
        }

        if (!readOnlySubvols.isEmpty()) {
            QAction *writeableAction = menu.addAction(tr("&Clear read-only flag"));
            connect(writeableAction, &QAction::triggered, this,
                    [setReadOnlyFlag, readOnlySubvols]() { setReadOnlyFlag(readOnlySubvols, false); });
        }

        QAction *deleteAction = menu.addAction(tr("&Delete"));
        connect(deleteAction, &QAction::triggered, this, &MainWindow::on_toolButton_subvolDelete_clicked);

        menu.exec(m_ui->tableView_subvols->mapToGlobal(pos));
    }
}

void MainWindow::on_tableWidget_snapperNew_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;

    QAction *action = menu.addAction(tr("Set cleanup algorithm to &timeline"));
    connect(action, &QAction::triggered, this, [=]() { this->setCleanup("timeline"); });

    action = menu.addAction(tr("Set cleanup algorithm to &number"));
    connect(action, &QAction::triggered, this, [=]() { this->setCleanup("number"); });

    action = menu.addAction(tr("&Remove cleanup algorithm"));
    connect(action, &QAction::triggered, this, [=]() { this->setCleanup(QString()); });

    action = menu.addAction(tr("&Delete snapshot"));
    connect(action, &QAction::triggered, this, &MainWindow::on_toolButton_snapperDelete_clicked);

    menu.exec(m_ui->tableView_subvols->mapToGlobal(pos));
}

void MainWindow::on_toolButton_bmApply_clicked()
{
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

    m_ui->toolButton_bmApply->clearFocus();
}

void MainWindow::on_toolButton_bmReset_clicked() { populateBmTab(); }

void MainWindow::on_toolButton_subvolumeBrowse_clicked()
{
    if (!m_ui->tableView_subvols->selectionModel()->hasSelection()) {
        displayError("You must select snapshot to browse!");
        return;
    }

    QVector<Subvolume> selectedSubvolumes;
    const QModelIndexList selectedIndexes = m_ui->tableView_subvols->selectionModel()->selectedRows(SubvolumeModel::Column::Name);

    for (const QModelIndex &idx : selectedIndexes) {
        QModelIndex sourceIdx = m_subvolumeFilterModel->mapToSource(idx);
        const Subvolume &s = m_subvolumeModel->subvolume(sourceIdx.row());
        selectedSubvolumes.append(s);
    }

    QString subvolPath = selectedSubvolumes.at(0).subvolName;

    const QString uuid = selectedSubvolumes.at(0).filesystemUuid;

    // We need to mount the root so we can browse from there
    const QString mountpoint = m_btrfs->mountRoot(uuid);

    auto fb = new FileBrowser(QDir::cleanPath(mountpoint + QDir::separator() + subvolPath), uuid, this);
    // Prefix the window title with target and snapshot number, so user can make sense of multiple windows
    fb->setWindowTitle(QString("%1 - %2").arg(subvolPath, fb->windowTitle()));
    fb->setAttribute(Qt::WA_DeleteOnClose, true);
    fb->show();
}

void MainWindow::on_toolButton_subvolDelete_clicked()
{
    m_ui->toolButton_subvolDelete->clearFocus();

    if (!m_ui->tableView_subvols->selectionModel()->hasSelection()) {
        displayError(tr("Please select a subvolume to delete first!"));
        return;
    }

    // Ask for confirmation
    if (QMessageBox::question(this, tr("Confirm"), tr("Are you sure you want to delete the selected subvolume(s)?")) != QMessageBox::Yes) {
        return;
    }

    // Get all the rows that were selected
    const QModelIndexList nameIndexes = m_ui->tableView_subvols->selectionModel()->selectedRows(SubvolumeModel::Column::Name);
    const QModelIndexList uuidIndexes = m_ui->tableView_subvols->selectionModel()->selectedRows(SubvolumeModel::Column::FilesystemUuid);

    // Check for a snapper snapshot and ask if the metadata should be cleaned up if found
    bool hasSnapshot = false;
    for (const auto &index : nameIndexes) {
        if (Btrfs::isSnapper(index.data().toString())) {
            hasSnapshot = true;
            break;
        }
    }

    bool cleanupSnapper = false;
    if (hasSnapshot && QMessageBox::question(this, tr("Snapper Snapshots Found"),
                                             tr("One or more of the selected subvolumes is a Snapper snapshot, would you like to remove "
                                                "the Snapper Metadata?(Recommended)")) == QMessageBox::Yes) {
        cleanupSnapper = true;
    }

    QSet<QString> uuids;

    for (int i = 0; i < nameIndexes.count(); i++) {
        QString subvol = nameIndexes.at(i).data().toString();
        QString uuid = uuidIndexes.at(i).data().toString();

        // Add the uuid to the set of uuids with subvolumes deleted
        uuids.insert(uuid);

        // Make sure the everything is good in the UI
        if (subvol.isEmpty() || uuid.isEmpty()) {
            continue;
        }

        // get the subvolid, if it isn't found abort
        uint64_t subvolid = m_btrfs->subvolId(uuid, subvol);
        if (subvolid == 0) {
            displayError(tr("Failed to delete subvolume!") + "\n\n" + tr("Invalid subvolume ID"));
            continue;
        }

        // ensure the subvol isn't mounted, btrfs will delete a mounted subvol but we probably shouldn't
        if (Btrfs::isMounted(uuid, subvolid)) {
            displayError(tr("You cannot delete mounted subvolume: ") + subvol + "\n\n" +
                         tr("Please unmount the subvolume before deleting"));
            continue;
        }

        if (!m_btrfs->deleteSubvol(uuid, subvolid)) {
            displayError(tr("Failed to delete subvolume " + subvol.toUtf8()));
            continue;
        }

        // If this is a Snapper snapshot and removing the metadata was agreed to, clean it up
        if (cleanupSnapper && Btrfs::isSnapper(subvol)) {
            const QString mountpoint = m_btrfs->mountRoot(uuid);
            const QFileInfo subvolumeFileInfo(QDir::cleanPath(mountpoint + QDir::separator() + subvol));
            subvolumeFileInfo.dir().removeRecursively();
        }
    }

    // Reload data and refresh the UI
    for (const auto &uuid : qAsConst(uuids)) {
        m_btrfs->loadSubvols(uuid);
    }
    m_subvolumeModel->load(m_btrfs->filesystems());
    refreshSubvolListUi();
}

void MainWindow::on_toolButton_subvolRefresh_clicked()
{
    const auto filesystems = m_btrfs->listFilesystems();
    for (const QString &uuid : filesystems) {
        m_btrfs->loadSubvols(uuid);
    }

    m_subvolumeModel->load(m_btrfs->filesystems());
    refreshSubvolListUi();

    m_ui->toolButton_subvolRefresh->clearFocus();
}

void MainWindow::on_toolButton_snapperRestore_clicked()
{
    if (m_ui->tableWidget_snapperRestore->currentRow() == -1) {
        displayError(tr("Nothing selected!"));
        return;
    }

    QString config = cleanTargetSubvol(m_ui->comboBox_snapperSubvols->currentText());
    QString subvol =
        m_ui->tableWidget_snapperRestore->item(m_ui->tableWidget_snapperRestore->currentRow(), (int)SnapperRestoreTableColumn::Subvolume)
            ->text();

    QVector<SnapperSubvolume> snapperSubvols = m_snapper->subvols(config);

    // This shouldn't be possible but check anyway
    if (snapperSubvols.isEmpty()) {
        displayError(tr("Failed to restore snapshot"));
        return;
    }

    // For a given subvol they all have the same uuid so we can just use the first one
    QString uuid = snapperSubvols.at(0).uuid;

    restoreSnapshot(uuid, subvol);

    m_ui->toolButton_snapperRestore->clearFocus();
}

void MainWindow::on_toolButton_snapperBrowse_clicked()
{
    const int currentRow = m_ui->tableWidget_snapperRestore->currentRow();
    if (currentRow == -1) {
        displayError("You must select snapshot to browse!");
        return;
    }

    QString subvolPath = m_ui->tableWidget_snapperRestore->item(currentRow, (int)SnapperRestoreTableColumn::Subvolume)->text();
    uint snapshotNumber =
        m_ui->tableWidget_snapperRestore->item(currentRow, (int)SnapperRestoreTableColumn::Number)->data(Qt::DisplayRole).toUInt();

    QString target = cleanTargetSubvol(m_ui->comboBox_snapperSubvols->currentText());
    QVector<SnapperSubvolume> snapperSubvols = m_snapper->subvols(target);

    // This shouldn't be possible but check anyway
    if (snapperSubvols.isEmpty()) {
        displayError(tr("Failed to find snapshot to browse"));
        return;
    }

    const QString uuid = snapperSubvols.at(0).uuid;

    // We need to mount the root so we can browse from there
    const QString mountpoint = m_btrfs->mountRoot(uuid);

    auto fb = new FileBrowser(m_snapper, QDir::cleanPath(mountpoint + QDir::separator() + subvolPath), uuid, this);
    // Prefix the window title with target and snapshot number, so user can make sense of multiple windows
    fb->setWindowTitle(QString("%1:%2 - %3").arg(target, QString::number(snapshotNumber), fb->windowTitle()));
    fb->setAttribute(Qt::WA_DeleteOnClose, true);
    fb->show();
}

void MainWindow::on_toolButton_snapperCreate_clicked()
{
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
    m_btrfs->loadVolumes();
    m_snapper->load();
    loadSnapperUI();
    m_ui->comboBox_snapperConfigs->setCurrentText(config);
    populateSnapperGrid();
    populateSnapperRestoreGrid();

    m_ui->toolButton_snapperCreate->clearFocus();
}

void MainWindow::on_toolButton_snapperDelete_clicked()
{
    // Get all the rows that were selected
    const QList<QTableWidgetItem *> list = m_ui->tableWidget_snapperNew->selectedItems();

    if (list.count() < 1) {
        displayError(tr("Nothing selected!"));
        return;
    }

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
    m_btrfs->loadVolumes();
    m_snapper->load();
    loadSnapperUI();
    m_ui->comboBox_snapperConfigs->setCurrentText(config);
    populateSnapperGrid();
    populateSnapperRestoreGrid();

    m_ui->toolButton_snapperDelete->clearFocus();
}

void MainWindow::subvolsSelectionChanged()
{
    if (m_ui->tableView_subvols->selectionModel()->hasSelection()) {

        const QModelIndexList selectedIndexes = m_ui->tableView_subvols->selectionModel()->selectedRows(SubvolumeModel::Column::Name);

        if (selectedIndexes.length() > 1) {
            m_ui->toolButton_subvolumeBrowse->setEnabled(false);
            m_ui->toolButton_subvolRestoreBackup->setEnabled(false);
        } else {
            m_ui->toolButton_subvolumeBrowse->setEnabled(true);

            QString subvolPath = m_subvolumeModel->subvolume(m_subvolumeFilterModel->mapToSource(selectedIndexes.at(0)).row()).subvolName;

            // Ensure it is a backup we created
            m_ui->toolButton_subvolRestoreBackup->setEnabled(m_btrfs->isSubvolumeBackup(subvolPath));
        }
    }
}

void MainWindow::on_tabWidget_mainWindow_currentChanged()
{
    if (m_ui->tabWidget_mainWindow->currentWidget() == m_ui->tab_btrfsmaintenance) {
        refreshBmUi();
    }
}

void MainWindow::on_toolButton_snapperNewRefresh_clicked()
{
    m_snapper->load();
    populateSnapperGrid();
}

void MainWindow::on_toolButton_snapperRestoreRefresh_clicked()
{
    m_btrfs->loadVolumes();
    m_snapper->loadSubvols();
    populateSnapperRestoreGrid();
}

void MainWindow::on_toolButton_subvolRestoreBackup_clicked()
{
    m_ui->toolButton_subvolRestoreBackup->clearFocus();

    if (!m_ui->tableView_subvols->selectionModel()->hasSelection()) {
        displayError(tr("Nothing selected!"));
        return;
    }

    // Get all the rows that were selected
    const QModelIndexList nameIndexes = m_ui->tableView_subvols->selectionModel()->selectedRows(SubvolumeModel::Column::Name);

    // Perform some sanity checks
    if (nameIndexes.count() != 1) {
        displayError(tr("Please select a single backup subvolume to restore!"));
        return;
    }

    const QString name = nameIndexes.at(0).data().toString();

    // Ensure it is a backup we created
    static QRegularExpression re("_backup_[0-9]{17}");
    const QStringList nameParts = name.split(re);

    if (nameParts.count() != 2) {
        displayError(tr("The subvolume you selected is not a Btrfs Assistant backup"));
        return;
    }

    const QString uuid =
        m_ui->tableView_subvols->selectionModel()->selectedRows(SubvolumeModel::Column::FilesystemUuid).at(0).data().toString();

    const uint64_t sourceId = m_btrfs->subvolId(uuid, name);
    const uint64_t targetId = m_btrfs->subvolId(uuid, nameParts[0]);

    if (sourceId == 0 or targetId == 0) {
        displayError(tr("The subvolume is missing!"));
        return;
    }

    // Ask for confirmation
    RestoreConfirmDialog confirmDialog("Confirm", tr("Are you sure you want to restore the selected backup?"));

    if (confirmDialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString backupName = confirmDialog.backupName();

    // Everything checks out, time to do the restore
    RestoreResult restoreResult = m_btrfs->restoreSubvol(uuid, sourceId, targetId, backupName);

    // Report the outcome to the end user
    if (restoreResult.isSuccess) {
        QMessageBox::information(this, tr("Backup Restore"),
                                 tr("Backup restoration complete.") + "\n\n" + tr("A copy of the original subvolume has been saved as ") +
                                     restoreResult.backupSubvolName + "\n\n" + tr("Please reboot immediately"));
    } else {
        displayError(restoreResult.failureMessage);
    }
}

void MainWindow::setCleanup(const QString &cleanupArg)
{
    const QList<QTableWidgetItem *> list = m_ui->tableWidget_snapperNew->selectedItems();

    if (list.count() < 1) {
        displayError(tr("Nothing selected!"));
        return;
    }

    QSet<uint> numbers;

    // Get the snapshot numbers for the selected rows
    for (const QTableWidgetItem *item : list) {
        numbers.insert(m_ui->tableWidget_snapperNew->item(item->row(), 0)->text().toUInt());
    }

    const QString config = m_ui->comboBox_snapperConfigs->currentText();

    for (const uint &number : qAsConst(numbers)) {
        SnapperResult sr = m_snapper->setCleanupAlgorithm(config, number, cleanupArg);
        if (sr.exitCode != 0) {
            displayError(tr("Failed to set cleanup algorithm for snapshot %1").arg(number));
        }
    }

    // Reload the data and refresh the UI
    m_snapper->load();
    loadSnapperUI();
    m_ui->comboBox_snapperConfigs->setCurrentText(config);
    populateSnapperGrid();
}

void MainWindow::setEnableQuotaButtonStatus()
{
    if (m_ui->comboBox_btrfsDevice->currentText().isEmpty()) {
        return;
    }
    const QString mountpoint = Btrfs::findAnyMountpoint(m_ui->comboBox_btrfsDevice->currentText());

    if (!mountpoint.isEmpty() && m_btrfs->isQuotaEnabled(mountpoint)) {
        m_ui->pushButton_enableQuota->setText(tr("Disable Btrfs Quotas"));
    } else {
        m_ui->pushButton_enableQuota->setText(tr("Enable Btrfs Quotas"));
    }
}
