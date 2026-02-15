#include "src/services/FleetService.h"
#include "src/db/DatabaseManager.h"
#include <QUuid>
#include <QDate>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

FleetService::FleetService() {
}

bool FleetService::deleteAircraft(QUuid aircraftId) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();

    // Обязательно используем транзакцию, так как удаляем из нескольких таблиц
    if (!db.transaction()) return false;

    QSqlQuery query(db);
    bool success = true;

    // 1. Удаляем активные дефекты этого самолета через репозиторий
    m_defectRepo.deleteActiveByAircraftId(aircraftId);

    // 2. Удаляем сам самолет
    if (m_aircraftRepo.deleteById(aircraftId)) {
        db.commit();
        return true;
    } else {
        db.rollback();
        qDebug() << "Error deleting aircraft record";
        return false;
    }
}

bool FleetService::seedDemoData() {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    // 1. Очистка старых данных через репозитории
    m_defectRepo.deleteAllActive();
    m_aircraftRepo.deleteAll();
    m_pilotRepo.deleteAll();
    m_modelRepo.deleteAll();

    qDebug() << "FleetService: Database cleared. Seeding demo data...";

    // 2. Создаем Модели (Типы ВС)
    QUuid cessnaId = QUuid::createUuid();
    QUuid piperId = QUuid::createUuid();

    AircraftModel m1;
    m1.id = cessnaId;
    m1.name = "Cessna 172N";
    m1.maxTakeoffWeight = 1043;
    m1.emptyWeight = 767;
    m1.fuelCapacity = 212;
    m1.fuelConsumption = 35;
    m_modelRepo.create(m1);

    AircraftModel m2;
    m2.id = piperId;
    m2.name = "Piper PA-28";
    m2.maxTakeoffWeight = 1155;
    m2.emptyWeight = 710;
    m2.fuelCapacity = 180;
    m2.fuelConsumption = 32;
    m_modelRepo.create(m2);

    // 3. Создаем Самолеты с разными статусами

    // 3.1. Зеленый (Исправный)
    Aircraft a1;
    a1.id = QUuid::createUuid();
    a1.modelId = cessnaId;
    a1.regNumber = "RA-01772";
    a1.engineHoursTotal = 1450.0;
    a1.engineHoursNextService = 1500.0;
    m_aircraftRepo.create(a1);

    // 3.2. Желтый (Скоро ТО, осталось 5 часов)
    Aircraft a2;
    a2.id = QUuid::createUuid();
    a2.modelId = cessnaId;
    a2.regNumber = "RA-02772";
    a2.engineHoursTotal = 1995.0;
    a2.engineHoursNextService = 2000.0;
    m_aircraftRepo.create(a2);

    // 3.3. Красный (Ресурс исчерпан)
    Aircraft a3;
    a3.id = QUuid::createUuid();
    a3.modelId = piperId;
    a3.regNumber = "RA-33028";
    a3.engineHoursTotal = 2005.0;
    a3.engineHoursNextService = 2000.0;
    m_aircraftRepo.create(a3);

    // 3.4. Красный (Критический дефект)
    Aircraft a4;
    a4.id = QUuid::createUuid();
    a4.modelId = piperId;
    a4.regNumber = "RA-44028";
    a4.engineHoursTotal = 500.0;
    a4.engineHoursNextService = 2000.0;
    m_aircraftRepo.create(a4);

    // 4. Создаем Пилотов
    Pilot p1;
    p1.fullName = "Иванов Иван Иванович";
    p1.licenseExpiryDate = QDate::currentDate().addYears(1);
    p1.medicalExpiryDate = QDate::currentDate().addMonths(6);
    p1.allowedModels << cessnaId << piperId; // Допуск на оба типа
    m_pilotRepo.create(p1);

    Pilot p2;
    p2.fullName = "Петров Петр Петрович";
    p2.licenseExpiryDate = QDate::currentDate().addYears(1);
    p2.medicalExpiryDate = QDate::currentDate().addMonths(6);
    p2.allowedModels << cessnaId; // Только Cessna
    m_pilotRepo.create(p2);

    // 5. Создаем Дефекты (Связываем с самолетами)
    // Сначала получаем ID типов дефектов из справочника
    std::vector<DefectType> defectTypes = m_defectRepo.getAllDefectTypes();
    QUuid criticalTypeId, minorTypeId;

    // Ищем подходящие типы дефектов
    for (const auto& d : defectTypes) {
        if (d.severity == "CRITICAL" && criticalTypeId.isNull()) criticalTypeId = d.id;
        if (d.severity == "MINOR" && minorTypeId.isNull()) minorTypeId = d.id;
    }

    if (!criticalTypeId.isNull()) {
        m_defectRepo.addActiveDefect(a4.id, criticalTypeId);
    }
    if (!minorTypeId.isNull()) {
        m_defectRepo.addActiveDefect(a2.id, minorTypeId);
    }

    return true;
}

