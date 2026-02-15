#include "src/repositories/AircraftModelRepository.h"
#include "src/db/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

AircraftModelRepository::AircraftModelRepository() {
}

std::vector<AircraftModel> AircraftModelRepository::getAll() {
    std::vector<AircraftModel> list;
    QSqlDatabase db = DatabaseManager::instance().getDatabase();

    // Сортируем по имени для удобства в выпадающих списках
    QSqlQuery query(db);
    query.prepare("SELECT * FROM aircraft_models ORDER BY name");

    if (query.exec()) {
        while (query.next()) {
            list.push_back(mapToEntity(query));
        }
    } else {
        qDebug() << "ModelRepo error (getAll):" << query.lastError().text();
    }
    return list;
}

AircraftModel AircraftModelRepository::getById(QUuid id) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT * FROM aircraft_models WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return mapToEntity(query);
    }
    return AircraftModel();
}

bool AircraftModelRepository::create(const AircraftModel& model) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    // Проверяем, существует ли уже модель с таким названием
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM aircraft_models WHERE name = :name");
    checkQuery.bindValue(":name", model.name);

    if (checkQuery.exec() && checkQuery.next()) {
        if (checkQuery.value(0).toInt() > 0) {
            // Если модель уже есть, не создаем дубликат, а просто возвращаем false (или true, если считать это успехом)
            qDebug() << "ModelRepo: Model already exists (" << model.name << "). Skipping creation.";
            return false;
        }
    }

    query.prepare("INSERT INTO aircraft_models "
                  "(id, name, max_takeoff_weight, empty_weight, fuel_capacity, fuel_consumption) "
                  "VALUES (:id, :name, :mtow, :ew, :cap, :cons)");

    // Генерируем ID, если его нет
    QUuid newId = model.id.isNull() ? QUuid::createUuid() : model.id;

    query.bindValue(":id", newId);
    query.bindValue(":name", model.name);
    query.bindValue(":mtow", model.maxTakeoffWeight);
    query.bindValue(":ew", model.emptyWeight);
    query.bindValue(":cap", model.fuelCapacity);
    query.bindValue(":cons", model.fuelConsumption);

    if (!query.exec()) {
        qDebug() << "ModelRepo error (create):" << query.lastError().text();
        return false;
    }
    return true;
}

AircraftModel AircraftModelRepository::mapToEntity(const QSqlQuery& query) {
    AircraftModel m;
    m.id = query.value("id").toUuid();
    m.name = query.value("name").toString();
    m.maxTakeoffWeight = query.value("max_takeoff_weight").toDouble();
    m.emptyWeight = query.value("empty_weight").toDouble();
    m.fuelCapacity = query.value("fuel_capacity").toDouble();
    m.fuelConsumption = query.value("fuel_consumption").toDouble();
    return m;
}

void AircraftModelRepository::deleteAll() {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    if (!query.exec("DELETE FROM aircraft_models")) {
        qDebug() << "ModelRepo error (deleteAll):" << query.lastError().text();
    }
}
