// Fuses:
//   Extended: 0xF9 (default)
//   High: 0xDF -> 0xD7
//     Preserve EEPROM
//   Low: 0x62 -> 0xFF
//     Don't divide clock by 8
//     Low Power Crystal Oscillator 16 MHz
//     Slowly rising power (just as the safest option)

#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "Log.h"
#include "Timer.h"
#include "Keyboard.h"
#include "TelMute.h"
#include "BluetoothModule.h"
#include "TasksQueue.h"

Button lastKnownSource = ButtonNone;

typedef enum TaskType {
	TaskTypeButtonEmulation,
	TaskTypeTelMuteActivation,
} TaskType;

int main(void)
{
	// Turn off all periphery to minimize power consumption.

	// Analog to Digital Converter.
	ADCSRA &= ~(1 << ADEN);

	// Analog Comparator.
	ACSR |= (1 << ACD);

	// Brown-out Detector is disabled by fuses.
	// Internal Voltage Reference is disabled when ADC, AC and BOD are disabled.

	// Watchdog Timer.
	wdt_disable();

	// Digital input buffers are used.

	// On-chip Debug System is disabled by fuses.

	// Power Reduction Register.
#ifdef LOG
	// USART is used for logging.
	PRR = (1 << PRTWI) | (1 << PRTIM0) | (1 << PRTIM1) | (1 << PRSPI) | (1 << PRADC);
#else
	PRR = (1 << PRTWI) | (1 << PRTIM0) | (1 << PRTIM1) | (1 << PRSPI) | (1 << PRUSART0) | (1 << PRADC);
#endif
	
	// On 8 MHz, pressing of some buttons was detected as continuous press and release sequence.
	// So it was decided not to use internal RC oscillator.
	
	Log_Init();
	LogLine("Start");

	Keyboard_Init();
	TelMute_Init();
	BluetoothModule_Init();
	Timer_Init();
	
	sei();	
	while (1)
	{
		if (!Keyboard_IsActive())
		{
			// When EHU is inactive, controller can enter deep sleep mode.

			// Before entering deep sleep, ensure that BT module is turned off.
			BluetoothModule_Deactivate(true);
			
			// MCU can wake on pin change. Here the time of wake-up is not critical.
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_mode();
		}
		else
		{
			// When EHU is active, MCU must wake quickly to scan/emulate pressed keys.
			// Also, MCU should be able to wake periodically on timer.
			set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
			sleep_mode();
		}
	}
}

void OnTimer()
{
	Keyboard_Tick();
	BluetoothModule_Tick();
	TasksQueue_Tick();
}

void OnActivated()
{
	LogLine("Activated");
}

void OnDeactivated()
{
	LogLine("Deactivated");
}

void OnButtonPressed(Button button)
{
	LogText("Pressed: ");
	LogInt(button >> 4);
	LogText(" - ");
	LogIntLn(button & 0x0F);

	if (!TelMute_IsActive())
	{
		// For now this is the only way to know the last active source.
		if ((button == ButtonFMAM) || (button == ButtonCD))
		{
			lastKnownSource = button;
		}
	}
	else
	{
		if (button == ButtonBC)
		{
			BluetoothModule_PressPlayPause();
		}
				
		if (button == ButtonLeft)
		{
			BluetoothModule_PressPrevious();
		}

		if (button == ButtonRight)
		{
			BluetoothModule_PressNext();
		}
	}
}

void OnButtonReleased(Button button)
{
	LogText("Released: ");
	LogInt(button >> 4);
	LogText(" - ");
	LogIntLn(button & 0x0F);
	
	if (!TelMute_IsActive())
	{
		if (button == ButtonBC)
		{
			BluetoothModule_Activate();

			// When TelMute is activated in CD mode, the CD continues to spin.
			// So, when the last source is CD or unknown, switch to FM first.
			// Otherwise, activate TelMute immediately.
			if (!lastKnownSource || (lastKnownSource == ButtonCD))
			{
				AddTask(MsToTimerTicks(200), TaskTypeButtonEmulation, ButtonFMAM);
				AddTask(MsToTimerTicks(300), TaskTypeTelMuteActivation, 0);
				
				lastKnownSource = ButtonFMAM;
			}
			else
			{
				TelMute_Activate();
			}
			
			// Press and release Bass button to turn off "Voice" equalizer after Phone Input became active.
			AddTask(MsToTimerTicks(500), TaskTypeButtonEmulation, ButtonBass);
			AddTask(MsToTimerTicks(400), TaskTypeButtonEmulation, ButtonBass);
		}
	}
	else
	{
		if ((button == ButtonFMAM) || (button == ButtonCD) || (button == ButtonOnOff))
		{
			TelMute_Deactivate();
			BluetoothModule_Deactivate(false);
			
			if ((button == ButtonCD))
			{
				if (lastKnownSource && (lastKnownSource != ButtonCD))
				{
					AddTask(MsToTimerTicks(200), TaskTypeButtonEmulation, ButtonCD);
				}
				
				lastKnownSource = ButtonCD;
			}
		}
	}
}

void OnTask(uint8_t taskType, uint8_t taskParameter)
{
	TaskType type = (TaskType)taskType;
	if (type == TaskTypeButtonEmulation)
	{
		Keyboard_SimulateButton((Button)taskParameter);
	}
	else if (type == TaskTypeTelMuteActivation)
	{
		TelMute_Activate();
	}
}