#include "BtrfsAssistant.h"
#include "Cli.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopWidget>
#include <QSettings>

int main(int argc, char *argv[]) {
    QApplication ba(argc, argv);
    QCoreApplication::setApplicationName(QCoreApplication::translate("main", "Btrfs Assistant"));
    QCoreApplication::setApplicationVersion("1.0");

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

    // Get the config settings
    QSettings settings("/etc/btrfs-assistant.conf", QSettings::NativeFormat);
    QString snapperPath = settings.value("snapper", "/usr/bin/snapper").toString();
    QString btrfsMaintenanceConfig = settings.value("bm_config", "/etc/default/btrfsmaintenance").toString();

    // Load the subvol mapping from the settings file
    settings.beginGroup("Subvol-Mapping");
    const QStringList keys = settings.childKeys();
    QMap<QString, QString> subvolMap;
    for (const QString &key : keys) {
        if (!key.isEmpty() && settings.value(key).toString().contains(",") && !settings.value(key).toString().startsWith("#")) {
            const QStringList mapList = settings.value(key).toString().split(",");
            if (mapList.count() == 3) {
                subvolMap.insert(mapList.at(0).trimmed(), mapList.at(1).trimmed() + "," + mapList.at(2).trimmed());
            }
        }
    }
    settings.endGroup();

    // The btrfs object is used to interact with the application
    Btrfs btrfs;

    // If Snapper is installed, instantiate the snapper object
    Snapper *snapper = nullptr;
    if (QFile::exists(snapperPath)) {
        snapper = new Snapper(&btrfs, snapperPath, subvolMap);
    }

    if (parser.isSet(listOption) && snapper != nullptr) {
        return Cli::listSnapshots(snapper);
    } else if (parser.isSet(restoreOption) && snapper != nullptr) {
        return Cli::restore(&btrfs, snapper, parser.value(restoreOption));
    } else {
        // If Btrfs Maintenance is installed, instantiate the btrfsMaintenance object
        BtrfsMaintenance *btrfsMaintenance = nullptr;
        if (QFile::exists(btrfsMaintenanceConfig)) {
            btrfsMaintenance = new BtrfsMaintenance(btrfsMaintenanceConfig,
                                                    settings.value("bm_refresh_service", "btrfsmaintenance-refresh.service").toString());
        }

        BtrfsAssistant mainWindow(btrfsMaintenance, &btrfs, snapper);
        mainWindow.show();
        return ba.exec();
    }
}
