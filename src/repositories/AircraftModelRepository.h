#ifndef AIRCRAFTMODELREPOSITORY_H
#define AIRCRAFTMODELREPOSITORY_H

#include "src/repositories/IRepository.h"
#include "src/models/Entities.h"
#include <QSqlDatabase>

class AircraftModelRepository : public IRepository<AircraftModel> {
public:
    AircraftModelRepository();

    // Реализация интерфейса
    std::vector<AircraftModel> getAll() override;
    AircraftModel getById(QUuid id) override;

    // Метод создания нового типа ВС
    bool create(const AircraftModel& model);

    void deleteAll();

private:
    AircraftModel mapToEntity(const class QSqlQuery& query);
};

#endif // AIRCRAFTMODELREPOSITORY_H
