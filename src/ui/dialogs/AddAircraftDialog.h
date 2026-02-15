#ifndef ADDAIRCRAFTDIALOG_H
#define ADDAIRCRAFTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "src/repositories/AircraftModelRepository.h"
#include "src/services/FleetService.h"

class AddAircraftDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddAircraftDialog(QWidget *parent = nullptr);
    ~AddAircraftDialog();

    QUuid getCreatedAircraftId() const { return m_createdId; }

private slots:
    void onSaveClicked();

private:
    // UI элементы
    QLineEdit *m_regNumberEdit;
    QComboBox *m_modelCombo;
    QDoubleSpinBox *m_totalHoursSpin;
    QDoubleSpinBox *m_serviceIntervalSpin; // "Часы следующего ТО"
    QPushButton *m_btnSave;
    QPushButton *m_btnCancel;

    // ID созданного объекта
    QUuid m_createdId;

    // Сервисы
    AircraftModelRepository m_modelRepo;
    FleetService m_fleetService;

    void setupUi();
    void loadModels(); // Загрузка типов ВС в выпадающий список
};

#endif // ADDAIRCRAFTDIALOG_H
