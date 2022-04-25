#ifndef CLI_H
#define CLI_H

#include "util/System.h"
#include "util/Snapper.h"
#include "util/Btrfs.h"

#include <QObject>
#include <QTextStream>

/**
 * @brief The Cli class that contains custom application logic used to invoke the various btrfs and snapper service classes functionality from the command line.
 */
class Cli : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief listSnapshots lists all the snapshots found.
     * @param snapper
     * @return
     */
    static int listSnapshots(Snapper *snapper);
    static int restore(Btrfs *btrfs, Snapper *snapper, const QString &restoreTarget);

private:
    explicit Cli(QObject *parent = nullptr);

signals:

};

#endif // CLI_H
