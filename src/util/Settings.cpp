#include "Settings.h"

Settings &Settings::instance()
{
    static Settings instance;
    return instance;
}

Settings::Settings(QObject *parent) : QObject{parent}
{
    // Get the config settings
    m_settings = new QSettings(m_filePath, QSettings::NativeFormat);

    // Load the subvol mapping from the settings file
    m_settings->beginGroup("Subvol-Mapping");
    const QStringList keys = m_settings->childKeys();

    for (const QString &key : keys) {
        if (!key.isEmpty() && m_settings->value(key).toString().contains(",") && !m_settings->value(key).toString().startsWith("#")) {
            const QStringList mapList = m_settings->value(key).toString().split(",");
            if (mapList.count() == 3) {
                m_subvolMap.insert(mapList.at(0).trimmed(), mapList.at(1).trimmed() + "," + mapList.at(2).trimmed());
            }
        }
    }
    m_settings->endGroup();
}
