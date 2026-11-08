#ifndef SensorReader_H
#define SensorReader_H

#include <Arduino.h>
#include <Wire.h>
#include "Mutexes.h"
#include "Logger.h" 
#include "MechLib.h"
#include "ADS1115.h"

extern TwoWire I2C_1;
extern ADS1115 ads1;
extern ADS1115 ads2;
extern KinematicChain Kinematics;
extern Logger log1;

void TaskSensorReader(void *parameter);
void initTaskSensorReader();

#endif // SensorReader_H