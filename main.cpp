#include "btrfs-assistant.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDesktopWidget>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QTranslator myappTranslator;
    myappTranslator.load("btrfsassistant_" + QLocale::system().name(), "/usr/share/btrfs-assistant/translations");
    a.installTranslator(&myappTranslator);

    BtrfsAssistant w;
    w.show();
    return a.exec();
}
