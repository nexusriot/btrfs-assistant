#pragma once

#include <QCoreApplication>
#include <QString>

class BtrfsStatus {
  public:
    Q_DECLARE_TR_FUNCTIONS(BtrfsStatus)

  public:
    /**
     * @brief Localizes the human-readable output of btrfs balance status.
     * @param output The original command output.
     * @return The localized status text.
     */
    static QString localizeBalance(const QString &output);

    /**
     * @brief Localizes the human-readable output of btrfs scrub status.
     * @param output The original command output.
     * @return The localized status text.
     */
    static QString localizeScrub(const QString &output);
};
