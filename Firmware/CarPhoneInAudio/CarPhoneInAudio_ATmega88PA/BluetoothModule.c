#include "BluetoothModule.h"

#include <avr/io.h>

#include "Timer.h"

#define BluetoothModuleDDR					DDRD
#define BluetoothModulePORT					PORTD

#define BluetoothModuleActivationPIN		4
#define BluetoothModuleButtonPlayPausePIN	5
#define BluetoothModuleButtonPreviousPIN	6
#define BluetoothModuleButtonNextPIN		7

#define BluetoothModuleButtonsPinMask		((1 << BluetoothModuleButtonPlayPausePIN) | (1 << BluetoothModuleButtonPreviousPIN) | (1 << BluetoothModuleButtonNextPIN))

#define ButtonPressDuration			MsToTimerTicks(100)

#define DeactivationDelay			MsToTimerTicks(20 * 1000)

static uint8_t buttonPressTimer = 0;
static uint16_t deactivationTimer = 0;

void BluetoothModule_Init()
{
	BluetoothModulePORT = (BluetoothModulePORT & ~BluetoothModuleButtonsPinMask) | (1 << BluetoothModuleActivationPIN);
	BluetoothModuleDDR = (BluetoothModuleDDR & ~BluetoothModuleButtonsPinMask) | (1 << BluetoothModuleActivationPIN);
}

void BluetoothModule_Tick()
{
	if (buttonPressTimer)
	{
		if (--buttonPressTimer == 0)
		{
			BluetoothModuleDDR &= ~BluetoothModuleButtonsPinMask;
		}
	}
	
	if (deactivationTimer)
	{
		if (--deactivationTimer == 0)
		{
			BluetoothModulePORT |= (1 << BluetoothModuleActivationPIN);

			// Also, in case of deactivation, release any currently pressed button.
			BluetoothModuleDDR &= ~BluetoothModuleButtonsPinMask;
			buttonPressTimer = 0;
		}
	}
}

void BluetoothModule_Activate()
{
	BluetoothModulePORT &= ~(1 << BluetoothModuleActivationPIN);
	deactivationTimer = 0;
}

void BluetoothModule_Deactivate(bool force)
{
	if (force)
	{
		BluetoothModulePORT |= (1 << BluetoothModuleActivationPIN);
		deactivationTimer = 0;

		// Also, in case of deactivation, release any currently pressed button.
		BluetoothModuleDDR &= ~BluetoothModuleButtonsPinMask;
		buttonPressTimer = 0;
	}
	else
	{
		deactivationTimer = DeactivationDelay;
	}
}

void BluetoothModule_PressPlayPause()
{
	BluetoothModuleDDR = (BluetoothModuleDDR & ~BluetoothModuleButtonsPinMask) | (1 << BluetoothModuleButtonPlayPausePIN);
	buttonPressTimer = ButtonPressDuration;
}

void BluetoothModule_PressPrevious()
{
	BluetoothModuleDDR = (BluetoothModuleDDR & ~BluetoothModuleButtonsPinMask) | (1 << BluetoothModuleButtonPreviousPIN);
	buttonPressTimer = ButtonPressDuration;
}

void BluetoothModule_PressNext()
{
	BluetoothModuleDDR = (BluetoothModuleDDR & ~BluetoothModuleButtonsPinMask) | (1 << BluetoothModuleButtonNextPIN);
	buttonPressTimer = ButtonPressDuration;
}
