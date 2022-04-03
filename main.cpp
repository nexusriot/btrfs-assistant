#include "BtrfsAssistant.h"
#include "Cli.h"
#include "Settings.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopWidget>

int main(int argc, char *argv[]) {
    QApplication ba(argc, argv);
    QCoreApplication::setApplicationName(QCoreApplication::translate("main", "Btrfs Assistant"));
    QCoreApplication::setApplicationVersion("1.3");
    ba.setWindowIcon(QIcon(":/btrfs-assistant.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription("An application for managing Btrfs and Snapper");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption listOption(QStringList() << "l"
                                                << "list",
                                  QCoreApplication::translate("main", "List snapshots"));
    parser.addOption(listOption);

    QCommandLineOption restoreOption(QStringList() << "r"
                                                   << "restore",
                                     QCoreApplication::translate("main", "Restore the given subvolume/UUID"),
                                     QCoreApplication::translate("main", "subvolume,UUID"));
    parser.addOption(restoreOption);
    parser.process(ba);

    QTranslator myappTranslator;
    myappTranslator.load("btrfsassistant_" + QLocale::system().name(), "/usr/share/btrfs-assistant/translations");
    ba.installTranslator(&myappTranslator);

    QString snapperPath = Settings::getInstance().value("snapper", "/usr/bin/snapper").toString();
    QString btrfsMaintenanceConfig = Settings::getInstance().value("bm_config", "/etc/default/btrfsmaintenance").toString();

    // Ensure we are running on a system with btrfs
    if (!System::runCmd("findmnt --real -no fstype ", false).output.contains("btrfs")) {
        QTextStream(stderr) << "Error: No Btrfs filesystems found" << Qt::endl;
        return 1;
    }

    // The btrfs object is used to interact with the application
    Btrfs btrfs;

    // If Snapper is installed, instantiate the snapper object
    Snapper *snapper = nullptr;
    if (QFile::exists(snapperPath)) {
        snapper = new Snapper(&btrfs, snapperPath);
    }

    if (parser.isSet(listOption) && snapper != nullptr) {
        return Cli::listSnapshots(snapper);
    } else if (parser.isSet(restoreOption) && snapper != nullptr) {
        return Cli::restore(&btrfs, snapper, parser.value(restoreOption));
    } else {
        // If Btrfs Maintenance is installed, instantiate the btrfsMaintenance object
        BtrfsMaintenance *btrfsMaintenance = nullptr;
        if (QFile::exists(btrfsMaintenanceConfig)) {
            btrfsMaintenance = new BtrfsMaintenance(btrfsMaintenanceConfig);
        }

        BtrfsAssistant mainWindow(btrfsMaintenance, &btrfs, snapper);
        mainWindow.show();
        return ba.exec();
    }
}
