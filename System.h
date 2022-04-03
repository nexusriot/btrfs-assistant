#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QSet>

// Stores the results from runCmd
struct Result {
    int exitCode;
    QString output;
};

/**
 * @brief The System class used to interact directly with the running operating system.
 */
class System : public QObject {
    Q_OBJECT
  public:

    /**
     * @brief Checks to see if the user running the application is root
     * @return True is the user has a UID of 0, false otherwise
     */
    static bool checkRootUid() { return System::runCmd("id -u", false).output == "0"; }

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

    /**
     * @brief An overloaded version of runCmd which takes a string and runs it with bash -c
     * @param cmd - The command to pass to bash -c
     * @param includeStderr - When true stderr is included in the Result.output
     * @param timeout - How long in seconds the command should run before timing out
     * @return A Result containing the exit code and the output from the running the command
     */
    static const Result runCmd(const QString &cmd, bool includeStderr, int timeout = 60);

    /**
     * @brief Runs a command on the host system
     * @param cmd - The absolute path to the binary/script to run
     * @param args - A list of arguments for @p cmd
     * @param includeStderr - When true stderr is included in the Result.output
     * @param timeout - How long in seconds the command should run before timing out
     * @return A Result containing the exit code and the output from the running the command
     */
    static const Result runCmd(const QString &cmd, const QStringList &args, bool includeStderr, int timeout = 60);

    /** @brief Starts the systemd unit with the unit name of @p unit
     *
     *  Returns a Result struct from runCmd()
     *
     */
    static const Result startUnit(const QString &unit) { return runCmd("systemctl", {"start", unit}, false); }

  private:
    // This class contains only static functions.  There is no reason to instantiate it.
    explicit System(QObject *parent = nullptr);
};

#endif // SYSTEM_H
