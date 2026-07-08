#include <STDLIB.H>
#include <STDIO.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBGS.H>
#include <LIBETC.H>
#include <LIBSPU.H>
#include <LIBDS.H>
#include <STRINGS.H>
#include <SYS/TYPES.H>
#include <libapi.h>
#include <libcd.h>
#include <libsnd.h>

#define RCntIntr		0x1000				/*Interrupt mode*/
#define true 1
#define false 0

//Declare Stuff Here
void clear_vram();
void ConvertSec(int);
void ResetCycles();
void cbvsync();

// Screen resolution and dither mode
int SCREEN_WIDTH, SCREEN_HEIGHT;
#define CENTERX	SCREEN_WIDTH/2
#define CENTERY	SCREEN_HEIGHT/2
#define DITHER 1

// Increasing this value (max is 14) reduces sorting errors in certain cases
#define OT_LENGTH	14
#define OT_ENTRIES	1<<OT_LENGTH
#define PACKETMAX	2048

typedef struct {
	int r;
	int g;
	int b;
} Color;

// Camera coordinates
struct {
	VECTOR	position;
	SVECTOR rotation;
	GsCOORDINATE2 coord2;
} Camera;

struct {
    int frames;
    int framesSinceBeginning;
	int seconds;
	int minutes;
	int hours;
	int days;
} time;

GsOT		orderingTable[2];
GsOT_TAG	orderingTable_TAG[2][OT_ENTRIES];
int			myActiveBuff=0;
int			myLastBuff=0;
volatile u_long VsyncWait, Frame;
u_long sync = 0;
PACKET GPUOutputPacket[2][PACKETMAX*24];
Color BGColor;
int nextVSyncCount;

void TimeUpdate () {

    //this timer is VERY inaccurate. It's based on the frame rate.
    //This will have to do until I figure out a better way.
    time.framesSinceBeginning += 1;
    time.frames += 1;

    ConvertSec (time.frames / 60);

    FntPrint("Timer: %d:%d:%d \n", time.hours, time.minutes, time.seconds);

}

int getRandom (int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

void ConvertSec (int s) {

    time.days = s / 86400;

    s = s % 86400;
    time.hours = s / 3600;

    s %= 3600;
    time.minutes = s / 60 ;

    s %= 60;
    time.seconds = s;

}

//Creates a color from RGB
Color createColor(int r, int g, int b) {
	Color color = {.r = r, .g = g, .b = b};
	return color;
}

void SetBGColor (int r, int g, int  b) {
	BGColor = createColor(r, g, b);
}

void initializeScreen() {

    ResetCallback();
	ResetGraph(0);

	// Automatically adjust screen to PAL or NTCS from license
	// In the TIM Tool go to Frame Buffer Options --> Set Graphics Mode and you will see
	//all supported resolutions for the PlayStation. When making TIM textures with the
	//TIM tool, be sure the resolution you set matches what you have below.
	if (*(char *)0xbfc7ff52=='E') { // SCEE string address
    	// PAL MODE
    	//High-Res is 640x512 (Needs fixing)
     	//Low-Res is 320x256
    	SCREEN_WIDTH = 320;
    	SCREEN_HEIGHT = 256;
    	printf("Setting the PlayStation Video Mode to (PAL %dx%d)\n",SCREEN_WIDTH,SCREEN_HEIGHT);
    	SetVideoMode(1);
    	printf("Video Mode is (%ld)\n",GetVideoMode());

    	if (SCREEN_HEIGHT == 256) {
            //A screen height of 256 is non interlaced
            //and the 2 display buffers are at different locations
            GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsNONINTER|GsOFSGPU, 1, 0);
            GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);
        } else {
            //the screen hight must be 512, so the 2 display
            //buffers are on top of each other and interlaced
            GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0);
            GsDefDispBuff(0, 0, 0, 0);
        }

   	} else {
     	// NTSC MODE
     	//High-Res is 640x480
     	//Low-Res is 320x240
     	SCREEN_WIDTH = 320;
     	SCREEN_HEIGHT = 240;
     	printf("Setting the PlayStation Video Mode to (NTSC %dx%d)\n",SCREEN_WIDTH,SCREEN_HEIGHT);
     	SetVideoMode(0);
     	printf("Video Mode is (%ld)\n",GetVideoMode());

     	if (SCREEN_HEIGHT == 240) {
            //A screen height of 240 is non interlaced
            //and the 2 display buffers are at different locations
            GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsNONINTER|GsOFSGPU, 1, 0);
            GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);
        } else {
            //the screen hight must be 480, so the 2 display
            //buffers are on top of each other and interlaced
            GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0);
            GsDefDispBuff(0, 0, 0, 0);
        }
   }

	// Prepare the ordering tables
	orderingTable[0].length	= OT_LENGTH;
	orderingTable[1].length	= OT_LENGTH;
	orderingTable[0].org = orderingTable_TAG[0];
	orderingTable[1].org = orderingTable_TAG[1];

	GsClearOt(0, 0, &orderingTable[0]);
	GsClearOt(0, 0, &orderingTable[1]);

	// Initialize debug font stream
	FntLoad(960, 0);
	FntOpen(-CENTERX + 10, -CENTERY + 18, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512);

	// Setup 3D and projection matrix
	GsInit3D();
	GsSetProjection(CENTERX);
	GsInitCoordinate2(WORLD, &Camera.coord2);

	// Set default lighting mode
	//0 = No Fog
	//1 = Fog
	GsSetLightMode(0);

	//This is a fix for 640x480 (NTSC) resolution. for more
	//information see LIBOVR46.PDF page 8-45 (PDF page 129)
	//SCREEN_HEIGHT >= 480
	if (0) {

        SetDispMask(1);
        VSync(0); /* for V-SYNC timing*/
        Frame = !GetODE(); /* initial evaluation value for GetODE() */

        /* set up callback function for V-SYNCs */
        VSyncCallback(cbvsync);
	}

}

