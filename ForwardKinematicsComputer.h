#ifndef ForwardKinematicsComputer_H
#define ForwardKinematicsComputer_H

#include <Arduino.h>
#include "MechLib.h"
#include "Mutexes.h"
#include "Logger.h"

extern KinematicChain Kinematics;

void TaskForwardKinematicsComputer(void *parameter);
void initTaskForwardKinematicsComputer();

#endif // ForwardKinematicsComputer_H

