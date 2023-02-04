#include "ui/Cli.h"
#include "ui/MainWindow.h"
#include "util/BtrfsMaintenance.h"
#include "util/Settings.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/btrfs-assistant.svg"));

    QTranslator translator;
    translator.load("btrfsassistant_" + QLocale::system().name(), "/usr/share/btrfs-assistant/translations");
    app.installTranslator(&translator);

    QCoreApplication::setApplicationName(QCoreApplication::translate("main", "Btrfs Assistant"));
    QCoreApplication::setApplicationVersion("1.8");
    // Disable question mark title bar buttons in QDialogs ("What's this?" help buttons)
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "An application for managing Btrfs and Snapper"));
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
    parser.process(app);

    QString snapperPath = Settings::instance().value("snapper", "/usr/bin/snapper").toString();
    QString btrfsMaintenanceConfig = Settings::instance().value("bm_config", "/etc/default/btrfsmaintenance").toString();

    // Ensure we are running on a system with btrfs
    if (!System::runCmd("findmnt --real -no fstype ", false).output.contains("btrfs")) {
        QTextStream(stderr) << QCoreApplication::translate("main", "Error: No Btrfs filesystems found") << Qt::endl;
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
        std::unique_ptr<BtrfsMaintenance> btrfsMaintenance;
        if (QFile::exists(btrfsMaintenanceConfig)) {
            btrfsMaintenance.reset(new BtrfsMaintenance(btrfsMaintenanceConfig));
        }

        MainWindow mainWindow(&btrfs, btrfsMaintenance.get(), snapper);
        mainWindow.show();
        return app.exec();
    }
}
