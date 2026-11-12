#include "PowerManager.h"

#include "Mutexes.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace stelle {

TaskHandle_t TaskPowerKeeperHandle = NULL;

void PowerState::begin() {
    const gpio_config_t io = {
        .pin_bit_mask = 1ULL << PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    gpio_set_level(static_cast<gpio_num_t>(PIN), powerState ? 1 : 0);
}

void initTaskPowerKeeper() {
    currentPowerState.begin();
    xTaskCreatePinnedToCore(PowerKeeper, "PowerKeeper_Task", 2048, NULL, 1,
                            &TaskPowerKeeperHandle, 1);
}

void PowerKeeper(void* parameter) {
    const gpio_num_t pin = static_cast<gpio_num_t>(PowerState::PIN);
    const TickType_t period = pdMS_TO_TICKS(CONFIG_STELLE_POWER_POLL_MS);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        if (xSemaphoreTake(mutexPowerState, portMAX_DELAY)) {
            const bool state = currentPowerState.getPowerState();
            gpio_set_level(pin, state ? 1 : 0);
            currentPowerState.applied = state;
            xSemaphoreGive(mutexPowerState);
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

} // namespace stelle