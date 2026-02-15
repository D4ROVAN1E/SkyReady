#include "src/ui/FlightPreparationDialog.h"
#include "src/db/DatabaseManager.h"
#include "src/repositories/AircraftRepository.h"
#include "src/repositories/DefectRepository.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>

FlightPreparationDialog::FlightPreparationDialog(QUuid aircraftId, QWidget *parent)
    : QDialog(parent), m_aircraftId(aircraftId)
{
    setupUi();
    loadData();

    // Сразу запускаем проверку с дефолтными значениями
    onCheckReadiness();
}

FlightPreparationDialog::~FlightPreparationDialog() {
}

void FlightPreparationDialog::setupUi() {
    setWindowTitle("Подготовка к вылету");
    resize(500, 650); // Немного увеличим высоту для комфорта

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. Инфо о самолете (Заголовок)
    m_lblAircraftInfo = new QLabel("Самолет: Загрузка...", this);
    QFont titleFont = m_lblAircraftInfo->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    m_lblAircraftInfo->setFont(titleFont);
    m_lblAircraftInfo->setAlignment(Qt::AlignCenter);
    m_lblAircraftInfo->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    mainLayout->addWidget(m_lblAircraftInfo);

    // 2. Форма ввода параметров
    QGroupBox *inputGroup = new QGroupBox("Параметры рейса", this);
    QFormLayout *formLayout = new QFormLayout(inputGroup);

    m_pilotCombo = new QComboBox(this);

    m_fuelSpin = new QDoubleSpinBox(this);
    m_fuelSpin->setRange(0, 500);
    m_fuelSpin->setSuffix(" л");
    m_fuelSpin->setValue(100);

    m_cargoSpin = new QDoubleSpinBox(this);
    m_cargoSpin->setRange(0, 1000);
    m_cargoSpin->setSuffix(" кг");
    m_cargoSpin->setValue(80);

    m_timeSpin = new QSpinBox(this);
    m_timeSpin->setRange(10, 600);
    m_timeSpin->setSuffix(" мин");
    m_timeSpin->setValue(60);

    formLayout->addRow("Командир ВС:", m_pilotCombo);
    formLayout->addRow("Топливо:", m_fuelSpin);
    formLayout->addRow("Загрузка (Люди+Груз):", m_cargoSpin);
    formLayout->addRow("План. время полета:", m_timeSpin);

    mainLayout->addWidget(inputGroup);

    // 3. Блок результата
    m_resultGroup = new QGroupBox(this); // Убрали текст заголовка
    QVBoxLayout *resLayout = new QVBoxLayout(m_resultGroup);

    // Метка статуса (ГОТОВ / НЕ ГОТОВ)
    m_resultLabel = new QLabel("ОЖИДАНИЕ РАСЧЕТА", this);
    m_resultLabel->setAlignment(Qt::AlignCenter);
    QFont resFont = m_resultLabel->font();
    resFont.setBold(true);
    resFont.setPointSize(16); // Сделали крупнее
    m_resultLabel->setFont(resFont);
    m_resultLabel->setStyleSheet("background-color: #cccccc; color: #333333; border-radius: 4px; padding: 10px;");

    // Текстовое поле с деталями (делаем его фиксированным по высоте, но растягивающимся)
    m_detailsText = new QTextEdit(this);
    m_detailsText->setReadOnly(true);
    m_detailsText->setMinimumHeight(100); // Чтобы всегда было место для списка ошибок

    resLayout->addWidget(m_resultLabel);
    resLayout->addWidget(m_detailsText);

    mainLayout->addWidget(m_resultGroup);

    // 4. Кнопки действий (В самом низу)
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnCommit = new QPushButton("ВЫПУСТИТЬ В РЕЙС", this);
    m_btnCommit->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 12px; font-size: 14px;");
    m_btnCommit->setEnabled(false); // Сначала проверка

    m_btnClose = new QPushButton("Отмена", this);
    m_btnClose->setStyleSheet("padding: 12px;");

    btnLayout->addWidget(m_btnClose);
    btnLayout->addWidget(m_btnCommit);
    mainLayout->addLayout(btnLayout);

    // Сигналы
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_btnCommit, &QPushButton::clicked, this, &FlightPreparationDialog::onCommitFlight);

    // Автопересчет при смене параметро
    connect(m_pilotCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FlightPreparationDialog::onCheckReadiness);
    connect(m_fuelSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FlightPreparationDialog::onCheckReadiness);
    connect(m_cargoSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FlightPreparationDialog::onCheckReadiness);
    connect(m_timeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &FlightPreparationDialog::onCheckReadiness);
}

