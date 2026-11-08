#ifndef PowerManager_H
#define PowerManager_H

#include "ConfigManager.h"
#include "Packets.h"
#include "Mutexes.h"
#include "Logger.h"
#include <Arduino.h>

class PowerState {
private:
    uint8_t pin;
    bool powerState = true;

public:
    PowerState(uint8_t pin) {
        setPin(pin);
        pinMode(pin, OUTPUT);
    }

    void setPin(uint8_t pin);
    void setPowerState(bool state);

    uint8_t getPin();
    bool getPowerState();
};

extern PowerState currentPowerState;

void PowerKeeper(void *parameter);
void initTaskPowerKeeper();

#endif