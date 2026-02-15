#include "src/db/DatabaseManager.h"
#include <QProcessEnvironment>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() {
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::connectToDatabase() {
    // Используем драйвер PostgreSQL
    m_db = QSqlDatabase::addDatabase("QPSQL");

    // НАСТРОЙКИ ПОДКЛЮЧЕНИЯ
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QString host = env.value("DB_HOST", "db");
    QString dbName = env.value("DB_NAME", "skyready_db");
    QString user = env.value("DB_USER", "postgres");
    QString password = env.value("DB_PASSWORD", "postgres");

    m_db.setHostName(host);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);

    qDebug() << "Connecting to database at:" << host << "User:" << user;

    if (!m_db.open()) {
        qDebug() << "Error: Connection with database failed:" << m_db.lastError().text();
        return false;
    } else {
        qDebug() << "Database: Connection ok (PostgreSQL)";

        // Сразу после подключения проверяем наличие таблиц
        initDatabase();
        return true;
    }
}

void DatabaseManager::initDatabase() {
    QSqlQuery query(m_db);
    bool success = true;

    // 1. Справочник моделей самолетов (хранит характеристики типа)
    // Используем JSONB для хранения конверта центровки, так как это массив точек
    success &= query.exec(
        "CREATE TABLE IF NOT EXISTS aircraft_models ("
        "   id UUID PRIMARY KEY,"
        "   name VARCHAR(100) NOT NULL,"
        "   max_takeoff_weight DOUBLE PRECISION NOT NULL,"
        "   empty_weight DOUBLE PRECISION NOT NULL,"
        "   fuel_capacity DOUBLE PRECISION NOT NULL,"
        "   fuel_consumption DOUBLE PRECISION NOT NULL,"
        "   cg_envelope_json JSONB"
        ");"
    );
    if (query.lastError().isValid()) qDebug() << "Table 'aircraft_models' error:" << query.lastError().text();

    // 2. Самолеты (конкретные борта)
    success &= query.exec(
        "CREATE TABLE IF NOT EXISTS aircrafts ("
        "   id UUID PRIMARY KEY,"
        "   model_id UUID REFERENCES aircraft_models(id),"
        "   reg_number VARCHAR(20) UNIQUE NOT NULL,"
        "   engine_hours_total DOUBLE PRECISION DEFAULT 0,"
        "   engine_hours_next_service DOUBLE PRECISION NOT NULL"
        ");"
    );
    if (query.lastError().isValid()) qDebug() << "Table 'aircrafts' error:" << query.lastError().text();

    // 3. Пилоты
    success &= query.exec(
        "CREATE TABLE IF NOT EXISTS pilots ("
        "   id UUID PRIMARY KEY,"
        "   full_name VARCHAR(100) NOT NULL,"
        "   license_expiry_date DATE NOT NULL,"
        "   medical_expiry_date DATE NOT NULL,"
        "   allowed_models_json JSONB"
        ");"
    );
    if (query.lastError().isValid()) qDebug() << "Table 'pilots' error:" << query.lastError().text();

    // 4. Типы неисправностей (Справочник MEL)
    success &= query.exec(
        "CREATE TABLE IF NOT EXISTS defect_types ("
        "   id UUID PRIMARY KEY,"
        "   description TEXT NOT NULL,"
        "   severity VARCHAR(20) CHECK (severity IN ('CRITICAL', 'MINOR'))"
        ");"
    );
    if (query.lastError().isValid()) qDebug() << "Table 'defect_types' error:" << query.lastError().text();

    // 5. Активные дефекты (Связь М:М)
    success &= query.exec(
        "CREATE TABLE IF NOT EXISTS active_defects ("
        "   id UUID PRIMARY KEY,"
        "   aircraft_id UUID REFERENCES aircrafts(id),"
        "   defect_type_id UUID REFERENCES defect_types(id),"
        "   created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");"
    );
    if (query.lastError().isValid()) qDebug() << "Table 'active_defects' error:" << query.lastError().text();

    if (success) {
        qDebug() << "Database tables initialized successfully.";
    }
}

QSqlDatabase DatabaseManager::getDatabase() const {
    return m_db;
}
