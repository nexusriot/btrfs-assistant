#include "util/BtrfsMaintenance.h"
#include "util/System.h"

#include <QIODevice>
#include <QSettings>

namespace {

/** @brief A QSettings reader for the Btrfs Maintenance config file
 *
 * Populates @p map with all the keys and values from the file.  Adds an additional key
 * to @p map named "raw" that contains the original contents of the file used for rewriting the
 * file later
 *
 * Returns true
 *
 */
bool readBmFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QStringList rawList;
    while (!device.atEnd()) {
        QString line = device.readLine();
        rawList.append(line);
        if (!line.trimmed().isEmpty() && !line.trimmed().startsWith("#")) {
            const QStringList lineList = line.simplified().trimmed().split("=");
            map.insert(lineList.at(0).trimmed(), lineList.at(1).trimmed().remove("\""));
        }
    }

    map.insert("raw", rawList);

    return true;
}

/** @brief A QSettings writer for the Btrfs Maintenance config file
 *
 * Reads the contents of the "raw" key in @p map and writes a new file based on the structure
 * of the original raw data.  It replaces the settings with the key value pairs in @p map
 *
 * Returns true
 *
 */
bool writeBmFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    QByteArray data;

    if (!map.contains("raw")) {
        return false;
    }

    const QStringList rawList = map.value("raw").toStringList();
    for (const QString &line : rawList) {
        if (line.trimmed().startsWith("#")) {
            data += line.toUtf8();
        } else {
            const QString key = line.simplified().split("=").at(0).trimmed();
            if (map.contains(key)) {
                data += key.toUtf8() + "=\"" + map.value(key).toString().toUtf8() + "\"\n";
            }
        }
    }

    device.write(data);

    return true;
}

} // namespace

BtrfsMaintenance::BtrfsMaintenance(const QString &configFile, QObject *parent) : QObject{parent}
{
    QSettings::Format bmFormat = QSettings::registerFormat("btrfsmaintenance", readBmFile, writeBmFile);
    m_settings = new QSettings(configFile, bmFormat);
}

void BtrfsMaintenance::refresh()
{
    QString script =
        Settings::instance().value("bm_refresh_script", "/usr/share/btrfsmaintenance/btrfsmaintenance-refresh-cron.sh").toString();
    if (System::hasSystemd()) {
        System::runCmd(script, QStringList() << "systemd-timer", false);
    } else {
        System::runCmd(script, QStringList() << "cron", false);
    }
}
