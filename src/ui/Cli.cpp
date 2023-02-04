#include "Cli.h"

static void displayError(const QString &error) { QTextStream(stderr) << "Error: " << error << Qt::endl; }

Cli::Cli(QObject *parent) : QObject{parent} {}

int Cli::listSnapshots(Snapper *snapper)
{
    // Ensure the application is running as root
    if (!System::checkRootUid()) {
        displayError(tr("You must run this application as root"));
        return 1;
    }

    const QStringList targets = snapper->subvolKeys();
    for (const QString &target : targets) {
        const QVector<SnapperSubvolume> subvols = snapper->subvols(target);
        for (const SnapperSubvolume &subvol : subvols) {
            QTextStream(stdout) << target << "\t" << subvol.snapshotNum << "\t" << subvol.time.toString() << "\t" << subvol.type << "\t"
                                << subvol.subvol << "," << subvol.uuid << Qt::endl;
        }
    }

    return 0;
}

int Cli::restore(Btrfs *btrfs, Snapper *snapper, const QString &restoreTarget)
{
    // Ensure the application is running as root
    if (!System::checkRootUid()) {
        displayError(tr("You must run this application as root"));
        return 1;
    }

    const QStringList restoreList = restoreTarget.split(',');
    if (restoreList.count() != 2) {
        displayError(tr("Incorrect format of restore parameter: ") + restoreTarget);
        return 1;
    }

    const QString subvolume = restoreList.at(0);
    const QString uuid = restoreList.at(1);

    if (!Btrfs::isSnapper(subvolume)) {
        displayError(tr("This is not a snapshot that can be restored by this application"));
        return 1;
    }

    // Ensure the list of subvolumes is not out-of-date
    btrfs->loadSubvols(uuid);

    const uint64_t subvolId = btrfs->subvolId(uuid, subvolume);
    if (subvolId == 0) {
        displayError(tr("Source snapshot not found"));
    }

    const SubvolResult subvolResultSnapshot = Snapper::findSnapshotSubvolume(subvolume);
    if (!subvolResultSnapshot.success) {
        displayError(tr("Snapshot subvolume not found"));
        return 1;
    }

    // Check the map for the target subvolume
    const SubvolResult sr = snapper->findTargetSubvol(subvolResultSnapshot.name, uuid);
    const uint64_t targetId = btrfs->subvolId(uuid, sr.name);

    if (targetId == 0 || !sr.success) {
        displayError(tr("Target not found"));
        return 1;
    }

    // Everything checks out, time to do the restore
    RestoreResult restoreResult = btrfs->restoreSubvol(uuid, subvolId, targetId);

    // Report the outcome to the end user
    if (restoreResult.isSuccess) {
        QTextStream(stdout) << tr("Snapshot restoration complete.") << Qt::endl
                            << tr("A copy of the original subvolume has been saved as ") << restoreResult.backupSubvolName << Qt::endl
                            << tr("Please reboot immediately");
        return 0;
    } else {
        displayError(restoreResult.failureMessage);
        return 1;
    }
}
