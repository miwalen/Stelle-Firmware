#pragma once

#include <cstdint>

namespace stelle {

// Pin rail daya (GPIO27) + status on/off yang dibaca PowerKeeper.
class PowerState {
public:
    static constexpr uint8_t PIN = 27;

    // Status yang terakhir benar-benar ditulis ke pin oleh PowerKeeper.
    bool applied = true;

    void begin();
    void setPowerState(bool state) { powerState = state; }
    bool getPowerState() const { return powerState; }

private:
    bool powerState = true;
};

// Instance global, seperti deklarasi di Stelle.ino.
extern PowerState currentPowerState;

void PowerKeeper(void* parameter);
void initTaskPowerKeeper();

} // namespace stelle
