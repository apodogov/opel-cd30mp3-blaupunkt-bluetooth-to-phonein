#include "Timer.h"

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void Timer_Init()
{
	TCCR2A = (1 << WGM21); // CTC, TOP = OCR2A
	OCR2A = TimerDivider -1; // Timer Divider
	TIMSK2 = (1 << OCIE2A); // Compare match Interrupt
#if (TimerPrescaler == 1024)
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); // Enable Timer with Prescaler /1024
#else
#error "Timer prescaler is not supported."
#endif
}

ISR(TIMER2_COMPA_vect)
{
	OnTimer();
}