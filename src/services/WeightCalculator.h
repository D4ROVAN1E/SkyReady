#ifndef WEIGHTCALCULATOR_H
#define WEIGHTCALCULATOR_H

#include "src/models/Entities.h"
#include <QString>
#include <QPolygonF>
#include <QJsonObject>

// Результат расчетов
struct BalanceResult {
    double totalWeight;  // Общий взлетный вес
    double cgPosition;  // Центр тяжести
    bool isWeightOk;  // Не перегружен ли самолет
    bool isCgOk;  // Попадает ли центровка в конверт
    bool isFuelOk;  // Хватает ли топлива на полет
    QString message;  // Текстовое пояснение
};

class WeightCalculator {
public:
    WeightCalculator();
    BalanceResult calculate(const AircraftModel& model, //Основной метод расчета
                            const Aircraft& aircraft,
                            const FlightParams& params);

private:
    // Плотность авиационного бензина (AvGas 100LL) ~ 0.72 кг/л
    const double FUEL_DENSITY = 0.72;

    // Стандартные плечи (расстояния от носа до груза) для упрощенной модели
    const double ARM_EMPTY = 38.0;  // Плечо пустого самолета
    const double ARM_PILOT = 37.0;  // Плечо передних кресел
    const double ARM_FUEL = 48.0;  // Плечо баков
    const double ARM_CARGO = 95.0;  // Плечо багажника
};

#endif // WEIGHTCALCULATOR_H
