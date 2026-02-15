#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include "src/repositories/AircraftRepository.h"
#include "src/repositories/DefectRepository.h"
#include "src/repositories/PilotRepository.h"
#include "src/services/FleetService.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectBtnClicked();
    void onRefreshBtnClicked();
    void onPrepareBtnClicked();
    void onSeedBtnClicked();

    // Слоты управления
    void onAddAircraftClicked();
    void onAddPilotClicked();
    void onAddDefectClicked();
    void onMaintenanceClicked();
    // Слоты очистки
    void onClearDbClicked();
    void onDeleteAircraftClicked();
    void onDeletePilotClicked();

private:
    QTableWidget *m_table;
    QPushButton *m_btnConnect;
    QPushButton *m_btnRefresh;
    QPushButton *m_btnPrepare;
    QPushButton *m_btnSeed;
    QPushButton *m_btnMaintenance; // Новая кнопка на панели

    QLabel *m_statusLabel;

    AircraftRepository m_aircraftRepo;
    DefectRepository m_defectRepo;
    FleetService m_fleetService;
    PilotRepository m_pilotRepo;

    void setupUi();
    void createMenus();
    void loadAircrafts();

    QColor calculateStatusColor(const Aircraft& plane);
    QString calculateStatusText(const Aircraft& plane);
};

#endif // MAINWINDOW_H
