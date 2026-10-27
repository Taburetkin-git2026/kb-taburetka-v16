#include "can_link.h"

void CanLink::init(uint8_t csPin) {
    mcp2515 = new MCP2515(csPin);
    mcp2515->reset();
    // Скорость сети: 500 Кбит/с, кварц на модуле MCP2515 стандартный — 8 МГц
    mcp2515->setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515->setNormalMode();
}

void CanLink::sendCoreTelemetry(uint16_t thermoRaw, uint16_t coValue) {
    can_frame frame;
    frame.can_id  = CAN_ID_TELEMETRY_CORE;
    frame.can_dlc = 4; // Передаем 4 байта данных
    frame.data[0] = (thermoRaw >> 8) & 0xFF;
    frame.data[1] = thermoRaw & 0xFF;
    frame.data[2] = (coValue >> 8) & 0xFF;
    frame.data[3] = coValue & 0xFF;
    mcp2515->sendMessage(&frame);
}

void CanLink::sendSaltTelemetry(uint16_t saltVoltage, uint8_t engineState) {
    can_frame frame;
    frame.can_id  = CAN_ID_TELEMETRY_SALT;
    frame.can_dlc = 3; // Передаем 3 байта данных
    frame.data[0] = (saltVoltage >> 8) & 0xFF;
    frame.data[1] = saltVoltage & 0xFF;
    frame.data[2] = engineState;
    mcp2515->sendMessage(&frame);
}

void CanLink::sendNemaTelemetry(uint16_t nemaSteps, uint8_t azotValveState) {
    can_frame frame;
    frame.can_id  = CAN_ID_TELEMETRY_NEMA;
    frame.can_dlc = 3; // Передаем 3 байта данных
    frame.data[0] = (nemaSteps >> 8) & 0xFF;
    frame.data[1] = nemaSteps & 0xFF;
    frame.data[2] = azotValveState;
    mcp2515->sendMessage(&frame);
}

bool CanLink::readCommand(uint8_t &cmdCode, uint16_t &targetStep) {
    can_frame frame;
    if (mcp2515->readMessage(&frame) == MCP2515::ERROR_OK) {
        // Жесткая проверка CAN ID и длины фрейма (строго 3 байта) от Linux
        if (frame.can_id == CAN_ID_COMMAND_LINUX && frame.can_dlc == 3) {
            cmdCode = frame.data[0];
            targetStep = ((uint16_t)frame.data[1] << 8) | frame.data[2];
            return true;
        }
    }
    return false;
}
