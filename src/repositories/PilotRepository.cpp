#include "src/repositories/PilotRepository.h"
#include "src/db/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

PilotRepository::PilotRepository() {
}

std::vector<Pilot> PilotRepository::getAll() {
    std::vector<Pilot> list;
    QSqlDatabase db = DatabaseManager::instance().getDatabase();

    QSqlQuery query(db);
    query.prepare("SELECT id, full_name, license_expiry_date, medical_expiry_date, allowed_models_json FROM pilots");

    if (!query.exec()) {
        qDebug() << "PilotRepo error (getAll):" << query.lastError().text();
        return list;
    }

    while (query.next()) {
        list.push_back(mapToEntity(query));
    }
    return list;
}

Pilot PilotRepository::getById(QUuid id) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM pilots WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return mapToEntity(query);
    }
    return Pilot();
}

bool PilotRepository::create(const Pilot& pilot) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    // Преобразуем список UUID в JSON массив для сохранения в PostgreSQL
    QJsonArray jsonArray;
    for (const QUuid& modelId : pilot.allowedModels) {
        jsonArray.append(modelId.toString());
    }
    QJsonDocument doc(jsonArray);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    query.prepare(
        "INSERT INTO pilots (id, full_name, license_expiry_date, medical_expiry_date, allowed_models_json) "
        "VALUES (:id, :name, :lic_date, :med_date, :json)"
    );

    QUuid newId = pilot.id.isNull() ? QUuid::createUuid() : pilot.id;

    query.bindValue(":id", newId);
    query.bindValue(":name", pilot.fullName);
    query.bindValue(":lic_date", pilot.licenseExpiryDate);
    query.bindValue(":med_date", pilot.medicalExpiryDate);
    // В Postgres JSON передается как строка, драйвер сам разберется, если колонка типа JSONB
    query.bindValue(":json", jsonString);

    if (!query.exec()) {
        qDebug() << "PilotRepo error (create):" << query.lastError().text();
        return false;
    }
    return true;
}

// Удаление
bool PilotRepository::deleteById(QUuid id) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM pilots WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

std::vector<Pilot> PilotRepository::findByName(const QString& namePart) {
    std::vector<Pilot> list;
    QSqlDatabase db = DatabaseManager::instance().getDatabase();

    QSqlQuery query(db);
    // Ищем регистронезависимо (ILIKE - фишка Postgres)
    query.prepare("SELECT * FROM pilots WHERE full_name ILIKE :name");
    query.bindValue(":name", "%" + namePart + "%");

    if (query.exec()) {
        while (query.next()) {
            list.push_back(mapToEntity(query));
        }
    }
    return list;
}

Pilot PilotRepository::mapToEntity(const QSqlQuery& query) {
    Pilot p;
    p.id = query.value("id").toUuid();
    p.fullName = query.value("full_name").toString();
    p.licenseExpiryDate = query.value("license_expiry_date").toDate();
    p.medicalExpiryDate = query.value("medical_expiry_date").toDate();

    // Парсинг JSON из базы обратно в список C++
    QByteArray jsonData = query.value("allowed_models_json").toByteArray();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const auto& val : arr) {
            // Извлекаем строку GUID и превращаем в QUuid
            p.allowedModels.append(QUuid(val.toString()));
        }
    }

    return p;
}

void PilotRepository::deleteAll() {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    if (!query.exec("DELETE FROM pilots")) {
        qDebug() << "PilotRepo error (deleteAll):" << query.lastError().text();
    }
}
