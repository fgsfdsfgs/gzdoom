#pragma once

enum SwitchButton
{
    B_A,
    B_B,
    B_X,
    B_Y,
    B_LSTICK,
    B_RSTICK,
    B_L,
    B_R,
    B_ZL,
    B_ZR,
    B_PLUS,
    B_MINUS,
    B_DLEFT,
    B_DUP,
    B_DRIGHT,
    B_DDOWN,

    B_COUNT
};

int IN_Init(void);
void IN_Update(void);
void IN_Free(void);

int IN_ButtonPressed(int btn);
void IN_WaitForButton(int btn);
int IN_GetFirstButton(void);

int IN_ButtonToKey(int btn);
int IN_KeyToButton(int key);
