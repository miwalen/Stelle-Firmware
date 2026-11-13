#ifndef SENSORREADER_H
#define SENSORREADER_H

#include "freertos/FreeRTOS.h"

namespace stelle {

void TaskSensorReader(void* parameter);
void initTaskSensorReader();

} // namespace stelle

#endif // SENSORREADER_H
