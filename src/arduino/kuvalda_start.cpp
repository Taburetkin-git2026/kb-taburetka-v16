#include "kuvalda_start.h"

void KuvaldaStart::init(uint16_t pulseDelayMs, uint8_t sparkCount) {
    _pulseDelayMs = pulseDelayMs;
    _sparkCount = sparkCount;

    pinMode(IGNITION_PIN_1, OUTPUT);
    pinMode(IGNITION_PIN_2, OUTPUT);
    
    // Держим ключи закрытыми, катушки обесточены — защита от паразитной искры
    digitalWrite(IGNITION_PIN_1, LOW);
    digitalWrite(IGNITION_PIN_2, LOW);
}

void KuvaldaStart::executePulse() {
    // Лютый Приказ: одновременный залп по всем фронтам без опроса датчика Холла
    for (uint8_t i = 0; i < _sparkCount; i++) {
        // Мгновенный силовой накат на первичные обмотки катушек
        digitalWrite(IGNITION_PIN_1, HIGH);
        digitalWrite(IGNITION_PIN_2, HIGH);
        
        delayMicroseconds(_pulseDelayMs); // Время насыщения сердечника катушки
        
        // Разрыв контура — в этот миг высоковольтная кувалда бьет по свечам!
        digitalWrite(IGNITION_PIN_1, LOW);
        digitalWrite(IGNITION_PIN_2, LOW);
        
        delayMicroseconds(_pulseDelayMs); // Пауза между ударами в пачке
    }
}

bool KuvaldaStart::isEngineRunning(uint16_t currentRpm) {
    // Если обороты векового чугуна превысили 400 об/мин — пуск «с курантов» успешен,
    // мотор подхватил маршевый «супер-газ» и вышел из пускового режима.
    return (currentRpm > 400);
}
