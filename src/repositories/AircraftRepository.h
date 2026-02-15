#ifndef AIRCRAFTREPOSITORY_H
#define AIRCRAFTREPOSITORY_H

#include "src/repositories/IRepository.h"
#include "src/models/Entities.h"
#include <QSqlDatabase>

class AircraftRepository : public IRepository<Aircraft> {
public:
    AircraftRepository();

    // Реализация интерфейса
    std::vector<Aircraft> getAll() override;
    Aircraft getById(QUuid id) override;

    // Специфичные методы
    Aircraft getByRegNumber(const QString& regNumber);

    // Обновление налета двигателя
    bool updateEngineHours(QUuid id, double hoursFlown);

    // Обновляет отметку следующего ТО
    bool updateNextService(QUuid id, double nextServiceHours);

    // Метод для создания самолета
    bool create(const Aircraft& aircraft);

    bool deleteById(QUuid id);

    void deleteAll();

private:
    // Вспомогательный метод для парсинга строки SQL ответа в структуру
    Aircraft mapToEntity(const class QSqlQuery& query);
};

#endif // AIRCRAFTREPOSITORY_H
