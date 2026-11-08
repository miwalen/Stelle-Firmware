#ifndef Communication_H
#define Communication_H

#include <Arduino.h>
#include <Stream.h>
#include "ConfigManager.h"
#include "Packets.h"
#include "Mutexes.h"
#include "Logger.h" 
#include "PowerManager.h"

#define HEADER_1 0xAA
#define HEADER_2 0x55

extern PowerState currentPowerState;
extern ConfigManager config; 
extern KinematicChain Kinematics;
extern InputChannels channels; 
extern Logger log1;
void executeCommand(Stream& port, uint8_t cmd, uint8_t* payload, uint8_t len);

void initTaskSerialComA();
void initTaskSerialUSB();

#endif // COMMUNICATION_H