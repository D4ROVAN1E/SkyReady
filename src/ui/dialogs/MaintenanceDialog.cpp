#include "src/ui/dialogs/MaintenanceDialog.h"
#include "src/ui/dialogs/AddDefectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QGroupBox>

MaintenanceDialog::MaintenanceDialog(QUuid aircraftId, const QString& regNumber, QWidget *parent)
    : QDialog(parent), m_aircraftId(aircraftId), m_regNumber(regNumber)
{
    setupUi();
    loadDefects();
}

MaintenanceDialog::~MaintenanceDialog() {
}

void MaintenanceDialog::setupUi() {
    setWindowTitle("Техническое обслуживание: " + m_regNumber);
    resize(500, 450);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Неисправности
    QGroupBox *grpDefects = new QGroupBox("Журнал неисправностей", this);
    QVBoxLayout *defectLayout = new QVBoxLayout(grpDefects);

    m_defectsList = new QListWidget(this);
    defectLayout->addWidget(m_defectsList);

    QHBoxLayout *defectBtnLayout = new QHBoxLayout();
    m_btnAdd = new QPushButton("Добавить дефект...", this);
    m_btnResolve = new QPushButton("Устранить выбранный", this);
    m_btnResolve->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");

    defectBtnLayout->addWidget(m_btnAdd);
    defectBtnLayout->addStretch();
    defectBtnLayout->addWidget(m_btnResolve);
    defectLayout->addLayout(defectBtnLayout);

    mainLayout->addWidget(grpDefects);

    // Двигатель
    QGroupBox *grpEngine = new QGroupBox("Двигатель и Планер", this);
    QHBoxLayout *engineLayout = new QHBoxLayout(grpEngine);

    m_btnEngineService = new QPushButton("Провести регламентное ТО (+100ч)", this);
    m_btnEngineService->setStyleSheet("background-color: #2196F3; color: white; font-weight: bold; padding: 5px;");

    engineLayout->addWidget(new QLabel("Действия:", this));
    engineLayout->addWidget(m_btnEngineService);

    mainLayout->addWidget(grpEngine);

    // Нижняя панель
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    m_btnClose = new QPushButton("Закрыть", this);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_btnClose);
    mainLayout->addLayout(bottomLayout);

    // Сигналы
    connect(m_btnAdd, &QPushButton::clicked, this, &MaintenanceDialog::onAddDefectClicked);
    connect(m_btnResolve, &QPushButton::clicked, this, &MaintenanceDialog::onResolveDefectClicked);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);

    // Новый сигнал
    connect(m_btnEngineService, &QPushButton::clicked, this, &MaintenanceDialog::onEngineServiceClicked);
}

void MaintenanceDialog::loadDefects() {
    m_defectsList->clear();
    std::vector<ActiveDefect> defects = m_defectRepo.getByAircraftId(m_aircraftId);

    if (defects.empty()) {
        m_defectsList->addItem("Неисправностей не обнаружено.");
        m_defectsList->setEnabled(false);
        m_btnResolve->setEnabled(false);
    } else {
        m_defectsList->setEnabled(true);
        m_btnResolve->setEnabled(true);

        for (const auto& d : defects) {
            QString text = QString("[%1] %2 (%3)").arg(d.createdAt.toString("dd.MM HH:mm"), d.description, d.severity);
            QListWidgetItem *item = new QListWidgetItem(text, m_defectsList);
            if (d.severity == "CRITICAL") item->setForeground(Qt::red);
            item->setData(Qt::UserRole, d.id.toString());
        }
    }
}

void MaintenanceDialog::onAddDefectClicked() {
    // Открываем диалог добавления с пре-выбранным самолетом
    AddDefectDialog dialog(this, m_aircraftId);
    if (dialog.exec() == QDialog::Accepted) {
        loadDefects();
    }
}

void MaintenanceDialog::onResolveDefectClicked() {
    QListWidgetItem *item = m_defectsList->currentItem();
    if (!item) return;
    QString idStr = item->data(Qt::UserRole).toString();
    if (idStr.isEmpty()) return;

    if (m_fleetService.resolveDefect(QUuid(idStr))) {
        QMessageBox::information(this, "Успех", "Неисправность устранена.");
        loadDefects();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить БД.");
    }
}

void MaintenanceDialog::onEngineServiceClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение ТО",
        "Вы подтверждаете проведение регламентного обслуживания?\n"
        "Ресурс двигателя будет продлен на 100 часов от текущего налета.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    if (m_fleetService.performEngineMaintenance(m_aircraftId)) {
        QMessageBox::information(this, "Успех", "ТО проведено успешно. Ресурс продлен.");
        // Диалог можно не закрывать, чтобы пользователь мог сделать что-то еще
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить данные о ресурсе.");
    }
}
