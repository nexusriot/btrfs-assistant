#include "BtrfsStatus.h"

#include <QList>
#include <QRegularExpression>
#include <QStringList>

namespace {
/**
 * @brief Localizes a value emitted by btrfs scrub status.
 * @param value The original field value.
 * @return The localized value when known, otherwise the original value.
 */
QString localizeScrubValue(const QString &value)
{
    if (value == "no stats available") {
        return BtrfsStatus::tr("no stats available");
    }
    if (value == "no errors found") {
        return BtrfsStatus::tr("no errors found");
    }
    if (value == "running") {
        return BtrfsStatus::tr("running");
    }
    if (value == "finished") {
        return BtrfsStatus::tr("finished");
    }
    if (value == "canceled") {
        return BtrfsStatus::tr("canceled");
    }
    if (value == "aborted") {
        return BtrfsStatus::tr("aborted");
    }
    if (value == "interrupted") {
        return BtrfsStatus::tr("interrupted");
    }
    return value;
}

/**
 * @brief Localizes one line emitted by btrfs scrub status.
 * @param line The original output line.
 * @return The localized output line.
 */
QString localizeScrubLine(const QString &line)
{
    qsizetype textStart = 0;
    while (textStart < line.size() && line.at(textStart).isSpace()) {
        ++textStart;
    }

    const QString indentation = line.left(textStart);
    const QString text = line.mid(textStart);

    if (text == "no stats available") {
        return indentation + BtrfsStatus::tr("no stats available");
    }

    struct FieldTranslation {
        const char *source;
        QString translation;
    };

    const QList<FieldTranslation> fields = {
        {"UUID:", BtrfsStatus::tr("UUID:")},
        {"Scrub started:", BtrfsStatus::tr("Scrub started:")},
        {"Scrub resumed:", BtrfsStatus::tr("Scrub resumed:")},
        {"Status:", BtrfsStatus::tr("Status:")},
        {"Duration:", BtrfsStatus::tr("Duration:")},
        {"Time left:", BtrfsStatus::tr("Time left:")},
        {"ETA:", BtrfsStatus::tr("ETA:")},
        {"Total to scrub:", BtrfsStatus::tr("Total to scrub:")},
        {"Bytes scrubbed:", BtrfsStatus::tr("Bytes scrubbed:")},
        {"Rate:", BtrfsStatus::tr("Rate:")},
        {"Error summary:", BtrfsStatus::tr("Error summary:")},
        {"read_errors:", BtrfsStatus::tr("read_errors:")},
        {"csum_errors:", BtrfsStatus::tr("csum_errors:")},
        {"verify_errors:", BtrfsStatus::tr("verify_errors:")},
        {"super_errors:", BtrfsStatus::tr("super_errors:")},
        {"malloc_errors:", BtrfsStatus::tr("malloc_errors:")},
        {"uncorrectable_errors:", BtrfsStatus::tr("uncorrectable_errors:")},
        {"unverified_errors:", BtrfsStatus::tr("unverified_errors:")},
    };

    for (const FieldTranslation &field : fields) {
        const QString source = QString::fromLatin1(field.source);
        if (text.startsWith(source)) {
            const QString value = localizeScrubValue(text.mid(source.size()).trimmed());
            return indentation + field.translation + value;
        }
    }

    return line;
}

/**
 * @brief Localizes one line emitted by btrfs balance status.
 * @param line The original output line.
 * @return The localized output line.
 */
QString localizeBalanceLine(const QString &line)
{
    static const QRegularExpression noBalancePattern(QStringLiteral("^No balance found on '(.+)'$"));
    static const QRegularExpression runningPattern(QStringLiteral("^Balance on '(.+)' is running$"));
    static const QRegularExpression pausedPattern(QStringLiteral("^Balance on '(.+)' is paused$"));
    static const QRegularExpression donePattern(QStringLiteral("^Done, had to relocate ([0-9]+) out of ([0-9]+) chunks$"));
    static const QRegularExpression progressPattern(
        QStringLiteral("^([0-9]+) out of about ([0-9]+) chunks balanced \\(([0-9]+) considered\\), ([0-9]+(?:\\.[0-9]+)?)% left$"));

    QRegularExpressionMatch match = noBalancePattern.match(line);
    if (match.hasMatch()) {
        return BtrfsStatus::tr("No balance operation found on %1").arg(match.captured(1));
    }

    match = runningPattern.match(line);
    if (match.hasMatch()) {
        return BtrfsStatus::tr("Balance operation on %1 is running").arg(match.captured(1));
    }

    match = pausedPattern.match(line);
    if (match.hasMatch()) {
        return BtrfsStatus::tr("Balance operation on %1 is paused").arg(match.captured(1));
    }

    match = donePattern.match(line);
    if (match.hasMatch()) {
        return BtrfsStatus::tr("Done, relocated %1 of %2 chunks").arg(match.captured(1), match.captured(2));
    }

    match = progressPattern.match(line);
    if (match.hasMatch()) {
        return BtrfsStatus::tr("%1 of about %2 chunks balanced (%3 considered), %4% left")
            .arg(match.captured(1), match.captured(2), match.captured(3), match.captured(4));
    }

    return line;
}

/**
 * @brief Applies a line-localization function while preserving line endings.
 * @param output The complete original command output.
 * @param localizeLine The function used to localize each line.
 * @return The localized command output.
 */
QString localizeLines(const QString &output, QString (*localizeLine)(const QString &))
{
    QStringList lines = output.split('\n', Qt::KeepEmptyParts);
    for (QString &line : lines) {
        line = localizeLine(line);
    }
    return lines.join('\n');
}
} // namespace

QString BtrfsStatus::localizeBalance(const QString &output) { return localizeLines(output, localizeBalanceLine); }

QString BtrfsStatus::localizeScrub(const QString &output) { return localizeLines(output, localizeScrubLine); }
