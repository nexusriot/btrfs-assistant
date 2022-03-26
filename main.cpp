#include "BtrfsAssistant.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopWidget>
#include <QSettings>

// We have to manually parse argv into QStringList because by the time QCoreApplication initializes it's already too late because Qt already
// picked the theme
QStringList parseArgs(int argc, char *argv[]) {
    QStringList list;
    const int ac = argc;
    char **const av = argv;
    for (int a = 0; a < ac; ++a) {
        list << QString::fromLocal8Bit(av[a]);
    }
    return list;
}

int main(int argc, char *argv[]) {
    QCommandLineParser cmdline;
    QCommandLineOption xdgDesktop("xdg-desktop", "Set XDG_CURRENT_DESKTOP via params", "desktop");
    cmdline.addOption(xdgDesktop);
    cmdline.process(parseArgs(argc, argv));
    if (cmdline.isSet(xdgDesktop))
        qputenv("XDG_CURRENT_DESKTOP", cmdline.value(xdgDesktop).toUtf8());

    QApplication a(argc, argv);
    QTranslator myappTranslator;
    myappTranslator.load("btrfsassistant_" + QLocale::system().name(), "/usr/share/btrfs-assistant/translations");
    a.installTranslator(&myappTranslator);

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
            if (mapList.count()==3) {
                subvolMap.insert(mapList.at(0).trimmed(), mapList.at(1).trimmed() + "," + mapList.at(2).trimmed());
            }
        }
    }
    settings.endGroup();

    // The btrfs object is used to interact with the application
    Btrfs btrfs;

    // If Btrfs Maintenance is installed, instantiate the btrfsMaintenance object
    BtrfsMaintenance *btrfsMaintenance = nullptr;
    if (QFile::exists(btrfsMaintenanceConfig)) {
        btrfsMaintenance = new BtrfsMaintenance(btrfsMaintenanceConfig,
                                                settings.value("bm_refresh_service", "btrfsmaintenance-refresh.service").toString());
    }

    // If Snapper is installed, instantiate the snapper object
    Snapper *snapper = nullptr;
    if (QFile::exists(snapperPath)) {
        snapper = new Snapper(&btrfs, snapperPath, subvolMap);
    }

    // For a GUI session, instantiate the Main Window
    BtrfsAssistant mainWindow(btrfsMaintenance, &btrfs, snapper);
    mainWindow.show();
    return a.exec();
}
