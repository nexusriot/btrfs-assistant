#ifndef BTRFSMAINTENANCE_H
#define BTRFSMAINTENANCE_H

#include <QIODevice>
#include <QObject>
#include <QSettings>

#include "Settings.h"
#include "System.h"

/**
 * @brief The BtrfsMaintenance class that handles interfacing with the btrfs maintenance configuration.
 */
class BtrfsMaintenance : public QObject {
    Q_OBJECT
  public:
    explicit BtrfsMaintenance(const QString &configFile, QObject *parent = nullptr);

    /** @brief Forces Btrfs Maintenance to reload the configuration file
     */
    void refresh();

    /** @brief Sets @p value for @p key in the settings file
     */
    void setValue(const QString &key, const QVariant &value) { m_settings->setValue(key, value); }

    /** @brief Returns the value associated with @p key from the settings file as a QString
     */
    const QString value(const QString &key) { return m_settings->value(key).toString(); }

  private:
    QSettings *m_settings;

  signals:
};

#endif // BTRFSMAINTENANCE_H
