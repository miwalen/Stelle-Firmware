#include "PowerManager.h"

TaskHandle_t TaskPowerKeeperHandle = NULL;

void PowerState::setPin(uint8_t pin)
{
    this->pin = pin;
}

void PowerState::setPowerState(bool state)
{
    this->powerState = state;
}

uint8_t PowerState::getPin()
{
    return pin;
}

bool PowerState::getPowerState()
{
    return powerState;
}

void initTaskPowerKeeper()
{
    xTaskCreatePinnedToCore(
        PowerKeeper,
        "PowerKeeper_Task",
        2048,
        NULL,
        1,
        &TaskPowerKeeperHandle,
        1
    );
}

void PowerKeeper(void *parameter)
{
    for (;;)
    {
        if (xSemaphoreTake(mutexPowerState, portMAX_DELAY))
        {
            digitalWrite(
                currentPowerState.getPin(),
                currentPowerState.getPowerState()
            );

            xSemaphoreGive(mutexPowerState);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}