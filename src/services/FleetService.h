#ifndef FLEETSERVICE_H
#define FLEETSERVICE_H

#include "src/models/Entities.h"
#include "src/repositories/AircraftRepository.h"
#include "src/repositories/AircraftModelRepository.h"
#include "src/repositories/PilotRepository.h"
#include "src/repositories/DefectRepository.h"

// Сервис управления флотом: отвечает за добавление и изменение данных
class FleetService {
public:
    FleetService();

    // Основной метод для автоматического заполнения базы демо-данными
    bool seedDemoData();

    // Методы управления флотом
    bool registerAircraft(const Aircraft& aircraft);
    bool registerPilot(const Pilot& pilot);
    bool reportDefect(QUuid aircraftId, QUuid defectTypeId);

    // Удаляет все оперативные данные (самолеты, пилоты, полеты, дефекты),
    // но сохраняет справочники (модели самолетов, типы дефектов).
    bool clearFleetData();

    // Фиксация совершенного рейса (Транзакция)
    // Обновляет налет самолета и создает запись в истории
    bool commitFlight(QUuid aircraftId, int flightTimeMinutes);
    bool resolveDefect(QUuid activeDefectId);

    // Провести регламентное обслуживание двигателя
    bool performEngineMaintenance(QUuid aircraftId);

    // Удаляет самолет и всю его историю (полеты, дефекты)
    bool deleteAircraft(QUuid aircraftId);

    // Удаляет пилота и его историю полетов
    bool deletePilot(QUuid pilotId);

private:
    AircraftRepository m_aircraftRepo;
    AircraftModelRepository m_modelRepo;
    PilotRepository m_pilotRepo;
    DefectRepository m_defectRepo;
};

#endif // FLEETSERVICE_H
