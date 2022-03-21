#ifndef BTRFSASSISTANT_H
#define BTRFSASSISTANT_H

#include <QDir>
#include <QFile>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QProcess>
#include <QSet>
#include <QSettings>
#include <QSignalMapper>
#include <QThread>
#include <QTime>
#include <QTranslator>
#include <QUuid>
#include <QXmlStreamReader>

#include "Btrfs.h"
#include "Snapper.h"
#include "BtrfsMaintenance.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class BtrfsAssistant;
}
QT_END_NAMESPACE

class BtrfsAssistant : public QMainWindow {
    Q_OBJECT

  protected:
    QHash<QString, QCheckBox *> configCheckBoxes;

    QStringList bmFreqValues = {"none", "daily", "weekly", "monthly"};

    QSet<QCheckBox *> changedCheckBoxes;
    bool hasSnapper = false;
    bool hasBtrfsmaintenance = false;
    QSettings *settings;
    Btrfs *m_btrfs;
    BtrfsMaintenance *m_btrfsMaint;
    Snapper *m_snapper;

    void refreshInterface();
    void setupConfigBoxes();
    void apply();
    void populateBtrfsUi(const QString &uuid);
    void populateSubvolList(const QString &uuid);
    void reloadSubvolList(const QString &uuid);
    void loadSnapperUI();
    void populateSnapperGrid();
    void populateSnapperConfigSettings();
    void restoreSnapshot(const QString &uuid, const QString &subvolume);
    void switchToSnapperRestore();
    QMap<QString, QString> getSnapshotBoot();
    void enableRestoreMode(bool enable);
    void loadSnapperRestoreMode();
    void snapperTimelineEnable(bool enable);
    void populateBmTab();
    void updateServices(QList<QCheckBox *>);

    void refreshBtrfsUi();
public:
    explicit BtrfsAssistant(QWidget *parent = 0);
    ~BtrfsAssistant();

    QString getVersion(QString name);

    QString version;
    QString output;

    bool setup();

  private slots:
    void on_checkBox_bmBalance_clicked(bool checked);
    void on_checkBox_bmDefrag_clicked(bool checked);
    void on_checkBox_bmScrub_clicked(bool checked);
    void on_checkBox_includesnapshots_clicked();
    void on_checkBox_snapper_enabletimeline_clicked(bool checked);
    void on_checkBox_snapper_restore_clicked(bool checked);
    void on_comboBox_btrfsdevice_activated(int);
    void on_comboBox_snapper_configs_activated(int);
    void on_comboBox_snapper_config_settings_activated(int);
    void on_pushButton_bmApply_clicked();
    void on_pushButton_deletesubvol_clicked();
    void on_pushButton_load_clicked();
    void on_pushButton_loadsubvol_clicked();
    void on_pushButton_restore_snapshot_clicked();
    void on_pushButton_snapper_create_clicked();
    void on_pushButton_snapper_delete_clicked();
    void on_pushButton_snapper_delete_config_clicked();
    void on_pushButton_snapper_new_config_clicked();
    void on_pushButton_snapper_save_config_clicked();
    void on_pushButton_SnapperUnitsApply_clicked();

  private:
    Ui::BtrfsAssistant *ui;
};
#endif // BTRFSASSISTANT_H
