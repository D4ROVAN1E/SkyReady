#ifndef MAINTENANCEDIALOG_H
#define MAINTENANCEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "src/repositories/DefectRepository.h"
#include "src/services/FleetService.h"
#include "src/models/Entities.h"

class MaintenanceDialog : public QDialog {
    Q_OBJECT

public:
    explicit MaintenanceDialog(QUuid aircraftId, const QString& regNumber, QWidget *parent = nullptr);
    ~MaintenanceDialog();

private slots:
    void onAddDefectClicked();
    void onResolveDefectClicked();
    void onEngineServiceClicked();

private:
    QUuid m_aircraftId;
    QString m_regNumber;

    QListWidget *m_defectsList;
    QPushButton *m_btnAdd;
    QPushButton *m_btnResolve;
    QPushButton *m_btnClose;
    QPushButton *m_btnEngineService;

    DefectRepository m_defectRepo;
    FleetService m_fleetService;

    void setupUi();
    void loadDefects();
};

#endif // MAINTENANCEDIALOG_H
