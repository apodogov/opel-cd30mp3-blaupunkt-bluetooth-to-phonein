#include "TasksQueue.h"

Task queue[TasksQueueCapacity];

uint8_t firstIndex = 0;
uint8_t length = 0;

uint8_t currentDelay = 0;

void StartFirstTask()
{
	currentDelay = queue[firstIndex].delay;
}

void TasksQueue_Tick()
{
	if (currentDelay)
	{
		if (--currentDelay == 0)
		{
			Task task = queue[firstIndex];
			OnTask(task.type, task.parameter);
			
			if (++firstIndex == TasksQueueCapacity) firstIndex = 0;
			if (--length != 0) StartFirstTask();
		}
	}
}

bool AddTask(uint8_t delay, uint8_t taskType, uint8_t taskParameter)
{
	if (length == TasksQueueCapacity) return false;
	
	uint8_t nextTaskIndex = firstIndex + length;
	if (nextTaskIndex >= TasksQueueCapacity) nextTaskIndex -= TasksQueueCapacity;
	
	queue[nextTaskIndex].delay = delay;
	queue[nextTaskIndex].type = taskType;
	queue[nextTaskIndex].parameter = taskParameter;
	
	length++;
	
	// If do nothing, start task immediately.
	if (!currentDelay) StartFirstTask();
	return true;
}
