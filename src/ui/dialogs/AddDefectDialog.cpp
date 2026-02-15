#include "src/ui/dialogs/AddDefectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>
#include <QVariant>

AddDefectDialog::AddDefectDialog(QWidget *parent, QUuid aircraftId)
    : QDialog(parent), m_preSelectedAircraftId(aircraftId)
{
    setupUi();
    loadData();
}

AddDefectDialog::~AddDefectDialog() {
}

void AddDefectDialog::setupUi() {
    setWindowTitle("Регистрация неисправности");
    resize(450, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *inputGroup = new QGroupBox("Детали инцидента", this);
    QFormLayout *formLayout = new QFormLayout(inputGroup);

    m_aircraftCombo = new QComboBox(this);
    m_defectTypeCombo = new QComboBox(this);

    m_severityLabel = new QLabel("-", this);
    m_severityLabel->setStyleSheet("font-weight: bold; padding: 3px;");

    formLayout->addRow("Воздушное судно:", m_aircraftCombo);
    formLayout->addRow("Тип неисправности:", m_defectTypeCombo);
    formLayout->addRow("Критичность:", m_severityLabel);

    mainLayout->addWidget(inputGroup);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnSave = new QPushButton("Добавить дефект", this);
    m_btnSave->setStyleSheet("background-color: #D32F2F; color: white; font-weight: bold;");

    m_btnCancel = new QPushButton("Отмена", this);

    btnLayout->addStretch();
    btnLayout->addWidget(m_btnCancel);
    btnLayout->addWidget(m_btnSave);

    mainLayout->addLayout(btnLayout);

    connect(m_btnSave, &QPushButton::clicked, this, &AddDefectDialog::onSaveClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_defectTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddDefectDialog::updateSeverityLabel);
}

void AddDefectDialog::loadData() {
    // 1. Загрузка самолетов
    m_aircraftCombo->clear();
    std::vector<Aircraft> fleet = m_aircraftRepo.getAll();

    if (fleet.empty()) {
        m_aircraftCombo->addItem("Нет самолетов в БД");
        m_aircraftCombo->setEnabled(false);
        m_btnSave->setEnabled(false);
    } else {
        for (const auto& plane : fleet) {
            QString label = QString("%1 (%2)").arg(plane.regNumber, plane.modelName);
            m_aircraftCombo->addItem(label, plane.id.toString());
        }
    }

    // Если ID передан в конструктор, находим его в списке и блокируем выбор
    if (!m_preSelectedAircraftId.isNull()) {
        int index = m_aircraftCombo->findData(m_preSelectedAircraftId.toString());
        if (index >= 0) {
            m_aircraftCombo->setCurrentIndex(index);
            m_aircraftCombo->setEnabled(false); // Блокируем, так как самолет уже определен контекстом
        }
    }

    // 2. Загрузка справочника дефектов
    m_defectTypeCombo->clear();
    std::vector<DefectType> defects = m_defectRepo.getAllDefectTypes();

    if (defects.empty()) {
        m_defectTypeCombo->addItem("Справочник дефектов пуст");
        m_defectTypeCombo->setEnabled(false);
    } else {
        for (const auto& d : defects) {
            QVariantList userData;
            userData << d.id.toString() << d.severity;
            m_defectTypeCombo->addItem(d.description, userData);
        }
    }

    updateSeverityLabel();
}

void AddDefectDialog::updateSeverityLabel() {
    QVariant data = m_defectTypeCombo->currentData();
    if (data.isValid()) {
        QVariantList list = data.toList();
        if (list.size() > 1) {
            QString severity = list[1].toString();
            m_severityLabel->setText(severity);

            if (severity == "CRITICAL") {
                m_severityLabel->setStyleSheet("color: red; font-weight: bold; border: 1px solid red; padding: 3px;");
            } else {
                m_severityLabel->setStyleSheet("color: #FBC02D; font-weight: bold; border: 1px solid #FBC02D; padding: 3px;");
            }
            return;
        }
    }
    m_severityLabel->setText("-");
    m_severityLabel->setStyleSheet("");
}

void AddDefectDialog::onSaveClicked() {
    if (!m_aircraftCombo->isEnabled() && m_aircraftCombo->count() == 0) return;

    // Даже если комбобокс выключен (disabled), мы все равно можем получить его текущие данные
    QString aircraftIdStr = m_aircraftCombo->currentData().toString();
    if (aircraftIdStr.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите самолет!");
        return;
    }

    QVariant defectData = m_defectTypeCombo->currentData();
    if (!defectData.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите тип неисправности!");
        return;
    }

    QUuid aircraftId(aircraftIdStr);
    QUuid defectTypeId(defectData.toList()[0].toString());

    if (m_fleetService.reportDefect(aircraftId, defectTypeId)) {
        QMessageBox::information(this, "Успех", "Неисправность зарегистрирована.");
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить запись о дефекте.");
    }
}
