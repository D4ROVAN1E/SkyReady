#ifndef FLIGHTPREPARATIONDIALOG_H
#define FLIGHTPREPARATIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

#include "src/services/ReadinessService.h"
#include "src/repositories/PilotRepository.h"
#include "src/services/FleetService.h"

class FlightPreparationDialog : public QDialog {
    Q_OBJECT

public:
    explicit FlightPreparationDialog(QUuid aircraftId, QWidget *parent = nullptr);
    ~FlightPreparationDialog();

private slots:
    void onCheckReadiness(); // Кнопка "Проверить" / Автопроверка
    void onCommitFlight();   // Кнопка "Выпустить в рейс"

private:
    QUuid m_aircraftId;

    // Сервисы
    ReadinessService m_readinessService;
    PilotRepository m_pilotRepo;
    FleetService m_fleetService;

    // UI Элементы
    QLabel *m_lblAircraftInfo;

    QComboBox *m_pilotCombo;
    QDoubleSpinBox *m_fuelSpin;     // Топливо (литры)
    QDoubleSpinBox *m_cargoSpin;    // Вес груза/пасс (кг)
    QSpinBox *m_timeSpin;           // Время полета (мин)

    // Блок результата
    QGroupBox *m_resultGroup;
    QLabel *m_resultLabel;
    QTextEdit *m_detailsText;
    QPushButton *m_btnCommit;
    QPushButton *m_btnClose;

    void setupUi();
    void loadData();
};

#endif // FLIGHTPREPARATIONDIALOG_H
