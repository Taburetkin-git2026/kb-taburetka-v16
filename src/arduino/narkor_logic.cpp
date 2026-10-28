#include <SPI.h>

// Константы пинов подключения оборудования
const int STEP_PIN = 3;       // Пин шага для драйвера NEMA 17
const int DIR_PIN = 4;        // Пин направления для драйвера NEMA 17
const int MQ9_PIN = A0;       // Аналоговый пин газового сенсора MQ-9
const int MAX6675_SO = 12;    // Пины для модуля термопары MAX6675
const int MAX6675_CS = 10;
const int MAX6675_SCK = 13;

// Рабочие параметры и аварийные пороги системы
const int TARGET_GAS_VAL = 400;     // Эталонный показатель концентрации CO/H2
const int THERMO_THRESHOLD = 800;   // Аварийный температурный порог (в условных единицах чипа)
int currentPosition = 0;            // Текущее положение ШРУС-клапана в шагах

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(MAX6675_CS, OUTPUT);
  digitalWrite(MAX6675_CS, HIGH);
  
  // Стартовое пусковое положение клапана от мелкого аккумулятора
  currentPosition = 0; 
}

// Функция ювелирного микро-шагового поворота ШРУС-клапана
void moveValve(int steps, bool direction) {
  digitalWrite(DIR_PIN, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(800);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(800);
  }
}

// Функция экстренного увода реторты в безопасный режим тления
void emergencyShutDown() {
  if (currentPosition > 0) {
    moveValve(currentPosition, LOW); // Полностью закрываем воздушную заслонку
    currentPosition = 0;
  }
}

void loop() {
  // 1. Считывание температуры газа с чипа MAX6675
  int rawTemp = 0;
  digitalWrite(MAX6675_CS, LOW);
  rawTemp = SPI.transfer(0x00) << 8;
  rawTemp |= SPI.transfer(0x00);
  digitalWrite(MAX6675_CS, HIGH);
  rawTemp = (rawTemp >> 3) & 0x0FFF;

  // 2. Аварийный термощит: защита от теплового разгона реторты
  if (rawTemp > THERMO_THRESHOLD) {
    emergencyShutDown();
    delay(5000); // Ожидание стабилизации и остывания
    return;
  }

  // 3. Цикл мониторинга смеси: опрос газового сенсора MQ-9
  int gasValue = analogRead(MQ9_PIN);

  // 4. Динамическое регулирование качества смеси через шаговик NEMA 17
  if (gasValue < TARGET_GAS_VAL && currentPosition < 200) {
    moveValve(5, HIGH); // Подмес воздуха для выравнивания пропорции
    currentPosition += 5;
  } 
  else if (gasValue > TARGET_GAS_VAL && currentPosition > 0) {
    moveValve(5, LOW);  // Прикрываем воздух, обогащая смесь газом
    currentPosition -= 5;
  }

  delay(200); // Тактовая задержка цикла бортового компьютера
}
