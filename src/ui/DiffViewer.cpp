#include "ui/DiffViewer.h"
#include "util/System.h"

#include <QDialog>
#include <QDir>
#include <QMessageBox>

DiffViewer::DiffViewer(Snapper *snapper, const QString &rootPath, const QString &filePath, const QString &uuid, QWidget *parent)
    : QDialog(parent), m_ui(new Ui::DiffViewer), m_snapper(snapper), m_uuid(uuid)
{

    m_ui->setupUi(this);

    this->setWindowTitle(tr("Diff Viewer"));

    m_twSnapshot = m_ui->tableWidget_snapshotList;

    // We will need the target path for both diffs and restores
    m_targetPath = m_snapper->findTargetPath(rootPath, filePath, uuid);

    // We need to find all the snapshots with the file in them and populate the UI
    LoadSnapshots(rootPath, filePath);
}

DiffViewer::~DiffViewer() { delete m_ui; }

void DiffViewer::on_pushButton_restore_clicked()
{
    if (QMessageBox::question(0, tr("Confirm"), tr("Are you sure you want to restore this the file over the current file?")) !=
        QMessageBox::Yes) {
        return;
    }

    // Restore the file
    const QString filePath = m_twSnapshot->item(m_twSnapshot->currentRow(), DiffColumn::filePath)->text();

    if (m_targetPath.isEmpty() || filePath.isEmpty() || !m_snapper->restoreFile(filePath, m_targetPath)) {
        QMessageBox::warning(this, tr("Restore Failed"), tr("The file failed to restore"));
        return;
    }

    QMessageBox::information(this, tr("Restore File"), tr("The file was succesfully restored"));
}

#include <QDebug>
void DiffViewer::LoadSnapshots(const QString &rootPath, const QString &filePath)
{
    QDir snapshotDir(rootPath);

    QString relPath = snapshotDir.relativeFilePath(filePath);

    static QRegularExpression re("\\/[0-9]*\\/snapshot$");
    const QStringList subvolSplit = rootPath.split(re);
    const QDir stemPath(subvolSplit.at(0));
    const QString wildcardPath = QDir::cleanPath(stemPath.canonicalPath() + "/*/snapshot/" + relPath).replace(" ", "\\ ");

    const QStringList resultList = System::runCmd("ls " + wildcardPath, false).output.split("\n");

    // Clear the table and set the headers
    m_twSnapshot->clear();
    m_twSnapshot->setColumnCount(4);
    m_twSnapshot->setHorizontalHeaderItem(DiffColumn::num, new QTableWidgetItem(tr("Num", "The number associated with a snapshot")));
    m_twSnapshot->setHorizontalHeaderItem(DiffColumn::dateTime, new QTableWidgetItem(tr("Date/Time")));
    m_twSnapshot->setHorizontalHeaderItem(DiffColumn::rootPath, new QTableWidgetItem(tr("Root Path")));
    m_twSnapshot->setHorizontalHeaderItem(DiffColumn::filePath, new QTableWidgetItem(tr("File Path")));
    m_twSnapshot->setColumnHidden(DiffColumn::rootPath, true);
    m_twSnapshot->setColumnHidden(DiffColumn::filePath, true);
    m_twSnapshot->setRowCount(resultList.count());

    // We need to the locale for displaying the date/time
    QLocale locale = QLocale::system();

    int row = 0;

    for (const QString &result : resultList) {
        const QString endPath = stemPath.relativeFilePath(result);

        // Find the snapshot number part of the path
        const QString snapshotNum = endPath.split("/").at(0);

        // Find the rootPath
        const QString thisRootPath = QDir::cleanPath(stemPath.canonicalPath() + QDir::separator() + endPath.split(relPath).at(0));

        // Get the date
        const QString metaFileName =
            QDir::cleanPath(stemPath.canonicalPath() + QDir::separator() + snapshotNum + QDir::separator() + "info.xml");
        const QString date = locale.toString(Snapper::readSnapperMeta(metaFileName).time, QLocale::ShortFormat);

        if (snapshotNum.isEmpty() || result.isEmpty() || thisRootPath.isEmpty()) {
            continue;
        }

        // Populate the row in the table
        QTableWidgetItem *number = new QTableWidgetItem(snapshotNum.toInt());
        number->setData(Qt::DisplayRole, snapshotNum.toInt());
        m_twSnapshot->setItem(row, DiffColumn::num, number);
        m_twSnapshot->setItem(row, DiffColumn::dateTime, new QTableWidgetItem(date));
        m_twSnapshot->setItem(row, DiffColumn::rootPath, new QTableWidgetItem(thisRootPath));
        m_twSnapshot->setItem(row, DiffColumn::filePath, new QTableWidgetItem(result.trimmed()));

        if (result.trimmed() == filePath) {
            m_twSnapshot->selectRow(row);
        }

        row++;

        qDebug() << relPath << subvolSplit.at(0) << Qt::endl;
    }
    m_twSnapshot->resizeColumnsToContents();
    m_twSnapshot->sortItems(DiffColumn::num, Qt::DescendingOrder);
}

void DiffViewer::on_tableWidget_snapshotList_itemSelectionChanged()
{
    const QString filePath = m_twSnapshot->item(m_twSnapshot->currentRow(), DiffColumn::filePath)->text();
    const QStringList diffOutput = System::runCmd("diff", {"-u", m_targetPath, filePath}, false).output.split("\n");

    if (diffOutput.isEmpty() || diffOutput.at(0).isEmpty()) {
        m_ui->textEdit_diff->setText(tr("There are no differences between the selected files"));
        return;
    }

    // Get a copy of the palette before we start changing colors
    QPalette defaultPalette = m_ui->textEdit_diff->palette();

    // Show the diff
    m_ui->textEdit_diff->clear();
    for (const QString &line : diffOutput) {
        // Colorize the output for lines added or removed
        if (line.startsWith("+")) {
            m_ui->textEdit_diff->setTextColor(Qt::darkGreen);
        } else if (line.startsWith("-")) {
            m_ui->textEdit_diff->setTextColor(Qt::red);
        }

        m_ui->textEdit_diff->append(line);
        m_ui->textEdit_diff->setTextColor(defaultPalette.text().color());
    }
}
