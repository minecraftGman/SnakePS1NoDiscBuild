//functions used in the snake game

#define up 1
#define down 2
#define left 3
#define right 4

#define straight 0
#define bend 1
#define tail 2

void snakeUpdate ();
void SpawnApple ();
void setBend ();
void stopAllSounds ();
void playDieSound ();
void playEatSound ();
void GameEnded ();

struct {
	int x;
	int y;
} apple;

struct {
	int x;
	int y;
} snake;

struct {
	char x;
	char y;
	char dir;
	char bendVal;
} segments [215];

VECTOR lastPosition;
int snakeLength = 3;

int tempDir = 1;
int currentDir = 1;
int gameStarted = 0;
int gameOver = 0;
char prevAppleX;
char prevAppleY;

int snakeSpeed = 3;

void snakeUpdate () {

    int i;

	//add a segment to the array at the length of the snake
    segments[snakeLength].x = snake.x;
    segments[snakeLength].y = snake.y;
    segments[snakeLength].dir = currentDir;


    if (currentDir == up) {
        snake.y -= 1;
    }

    if (currentDir == down) {
        snake.y += 1;
    }

    if (currentDir == left) {
        snake.x -= 1;
    }

    if (currentDir == right) {
        snake.x += 1;
    }

    //Eat Apple
    if (snake.x == apple.x &&
        snake.y == apple.y) {

        playEatSound();

        //new apple location
         SpawnApple();

        snakeLength += 1;

	} else {

	    //shift all segments down by 1
        for(i = 0; i < 215; i++) {
            segments[i] = segments[i+1];
        }

	}

	if (snake.x <= 1 ||
        snake.x >= 20 ||
        snake.y <= 1 ||
        snake. y >= 14) {
        //Hit Wall
        GameEnded();
	}

	for(i = 0; i < 215; i++) {
        if (segments[i].x == snake.x &&
            segments[i].y == snake.y) {
            //Hit Tail
            GameEnded();
        }
    }

    tempDir = currentDir;

	lastPosition.vx = snake.x;
	lastPosition.vy = snake.y;

	//Calculate and set bend values
	setBend();

}

void renderTile(char objNumber, char x, char y, char dir) {

    int realX;
    int realY;
    VECTOR tilePos;
    SVECTOR tileRotation;

    //2 * METER
    int distBetweenTiles = 2048;

    //Render on the X Z plane (flat not upright like X Y)
    realX = x * distBetweenTiles;
    realY = y * distBetweenTiles;

    //Orient on the flat X Z plane
    tilePos.vx = realX;
    tilePos.vz = -realY;

    if (dir == 1) {
        //up
        tileRotation.vy = 0;
    } else if (dir == 2) {
        //down
        //tileRotation.vy = 180 * DEG;
        tileRotation.vy = 2048;
    } else if (dir == 3) {
        //left
        //tileRotation.vy = -90 * DEG;
        tileRotation.vy = -1024;
    } else if (dir == 4) {
        //right
        //tileRotation.vy = 90 * DEG;
        tileRotation.vy = 1024;
    }
    tileRotation.vx = 0;
    tileRotation.vz = 0;

    RenderObject(tilePos, tileRotation, &Object[(u_char)objNumber]);

}

void GameEnded () {
    gameStarted = 0;
    gameOver = 1;
    playDieSound();
}

void InitGame () {

    int i;
    for(i = 0; i < 215; i++) {
        segments[i].x = 0;
        segments[i].y = 0;
        segments[i].dir = 0;
    }

    lastPosition.vx = 0;
    lastPosition.vy = 0;
    lastPosition.vz = 0;
    snakeLength = 3;
    tempDir = 1;
    currentDir = 1;
    gameStarted = 0;
    gameOver = 0;

    //Snake Head (position based on tiles, not meters)
	snake.x = 11;
	snake.y = 7;

    //Tail
    segments[0].x = snake.x + 2;
    segments[0].y = snake.y + 1;
    segments[0].dir = left;
    //Straight
    segments[1].x = snake.x + 1;
    segments[1].y = snake.y + 1;
    segments[1].dir = left;
    //Bend
	segments[2].x = snake.x;
    segments[2].y = snake.y + 1;
    segments[2].dir = up;

    SpawnApple();

    setBend();

    stopAllSounds();

    //Hiss Sound
    audio_play(SPU_08CH, 1);

}

