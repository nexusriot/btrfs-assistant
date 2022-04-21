#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>

// Stores the results from runCmd
struct Result {
    int exitCode;
    QString output;
};

/**
 * @brief The System class used to interact directly with the running operating system.
 */
class System {
  public:
    /**
     * @brief Checks to see if the user running the application is root
     * @return True is the user has a UID of 0, false otherwise
     */
    static bool checkRootUid();

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
     * @brief Checks if the system is running systemd
     * @return true if the system is running systemd, false otherwise
     */
    static bool hasSystemd();

    /**
     * @brief An overloaded version of runCmd which takes a string and runs it with bash -c
     * @param cmd - The command to pass to bash -c
     * @param includeStderr - When true stderr is included in the Result.output
     * @param timeout - How long in seconds the command should run before timing out
     * @return A Result containing the exit code and the output from the running the command
     */
    static Result runCmd(const QString &cmd, bool includeStderr, int timeout = 60);

    /**
     * @brief Runs a command on the host system
     * @param cmd - The absolute path to the binary/script to run
     * @param args - A list of arguments for @p cmd
     * @param includeStderr - When true stderr is included in the Result.output
     * @param timeout - How long in seconds the command should run before timing out
     * @return A Result containing the exit code and the output from the running the command
     */
    static Result runCmd(const QString &cmd, const QStringList &args, bool includeStderr, int timeout = 60);

    /** @brief Starts the systemd unit with the unit name of @p unit
     *
     *  Returns a Result struct from runCmd()
     *
     */
    static Result startUnit(const QString &unit) { return runCmd("systemctl", {"start", unit}, false); }

    /**
     * @brief Converts a number to a human readable string for displaying data storage amounts
     * @param number - A double containing the number to convert
     * @return A string containing the converted value
     */
    static QString toHumanReadable(double number);

  private:
    // This class contains only static functions.  There is no reason to instantiate it.
    System() = delete;
};

#endif // SYSTEM_H
