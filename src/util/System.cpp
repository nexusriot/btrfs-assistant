#include "System.h"

#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <unistd.h>

bool System::checkRootUid() { return geteuid() == 0; }

bool System::enableService(QString serviceName, bool enable)
{
    int exitCode;

    if (enable) {
        exitCode = System::runCmd("systemctl", {"enable", "--now", serviceName}, false).exitCode;
    } else {
        exitCode = System::runCmd("systemctl", {"disable", "--now", serviceName}, false).exitCode;
    }

    return exitCode == 0;
}

QStringList System::findEnabledUnits()
{

    const QString bashOutput = System::runCmd("systemctl list-unit-files --state=enabled -q --no-pager", false).output;
    const QStringList outputList = bashOutput.split('\n');
    QStringList serviceList;
    for (const QString &line : outputList) {
        serviceList.append(line.split(QRegExp("[\t ]+")).at(0));
    }

    return serviceList;
}

bool System::hasSystemd()
{
    // /proc/1/comm contains the command use to run the init system
    QFile file(QStringLiteral("/proc/1/comm"));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (file.readAll().trimmed() != "systemd") {
        return false;
    }

    return true;
}

bool System::isSubvolidInFstab()
{
    QFile file(QStringLiteral("/etc/fstab"));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QTextStream fstabStream(&file);
    QString line;
    while (fstabStream.readLineInto(&line)) {
        if (!line.startsWith("#") && line.contains("subvolid")) {
            return true;
        }
    };

    return false;
}

Result System::runCmd(const QString &cmd, bool includeStderr, milliseconds timeout)
{
    return runCmd("/bin/bash", QStringList() << "-c" << cmd, includeStderr, timeout);
}

Result System::runCmd(const QString &cmd, const QStringList &args, bool includeStderr, milliseconds timeout)
{
    QProcess proc;

    if (includeStderr)
        proc.setProcessChannelMode(QProcess::MergedChannels);

    proc.start(cmd, args);

    proc.waitForFinished(static_cast<int>(timeout.count()));
    return {proc.exitCode(), proc.readAllStandardOutput().trimmed()};
}

QString System::toHumanReadable(const uint64_t number)
{
    auto result = static_cast<double>(number);

    int i = 0;
    const QVector<QString> units = {"B", "kiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    while (result > 1024) {
        result /= 1024;
        i++;
    }
    return QString::number(result, 'f', 2) + " " + units[i];
}
