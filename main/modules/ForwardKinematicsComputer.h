#pragma once

#include "freertos/FreeRTOS.h"

namespace stelle {
    
extern KinematicChain Kinematics;
void TaskForwardKinematicsComputer(void* parameter);
void initTaskForwardKinematicsComputer();

} // namespace stelle
