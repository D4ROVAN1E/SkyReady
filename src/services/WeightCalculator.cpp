#include "src/services/WeightCalculator.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QPointF>

// Если FUEL_DENSITY не определен в хедере, задаем стандартную плотность авиабензина
#ifndef FUEL_DENSITY
#define FUEL_DENSITY 0.72
#endif

WeightCalculator::WeightCalculator() {
}

BalanceResult WeightCalculator::calculate(const AircraftModel& model,
                                          const Aircraft& aircraft,
                                          const FlightParams& params)
{
    BalanceResult result;
    result.message = "";

    // 1. Настройка параметров центровки под конкретную модель
    double armEmpty, armFuel, armPilot;
    double cgMin, cgMax;

    // Используем modelName из объекта aircraft для выбора профиля
    if (aircraft.modelName.contains("Piper")) {
        armEmpty = 85.0;
        armFuel = 95.0;
        armPilot = 85.5;
        cgMin = 82.0;
        cgMax = 93.0;
    } else {
        armEmpty = 39.0;
        armFuel = 48.0;
        armPilot = 37.0;
        cgMin = 35.0;
        cgMax = 47.5;
    }

    // 2. Расчет весов
    double fuelWeight = params.fuelAmount * FUEL_DENSITY;
    double payloadWeight = params.cargoWeight;
    double emptyWeight = model.emptyWeight;

    result.totalWeight = emptyWeight + fuelWeight + payloadWeight;

    // 3. Проверка максимального взлетного веса (MTOW)
    if (result.totalWeight > model.maxTakeoffWeight) {
        result.isWeightOk = false;
        result.message += QString("Перегруз! Текущий вес: %1 кг (Макс: %2). \n")
                          .arg(result.totalWeight)
                          .arg(model.maxTakeoffWeight);
    } else {
        result.isWeightOk = true;
    }

    // 4. Расчет моментов (Weight * Arm) с использованием динамических плеч
    double momentEmpty = emptyWeight * armEmpty;
    double momentFuel = fuelWeight * armFuel;
    double momentPayload = payloadWeight * armPilot;

    double totalMoment = momentEmpty + momentFuel + momentPayload;

    // 5. Расчет центра тяжести (CG)
    if (result.totalWeight > 0) {
        result.cgPosition = totalMoment / result.totalWeight;
    } else {
        result.cgPosition = 0;
    }

    // 6. Проверка топлива (с запасом 10%)
    double hours = (double)params.flightTimeMinutes / 60.0;
    double fuelNeededLiters = (hours * model.fuelConsumption) * 1.1;

    if (params.fuelAmount < fuelNeededLiters) {
        result.isFuelOk = false;
        result.message += QString("Мало топлива! Нужно: %1 л (с запасом), В баках: %2 л. \n")
                          .arg(QString::number(fuelNeededLiters, 'f', 1))
                          .arg(params.fuelAmount);
    } else if (params.fuelAmount > model.fuelCapacity) {
        result.isFuelOk = false;
        result.message += QString("Топлива больше объема баков! Баки: %1 л. \n")
                          .arg(model.fuelCapacity);
    } else {
        result.isFuelOk = true;
    }

    // 7. Проверка конверта
    QPolygonF envelope;
    envelope << QPointF(cgMin, 0.0)
             << QPointF(cgMin, model.maxTakeoffWeight)
             << QPointF(cgMax, model.maxTakeoffWeight)
             << QPointF(cgMax, 0.0);

    if (envelope.containsPoint(QPointF(result.cgPosition, result.totalWeight), Qt::OddEvenFill)) {
        result.isCgOk = true;
    } else {
        result.isCgOk = false;
        result.message += QString("Нарушена центровка! CG: %1 (Допуск: %2-%3). \n")
                          .arg(QString::number(result.cgPosition, 'f', 1))
                          .arg(cgMin)
                          .arg(cgMax);
    }

    return result;
}

