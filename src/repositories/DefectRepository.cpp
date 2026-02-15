#include "src/repositories/DefectRepository.h"
#include "src/db/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QUuid>

DefectRepository::DefectRepository() {
    // При создании репозитория проверяем, заполнен ли справочник
    ensureDefaultDefectsExist();
}

// Справочник

std::vector<DefectType> DefectRepository::getAllDefectTypes() {
    ensureDefaultDefectsExist();
    std::vector<DefectType> list;
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query("SELECT id, description, severity FROM defect_types ORDER BY description", db);

    while (query.next()) {
        DefectType dt;
        dt.id = query.value("id").toUuid();
        dt.description = query.value("description").toString();
        dt.severity = query.value("severity").toString();
        list.push_back(dt);
    }
    return list;
}

void DefectRepository::ensureDefaultDefectsExist() {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();

    if (!db.isOpen()) {
        return; // Если база закрыта, просто выходим, не пытаясь писать
    }

    QSqlQuery checkQuery("SELECT COUNT(*) FROM defect_types", db);

    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        return; // База уже заполнена
    }

    qDebug() << "DefectRepo: Seeding default defect types...";

    // Список типовых неисправностей
    struct InitData { QString desc; QString sev; };
    std::vector<InitData> defaults = {
        // MINOR (Легкие - можно лететь с ограничениями)
        {"Перегорела посадочная фара", "MINOR"},
        {"Потертость обшивки кресла", "MINOR"},
        {"Не работает подсветка приборов (день)", "MINOR"},
        {"Скол ЛКП на фюзеляже", "MINOR"},
        {"Неисправен прикуриватель", "MINOR"},
        {"Люфт подлокотника", "MINOR"},
        {"Шум в гарнитуре второго пилота", "MINOR"},
        {"Перегорел БАНО (дублирующий)", "MINOR"},
        {"Сломан держатель карт", "MINOR"},
        {"Заедает замок багажника", "MINOR"},

        // CRITICAL (Критические - вылет запрещен)
        {"Падение давления масла", "CRITICAL"},
        {"Стружка в масле", "CRITICAL"},
        {"Трещина лобового стекла", "CRITICAL"},
        {"Несимметричный выпуск закрылков", "CRITICAL"},
        {"Отказ генератора", "CRITICAL"},
        {"Течь топлива", "CRITICAL"},
        {"Люфт элеронов выше нормы", "CRITICAL"},
        {"Отказ радиостанции", "CRITICAL"},
        {"Вибрация двигателя", "CRITICAL"},
        {"Порез пневматика шасси", "CRITICAL"},

        // Особый пункт
        {"ПРОЧЕЕ (Требует проверки)", "CRITICAL"}
    };

    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO defect_types (id, description, severity) VALUES (:id, :desc, :sev)");

    for (const auto& item : defaults) {
        insertQuery.bindValue(":id", QUuid::createUuid());
        insertQuery.bindValue(":desc", item.desc);
        insertQuery.bindValue(":sev", item.sev);
        if (!insertQuery.exec()) {
            qDebug() << "Error seeding defects:" << insertQuery.lastError().text();
        }
    }
}

// Активные дефекты

bool DefectRepository::addActiveDefect(QUuid aircraftId, QUuid defectTypeId) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    query.prepare("INSERT INTO active_defects (id, aircraft_id, defect_type_id, created_at) VALUES (:id, :aid, :dtid, :date)");

    query.bindValue(":id", QUuid::createUuid());
    query.bindValue(":aid", aircraftId);
    query.bindValue(":dtid", defectTypeId);
    query.bindValue(":date", QDateTime::currentDateTime());

    if (!query.exec()) {
        qDebug() << "DefectRepo error (add):" << query.lastError().text();
        return false;
    }
    return true;
}

bool DefectRepository::removeActiveDefect(QUuid defectId) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM active_defects WHERE id = :id");
    query.bindValue(":id", defectId);

    return query.exec();
}

std::vector<ActiveDefect> DefectRepository::getByAircraftId(QUuid aircraftId) {
    std::vector<ActiveDefect> list;
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    // JOIN нужен, чтобы получить название и критичность дефекта одной строкой
    query.prepare(
        "SELECT ad.id, ad.aircraft_id, ad.defect_type_id, ad.created_at, "
        "       dt.description, dt.severity "
        "FROM active_defects ad "
        "JOIN defect_types dt ON ad.defect_type_id = dt.id "
        "WHERE ad.aircraft_id = :aid "
        "ORDER BY ad.created_at DESC"
    );
    query.bindValue(":aid", aircraftId);

    if (query.exec()) {
        while (query.next()) {
            ActiveDefect d;
            d.id = query.value("id").toUuid();
            d.aircraftId = query.value("aircraft_id").toUuid();
            d.defectTypeId = query.value("defect_type_id").toUuid();
            d.createdAt = query.value("created_at").toDateTime();
            d.description = query.value("description").toString();
            d.severity = query.value("severity").toString();
            list.push_back(d);
        }
    }
    return list;
}

int DefectRepository::countMinorDefects(QUuid aircraftId) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    // Считаем только те дефекты, у которых severity = 'MINOR'
    query.prepare(
        "SELECT COUNT(*) "
        "FROM active_defects ad "
        "JOIN defect_types dt ON ad.defect_type_id = dt.id "
        "WHERE ad.aircraft_id = :aid AND dt.severity = 'MINOR'"
    );
    query.bindValue(":aid", aircraftId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool DefectRepository::hasCriticalDefects(QUuid aircraftId) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);

    // Ищем хотя бы один CRITICAL
    query.prepare(
        "SELECT COUNT(*) "
        "FROM active_defects ad "
        "JOIN defect_types dt ON ad.defect_type_id = dt.id "
        "WHERE ad.aircraft_id = :aid AND dt.severity = 'CRITICAL'"
    );
    query.bindValue(":aid", aircraftId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

void DefectRepository::deleteAllActive() {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    if (!query.exec("DELETE FROM active_defects")) {
        qDebug() << "DefectRepo error (deleteAllActive):" << query.lastError().text();
    }
}

void DefectRepository::deleteActiveByAircraftId(QUuid aircraftId) {
    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM active_defects WHERE aircraft_id = :id");
    query.bindValue(":id", aircraftId);
    if (!query.exec()) {
        qDebug() << "DefectRepo error (deleteActiveByAircraftId):" << query.lastError().text();
    }
}
