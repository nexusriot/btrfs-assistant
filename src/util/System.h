#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <chrono>

// Stores the results from runCmd
struct Result {
    int exitCode = -1;
    QString output;
};

/**
 * @brief The System class used to interact directly with the running operating system.
 */
class System {
  public:
    using milliseconds = std::chrono::milliseconds;
    using seconds = std::chrono::seconds;
    using minutes = std::chrono::minutes;

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
     *  @brief Finds the UUID of the filesystem path specified
     *  @param path The absolute path to find the UUID for
     *  @return Returns a QString containing the UUID or an empty string if not found
     *
     */
    static QString findUuid(const QString path) { return runCmd("findmnt", {"-no", "uuid", path}, false).output; }

    /**
     * @brief Checks if the system is running systemd
     * @return true if the system is running systemd, false otherwise
     */
    static bool hasSystemd();

    /**
     * @brief Checks if any of the lines in /etc/fstab contain a subvolid which could impact restores
     * @return true is there is a line with subvolid, false otherwise
     */
    static bool isSubvolidInFstab();

    /**
     * @brief Returns an absolute path to be used for generating the location to mount a btrfs filesystem
     */
    static QString mountPathRoot() { return QStringLiteral("/run/BtrfsAssistant"); }

    /**
     * @brief An overloaded version of runCmd which takes a string and runs it with bash -c
     * @param cmd - The command to pass to bash -c
     * @param includeStderr - When true stderr is included in the Result.output
     * @param timeout - How long (in milliseconds resolution) the command should run before timing out
     * @return A Result containing the exit code and the output from the running the command
     */
    static Result runCmd(const QString &cmd, bool includeStderr, milliseconds timeout = minutes(1));

    /**
     * @brief Runs a command on the host system
     * @param cmd - The absolute path to the binary/script to run
     * @param args - A list of arguments for @p cmd
     * @param includeStderr - When true stderr is included in the Result.output
     * @param timeout - How long (in milliseconds resolution) the command should run before timing out
     * @return A Result containing the exit code and the output from the running the command
     */
    static Result runCmd(const QString &cmd, const QStringList &args, bool includeStderr, milliseconds timeout = minutes(1));

    /** @brief Starts the systemd unit with the unit name of @p unit
     *
     *  Returns a Result struct from runCmd()
     *
     */
    static Result startUnit(const QString &unit) { return runCmd("systemctl", {"start", unit}, false); }

    /**
     * @brief Converts a number to a human readable string for displaying data storage amounts
     * @param number - A uint64_t containing the number to convert
     * @return A string containing the converted value
     */
    static QString toHumanReadable(uint64_t number);

  private:
    // This class contains only static functions.  There is no reason to instantiate it.
    System() = delete;
};

#endif // SYSTEM_H
