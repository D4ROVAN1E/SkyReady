#include "src/ui/MainWindow.h"
#include "src/ui/FlightPreparationDialog.h"
#include "src/ui/dialogs/AddAircraftDialog.h"
#include "src/ui/dialogs/AddPilotDialog.h"
#include "src/ui/dialogs/AddDefectDialog.h"
#include "src/ui/dialogs/MaintenanceDialog.h"
#include "src/db/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QMenuBar>
#include <QUuid>
#include <QInputDialog> // Для выбора пилота из списка

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    if (DatabaseManager::instance().getDatabase().isOpen()) {
        m_btnConnect->setEnabled(false);
        m_btnRefresh->setEnabled(true);
        m_btnPrepare->setEnabled(true);
        m_btnSeed->setEnabled(true);
        m_btnMaintenance->setEnabled(true);
        loadAircrafts();
    }
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    setWindowTitle("SkyReady - Flight Readiness System");
    resize(1000, 600);

    createMenus();

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *topPanel = new QHBoxLayout();

    m_btnConnect = new QPushButton("Подключить БД", this);
    m_btnRefresh = new QPushButton("Обновить", this);
    m_btnSeed = new QPushButton("Демо-данные", this);

    m_btnMaintenance = new QPushButton("Обслуживание", this);
    m_btnPrepare = new QPushButton("Подготовка к вылету", this);

    m_btnConnect->setStyleSheet("padding: 6px;");
    m_btnRefresh->setStyleSheet("background-color: #2196F3; color: white; padding: 6px; font-weight: bold;");
    m_btnSeed->setStyleSheet("background-color: #9C27B0; color: white; padding: 6px;");

    m_btnMaintenance->setStyleSheet("background-color: #FF9800; color: white; padding: 6px; font-weight: bold;");
    m_btnPrepare->setStyleSheet("background-color: #4CAF50; color: white; padding: 6px; font-weight: bold;");

    m_btnRefresh->setEnabled(false);
    m_btnPrepare->setEnabled(false);
    m_btnSeed->setEnabled(false);
    m_btnMaintenance->setEnabled(false);

    topPanel->addWidget(m_btnConnect);
    topPanel->addWidget(m_btnSeed);
    topPanel->addWidget(m_btnRefresh);
    topPanel->addStretch();
    topPanel->addWidget(m_btnMaintenance);
    topPanel->addWidget(m_btnPrepare);

    mainLayout->addLayout(topPanel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    QStringList headers;
    headers << "Бортовой номер" << "Тип ВС" << "Налет (ч)" << "Ресурс (ч)" << "Статус";
    m_table->setHorizontalHeaderLabels(headers);

    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(m_table);

    m_statusLabel = new QLabel("Ожидание подключения к базе данных...", this);
    mainLayout->addWidget(m_statusLabel);

    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectBtnClicked);
    connect(m_btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshBtnClicked);
    connect(m_btnPrepare, &QPushButton::clicked, this, &MainWindow::onPrepareBtnClicked);
    connect(m_btnSeed, &QPushButton::clicked, this, &MainWindow::onSeedBtnClicked);
    connect(m_btnMaintenance, &QPushButton::clicked, this, &MainWindow::onMaintenanceClicked);
}

void MainWindow::createMenus() {
    QMenuBar *bar = menuBar();
    QMenu *fleetMenu = bar->addMenu("Управление БД");

    QAction *actAddPlane = fleetMenu->addAction("Добавить самолет...");
    QAction *actAddPilot = fleetMenu->addAction("Добавить пилота...");
    fleetMenu->addSeparator();
    QAction *actAddDefect = fleetMenu->addAction("Зарегистрировать дефект...");

    fleetMenu->addSeparator();
    QAction *actDeletePlane = fleetMenu->addAction("Удаление воздушного судна");
    // Добавляем действие удаления
    QAction *actDeletePilot = fleetMenu->addAction("Удаление пилота");

    fleetMenu->addSeparator();
    QAction *actClearDb = fleetMenu->addAction("Очистить базу данных...");

    connect(actAddPlane, &QAction::triggered, this, &MainWindow::onAddAircraftClicked);
    connect(actAddPilot, &QAction::triggered, this, &MainWindow::onAddPilotClicked);
    connect(actAddDefect, &QAction::triggered, this, &MainWindow::onAddDefectClicked);
    connect(actDeletePlane, &QAction::triggered, this, &MainWindow::onDeleteAircraftClicked);
    connect(actDeletePilot, &QAction::triggered, this, &MainWindow::onDeletePilotClicked);
    connect(actClearDb, &QAction::triggered, this, &MainWindow::onClearDbClicked);
}