void SpawnApple () {

    int i;
    int tempX;
    int tempY;
    int matchFound = 1;

    //ensure that the new apple location is not on a snake tile
    while (matchFound) {

        matchFound = 0;

        tempX = getRandom(2, 19);
        tempY = getRandom(2, 13);

        if (snake.x == tempX ||
            snake.y == tempY) {
            //same location as head
            matchFound = 1;
        }

        if (prevAppleX == tempX ||
            prevAppleY == tempY) {
            //same location as before
            matchFound = 1;
        }

        for(i = 0; i < 215; i++) {
            if (segments[i].x == tempX &&
                segments[i].y == tempY) {
                //on a segment
                matchFound = 1;
            }
        }

    }

    apple.x = tempX;
    apple.y = tempY;
    prevAppleX = tempX;
    prevAppleY = tempY;

}

int timer;
int clockRate (int a) {

    if (VSync(-1) > nextVSyncCount) {
        //30 = half a second
        //60 = 1 second
        nextVSyncCount = VSync(-1) + a;
        return true;
    } else {
        return false;
    }

}

int calcBend (int prevDir, int thisDir) {

    int returnVal = 0;

    if (prevDir == up) {
        if (thisDir == right) {
            //bend left
            returnVal = left;
        } else if (thisDir == left) {
            //bend up
            returnVal = up;
        }
    }

    if (prevDir == down) {
        if (thisDir == right) {
            //bend down
            returnVal = down;
        } else if (thisDir == left) {
            //bend right
            returnVal = right;
        }
    }

    if (prevDir == left) {
        if (thisDir == up) {
            //bend down
            returnVal = down;
        } else if (thisDir == down) {
            //bend left
            returnVal = left;
        }
    }

    if (prevDir == right) {
        if (thisDir == up) {
            //bend right
            returnVal = right;
        } else if (thisDir == down) {
            //bend up
            returnVal = up;
        }
    }

    if (prevDir == thisDir) {
        //1-4 is a direction
        //5 means no bend so use a straight piece
        returnVal = 5;
    }

    return returnVal;

}

void setBend () {
    int i;
    //Segments
	for(i = 0; i < 215; i++) {
		//the max number of segments is 215
		//((boardX - 2) * (boardY - 2)) - 1
		if (segments[i].x > 0 && segments[i].y > 0) {
            //the position has been set
            if (i >=  1) {
                segments[i].bendVal = calcBend(segments[i-1].dir, segments[i].dir);
            }
		}
	}
}

void playEatSound () {

    int soundID;
    soundID = getRandom(0, 1);
    if (soundID == 0) {
        audio_play(SPU_00CH, 1);
    } else {
        audio_play(SPU_01CH, 1);
    }

}

void playDieSound () {

    int soundID;
    soundID = getRandom(2, 5);
    if (snakeLength >= 30) {

        if (snakeLength >= 60) {
            //really sad sound
            audio_play(SPU_07CH, 1);
        } else {
            //sad sound
            audio_play(SPU_06CH, 1);
        }

    } else {
        //regular die sound
        switch (soundID) {
        case 2:  audio_play(SPU_02CH, 1);
            break;
        case 3: audio_play(SPU_03CH, 1);
            break;
        case 4: audio_play(SPU_04CH, 1);
            break;
        case 5: audio_play(SPU_05CH, 1);
            break;
        }
    }

}

void stopAllSounds () {
    audio_play(SPU_00CH, 0);
    audio_play(SPU_01CH, 0);
    audio_play(SPU_02CH, 0);
    audio_play(SPU_03CH, 0);
    audio_play(SPU_04CH, 0);
    audio_play(SPU_05CH, 0);
    audio_play(SPU_06CH, 0);
    audio_play(SPU_07CH, 0);
    audio_play(SPU_08CH, 0);
}








