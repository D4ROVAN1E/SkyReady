#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

class DatabaseManager {
public:
    // Singleton pattern
    static DatabaseManager& instance();

    // Подключение к PostgreSQL
    // Возвращает true, если подключение успешно
    bool connectToDatabase();

    // Создание структуры таблиц, если их нет
    void initDatabase();

    // Получение текущего объекта базы
    QSqlDatabase getDatabase() const;

private:
    DatabaseManager();
    ~DatabaseManager();
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
