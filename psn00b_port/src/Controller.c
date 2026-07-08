//Thanks to ORION for his amazing PSX library.
//This is almost directly taken from his library, from System.h and System.c.
//A good resource from ORION -> http://onorisoft.free.fr/retro.htm?psx/tutorial/tuto.htm

#include <psxpad.h>
#include <stdint.h>

int	SysPad, SysPadT;
uint8_t padbuff[2][34];

#define	padCheck(_p_)	(SysPad & (_p_))
#define	padCheckPressed(_p_)	(SysPadT & (_p_))
#define Pad1Up			PAD_UP
#define Pad1Down		PAD_DOWN
#define Pad1Left		PAD_LEFT
#define Pad1Right		PAD_RIGHT
#define Pad1Triangle	PAD_TRIANGLE
#define Pad1Cross		PAD_CROSS
#define Pad1Square		PAD_SQUARE
#define Pad1Circle		PAD_CIRCLE
#define Pad1L1			PAD_L1
#define Pad1L2			PAD_L2
#define Pad1R1			PAD_R1
#define Pad1R2			PAD_R2
#define Pad1Start		PAD_START
#define Pad1Select		PAD_SELECT
#define Pad2Up			PAD_UP
#define Pad2Down		PAD_DOWN
#define Pad2Left		PAD_LEFT
#define Pad2Right		PAD_RIGHT
#define Pad2Triangle	PAD_TRIANGLE
#define Pad2Cross		PAD_CROSS
#define Pad2Square		PAD_SQUARE
#define Pad2Circle		PAD_CIRCLE
#define Pad2L1			PAD_L1
#define Pad2L2			PAD_L2
#define Pad2R1			PAD_R1
#define Pad2R2			PAD_R2
#define Pad2Start		PAD_START
#define Pad2Select		PAD_SELECT

void initializePad(void) {
	InitPAD(padbuff[0], 34, padbuff[1], 34);
	StartPAD();
	ChangeClearPAD(0);
}

void padReset(void) {
	SysPad = 0;
	SysPadT = 0;
}

void padUpdate(void){
	PADTYPE *pad = (PADTYPE*)padbuff[0];
	int pad_state = 0;
	if (pad->stat == 0) {
		pad_state = ~pad->btn & 0xFFFF;
	}
	SysPadT = pad_state & (pad_state ^ SysPad);
	SysPad = pad_state;
}
