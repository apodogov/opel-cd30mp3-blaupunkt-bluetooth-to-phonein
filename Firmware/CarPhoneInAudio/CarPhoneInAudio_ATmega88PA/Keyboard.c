#include "Keyboard.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Log.h"
#include "Timer.h"

#define RowsInPortD_Mask		0b00001100
#define RowsInPortC_Mask		0b00110000
#define ColsInPortC_Mask		0b00000111
#define OnOffInPortC_Mask		0b00001000

#define RowsNumber		4
#define ColsNumber		3

#define ButtonFromRowCol(r, c)		((r << 4) | c)
#define RowFromButton(b)			(b >> 4)
#define ColFromButton(b)			(b & 0x0F)

static uint8_t states[RowsNumber] = { ColsInPortC_Mask, ColsInPortC_Mask, ColsInPortC_Mask, ColsInPortC_Mask };
static uint8_t onOffState = OnOffInPortC_Mask;

#define ButtonPressEmulationDuration		MsToTimerTicks(200)

static uint8_t emulationTimer = 0;
static uint8_t emulationRowMask;
static uint8_t emulationColMask;

// When Keyboard is not checked by EHU for 2 seconds, it means that EHU was OFF.
#define DeactivationDetectionDuration		MsToTimerTicks(2 * 1000)

// Deactivation timer is also used as indicator of activity. When it is set, EHU is active.
static uint8_t deactivationTimer = 0;

void Keyboard_Init()
{
	// Two of four Rows (EHU's outs).
	DDRD &= ~RowsInPortD_Mask;
	
	// Two of four Rows (EHU's outs) & Columns (EHU's inputs, MCU's inputs/outputs) & ON/OFF button.
	DDRC = ~(RowsInPortC_Mask | ColsInPortC_Mask | OnOffInPortC_Mask);
	
	// Outputs in EHU are either low level or just left open. So, MCU can't understand real values.
	// To fix this we have to enable pull-ups. Theoretically it is normal state because when a button
	// in the matrix is pressed, these outputs are pulled-up through this button inside the EHU.
	// WARNING! EHU's MCU work on 3.3V, so we must not enable pull-ups on Arduino board that is powered by 5V.
	PORTD |= RowsInPortD_Mask;
	PORTC = RowsInPortC_Mask;
	
#ifdef KB_INPUT_PULLUPS
	// The EHU has its own pull-ups on inputs, but its voltage is ~3.0-3.3 Volts.
	// Arduino board powers MCU by 5v, so EHU's voltage is too small to detect high level.
	// Enabling pull-ups on these input pins fixes the problem.
	// WARNING! EHU's MCU work on 3.3V, so we must not enable pull-ups on Arduino board that is powered by 5V.
	PORTC = 0xFF;
#endif
	
	// Do not interrupt on EHU's inputs change to avoid redundancy.
	// Interrupt on:
	// Rows: D2-D3 (PCINT18-19), C4-C5 (PCINT12-13)
	// ON/OFF: C3 (PCINT11)
	PCMSK2 = RowsInPortD_Mask;
	PCMSK1 = RowsInPortC_Mask | OnOffInPortC_Mask;
	PCICR = (1 << PCIE2) | (1 << PCIE1);
}

inline void ResetOut()
{
	DDRC = ~(RowsInPortC_Mask | ColsInPortC_Mask | OnOffInPortC_Mask);
#ifdef KB_INPUT_PULLUPS
	PORTC = 0xFF;
#endif
}

void Keyboard_Tick()
{
	if (emulationTimer)
	{
		if (--emulationTimer == 0)
		{
			ResetOut();
			
			LogLine("Emulate stop");
		}
	}
	
	if (deactivationTimer)
	{
		if (--deactivationTimer == 0)
		{
			OnDeactivated();
		}
	}
}

bool Keyboard_IsActive()
{
	return (deactivationTimer != 0);
}

void Keyboard_SimulateButton(Button button)
{
	// Release any previously pressed button.
	ResetOut();
	
	emulationTimer = ButtonPressEmulationDuration;
	emulationRowMask = (1 << RowFromButton(button));
	emulationColMask = (1 << ColFromButton(button));
	
	LogText("Emulate press: ");
	LogInt(button >> 4);
	LogText(" - ");
	LogIntLn(button & 0x0F);
}

uint8_t debug = 100;

void CheckButtons()
{
	uint8_t pinD = PIND;
	uint8_t pinC = PINC;
	
	if (++debug == 0) debug = 100;
	
	// If EHU was inactive and started to check buttons, then it was activated.
	// Each time reset deactivation timer to detect deactivation when it stops to check buttons.
	if (deactivationTimer == 0) OnActivated();
	deactivationTimer = DeactivationDetectionDuration;

	uint8_t rows = ((pinC & RowsInPortC_Mask) >> 4);
	rows |= (pinD & RowsInPortD_Mask);
	
	uint8_t cols = pinC;

	if (emulationTimer)
	{
		if ((rows & emulationRowMask) == 0)
		{
#ifdef KB_INPUT_PULLUPS
			PORTC &= ~emulationColMask;
#endif
			DDRC |= emulationColMask;
		}
		else
		{
			ResetOut();
		}
		
		return;
	}
	
	for (uint8_t r = 0; r < RowsNumber; r++)
	{
		if ((rows & 0x01) == 0)
		{
			uint8_t newState = cols;
			uint8_t oldState = states[r];

			for (uint8_t c = 0; c < ColsNumber; c++)
			{
				uint8_t newBit  = newState & 0x01;
				uint8_t oldBit = oldState & 0x01;
				if (newBit != oldBit)
				{
					Button btn = ButtonFromRowCol(r, c);
					LogText("INT ");
					LogInt(debug);
					LogText(": ");
					if (newBit == 0) OnButtonPressed(btn);
					else OnButtonReleased(btn);
				}
				
				newState >>= 1;
				oldState >>= 1;
			}

			states[r] = cols;
		}
		
		rows >>= 1;
	}
	
	uint8_t onOff = cols & OnOffInPortC_Mask;
	if (onOff != onOffState)
	{
		if (onOff == 0) OnButtonPressed(ButtonOnOff);
		else OnButtonReleased(ButtonOnOff);
		
		onOffState = onOff;
	}
}

ISR(PCINT2_vect)
{
	CheckButtons();
}

ISR(PCINT1_vect)
{
	CheckButtons();
}
