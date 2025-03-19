#include "TelMute.h"

#include <avr/io.h>

#define TelMuteDDR		DDRB
#define TelMutePORT		PORTB
#define TelMutePIN		0

static bool isActive = false;

void TelMute_Init()
{
	TelMutePORT &= ~(1 << TelMutePIN);

#ifdef TEL_MUTE_LOW
	// There were no transistor in the prototype on Arduino.
	TelMuteDDR &= ~(1 << TelMutePIN);
#else
	TelMuteDDR |= (1 << TelMutePIN);
#endif
}

void TelMute_Activate()
{
#ifdef TEL_MUTE_LOW
	TelMuteDDR |= (1 << TelMutePIN);
#else
	TelMutePORT |= (1 << TelMutePIN);
#endif
	isActive = true;
}

void TelMute_Deactivate()
{
#ifdef TEL_MUTE_LOW
	TelMuteDDR &= ~(1 << TelMutePIN);
#else
	TelMutePORT &= ~(1 << TelMutePIN);
#endif
	isActive = false;
}

bool TelMute_IsActive()
{
	return isActive;
}
