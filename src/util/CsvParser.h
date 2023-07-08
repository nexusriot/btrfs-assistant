#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <QString>
#include <QStringList>

/**
 * @brief Parses a CSV line, while handling fields containing commas and quotes
 * @param line - The CSV line to parse
 * @return A QStringList containing the parsed line (fields are trimmed)
 */
QStringList parseCsvLine(const QString &line);

/**
 * @brief A function that documents the test cases that where thought of when writing the CSV parser
 * @return 0 if all tests passed, 1 if a test failed
 */
int testCsvParser();

#endif // CSV_PARSER_H
