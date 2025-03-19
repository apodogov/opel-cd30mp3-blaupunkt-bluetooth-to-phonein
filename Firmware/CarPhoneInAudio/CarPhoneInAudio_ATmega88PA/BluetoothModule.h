#pragma once

#include <stdio.h>
#include <stdbool.h>

void BluetoothModule_Init();

void BluetoothModule_Tick();

void BluetoothModule_Activate();

// Because startup time of BT module takes some time, its deactivation is implemented with a delay.
// So, when a user switches sources quickly, BT will become active quickly too.
// However, the immediate deactivation can be forced.
void BluetoothModule_Deactivate(bool force);

void BluetoothModule_PressPlayPause();

void BluetoothModule_PressPrevious();

void BluetoothModule_PressNext();
