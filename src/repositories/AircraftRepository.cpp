#include "src/repositories/AircraftRepository.h"
#include "src/db/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

AircraftRepository::AircraftRepository() {
}

std::vector<Aircraft> AircraftRepository::getAll() {
    std::vector<Aircraft> list;

    QSqlDatabase db = DatabaseManager::instance().getDatabase();

    // Используем LEFT JOIN, чтобы получить данные о модели самолета
    QSqlQuery query(db);
    query.prepare(
        "SELECT a.id, a.model_id, a.reg_number, a.engine_hours_total, a.engine_hours_next_service, "
        "       m.name as model_name, m.fuel_capacity "
        "FROM aircrafts a "
        "LEFT JOIN aircraft_models m ON a.model_id = m.id"
    );

    if (!query.exec()) {
        qDebug() << "AircraftRepo error (getAll):" << query.lastError().text();
        return list;
    }

    while (query.next()) {
        list.push_back(mapToEntity(query));
    }

    return list;
}

Aircraft AircraftRepository::getById(QUuid id) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    query.prepare(
        "SELECT a.id, a.model_id, a.reg_number, a.engine_hours_total, a.engine_hours_next_service, "
        "       m.name as model_name, m.fuel_capacity "
        "FROM aircrafts a "
        "LEFT JOIN aircraft_models m ON a.model_id = m.id "
        "WHERE a.id = :id"
    );
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return mapToEntity(query);
    } else {
        qDebug() << "AircraftRepo error (getById):" << query.lastError().text();
        return Aircraft(); // Возвращаем пустой объект
    }
}

Aircraft AircraftRepository::getByRegNumber(const QString& regNumber) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    query.prepare(
        "SELECT a.id, a.model_id, a.reg_number, a.engine_hours_total, a.engine_hours_next_service, "
        "       m.name as model_name, m.fuel_capacity "
        "FROM aircrafts a "
        "LEFT JOIN aircraft_models m ON a.model_id = m.id "
        "WHERE a.reg_number = :reg"
    );
    query.bindValue(":reg", regNumber);

    if (query.exec() && query.next()) {
        return mapToEntity(query);
    } else {
        // Если не найдено, не считаем это ошибкой SQL, просто вернем пустой объект
        // qDebug() << "Aircraft not found:" << regNumber;
        return Aircraft();
    }
}

bool AircraftRepository::updateNextService(QUuid id, double nextServiceHours) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    query.prepare("UPDATE aircrafts SET engine_hours_next_service = :next WHERE id = :id");
    query.bindValue(":next", nextServiceHours);
    query.bindValue(":id", id);
    return query.exec();
}

bool AircraftRepository::updateEngineHours(QUuid id, double hoursFlown) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    // Увеличиваем общий налет на hoursFlown
    query.prepare(
        "UPDATE aircrafts "
        "SET engine_hours_total = engine_hours_total + :hours "
        "WHERE id = :id"
    );

    query.bindValue(":hours", hoursFlown);
    query.bindValue(":id", id);

    if (!query.exec()) {
         qDebug() << "AircraftRepo error (updateHours):" << query.lastError().text();
         return false;
    }
    return true;
}

bool AircraftRepository::create(const Aircraft& aircraft) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    query.prepare(
        "INSERT INTO aircrafts (id, model_id, reg_number, engine_hours_total, engine_hours_next_service) "
        "VALUES (:id, :model_id, :reg, :total, :next_service)"
    );

    // Если ID пустой, генерируем новый UUID
    QUuid newId = aircraft.id.isNull() ? QUuid::createUuid() : aircraft.id;

    query.bindValue(":id", newId);
    query.bindValue(":model_id", aircraft.modelId);
    query.bindValue(":reg", aircraft.regNumber);
    query.bindValue(":total", aircraft.engineHoursTotal);
    query.bindValue(":next_service", aircraft.engineHoursNextService);

    if (!query.exec()) {
        qDebug() << "AircraftRepo error (create):" << query.lastError().text();
        return false;
    }
    return true;
}

// Удаление
bool AircraftRepository::deleteById(QUuid id) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM aircrafts WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

Aircraft AircraftRepository::mapToEntity(const QSqlQuery& query) {
    Aircraft a;
    a.id = query.value("id").toUuid();
    a.modelId = query.value("model_id").toUuid();
    a.regNumber = query.value("reg_number").toString();
    a.engineHoursTotal = query.value("engine_hours_total").toDouble();
    a.engineHoursNextService = query.value("engine_hours_next_service").toDouble();

    // Данные из JOIN
    a.modelName = query.value("model_name").toString();
    a.fuelCapacity = query.value("fuel_capacity").toDouble();

    return a;
}

void AircraftRepository::deleteAll() {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    if (!query.exec("DELETE FROM aircrafts")) {
        qDebug() << "AircraftRepo error (deleteAll):" << query.lastError().text();
    }
}
