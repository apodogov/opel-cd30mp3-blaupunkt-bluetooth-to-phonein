#pragma once

#include <stdio.h>

#ifdef LOG

#define Log_Init()				Log_Init_Imp()
#define LogInt(...)				LogInt_Imp(__VA_ARGS__)
#define LogIntLn(...)			LogIntLn_Imp(__VA_ARGS__)
#define LogText(...)			LogText_Imp(__VA_ARGS__)
#define LogLine(...)			LogLine_Imp(__VA_ARGS__)

#else

#define Log_Init()
#define LogInt(...)
#define LogIntLn(...)
#define LogText(...)
#define LogLine(...)

#endif

void Log_Init_Imp();

void LogChar(unsigned char c);

void LogInt_Imp(uint16_t val);

void LogIntLn_Imp(uint16_t val);

void LogText_Imp(const char* text);

void LogLine_Imp(const char* text);