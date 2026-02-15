#include "src/ui/dialogs/AddAircraftDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>
#include <QGroupBox>

AddAircraftDialog::AddAircraftDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    loadModels();
}

AddAircraftDialog::~AddAircraftDialog() {
}

void AddAircraftDialog::setupUi() {
    setWindowTitle("Регистрация нового самолета");
    resize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Группа полей ввода
    QGroupBox *inputGroup = new QGroupBox("Данные воздушного судна", this);
    QFormLayout *formLayout = new QFormLayout(inputGroup);

    m_regNumberEdit = new QLineEdit(this);
    m_regNumberEdit->setPlaceholderText("Например: RA-12345");

    m_modelCombo = new QComboBox(this);

    m_totalHoursSpin = new QDoubleSpinBox(this);
    m_totalHoursSpin->setRange(0, 50000);
    m_totalHoursSpin->setSuffix(" ч");
    m_totalHoursSpin->setDecimals(1);

    m_serviceIntervalSpin = new QDoubleSpinBox(this);
    m_serviceIntervalSpin->setRange(0, 50000);
    m_serviceIntervalSpin->setSuffix(" ч");
    m_serviceIntervalSpin->setDecimals(1);
    m_serviceIntervalSpin->setToolTip("На каком значении счетчика нужно делать следующее ТО");

    formLayout->addRow("Бортовой номер:", m_regNumberEdit);
    formLayout->addRow("Модель (Тип ВС):", m_modelCombo);
    formLayout->addRow("Текущий налет:", m_totalHoursSpin);
    formLayout->addRow("Следующее ТО на:", m_serviceIntervalSpin);

    mainLayout->addWidget(inputGroup);

    // Кнопки
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnSave = new QPushButton("Сохранить", this);
    m_btnSave->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");

    m_btnCancel = new QPushButton("Отмена", this);

    btnLayout->addStretch();
    btnLayout->addWidget(m_btnCancel);
    btnLayout->addWidget(m_btnSave);

    mainLayout->addLayout(btnLayout);

    // Сигналы
    connect(m_btnSave, &QPushButton::clicked, this, &AddAircraftDialog::onSaveClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    // Небольшая автоматизация: при изменении налета предлагаем следующее ТО через 50 часов
    connect(m_totalHoursSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val){
        if (m_serviceIntervalSpin->value() == 0) {
            m_serviceIntervalSpin->setValue(val + 50.0);
        }
    });
}

void AddAircraftDialog::loadModels() {
    m_modelCombo->clear();
    std::vector<AircraftModel> models = m_modelRepo.getAll();

    if (models.empty()) {
        m_modelCombo->addItem("Нет моделей в БД", "");
        m_btnSave->setEnabled(false);
        return;
    }

    for (const auto& model : models) {
        // Сохраняем ID модели в user data комбобокса
        m_modelCombo->addItem(model.name, model.id.toString());
    }
}

void AddAircraftDialog::onSaveClicked() {
    // 1. Валидация
    QString regNumber = m_regNumberEdit->text().trimmed();
    if (regNumber.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите бортовой номер!");
        return;
    }

    QString modelIdStr = m_modelCombo->currentData().toString();
    if (modelIdStr.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите модель самолета!");
        return;
    }

    // 2. Сборка объекта
    Aircraft newAircraft;
    newAircraft.id = QUuid::createUuid(); // Генерируем новый ID
    m_createdId = newAircraft.id;
    newAircraft.modelId = QUuid(modelIdStr);
    newAircraft.regNumber = regNumber;
    newAircraft.engineHoursTotal = m_totalHoursSpin->value();
    newAircraft.engineHoursNextService = m_serviceIntervalSpin->value();

    // 3. Сохранение через сервис
    if (m_fleetService.registerAircraft(newAircraft)) {
        QMessageBox::information(this, "Успех", "Самолет успешно добавлен в флот.");
        accept(); // Закрываем диалог с кодом Accepted
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить самолет.\nВозможно, такой бортовой номер уже существует.");
    }
}