/* VSync callback routine*/
void cbvsync(void){

	/* GetODE() *****************************************************:
        Only even-numbered frames are returned in the callback function, so the
        static variable Frame is used for comparison.*/
	if (GetODE()) {
        //odd
        return;
	}

	/* In particular, the switching between odd -> even frames takes
	place right after V-BLNK so the display/rendering environment
	switching should be performed as soon as possible.*/
	PutDispEnv(&GsDISPENV);
	PutDrawEnv(&GsDRAWENV);

	//Wait for termination of all non-blocking
    //functions registered in the queue
    DrawSync(0);

    //DrawSync(1) returns the number of positions
    //in the current queue if the primitive list
    //has been corrupted in some way, this will help.
    while(DrawSync(1));

	if(VsyncWait){
        GsSwapDispBuff();
        GsSortClear(BGColor.r, BGColor.g, BGColor.b, &orderingTable[myActiveBuff]);
		GsDrawOt(&orderingTable[myActiveBuff]);
		myLastBuff = myActiveBuff;
		VsyncWait = 0;
	} else {
	    GsDrawOt(&orderingTable[myLastBuff]);
	}

	FntFlush(0);

}

void clear_vram() {
    RECT rectTL;
    setRECT(&rectTL, 0, 0, 1024, 512);
    ClearImage2(&rectTL, 0, 0, 0);
    DrawSync(0);
    return;
}

void clear_display() {

    myActiveBuff = GsGetActiveBuff();
    GsSetWorkBase((PACKET*)GPUOutputPacket[myActiveBuff]);
    GsClearOt(0, 0, &orderingTable[myActiveBuff]);

}

void Display() {

	if (0) {
        VsyncWait = 1;
        while(VsyncWait == 1);
		sync = VSync(1);
	} else {
	    //0 = 60 FPS
	    //2 = 30 FPS
	    //3 = 15 FPS
        VSync(3);
        GsSwapDispBuff();
        GsSortClear(BGColor.r, BGColor.g, BGColor.b, &orderingTable[myActiveBuff]);
        GsDrawOt(&orderingTable[myActiveBuff]);
        FntFlush(0);
	}

}
