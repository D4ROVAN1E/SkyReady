#ifndef PILOTREPOSITORY_H
#define PILOTREPOSITORY_H

#include "src/repositories/IRepository.h"
#include "src/models/Entities.h"
#include <QSqlDatabase>

class PilotRepository : public IRepository<Pilot> {
public:
    PilotRepository();

    // Стандартные методы
    std::vector<Pilot> getAll() override;
    Pilot getById(QUuid id) override;

    // Создание пилота
    bool create(const Pilot& pilot);

    bool deleteById(QUuid id);

    // Поиск пилота по имени
    std::vector<Pilot> findByName(const QString& namePart);

    void deleteAll();

private:
    Pilot mapToEntity(const class QSqlQuery& query);
};

#endif // PILOTREPOSITORY_H
