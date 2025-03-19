#include "Log.h"

#include <avr/io.h>

void Log_Init_Imp()
{
	PORTD &= ~(1 << PORTD1);
	DDRD |= (1 << DDD1);
	
	UBRR0 = F_CPU/8/500000 - 1;
	UCSR0A = (1 << U2X0);
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void LogChar(unsigned char c)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
	while (!(UCSR0A & (1 << TXC0)));
	UCSR0A |= (1 << TXC0);
}

void LogInt_Imp(uint16_t val)
{
	uint8_t buffer[5];
	uint8_t size = 0;
	
	if (val == 0)
	{
		buffer[size++] = 0;
	}
	
	while (val > 0)
	{
		buffer[size++] = val % 10;
		val /= 10;
	}
	
	while (size > 0)
	{
		size--;
		LogChar('0' + buffer[size]);
	}
}

void LogIntLn_Imp(uint16_t val)
{
	LogInt_Imp(val);
	LogChar('\n');
}

void LogText_Imp(const char* text)
{
	while (*text)
	{
		LogChar(*text);
		text++;
	}
}

void LogLine_Imp(const char* text)
{
	LogText_Imp(text);
	LogChar('\n');
}