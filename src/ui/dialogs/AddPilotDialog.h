#ifndef ADDPILOTDIALOG_H
#define ADDPILOTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QListWidget>
#include <QPushButton>
#include "src/repositories/AircraftModelRepository.h"
#include "src/services/FleetService.h"

class AddPilotDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddPilotDialog(QWidget *parent = nullptr);
    ~AddPilotDialog();

private slots:
    void onSaveClicked();

private:
    // UI элементы
    QLineEdit *m_nameEdit;
    QDateEdit *m_licenseDate;
    QDateEdit *m_medicalDate;
    QListWidget *m_modelsList; // Список для множественного выбора допусков
    QPushButton *m_btnSave;
    QPushButton *m_btnCancel;

    // Сервисы
    AircraftModelRepository m_modelRepo;
    FleetService m_fleetService;

    void setupUi();
    void loadModels(); // Загрузка типов ВС для чек-листа
};

#endif // ADDPILOTDIALOG_H
