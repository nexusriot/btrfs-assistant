#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QSet>

// Stores the results from runCmd
struct Result {
    int exitCode;
    QString output;
};

class System : public QObject {
    Q_OBJECT
  public:
    /** @brief Enables or disables a service
     *
     * Enables or disables the service specified by name in @p serviceName. If @p enable is true,
     * the service is enabled, otherwise it is disabled
     *
     * Returns a bool which is true on success
     */
    static bool enableService(QString serviceName, bool enable);

    /** @brief Returns a list of the enabled systemd units on the host system
     *
     */
    static QStringList findEnabledUnits();

    /** @brief Runs a command on the host system
     *
     * Runs the command specified in @p cmd on the host system.  If @p includeStderr is true,
     * the output from stderr is included in the Result.output.  @p timeout specifies the timeout
     * in seconds.
     *
     * The function returns a Result containing the exist code and the output from the runing the command
     *
     */
    static const Result runCmd(const QString &cmd, bool includeStderr, int timeout = 60);

    /** @brief Runs a list of commands on the host system
     *
     * A convenience overload which takes a list of commands in @p cmdList.  The commands are combined
     * and run.  Everything else is identical to the simpler form of runCmd above.
     *
     */

    static const Result runCmd(const QStringList &cmdList, bool includeStderr, int timeout = 60);

  private:
    // This class contains only static functions.  There is no reason to instantiate it.
    explicit System(QObject *parent = nullptr);
};

#endif // SYSTEM_H
