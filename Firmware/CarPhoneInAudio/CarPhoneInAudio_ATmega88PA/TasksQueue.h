#pragma once

#include <stdio.h>
#include <stdbool.h>

#define TasksQueueCapacity		10

typedef struct Task {
	uint8_t delay;
	uint8_t type;
	uint8_t parameter;
} Task;

void TasksQueue_Tick();

bool AddTask(uint8_t delay, uint8_t taskType, uint8_t taskParameter);

extern void OnTask(uint8_t taskType, uint8_t taskParameter);
