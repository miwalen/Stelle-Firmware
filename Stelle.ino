#include "GeoLib.h"
#include "MechLib.h"
#include "Communication.h"
#include "ConfigManager.h"
#include "Packets.h"
#include "Logger.h" 
#include "Mutexes.h"
#include "PowerManager.h"
#include "SensorReader.h"
#include "ADS1115.h"
#include "ForwardKinematicsComputer.h"

Logger log1;

InputChannels channels;
ConfigManager config;

PowerState currentPowerState(27);

TwoWire I2C_1(1);

ADS1115 ads1;
ADS1115 ads2;

Vector3 offsetPos;
Quaternion offsetRot;
Quaternion offsetRotIMU;

KinematicChain Kinematics;

void setup() {
  initMutexes();
  initTaskPowerKeeper();

  log1.begin();
  I2C_1.begin(25, 26);
  config.begin();

  initTaskSerialComA();
  initTaskSerialUSB();
  initTaskForwardKinematicsComputer();
  initTaskSensorReader();

}

void loop() {

}