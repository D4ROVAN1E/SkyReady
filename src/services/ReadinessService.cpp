#include "src/services/ReadinessService.h"
#include <QVariant>
#include <QDebug>

ReadinessService::ReadinessService() {
}

ReadinessReport ReadinessService::checkReadiness(QUuid aircraftId, QUuid pilotId, const FlightParams& params) {
    ReadinessReport report;
    report.isReady = true; // По умолчанию считаем, что готов, пока не найдем проблему

    // 1. Проверка самолёта
    Aircraft aircraft = m_aircraftRepo.getById(aircraftId);
    if (aircraft.id.isNull()) {
        report.isReady = false;
        report.errors.append("Ошибка: Самолет не найден в базе данных.");
        return report; // Дальше проверять нет смысла
    }

    // Проверка ресурса двигателя
    // engineHoursNextService - это отметка, когда нужно делать ТО.
    // Остаток = NextService - Total
    double hoursRemaining = aircraft.engineHoursNextService - aircraft.engineHoursTotal;

    if (hoursRemaining <= 0) {
        report.isReady = false;
        report.errors.append(QString("Ресурс двигателя исчерпан! Переработка: %1 ч.").arg(qAbs(hoursRemaining)));
    } else if (hoursRemaining < 10.0) {
        report.warnings.append(QString("Внимание: Скоро ТО двигателя. Осталось %1 ч.").arg(hoursRemaining));
    }

    // 2. Проверка дефектов
    if (m_defectRepo.hasCriticalDefects(aircraft.id)) {
        report.isReady = false;
        report.errors.append("Запрет вылета: На борту есть КРИТИЧЕСКИЕ неисправности!");
    }

    int minorCount = m_defectRepo.countMinorDefects(aircraft.id);
    if (minorCount >= 3) {
        report.isReady = false;
        report.errors.append(QString("Запрет вылета: Превышен лимит мелких неисправностей (%1 из 3 допустимых).").arg(minorCount));
    } else if (minorCount > 0) {
        report.warnings.append(QString("На борту имеются мелкие неисправности: %1 шт.").arg(minorCount));
    }

    // 3. Проверка пилота
    Pilot pilot = m_pilotRepo.getById(pilotId);
    if (pilot.id.isNull()) {
        report.isReady = false;
        report.errors.append("Ошибка: Пилот не выбран или не найден.");
    } else {
        QDate today = QDate::currentDate();

        // Лицензия
        if (pilot.licenseExpiryDate < today) {
            report.isReady = false;
            report.errors.append(QString("Лицензия пилота истекла %1").arg(pilot.licenseExpiryDate.toString("dd.MM.yyyy")));
        } else if (pilot.licenseExpiryDate < today.addDays(30)) {
            report.warnings.append("Срок действия лицензии пилота истекает менее чем через месяц.");
        }

        // Медицина (ВЛЭК)
        if (pilot.medicalExpiryDate < today) {
            report.isReady = false;
            report.errors.append("Медицинская справка пилота просрочена.");
        }

        // Допуск на тип (Type Rating)
        if (!pilot.allowedModels.contains(aircraft.modelId)) {
            report.isReady = false;
            report.errors.append(QString("У пилота %1 нет допуска к управлению типом '%2'").arg(pilot.fullName, aircraft.modelName));
        }
    }

    // 4.Расчёт загрузки и топлива

    // Нам нужно получить полные данные модели (вес, расход, конверт) для калькулятора
    AircraftModel model = m_modelRepo.getById(aircraft.modelId);

    if (model.id.isNull()) {
        report.isReady = false;
        report.errors.append("Ошибка данных: Не найдены характеристики модели самолета.");
    } else {
        // Запускаем математический расчет
        BalanceResult calcResult = m_calculator.calculate(model, aircraft, params);

        // Собираем список отказавших систем
        QStringList failedSystems;
        if (!calcResult.isWeightOk) failedSystems << "Масса";
        if (!calcResult.isCgOk) failedSystems << "Центровка";
        if (!calcResult.isFuelOk) failedSystems << "Топливо";

        if (!failedSystems.isEmpty()) {
            // Если есть проблемы, формируем компактный отчет
            report.isReady = false;

            // 1. Заголовок с перечислением
            report.errors.append("Нарушения ограничений: " + failedSystems.join(", "));

            // 2. Детальный текст (один раз)
            if (!calcResult.message.isEmpty()) {
                report.errors.append(calcResult.message);
            }
        }
    }

    return report;
}
