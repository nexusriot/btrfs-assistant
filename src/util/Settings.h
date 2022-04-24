#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

/**
 * @brief The Settings class is a singleton that allows read access to the settings file
 */
class Settings : public QObject {
    Q_OBJECT
  public:
    /**
     * @brief Gets a reference to the Settings object
     */
    static Settings &instance();

    /**
     * @brief Gets a reference to the subvol mapping
     */
    QMap<QString, QString> *subvolMap() { return &m_subvolMap; }

    /**
     * @brief Wraps QSettings.value() to access a setting specified by @p key
     * @param key - The key to find the value of
     * @param defaultValue - A default value if the key is not found
     * @return The value associated with @p key.  If key is not found, returns @p defaultValue or default constructed QVariant
     */
    QVariant value(const QString &key, const QVariant &defaultValue) { return m_settings->value(key, defaultValue); }

  private:
    explicit Settings(QObject *parent = nullptr);
    // Delete the copy constructor and the assignment operator
    Settings(Settings const &) = delete;
    void operator=(Settings const &) = delete;

    // The absolute path to the config file
    const QString m_filePath = QStringLiteral("/etc/btrfs-assistant.conf");
    QSettings *m_settings;
    // The mapping of manual snapshot subvols to target subvols, the key is the snapshot subvol
    QMap<QString, QString> m_subvolMap;
};

#endif // SETTINGS_H
