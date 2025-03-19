// Buttons matrix (connected only):
//
// ------------------------------------------------------------------
// | Index ---------> |  0         1         2         | 3          |
// | |  EHU Pin ----> |  x1 (12)   x2 (13)   x4 (15)   | ON (19)    |
// | |  |  MCU Pin -> |  C0        C1        C2        | C3         |
// | ?  ?       ?     |                                |            |
// ------------------------------------------------------------------
// | 0  0x (7)  C4    | "5"       "Up"      "CD"       |            |
// | 1  1x (8)  C5    | "6"       "Left"    "FMAM"     |            |
// | 2  3x (10) D2    |                     "BC"       |            |
// | 3  4x (11) D3    | "Right"   "Bass"               |            |
// ------------------------------------------------------------------
// | 4  GND (2) GND   |                                | "ON"/"OFF" |
// ------------------------------------------------------------------
//
// *C3 - Power ON/OFF has dedicated pin and pulled low when pressed.

#pragma once

#include <stdio.h>
#include <stdbool.h>

// 4 MSBs - Row index (Keyboard's output); 4 LSBs - Column index (Keyboard's input)
typedef enum Button {
	ButtonNone  = 0x00,
	ButtonCD    = 0x02,
	ButtonLeft  = 0x11,
	ButtonFMAM  = 0x12,
	ButtonBC    = 0x22,
	ButtonRight = 0x30,
	ButtonBass  = 0x31,
	ButtonOnOff = 0x43,
} Button;

void Keyboard_Init();

void Keyboard_Tick();

// Keyboard is used to detect EHU's activity. If EHU checks buttons, its means that it is active.
// We register deactivation after a little delay after EHU stops checking.
bool Keyboard_IsActive();

// The simulation of On/Off button is not supported now.
void Keyboard_SimulateButton(Button button);

extern void OnButtonPressed(Button button);

extern void OnButtonReleased(Button button);

extern void OnActivated();

extern void OnDeactivated();
