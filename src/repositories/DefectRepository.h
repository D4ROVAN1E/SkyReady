#ifndef DEFECTREPOSITORY_H
#define DEFECTREPOSITORY_H

#include "src/models/Entities.h"
#include <vector>
#include <QUuid>

class DefectRepository {
public:
    DefectRepository();

    // Получить весь список возможных поломок (для выпадающего списка)
    std::vector<DefectType> getAllDefectTypes();

    // Инициализация БД начальными значениями (20 типовых поломок)
    void ensureDefaultDefectsExist();

    // Добавить новую поломку на самолет
    bool addActiveDefect(QUuid aircraftId, QUuid defectTypeId);

    // Удалить поломку
    bool removeActiveDefect(QUuid defectId);

    // Получить список всех поломок конкретного самолета
    std::vector<ActiveDefect> getByAircraftId(QUuid aircraftId);

    // Подсчет количества легких дефектов
    int countMinorDefects(QUuid aircraftId);

    // Проверка наличия хотя бы одного критического дефекта
    bool hasCriticalDefects(QUuid aircraftId);

    void deleteAllActive();
    void deleteActiveByAircraftId(QUuid aircraftId);
};

#endif // DEFECTREPOSITORY_H
