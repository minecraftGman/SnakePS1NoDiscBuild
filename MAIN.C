
// Snake Game by Rubixcube6 aka MBDesigns

#include "Constants.c"
#include "Controller.c"
#include "ReadCD.c"
#include "Audio.c"
#include "2D.c"
#include "3D.c"
#include "Font.c"

// Assets embedded directly in the executable -- no CD required.
#include "AssetsTextures.c"
#include "AssetsModels.c"
#include "AssetsAudio.c"

//Snake functions
#include "Snake.c"

//Declare any function you make here
void Initialize();
void Update();
void PreRender();
void Render();
void Controls();
void CameraControls();
void Start();

//void CameraControls ();

int camZoom = 21 * METER;

//Store all the CD Files Here
u_long* textures[10]; // was [9] in the original -- textures[9] (CONTROL.TIM) wrote out of bounds
u_long* models[12];
u_long* audio[12];
Sprite* sprites[2];

int loadingStep;

struct {
	VECTOR position;
	SVECTOR rotation;
} jungle;

VECTOR hiddenPos;
int readyToPlay;

int main() {

	Initialize();

    while (readyToPlay == 0) {
        padUpdate();
        if (padCheckPressed(Pad1Cross)) {
            readyToPlay = 1;
        }
    }

	Start();
    
	while(1) {
		Update();
		Render();
	}

	return 0;
}

void Initialize() {

    int i;

	initializeScreen();
	initializePad();
	audio_init();

	//Loading Screen Color
	//SetBGColor(20, 20, 20);

	ReadCDInit();

        //Loading Screen
        //(no CD needed -- assets are embedded in the executable)
        textures[0] = (u_long*) tex_loading;
        sprite_create((u_char *)textures[0], 0, 0, &sprites[0]);
        sprites[0]->scalex = ONE - 1024;
        sprites[0]->scaley = ONE - 1024;
        //Render to both ordering tables
        PreRender();
        PreRender();

        //Controls Screen
        textures[9] = (u_long*) tex_control;
        sprite_create((u_char *)textures[9], 0, 0, &sprites[0]);
        sprites[0]->scalex = ONE + 1024;
        sprites[0]->scaley = ONE - 512;

        //Textures
        textures[1] = (u_long*) tex_apple;
        textures[2] = (u_long*) tex_ground;
        textures[3] = (u_long*) tex_rock;
        textures[4] = (u_long*) tex_wood;
        textures[5] = (u_long*) tex_plant1;
        textures[6] = (u_long*) tex_plant2;
        textures[7] = (u_long*) tex_plant3;
        textures[8] = (u_long*) tex_font;

        //Load all textures
        //the number is the number of textures
        for(i = 0; i < 9; i++) {
            loadTexture((u_char *)textures[i]);
            //free3(textures[i]);
        }

        //Models
        models[0]  = (u_long*) mdl_apple;
        models[1]  = (u_long*) mdl_head;
        models[2]  = (u_long*) mdl_straight;
        models[3]  = (u_long*) mdl_curve;
        models[4]  = (u_long*) mdl_tail;
        models[5]  = (u_long*) mdl_ground;
        models[6]  = (u_long*) mdl_rocks;
        models[7]  = (u_long*) mdl_trees;
        models[8]  = (u_long*) mdl_plant1;
        models[9]  = (u_long*) mdl_plant2;
        models[10] = (u_long*) mdl_plant3;
        models[11] = (u_long*) mdl_test;

        //Audio
        audio[0] = (u_long*) snd_bite1;
        audio[1] = (u_long*) snd_bite2;
        audio[2] = (u_long*) snd_die1;
        audio[3] = (u_long*) snd_die2;
        audio[4] = (u_long*) snd_die3;
        audio[5] = (u_long*) snd_die4;
        audio[6] = (u_long*) snd_sad1;
        audio[7] = (u_long*) snd_sad2;
        audio[8] = (u_long*) snd_hiss;

	//audio_init();

	ObjectCount += LoadTMD(models[0], &Object[0], 1);
    ObjectCount += LoadTMD(models[1], &Object[1], 0);
    ObjectCount += LoadTMD(models[2], &Object[2], 0);
    ObjectCount += LoadTMD(models[3], &Object[3], 0);
    ObjectCount += LoadTMD(models[4], &Object[4], 0);
    ObjectCount += LoadTMD(models[5], &Object[5], 0);
    ObjectCount += LoadTMD(models[6], &Object[6], 0);
    ObjectCount += LoadTMD(models[7], &Object[7], 0);
    ObjectCount += LoadTMD(models[8], &Object[8], 0);
    ObjectCount += LoadTMD(models[9], &Object[9], 0);
    ObjectCount += LoadTMD(models[10], &Object[10], 0);
    ObjectCount += LoadTMD(models[11], &Object[11], 0);

    // NOTE: the original code freed models[i] here with free3(). That was
    // only correct when models[i] pointed at a heap buffer malloc3()'d by
    // cd_read_file(). Now that models[i] points directly at the embedded,
    // statically-allocated asset arrays above, calling free3() on it would
    // corrupt the heap -- so that loop is intentionally removed.

    audio_transfer_vag_to_spu((u_char *)audio[0], SECTOR * 3, SPU_00CH, 1);
    audio_transfer_vag_to_spu((u_char *)audio[1], SECTOR * 4, SPU_01CH, 1);
    audio_transfer_vag_to_spu((u_char *)audio[2], SECTOR * 4, SPU_02CH, 1);
    audio_transfer_vag_to_spu((u_char *)audio[3], SECTOR * 3, SPU_03CH, 1);
    audio_transfer_vag_to_spu((u_char *)audio[4], SECTOR * 7, SPU_04CH, 1);
    audio_transfer_vag_to_spu((u_char *)audio[5], SECTOR * 5, SPU_05CH, 1);
    audio_transfer_vag_to_spu((u_char *)audio[6], SECTOR * 55, SPU_6CH, 1);
    audio_transfer_vag_to_spu((u_char *)audio[7], SECTOR * 38, SPU_7CH, 1);
    audio_transfer_vag_to_spu((u_char *)audio[8], SECTOR * 8, SPU_8CH, 1);

    initCustomFont((u_char *)textures[8], 35);

    // cdMusicInit() is intentionally NOT called: it busy-waits forever for
    // a physical disc to report ready (while (DsSystemStatus() !=
    // DslReady)), and CD-DA background music can't be embedded in the
    // executable at all -- it's real audio read live off a spinning disc,
    // not a file. Without a CD, calling it would hang here forever instead
    // of just going black. See Audio.c for details.
    //cdMusicInit();

    //this is for rendering the controls sprite that was
    //loaded right after the loading screen was displayed.
    PreRender();
    PreRender();

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

	//Jungle Level Position
	jungle.position.vx = 21 * METER;
	jungle.position.vy = -1 * METER;
	jungle.position.vz = -15 * METER;

	jungle.rotation.vx = 0;
	jungle.rotation.vy = 0;
	jungle.rotation.vz = 0;

	hiddenPos.vx = jungle.position.vx;
	hiddenPos.vy = jungle.position.vy;
	hiddenPos.vy = jungle.position.vz;

	InitGame();

	SetBGColor(123, 123, 32);
	SetAmbientLight(67, 67, 67);
	SetSunColor(255, 255, 255);
	SetSunDirection(0, -1, 1);

}