bool FleetService::registerAircraft(const Aircraft& aircraft) {
    // Валидация перед записью
    if (aircraft.regNumber.trimmed().isEmpty()) {
        qDebug() << "FleetService: Ошибка - пустой бортовой номер";
        return false;
    }
    if (aircraft.modelId.isNull()) {
        qDebug() << "FleetService: Ошибка - не выбрана модель самолета";
        return false;
    }

    return m_aircraftRepo.create(aircraft);
}

bool FleetService::registerPilot(const Pilot& pilot) {
    // Валидация
    if (pilot.fullName.trimmed().isEmpty()) {
        qDebug() << "FleetService: Ошибка - пустое имя пилота";
        return false;
    }
    if (!pilot.licenseExpiryDate.isValid() || !pilot.medicalExpiryDate.isValid()) {
         qDebug() << "FleetService: Ошибка - некорректные даты";
         return false;
    }

    return m_pilotRepo.create(pilot);
}

// Удаление пилота
bool FleetService::deletePilot(QUuid pilotId) {
    return m_pilotRepo.deleteById(pilotId);
}

bool FleetService::reportDefect(QUuid aircraftId, QUuid defectTypeId) {
    // Валидация
    if (aircraftId.isNull()) {
        qDebug() << "FleetService: Ошибка - самолет не выбран";
        return false;
    }
    if (defectTypeId.isNull()) {
        qDebug() << "FleetService: Ошибка - тип дефекта не выбран";
        return false;
    }

    return m_defectRepo.addActiveDefect(aircraftId, defectTypeId);
}

// Реализация удаления неисправности
bool FleetService::resolveDefect(QUuid activeDefectId) {
    if (activeDefectId.isNull()) return false;
    return m_defectRepo.removeActiveDefect(activeDefectId);
}

bool FleetService::commitFlight(QUuid aircraftId, int flightTimeMinutes) {
    // Рассчитываем часы
    double hoursFlown = (double)flightTimeMinutes / 60.0;

    // Обновляем налет самолета
    if (m_aircraftRepo.updateEngineHours(aircraftId, hoursFlown)) {
        qDebug() << "Flight committed. Hours added:" << hoursFlown;
        return true;
    } else {
        qDebug() << "Error updating engine hours";
        return false;
    }
}

bool FleetService::performEngineMaintenance(QUuid aircraftId) {
    // 1. Получаем текущее состояние самолета
    Aircraft plane = m_aircraftRepo.getById(aircraftId);
    if (plane.id.isNull()) return false;

    // 2. Рассчитываем новый лимит
    double currentHours = plane.engineHoursTotal;
    double newLimit = currentHours + 100.0;

    return m_aircraftRepo.updateNextService(aircraftId, newLimit);
}

// Очистка
bool FleetService::clearFleetData() {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();

    if (!db.transaction()) return false;

    // Чистим данные через репозитории
    m_defectRepo.deleteAllActive();
    m_aircraftRepo.deleteAll();
    m_pilotRepo.deleteAll();

    db.commit();
    qDebug() << "FleetService: Operational data cleared.";
    return true;
}
