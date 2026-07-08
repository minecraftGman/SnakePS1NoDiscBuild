//Thanks to ORION for his amazing PSX library.
//This is almost directly taken from his library, from System.h and System.c.
//A good resource from ORION -> http://onorisoft.free.fr/retro.htm?psx/tutorial/tuto.htm

#include <psxpad.h>

int	SysPad, SysPadT;
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

void initializePad() {
	InitPAD(1);
}

void padReset(void) {
	SysPad = 0;
	SysPadT = 0;
}

void padUpdate(void){
	int	pad = ReadPAD(0);
	SysPadT = pad & (pad ^ SysPad);
	SysPad = pad;
}