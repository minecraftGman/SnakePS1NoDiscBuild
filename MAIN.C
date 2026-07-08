
// Snake Game by Rubixcube6 aka MBDesigns
// -- Disc-free BIOS-shell build --
//
// Modified to run with NO CD required: all gameplay-critical assets
// (apple/snake models, apple texture, UI font) are compiled directly into
// the executable as byte arrays (see Assets.c) instead of being streamed
// from a CD-ROM at runtime. The decorative jungle scenery (models+textures),
// the loading/control splash images, sound effects, and CD-DA music have
// all been removed, since none of them can exist without a physical disc
// and/or don't fit the BIOS shell's ~300KB size budget.
//
// This file is meant to be compiled (PsyQ ccpsx or PSn00bSDK gcc) into a
// PS-X EXE and then fed into the BIOS shell patcher.

#include "Constants.c"
#include "Controller.c"
#include "Audio.c"
#include "2D.c"
#include "3D.c"
#include "Font.c"
#include "Assets.c"

//Snake functions
#include "Snake.c"

void Initialize();
void Update();
void Render();
void Controls();
void Start();
void ShowStartScreen();

int camZoom = 21 * METER;

// Only the objects Snake gameplay actually needs.
// Object[0]=apple, [1]=head, [2]=straight, [3]=curve, [4]=tail
// (matches the indices renderTile()/RenderObject() already use in Snake.c)

int readyToPlay;

int main() {

	Initialize();

	ShowStartScreen();

	while (readyToPlay == 0) {
		padUpdate();
		if (padCheckPressed(Pad1Cross)) {
			readyToPlay = 1;
		}
	}

	Start();

	while (1) {
		Update();
		Render();
	}

	return 0;
}

void Initialize() {

	// Small heap still needed by loadTexture()/initCustomFont() for their
	// temporary GsIMAGE/RECT allocations, even though nothing is read from
	// CD anymore.
	InitHeap3((void*)0x800F8000, 0x00020000);

	initializeScreen();
	initializePad();
	audio_init();

	// Textures now live directly in ROM/RAM (see Assets.c) -- no CD read.
	loadTexture((u_char *)TEX_APPLE);
	loadTexture((u_char *)TEX_FONT);

	// Models: only what gameplay needs (apple, head, straight, curve, tail).
	ObjectCount += LoadTMD((u_long *)MDL_APPLE,    &Object[0], 1);
	ObjectCount += LoadTMD((u_long *)MDL_HEAD,     &Object[1], 0);
	ObjectCount += LoadTMD((u_long *)MDL_STRAIGHT, &Object[2], 0);
	ObjectCount += LoadTMD((u_long *)MDL_CURVE,    &Object[3], 0);
	ObjectCount += LoadTMD((u_long *)MDL_TAIL,     &Object[4], 0);

	initCustomFont((u_char *)TEX_FONT, 35);

}

void ShowStartScreen() {
	// Replaces the old CD-loaded "controls" splash image with a plain
	// text prompt drawn with the (embedded) custom font, on both back
	// buffers so it's stable before the first real Render() call.
	int i;
	for (i = 0; i < 2; i++) {
		clear_display();
		CalculateCamera();
		displayString(60, 100, -6, 0, 2888, "SNAKE");
		displayString(30, 130, -6, 0, 2888, "PRESS X TO START");
		Display();
	}
}

void Start() {

	////////////////////////////////
	///   Setup the scene here   ///
	////////////////////////////////

	//Camera
	Camera.position.vx = -21401;
	Camera.position.vy = 21198;
	Camera.position.vz = 27090;

	Camera.rotation.vx = 697;

	InitGame();

	SetBGColor(20, 90, 20);
	SetAmbientLight(67, 67, 67);
	SetSunColor(255, 255, 255);
	SetSunDirection(0, -1, 1);

}

void Update () {

	padUpdate();
	Controls();

	if (gameStarted == 1 && gameOver == 0) {

		int cr;
		switch (snakeSpeed) {
			case 6:
				cr = 5;
				break;
			case 5:
				cr = 10;
				break;
			case 4:
				cr = 15;
				break;
			case 3:
				cr = 20;
				break;
			case 2:
				cr = 25;
				break;
			case 1:
				cr = 30;
				break;
			default:
				cr = 15;
				break;
		}

		if (clockRate(cr)) {
			//1/4th of a sec
			snakeUpdate();
		}

	}

}

void Render () {

	int i;
	char snkLengthChar[3];
	char snkSpeedChar[1];

	clear_display();
	CalculateCamera();

	//Apple
	renderTile(0, apple.x, apple.y, up);

	//Head
	renderTile(1, snake.x, snake.y, tempDir);

	//Segments
	for(i = 0; i < 215; i++) {
		//the max number of segments is 215
		//((boardX - 2) * (boardY - 2)) - 1
		if (segments[i].x > 0 && segments[i].y > 0) {

			if (i >=  1) {

				if (segments[i].bendVal == 5) {
					//No Bend
					renderTile(2, segments[i].x, segments[i].y, segments[i].dir);
				} else {
					renderTile(3, segments[i].x, segments[i].y, segments[i].bendVal);
				}

			}

		}

	}

	//Tail
	renderTile(4, segments[0].x, segments[0].y, segments[0].dir);

	sprintf(snkLengthChar, "%d", snakeLength);
	sprintf(snkSpeedChar, "%d", snakeSpeed);
	displayString(10, 7, -7, 0, 2888, "Le");
	displayString(33, 7, -6, 0, 2888, "ngt");
	displayString(65, 7, -6, 0, 2888, "h:");
	displayString(85, 7, -3, 0, 2888, snkLengthChar);

	displayString(13, 30, -6, 0, 2888, "Speed:");
	displayString(85, 30, -3, 0, 2888, snkSpeedChar);

	Display();

}

void Controls () {

	if (tempDir == left || tempDir == right) {
		//facing left or right
		if (padCheck(Pad1Up)) {
			currentDir = up;
		}

		if (padCheck(Pad1Down)) {
			currentDir = down;
		}
	}

	if (tempDir == up || tempDir == down) {
		//facing up or down
		if (padCheck(Pad1Left)) {
			currentDir = left;
		}

		if (padCheck(Pad1Right)) {
			currentDir = right;
		}
	}

	if (padCheck(Pad1Up) ||
		padCheck(Pad1Left) ||
		padCheck(Pad1Right)) {
		gameStarted = 1;
	}

	if (padCheckPressed(Pad1L1)) {
		snakeSpeed -= 1;
	}

	if (padCheckPressed(Pad1R1)) {
		snakeSpeed += 1;
	}

	if (snakeSpeed > 6) {
		snakeSpeed = 6;
	}

	if (snakeSpeed < 1) {
		snakeSpeed = 1;
	}

	if (padCheck(Pad1Start) || padCheck(Pad1Cross)) {
		if (gameOver == 1) {
			InitGame();
		}
	}

}