void MainWindow::onDeletePilotClicked() {
    if (!DatabaseManager::instance().getDatabase().isOpen()) return;

    // 1. Получаем список всех пилотов
    std::vector<Pilot> pilots = m_pilotRepo.getAll();
    if (pilots.empty()) {
        QMessageBox::information(this, "Информация", "Список пилотов пуст.");
        return;
    }

    // 2. Формируем список строк для выбора
    QStringList items;
    for (const auto& p : pilots) {
        items << QString("%1").arg(p.fullName); // Можно добавить ID или дату для точности
    }

    // 3. Показываем диалог выбора
    bool ok;
    QString item = QInputDialog::getItem(this, "Удаление пилота",
                                         "Выберите пилота для удаления:", items, 0, false, &ok);

    if (ok && !item.isEmpty()) {
        // Находим выбранного пилота
        int index = items.indexOf(item);
        if (index >= 0 && index < pilots.size()) {
            QUuid pilotId = pilots[index].id;

            QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение",
                QString("Удалить пилота '%1'?\nВся история его полетов также будет удалена.").arg(item),
                QMessageBox::Yes|QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                if (m_fleetService.deletePilot(pilotId)) {
                    QMessageBox::information(this, "Успех", "Пилот удален.");
                } else {
                    QMessageBox::critical(this, "Ошибка", "Не удалось удалить данные пилота.");
                }
            }
        }
    }
}

void MainWindow::onDeleteAircraftClicked() {
    // 1. Проверяем выбор в таблице
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Внимание", "Выберите самолет в списке, который нужно удалить.");
        return;
    }

    // 2. Получаем данные
    QString regNum = m_table->item(row, 0)->text();
    QString idStr = m_table->item(row, 0)->data(Qt::UserRole).toString();
    QUuid id(idStr);

    // 3. Подтверждение
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Удаление",
        QString("Вы уверены, что хотите удалить самолет %1?\n"
                "Все связанные данные (история полетов, дефекты) также будут безвозвратно удалены.").arg(regNum),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    // 4. Удаление через сервис
    if (m_fleetService.deleteAircraft(id)) {
        loadAircrafts(); // Обновляем таблицу
        QMessageBox::information(this, "Успех", "Самолет удален.");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось удалить самолет.");
    }
}

void MainWindow::onClearDbClicked() {
    if (!DatabaseManager::instance().getDatabase().isOpen()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Очистка базы данных",
                                  "Вы уверены, что хотите удалить все самолеты, пилотов и полеты?\n"
                                  "(Справочники моделей и типов дефектов останутся)",
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_fleetService.clearFleetData()) {
            loadAircrafts(); // Обновляем (таблица станет пустой)
            QMessageBox::information(this, "Успех", "Оперативные данные удалены.");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось очистить базу.");
        }
    }
}

void MainWindow::onAddAircraftClicked() {
    if (!DatabaseManager::instance().getDatabase().isOpen()) return;
    AddAircraftDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        loadAircrafts(); // Обновляем список, чтобы новый самолет появился

        // Сразу предлагаем подготовить рейс для нового борта
        QUuid newId = dialog.getCreatedAircraftId();

        if (!newId.isNull()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Подготовка к вылету",
                "Самолет успешно добавлен.\n"
                "Хотите сразу перейти к подготовке первого рейса (назначить пилота, загрузку)?",
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                FlightPreparationDialog prepDialog(newId, this);
                if (prepDialog.exec() == QDialog::Accepted) {
                    loadAircrafts(); // Обновляем снова, если полет состоялся
                }
            }
        }
    }
}

void MainWindow::onAddPilotClicked() {
    if (!DatabaseManager::instance().getDatabase().isOpen()) return;
    AddPilotDialog dialog(this);
    dialog.exec();
}

void MainWindow::onAddDefectClicked() {
    if (!DatabaseManager::instance().getDatabase().isOpen()) return;
    AddDefectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) loadAircrafts();
}

void MainWindow::onMaintenanceClicked() {
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Внимание", "Выберите самолет для обслуживания.");
        return;
    }

    QString idStr = m_table->item(row, 0)->data(Qt::UserRole).toString();
    QString regNum = m_table->item(row, 0)->text();
    QUuid aircraftId(idStr);

    MaintenanceDialog dialog(aircraftId, regNum, this);
    dialog.exec();

    loadAircrafts();
}

