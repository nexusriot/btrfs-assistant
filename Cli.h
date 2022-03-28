#ifndef CLI_H
#define CLI_H

#include "System.h"
#include "Snapper.h"
#include "Btrfs.h"

#include <QObject>
#include <QTextStream>

class Cli : public QObject
{
    Q_OBJECT
public:
    static int listSnapshots(Snapper *snapper);
    static int restore(Btrfs *btrfs, Snapper *snapper, const QString &restoreTarget);

private:
    explicit Cli(QObject *parent = nullptr);

signals:

};

#endif // CLI_H
