#ifndef ADDDEFECTDIALOG_H
#define ADDDEFECTDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include "src/repositories/AircraftRepository.h"
#include "src/repositories/DefectRepository.h"
#include "src/services/FleetService.h"

class AddDefectDialog : public QDialog {
    Q_OBJECT

public:
    // Добавляем параметр aircraftId
    explicit AddDefectDialog(QWidget *parent = nullptr, QUuid aircraftId = QUuid());
    ~AddDefectDialog();

private slots:
    void onSaveClicked();
    void updateSeverityLabel();

private:
    QUuid m_preSelectedAircraftId; // ID самолета, если он был передан

    QComboBox *m_aircraftCombo;
    QComboBox *m_defectTypeCombo;
    QLabel *m_severityLabel;
    QPushButton *m_btnSave;
    QPushButton *m_btnCancel;

    AircraftRepository m_aircraftRepo;
    DefectRepository m_defectRepo;
    FleetService m_fleetService;

    void setupUi();
    void loadData();
};

#endif // ADDDEFECTDIALOG_H
