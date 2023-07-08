#include "CsvParser.h"
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <iostream>

QStringList parseCsvLine(const QString &line)
{
    QStringList fields;

    bool insideQuotes = false;
    QString field;
    QTextStream stream(&field);

    for (int i = 0; i < line.size(); ++i) {
        const QChar &c = line.at(i);

        if (c == '"') {
            if (insideQuotes && (i + 1 < line.size() && line.at(i + 1) == '"')) {
                stream << '"';
                ++i;
            } else {
                insideQuotes = !insideQuotes;
            }
        } else if (c == ',' && !insideQuotes) {
            fields.append(field.trimmed());
            field.clear();
            stream.setString(&field);
        } else {
            stream << c;
        }
    }

    fields.append(field.trimmed());

    return fields;
}

int testCsvParser()
{
    // A list of rows and the expected result fields
    QMap<QString, QStringList> TestCases;

    TestCases.insert(R"(...,simple description,...)", {"...", "simple description", "..."});
    // The following test cases are only testing one field per row
    TestCases.insert(R"("just,some,commas,in,this,field")", {R"(just,some,commas,in,this,field)"});
    TestCases.insert(R"("just one single quote in the colum "" like that")", {R"(just one single quote in the colum " like that)"});
    TestCases.insert(R"("with,and""a single double-quote its harder")", {R"(with,and"a single double-quote its harder)"});
    TestCases.insert(R"("""quoted"" in the start and ""end""")", {R"("quoted" in the start and "end")"});
    TestCases.insert(R"("""even ""harder"" ""quoted,"" is it like that")", {R"("even "harder" "quoted," is it like that)"});
    TestCases.insert(R"("""even ""harder"" quoted"", is it like ""that,""")", {R"("even "harder" quoted", is it like "that,")"});

    for (auto it = TestCases.begin(); it != TestCases.end(); ++it) {
        const QString &line = it.key();
        const QStringList &expected = it.value();
        const QStringList &result = parseCsvLine(line);

        if (result != expected) {
            std::cout << "Test failed: " << line.toStdString() << std::endl;
            std::cout << "Expected: " << expected.join(", ").toStdString() << std::endl;
            std::cout << "Got: " << result.join(", ").toStdString() << std::endl;
            return 1;
        }
        std::cout << "Test passed: " << line.toStdString() << std::endl;
    }

    return 0;
}
