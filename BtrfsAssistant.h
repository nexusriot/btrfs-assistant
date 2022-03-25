#ifndef BTRFSASSISTANT_H
#define BTRFSASSISTANT_H

#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QSignalMapper>
#include <QTranslator>

#include "Btrfs.h"
#include "BtrfsMaintenance.h"
#include "Snapper.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class BtrfsAssistant;
}
QT_END_NAMESPACE

class BtrfsAssistant : public QMainWindow {
    Q_OBJECT

  public:
    explicit BtrfsAssistant(BtrfsMaintenance *btrfsMaintenance, Btrfs *btrfs, Snapper *snapper, QWidget *parent = 0);
    ~BtrfsAssistant();

  private:
    const QStringList m_bmFreqValues = {"none", "daily", "weekly", "monthly"};
    Btrfs *m_btrfs;
    BtrfsMaintenance *m_btrfsMaint;
    QSet<QCheckBox *> m_changedCheckBoxes;
    QHash<QString, QCheckBox *> m_configCheckBoxes;
    bool m_hasSnapper = false;
    bool m_hasBtrfsmaintenance = false;
    Snapper *m_snapper;
    Ui::BtrfsAssistant *m_ui;

    void enableRestoreMode(bool enable);
    void loadSnapperRestoreMode();
    void loadSnapperUI();
    void populateBmTab();
    void populateBtrfsUi(const QString &uuid);
    void populateSnapperGrid();
    void populateSnapperConfigSettings();
    void refreshSubvolListUi(const QString &uuid);
    void refreshBtrfsUi();
    void refreshSnapperServices();
    void restoreSnapshot(const QString &uuid, const QString &subvolume);
    bool setup();
    void setupConfigBoxes();
    void snapperTimelineEnable(bool enable);
    void updateServices(QList<QCheckBox *>);

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
};
#endif // BTRFSASSISTANT_H
