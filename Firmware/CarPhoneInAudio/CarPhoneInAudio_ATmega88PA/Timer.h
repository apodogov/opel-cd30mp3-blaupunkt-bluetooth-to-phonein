#pragma once

// System Clock: 16 MHz. Target: 12.5 ms (80 Hz)
// 20 us - /1 /1024 /195 /0 = 12.48 ms (80.128 Hz)

#define SystemPrescaler		1
#define TimerPrescaler		1024
#define TimerDivider		195
#define ProgramDivider		1

#define TimerFreq			(F_CPU / SystemPrescaler / TimerPrescaler / TimerDivider / ProgramDivider)
#define TimerPeriodMs		(1000 / TimerFreq)

#define MsToTimerTicks(ms)		(ms / TimerPeriodMs)

void Timer_Init();

extern void OnTimer();