void FlightPreparationDialog::loadData() {
    // 1. Загрузка инфо о самолете для заголовка
    AircraftRepository repo;
    Aircraft plane = repo.getById(m_aircraftId);
    if (!plane.id.isNull()) {
        m_lblAircraftInfo->setText(QString("%1 (%2)").arg(plane.regNumber, plane.modelName));
    }

    // 2. Загрузка пилотов
    std::vector<Pilot> pilots = m_pilotRepo.getAll();
    m_pilotCombo->clear();

    for (const auto& p : pilots) {
        m_pilotCombo->addItem(p.fullName, p.id.toString());
    }

    if (pilots.empty()) {
         m_pilotCombo->addItem("Нет пилотов в БД", "");
         m_detailsText->append("Внимание: База пилотов пуста. Функционал ограничен.");
    }
}

void FlightPreparationDialog::onCheckReadiness() {
    m_detailsText->clear();

    // Сбор данных
    FlightParams params;
    params.fuelAmount = m_fuelSpin->value();
    params.cargoWeight = m_cargoSpin->value();
    params.flightTimeMinutes = m_timeSpin->value();

    QUuid pilotId;
    QString pilotIdStr = m_pilotCombo->currentData().toString();
    if (!pilotIdStr.isEmpty()) {
        pilotId = QUuid(pilotIdStr);
    }

    // Получаем реальные названия дефектов, чтобы вывести их в скобках
    DefectRepository defectRepo;
    std::vector<ActiveDefect> defects = defectRepo.getByAircraftId(m_aircraftId);
    QStringList criticalDefectNames;
    QStringList minorDefectNames;

    for (const auto& d : defects) {
        if (d.severity == "CRITICAL") {
            criticalDefectNames.append(d.description);
        } else {
            minorDefectNames.append(d.description);
        }
    }

    // Вызов бизнес-логики
    ReadinessReport report = m_readinessService.checkReadiness(m_aircraftId, pilotId, params);

    // Обновление UI
    if (report.isReady) {
        m_resultLabel->setText("ГОТОВ К ВЫЛЕТУ");
        // Зеленый фон, белый текст
        m_resultLabel->setStyleSheet("background-color: #4CAF50; color: white; border-radius: 4px; padding: 10px; font-weight: bold; font-size: 16px;");
        m_resultGroup->setStyleSheet("QGroupBox { border: 1px solid #4CAF50; font-weight: bold; }");

        m_btnCommit->setEnabled(true);
        m_detailsText->append("Все системы в норме. Расчет центровки: ОК.\n");
    } else {
        m_resultLabel->setText("ВЫЛЕТ ЗАПРЕЩЕН");
        // Красный фон, белый текст
        m_resultLabel->setStyleSheet("background-color: #F44336; color: white; border-radius: 4px; padding: 10px; font-weight: bold; font-size: 16px;");
        m_resultGroup->setStyleSheet("QGroupBox { border: 1px solid #F44336; font-weight: bold; }");

        m_btnCommit->setEnabled(false);
    }

    // Вывод ошибок (с красными маркерами)
    for (const QString& err : report.errors) {
        QString displayText = err;

        // Если сообщение касается критических дефектов, добавляем их список
        if (displayText.contains("КРИТИЧЕСКИЕ неисправности") && !criticalDefectNames.isEmpty()) {
            displayText += QString(" (%1)").arg(criticalDefectNames.join(", "));
        }
        // Если сообщение о превышении лимита мелких дефектов (это ошибка)
        if (displayText.contains("мелких неисправностей") && !minorDefectNames.isEmpty()) {
            displayText += QString(" (%1)").arg(minorDefectNames.join(", "));
        }

        // Используем HTML для цвета внутри текстового поля
        m_detailsText->insertHtml("<font color='red'>" + displayText + "</font><br>");
    }

    // Вывод предупреждений (с оранжевыми маркерами)
    for (const QString& warn : report.warnings) {
        QString displayText = warn;

        // Если сообщение о наличии мелких дефектов (это предупреждение)
        if (displayText.contains("мелкие неисправности") && !minorDefectNames.isEmpty()) {
            displayText += QString(" (%1)").arg(minorDefectNames.join(", "));
        }

        m_detailsText->insertHtml("<font color='#FF9800'> " + displayText + "</font><br>");
    }
}

void FlightPreparationDialog::onCommitFlight() {
    // 1. Собираем данные
    int timeMinutes = m_timeSpin->value();

    // 2. Подтверждение от пользователя
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение",
        QString("Выпустить самолет в рейс на %1 мин?\n"
                "Это действие добавит налет и спишет ресурс двигателя.").arg(timeMinutes),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    // 3. Вызов сервиса
    bool success = m_fleetService.commitFlight(m_aircraftId, timeMinutes);

    if (success) {
        QMessageBox::information(this, "Рейс завершен",
            "Полет успешно зафиксирован.\nМоточасы самолета обновлены.");
        accept(); // Закрываем диалог с кодом Accepted (MainWindow обновит таблицу)
    } else {
        QMessageBox::critical(this, "Ошибка",
            "Не удалось записать данные о полете в базу данных.");
    }
}
