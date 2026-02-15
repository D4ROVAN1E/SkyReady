#include "src/ui/dialogs/AddPilotDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDate>
#include <QGroupBox>
#include <QLabel>
#include <QDebug>

AddPilotDialog::AddPilotDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    loadModels();
}

AddPilotDialog::~AddPilotDialog() {
}

void AddPilotDialog::setupUi() {
    setWindowTitle("Регистрация нового пилота");
    resize(400, 500); // Окно повыше из-за списка моделей

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Группа основных данных
    QGroupBox *inputGroup = new QGroupBox("Данные пилота", this);
    QFormLayout *formLayout = new QFormLayout(inputGroup);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("ФИО полностью");

    m_licenseDate = new QDateEdit(this);
    m_licenseDate->setCalendarPopup(true);
    m_licenseDate->setDisplayFormat("dd.MM.yyyy");
    m_licenseDate->setDate(QDate::currentDate().addYears(1)); // По дефолту +1 год

    m_medicalDate = new QDateEdit(this);
    m_medicalDate->setCalendarPopup(true);
    m_medicalDate->setDisplayFormat("dd.MM.yyyy");
    m_medicalDate->setDate(QDate::currentDate().addMonths(6)); // По дефолту +6 месяцев

    formLayout->addRow("ФИО:", m_nameEdit);
    formLayout->addRow("Срок лицензии:", m_licenseDate);
    formLayout->addRow("Срок мед. справки:", m_medicalDate);

    mainLayout->addWidget(inputGroup);

    // Группа допусков
    QGroupBox *modelsGroup = new QGroupBox("Допуски на типы ВС (Type Ratings)", this);
    QVBoxLayout *modelsLayout = new QVBoxLayout(modelsGroup);

    QLabel *hintLabel = new QLabel("Отметьте типы самолетов, которыми пилот может управлять:", this);
    hintLabel->setWordWrap(true);
    modelsLayout->addWidget(hintLabel);

    m_modelsList = new QListWidget(this);
    modelsLayout->addWidget(m_modelsList);

    mainLayout->addWidget(modelsGroup);

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
    connect(m_btnSave, &QPushButton::clicked, this, &AddPilotDialog::onSaveClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void AddPilotDialog::loadModels() {
    m_modelsList->clear();
    std::vector<AircraftModel> models = m_modelRepo.getAll();

    if (models.empty()) {
        m_modelsList->addItem("Нет моделей в БД (заполните флот)");
        m_modelsList->setEnabled(false);
        return;
    }

    for (const auto& model : models) {
        QListWidgetItem *item = new QListWidgetItem(model.name, m_modelsList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // Разрешаем ставить галочки
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, model.id.toString()); // Храним ID
    }
}

void AddPilotDialog::onSaveClicked() {
    // 1. Валидация имени
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите ФИО пилота!");
        return;
    }

    // 2. Сбор выбранных моделей
    QList<QUuid> allowedModels;
    for (int i = 0; i < m_modelsList->count(); ++i) {
        QListWidgetItem *item = m_modelsList->item(i);
        if (item->checkState() == Qt::Checked) {
            allowedModels.append(QUuid(item->data(Qt::UserRole).toString()));
        }
    }

    if (allowedModels.isEmpty()) {
        // Предупреждение, но разрешаем сохранить (может быть стажер)
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Внимание",
            "Не выбрано ни одного типа ВС.\nПилот не сможет быть назначен на рейсы.\nПродолжить?",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::No) return;
    }

    // 3. Сборка объекта
    Pilot newPilot;
    newPilot.id = QUuid::createUuid();
    newPilot.fullName = name;
    newPilot.licenseExpiryDate = m_licenseDate->date();
    newPilot.medicalExpiryDate = m_medicalDate->date();
    newPilot.allowedModels = allowedModels;

    // 4. Сохранение
    if (m_fleetService.registerPilot(newPilot)) {
        QMessageBox::information(this, "Успех", "Пилот успешно зарегистрирован.");
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить данные пилота.");
    }
}