void MainWindow::onConnectBtnClicked() {
    bool success = DatabaseManager::instance().connectToDatabase();
    if (success) {
        m_statusLabel->setText("База данных подключена успешно.");
        m_btnConnect->setEnabled(false);
        m_btnRefresh->setEnabled(true);
        m_btnPrepare->setEnabled(true);
        m_btnSeed->setEnabled(true);
        m_btnMaintenance->setEnabled(true);
        loadAircrafts();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к БД.");
    }
}

void MainWindow::onRefreshBtnClicked() {
    loadAircrafts();
}

void MainWindow::onSeedBtnClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Перезапись данных",
                                  "Вы уверены? Это сотрет ВСЕ текущие данные.",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No) return;

    if (m_fleetService.seedDemoData()) {
        QMessageBox::information(this, "Успех", "Демо-данные загружены.");
        loadAircrafts();
    } else {
        QMessageBox::critical(this, "Ошибка", "Ошибка при генерации данных.");
    }
}

void MainWindow::onPrepareBtnClicked() {
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Внимание", "Выберите самолет для вылета.");
        return;
    }
    QString idStr = m_table->item(row, 0)->data(Qt::UserRole).toString();
    FlightPreparationDialog dialog(QUuid(idStr), this);
    if (dialog.exec() == QDialog::Accepted) loadAircrafts();
}

void MainWindow::loadAircrafts() {
    m_statusLabel->setText("Загрузка данных...");
    m_table->setRowCount(0);
    std::vector<Aircraft> fleet = m_aircraftRepo.getAll();
    if (fleet.empty()) {
        m_statusLabel->setText("Флот пуст.");
        return;
    }
    m_table->setRowCount(fleet.size());
    for (size_t i = 0; i < fleet.size(); ++i) {
        const Aircraft& plane = fleet[i];

        QTableWidgetItem *itemReg = new QTableWidgetItem(plane.regNumber);
        itemReg->setTextAlignment(Qt::AlignCenter);
        itemReg->setData(Qt::UserRole, plane.id.toString());
        m_table->setItem(i, 0, itemReg);

        QTableWidgetItem *itemModel = new QTableWidgetItem(plane.modelName);
        itemModel->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 1, itemModel);

        QTableWidgetItem *itemHours = new QTableWidgetItem(QString::number(plane.engineHoursTotal, 'f', 1));
        itemHours->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 2, itemHours);

        double remaining = plane.engineHoursNextService - plane.engineHoursTotal;
        QTableWidgetItem *itemRes = new QTableWidgetItem(QString::number(remaining, 'f', 1));
        if (remaining < 10) itemRes->setForeground(Qt::red);
        itemRes->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 3, itemRes);

        QString statusText = calculateStatusText(plane);
        QColor statusColor = calculateStatusColor(plane);
        QTableWidgetItem *itemStatus = new QTableWidgetItem(statusText);
        itemStatus->setBackground(statusColor);
        itemStatus->setTextAlignment(Qt::AlignCenter);
        if (statusColor == Qt::red || statusColor == Qt::green) itemStatus->setForeground(Qt::white);
        else itemStatus->setForeground(Qt::black);
        itemStatus->setFont(QFont("Arial", 9, QFont::Bold));
        m_table->setItem(i, 4, itemStatus);
    }
    m_statusLabel->setText(QString("Загружено %1 бортов.").arg(fleet.size()));
}

QColor MainWindow::calculateStatusColor(const Aircraft& plane) {
    double remaining = plane.engineHoursNextService - plane.engineHoursTotal;
    if (remaining <= 0 || m_defectRepo.hasCriticalDefects(plane.id)) return Qt::red;
    if (m_defectRepo.countMinorDefects(plane.id) >= 3) return Qt::red;
    if (remaining < 10.0 || m_defectRepo.countMinorDefects(plane.id) > 0) return Qt::yellow;
    return Qt::green;
}

QString MainWindow::calculateStatusText(const Aircraft& plane) {
    if (m_defectRepo.hasCriticalDefects(plane.id)) return "CRITICAL DEFECT";
    double remaining = plane.engineHoursNextService - plane.engineHoursTotal;
    if (remaining <= 0) return "SERVICE REQ";
    int minorCount = m_defectRepo.countMinorDefects(plane.id);
    if (minorCount >= 3) return "TOO MANY DEFECTS";
    if (remaining < 10.0) return "SERVICE SOON";
    if (minorCount > 0) return QString("WARNINGS (%1)").arg(minorCount);
    return "READY";
}
