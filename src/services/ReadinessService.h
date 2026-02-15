#ifndef READINESSSERVICE_H
#define READINESSSERVICE_H

#include "src/models/Entities.h"
#include "src/repositories/AircraftRepository.h"
#include "src/repositories/PilotRepository.h"
#include "src/repositories/DefectRepository.h"
#include "src/services/WeightCalculator.h"
#include "src/repositories/AircraftModelRepository.h"

// Этот класс отвечает за принятие решения "Готов / Не готов"
class ReadinessService {
public:
    ReadinessService();
    ReadinessReport checkReadiness(QUuid aircraftId, QUuid pilotId, const FlightParams& params);  // Комплексная проверка перед вылетом

private:
    AircraftRepository m_aircraftRepo;
    PilotRepository m_pilotRepo;
    DefectRepository m_defectRepo;
    WeightCalculator m_calculator;
    AircraftModelRepository m_modelRepo;
};

#endif // READINESSSERVICE_H
