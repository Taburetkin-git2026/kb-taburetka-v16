#ifndef KUVALDA_START_H
#define KUVALDA_START_H

#include <Arduino.h>

// Конспиративные пины управления катушками зажигания через MOSFET-ключи
#define IGNITION_PIN_1  5 
#define IGNITION_PIN_2  6

class KuvaldaStart {
public:
    void init(uint16_t pulseDelayMs, uint8_t sparkCount);
    void executePulse();
    bool isEngineRunning(uint16_t currentRpm);

private:
    uint16_t _pulseDelayMs; // Длительность одного разряда искры в микросекундах
    uint8_t _sparkCount;    // Количество импульсов в одной кувалда-пачке
};

#endif
