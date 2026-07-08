#include <psxpad.h>
#include <psxapi.h>
#include <stdint.h>

int SysPad, SysPadT;
uint8_t padbuff[2][34];

void initializePad(void) {
    InitPAD(padbuff[0], 34, padbuff[1], 34);
    StartPAD();
    ChangeClearPAD(0);
}

void padReset(void) {
    SysPad = 0;
    SysPadT = 0;
}

void padUpdate(void) {
    PADTYPE *pad = (PADTYPE*)padbuff[0];
    int pad_state = 0;
    if (pad->stat == 0) {
        pad_state = ~pad->btn & 0xFFFF;
    }
    SysPadT = pad_state & (pad_state ^ SysPad);
    SysPad = pad_state;
}
