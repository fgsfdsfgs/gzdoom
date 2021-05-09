#include "config.h"
#include "utils.h"
#include "input.h"

static PadState padstate;
static u64 pad, pad_old;

int IN_Init(void)
{
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&padstate);
    padUpdate(&padstate);
    pad = padGetButtons(&padstate);
    return 0;
}

void IN_Update(void)
{
    appletMainLoop();
    pad_old = pad;
    padUpdate(&padstate);
    pad = padGetButtons(&padstate);
}

void IN_Free(void)
{
}

int IN_ButtonPressed(int btn)
{
    return (pad & BIT(btn)) && !(pad_old & BIT(btn));
}

void IN_WaitForButton(int btn)
{
    do
    {
        IN_Update();
    }
    while (!IN_ButtonPressed(btn));
}

int IN_GetFirstButton(void)
{
    while (1)
    {
        IN_Update();
        for (int b = B_A; b < B_COUNT; ++b)
        {
            if (IN_ButtonPressed(b))
                return b;
        }
    }

    return -1;
}

int IN_ButtonToKey(int btn)
{
    return btn; // TODO: convert to GZDoom keys here
}

int IN_KeyToButton(int key)
{
    return key; // TODO: convert from GZDoom keys here
}
