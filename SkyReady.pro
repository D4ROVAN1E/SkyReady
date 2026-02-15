QT       += core gui sql widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += src

# Исходный код
SOURCES += \
    main.cpp \
    src/db/DatabaseManager.cpp \
    src/repositories/AircraftRepository.cpp \
    src/repositories/PilotRepository.cpp \
    src/services/ReadinessService.cpp \
    src/ui/MainWindow.cpp \
    src/repositories/DefectRepository.cpp \
    src/services/WeightCalculator.cpp \
    src/ui/FlightPreparationDialog.cpp \
    src/repositories/AircraftModelRepository.cpp \
    src/services/FleetService.cpp \
    src/ui/dialogs/AddAircraftDialog.cpp \
    src/ui/dialogs/AddPilotDialog.cpp \
    src/ui/dialogs/AddDefectDialog.cpp \
    src/ui/dialogs/MaintenanceDialog.cpp

# Заголовки
HEADERS += \
    src/models/Entities.h \
    src/db/DatabaseManager.h \
    src/repositories/IRepository.h \
    src/repositories/AircraftRepository.h \
    src/repositories/PilotRepository.h \
    src/services/ReadinessService.h \
    src/ui/MainWindow.h \
    src/repositories/DefectRepository.h \
    src/services/WeightCalculator.h \
    src/ui/FlightPreparationDialog.h \
    src/repositories/AircraftModelRepository.h \
    src/services/FleetService.h \
    src/ui/dialogs/AddAircraftDialog.h \
    src/ui/dialogs/AddPilotDialog.h \
    src/ui/dialogs/AddDefectDialog.h \
    src/ui/dialogs/MaintenanceDialog.h

TARGET = SkyReady
