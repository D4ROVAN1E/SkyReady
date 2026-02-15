#ifndef ENTITIES_H
#define ENTITIES_H

#include <QString>
#include <QUuid>
#include <QDate>
#include <QJsonObject>
#include <vector>

// Модель самолета
struct AircraftModel {
    QUuid id;
    QString name;
    double maxTakeoffWeight;
    double emptyWeight;
    double fuelCapacity;
    double fuelConsumption; // л/час
};

// Конкретный борт
struct Aircraft {
    QUuid id;
    QUuid modelId;
    QString regNumber;
    double engineHoursTotal;
    double engineHoursNextService;
    QString modelName;
    double fuelCapacity;
};

// Пилот
struct Pilot {
    QUuid id;
    QString fullName;
    QDate licenseExpiryDate;
    QDate medicalExpiryDate;
    // Список ID моделей, которыми пилот может управлять
    // В базе это хранится как JSONB массив: ["uuid1", "uuid2"]
    QList<QUuid> allowedModels;
};

// Тип неисправности (из справочника)
struct DefectType {
    QUuid id;
    QString description;
    QString severity; // "CRITICAL" или "MINOR"
};

// Активная неисправность на борту
struct ActiveDefect {
    QUuid id;
    QUuid aircraftId;
    QUuid defectTypeId;
    QDateTime createdAt;
    QString description;
    QString severity;
};

// Параметры для расчета рейса (от пользователя)
struct FlightParams {
    double fuelAmount;      // Заливаемое топливо
    double cargoWeight;     // Груз + Пассажиры
    int flightTimeMinutes;  // Планируемое время
};

// Результат проверки
struct ReadinessReport {
    bool isReady;           // GO / NO-GO
    QStringList warnings;   // Список предупреждений (желтый статус)
    QStringList errors;     // Список причин отказа (красный статус)
};

#endif // ENTITIES_H