void Update () {

	padUpdate();
	Controls();
	//CameraControls();
	cdMusicUpdate();

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

void PreRender () {
    clear_display();
    CalculateCamera();
    draw_sprite(sprites[0]); //Loading Graphic
    Display();
}

void Render () {

    int i;
    char snkLengthChar[3];
    char snkSpeedChar[1];

	clear_display();
	CalculateCamera();

    RenderObject(jungle.position, jungle.rotation, &Object[5]); //ground
    RenderObject(jungle.position, jungle.rotation, &Object[6]); //rocks
    RenderObject(jungle.position, jungle.rotation, &Object[7]); //trees
    RenderObject(jungle.position, jungle.rotation, &Object[8]); //Plant 1
    RenderObject(jungle.position, jungle.rotation, &Object[9]); //Plant 2
    RenderObject(jungle.position, jungle.rotation, &Object[10]); //Plant 3

    //Apple
    renderTile(0, apple.x, apple.y, up);

    //Head
    renderTile(1, snake.x, snake.y, tempDir);

    //Segments
	for(i = 0; i < 215; i++) {
		//the max number of segments is 215
		//((boardX - 2) * (boardY - 2)) - 1
		if (segments[i].x > 0 && segments[i].y > 0) {
            //the position has been set, so render it
            //anything with a pos of (0 x 0) will not render

            if (i >=  1) {

                if (segments[i].bendVal == 5) {
                    //No Bend
                    renderTile(2, segments[i].x, segments[i].y, segments[i].dir);
                } else {
                    renderTile(3, segments[i].x, segments[i].y, segments[i].bendVal);
                }

            }

		} else {
		    //not set. render it hidden under the level
		    //STRESS TEST
            //renderTile(2, hiddenPos.vx, hiddenPos.vy, segments[i].bendVal);
		}

	}

	//Tail
	renderTile(4, segments[0].x, segments[0].y, segments[0].dir);

	// Render debug text
	//FntPrint("Cam Position: (%d, %d, %d)\n", Camera.position.vx, Camera.position.vy, Camera.position.vz);
	//FntPrint("Cam Rotation X: %d\n", Camera.rotation.vx);
	//FntPrint("length: %d\n\n", snakeLength);



    //I only broke it up into multiple lines to manually
    //adjust the spacing between certain letters.
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

void CameraControls () {

    if (padCheck(Pad1R2)) {
        camZoom += 5 * CENTIMETER;
    }

    if (padCheck(Pad1R1)) {
        camZoom -= 5 * CENTIMETER;
    }

    if (padCheck(Pad1L2)) {
        Camera.rotation.vx += 1 * DEG;
    }

    if (padCheck(Pad1L1)) {
        Camera.rotation.vx -= 1 * DEG;
    }

    if (padCheck(Pad1Up)) {
        Camera.position.vz -= 5 * CENTIMETER;
    }

    if (padCheck(Pad1Down)) {
        Camera.position.vz += 5 * CENTIMETER;
    }

    if (padCheck(Pad1Left)) {
        Camera.position.vx += 5 * CENTIMETER;
    }

    if (padCheck(Pad1Right)) {
        Camera.position.vx -= 5 * CENTIMETER;
    }

    Camera.position.vy = camZoom;

}
