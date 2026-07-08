void initCustomFont ();
void displayString (int x, int y, int trackingAdjust, int leadingAdjust, int scale, char *string);

int charHeight = 0;
int charWidth = 0;
int charXOffset = 0;
int charRow = 0;

//103 unique characters
Sprite* fntSprite;
int fntTextureX;
int fntTextureY;

void initCustomFont (unsigned char fontTexture[], int fntHeight) {

    //Get font texture info
	GsIMAGE* tim_data;
	tim_data = malloc3(sizeof(GsIMAGE));
	GsGetTimInfo ((u_long *)(fontTexture+4),tim_data);

	//Init the font sprite
    sprite_create((u_char *)fontTexture, 0, 0, &fntSprite);

    //set the custom font's character height
    charHeight = fntHeight;

    //store the font texture's width and height
    fntTextureX = tim_data->px;
    fntTextureY = tim_data->py;

    //Set the initial sprite properties
    fntSprite->mx = 0;
    fntSprite->my = 0;

    free3(tim_data);

}

//Use these individually in the render function to test all the characters
//displayString(0, 0, 0, 0, ONE, "ABCDEFG\nHIJKLMNOP\nQRSTUV\nWXYZ\n");
//displayString(0, 0, 0, 0, ONE, "abcdefg\nhijklmnop\nqrstuv\nwxyz\n");
//displayString(0, 0, 0, 0, ONE, "`~!@#$%^&*()=+-_,./\n;'[]\\<>?:\"{}|\n");
//displayString(0, 0, 0, 0, ONE, "1234567890\n™©®¤³†¹²§\n");

void displayString (int x, int y, int trackingAdjust, int leadingAdjust, int scale, char *string) {

    int xOffset = 0;
    int yOffset = 0;
    int spaceWidth = 10;
    int displayChar = 1;
    int i;
    for (i = 0; i < strlen(string); i++) {

        switch (string[i]) {

        //This is where you setup the size and position
        //of each character in the fontTexture.
        //charWidth = how wide is the character in pixels?
        //charRow = What row is the character on?
        //          The first row is 0. The row height is
        //          already set in the initCustomFont function.
        //          Each row in your texture should be the same height.
        //charXOffset = how many pixels from the left is this character?

        //Upper Case
        case 65: //A
            charWidth = 22;
            charRow = 0;
            charXOffset = 0;
            break;
        case 66: //B
            charWidth = 19;
            charRow = 0;
            charXOffset = 23;
            break;
        case 67: //C
            charWidth = 21;
            charRow = 0;
            charXOffset = 42;
            break;
        case 68: //D
            charWidth = 21;
            charRow = 0;
            charXOffset = 63;
            break;
        case 69: //E
            charWidth = 18;
            charRow = 0;
            charXOffset = 84;
            break;
        case 70: //F
            charWidth = 19;
            charRow = 0;
            charXOffset = 102;
            break;
        case 71: //G
            charWidth = 21;
            charRow = 0;
            charXOffset = 121;
            break;
        case 72: //H
            charWidth = 21;
            charRow = 0;
            charXOffset = 142;
            break;
        case 73: //I
            charWidth = 7;
            charRow = 0;
            charXOffset = 163;
            break;
        case 74: //J
            charWidth = 17;
            charRow = 0;
            charXOffset = 170;
            break;
        case 75: //K
            charWidth = 18;
            charRow = 0;
            charXOffset = 187;
            break;
        case 76: //L
            charWidth = 18;
            charRow = 0;
            charXOffset = 205;
            break;
        case 77: //M
            charWidth = 25;
            charRow = 0;
            charXOffset = 223;
            break;
        case 78: //N
            charWidth = 20;
            charRow = 1;
            charXOffset = 0;
            break;
        case 79: //O
            charWidth = 25;
            charRow = 1;
            charXOffset = 20;
            break;
        case 80: //P
            charWidth = 17;
            charRow = 1;
            charXOffset = 45;
            break;
        case 81: //Q
            charWidth = 25;
            charRow = 1;
            charXOffset = 62;
            break;
        case 82: //R
            charWidth = 18;
            charRow = 1;
            charXOffset = 87;
            break;
        case 83: //S
            charWidth = 18;
            charRow = 1;
            charXOffset = 105;
            break;
        case 84: //T
            charWidth = 21;
            charRow = 1;
            charXOffset = 123;
            break;
        case 85: //U
            charWidth = 21;
            charRow = 1;
            charXOffset = 144;
            break;
        case 86: //V
            charWidth = 22;
            charRow = 1;
            charXOffset = 165;
            break;
        case 87: //W
            charWidth = 29;
            charRow = 1;
            charXOffset = 187;
            break;
        case 88: //X
            charWidth = 20;
            charRow = 1;
            charXOffset = 216;
            break;
        case 89: //Y
            charWidth = 21;
            charRow = 2;
            charXOffset = 0;
            break;
        case 90: //Z
            charWidth = 21;
            charRow = 2;
            charXOffset = 21;
            break;

        //Lower Case
        case 97: //a
            charWidth = 19;
            charRow = 2;
            charXOffset = 42;
            break;
        case 98: //b
            charWidth = 18;
            charRow = 2;
            charXOffset = 61;
            break;
        case 99: //c
            charWidth = 16;
            charRow = 2;
            charXOffset = 79;
            break;
        case 100: //d
            charWidth = 19;
            charRow = 2;
            charXOffset = 95;
            break;
        case 101: //e
            charWidth = 18;
            charRow = 2;
            charXOffset = 114;
            break;
        case 102: //f
            charWidth = 13;
            charRow = 2;
            charXOffset = 132;
            break;
        case 103: //g
            charWidth = 18;
            charRow = 2;
            charXOffset = 145;
            break;
        case 104: //h
            charWidth = 17;
            charRow = 2;
            charXOffset = 163;
            break;
        case 105: //i
            charWidth = 6;
            charRow = 2;
            charXOffset = 180;
            break;
        case 106: //j
            charWidth = 11;
            charRow = 2;
            charXOffset = 186;
            break;
        case 107: //k
            charWidth = 15;
            charRow = 2;
            charXOffset = 197;
            break;
        case 108: //l
            charWidth = 9;
            charRow = 2;
            charXOffset = 212;
            break;
        case 109: //m
            charWidth = 24;
            charRow = 2;
            charXOffset = 221;
            break;
        case 110: //n
            charWidth = 17;
            charRow = 3;
            charXOffset = 0;
            break;
        case 111: //o
            charWidth = 18;
            charRow = 3;
            charXOffset = 17;
            break;
        case 112: //p
            charWidth = 19;
            charRow = 3;
            charXOffset = 35;
            break;
        case 113: //q
            charWidth = 18;
            charRow = 3;
            charXOffset = 54;
            break;
        case 114: //r
            charWidth = 14;
            charRow = 3;
            charXOffset = 72;
            break;
        case 115: //s
            charWidth = 16;
            charRow = 3;
            charXOffset = 86;
            break;
        case 116: //t
            charWidth = 13;
            charRow = 3;
            charXOffset = 102;
            break;
        case 117: //u
            charWidth = 18;
            charRow = 3;
            charXOffset = 115;
            break;
        case 118: //v
            charWidth = 17;
            charRow = 3;
            charXOffset = 133;
            break;
        case 119: //w
            charWidth = 22;
            charRow = 3;
            charXOffset = 150;
            break;
        case 120: //x
            charWidth = 16;
            charRow = 3;
            charXOffset = 172;
            break;
        case 121: //y
            charWidth = 18;
            charRow = 3;
            charXOffset = 188;
            break;
        case 122: //z
            charWidth = 15;
            charRow = 3;
            charXOffset = 206;
            break;

        //Symbols
        //`~!@#$%^&*()=+-_,./;'[] backslash <>?:"{}|
        case 96: // `
            charWidth = 8;
            charRow = 3;
            charXOffset = 221;
            break;
        case 126: // ~
            charWidth = 15;
            charRow = 3;
            charXOffset = 229;
            break;
        case 33: // !
            charWidth = 7;
            charRow = 3;
            charXOffset = 244;
            break;
        case 64: // @
            charWidth = 31;
            charRow = 4;
            charXOffset = 0;
            break;
        case 35: // #
            charWidth = 20;
            charRow = 4;
            charXOffset = 31;
            break;
        case 36: // $
            charWidth = 18;
            charRow = 4;
            charXOffset = 51;
            break;
        case 37: // %
            charWidth = 26;
            charRow = 4;
            charXOffset = 69;
            break;
        case 94: // ^
            charWidth = 14;
            charRow = 4;
            charXOffset = 95;
            break;
        case 38: // &
            charWidth = 21;
            charRow = 4;
            charXOffset = 109;
            break;
        case 42: // *
            charWidth = 11;
            charRow = 4;
            charXOffset = 130;
            break;
        case 40: // (
            charWidth = 9;
            charRow = 4;
            charXOffset = 141;
            break;
        case 41: // )
            charWidth = 8;
            charRow = 4;
            charXOffset = 150;
            break;
        case 61: // =
            charWidth = 15;
            charRow = 4;
            charXOffset = 158;
            break;
        case 43: // +
            charWidth = 16;
            charRow = 4;
            charXOffset = 173;
            break;
        case 45: // -
            charWidth = 12;
            charRow = 4;
            charXOffset = 189;
            break;
        case 95: // _
            charWidth = 18;
            charRow = 4;
            charXOffset = 201;
            break;
        case 44: // ,
            charWidth = 7;
            charRow = 4;
            charXOffset = 219;
            break;
        case 46: // .
            charWidth = 7;
            charRow = 4;
            charXOffset = 226;
            break;
        case 47: // /
            charWidth = 14;
            charRow = 4;
            charXOffset = 233;
            break;
        case 59: // ;
            charWidth = 7;
            charRow = 5;
            charXOffset = 0;
            break;
        case 39: // '
            charWidth = 7;
            charRow = 5;
            charXOffset = 7;
            break;
        case 91: // [
            charWidth = 10;
            charRow = 5;
            charXOffset = 14;
            break;
        case 93: // ]
            charWidth = 10;
            charRow = 5;
            charXOffset = 24;
            break;
        case 92: // backslash
            charWidth = 15;
            charRow = 5;
            charXOffset = 34;
            break;
        case 60: // <
            charWidth = 16;
            charRow = 5;
            charXOffset = 49;
            break;
        case 62: // >
            charWidth = 16;
            charRow = 5;
            charXOffset = 65;
            break;
        case 63: // ?
            charWidth = 16;
            charRow = 5;
            charXOffset = 81;
            break;
        case 58: // :
            charWidth = 8;
            charRow = 5;
            charXOffset = 97;
            break;
        case 34: // "
            charWidth = 11;
            charRow = 5;
            charXOffset = 105;
            break;
        case 123: // {
            charWidth = 12;
            charRow = 5;
            charXOffset = 116;
            break;
        case 125: // }
            charWidth = 12;
            charRow = 5;
            charXOffset = 128;
            break;
        case 124: // |
            charWidth = 6;
            charRow = 5;
            charXOffset = 140;
            break;

        //Numbers
        case 49: //1
            charWidth = 10;
            charRow = 5;
            charXOffset = 146;
            break;
        case 50: //2
            charWidth = 16;
            charRow = 5;
            charXOffset = 156;
            break;
        case 51: //3
            charWidth = 16;
            charRow = 5;
            charXOffset = 172;
            break;
        case 52: //4
            charWidth = 19;
            charRow = 5;
            charXOffset = 188;
            break;
        case 53: //5
            charWidth = 17;
            charRow = 5;
            charXOffset = 207;
            break;
        case 54: //6
            charWidth = 16;
            charRow = 5;
            charXOffset = 224;
            break;
        case 55: //7
            charWidth = 16;
            charRow = 5;
            charXOffset = 240;
            break;
        case 56: //8
            charWidth = 16;
            charRow = 6;
            charXOffset = 0;
            break;
        case 57: //9
            charWidth = 16;
            charRow = 6;
            charXOffset = 16;
            break;
        case 48: //0
            charWidth = 17;
            charRow = 6;
            charXOffset = 32;
            break;

        //Extras ™©®¤³†¹²§

        case 153: //™
            //Triangle Button
            charWidth = 28;
            charRow = 6;
            charXOffset = 49;
            break;
        case 169: //©
            //Circle Button
            charWidth = 26;
            charRow = 6;
            charXOffset = 77;
            break;
        case 174: //®
            //X Button
            charWidth = 24;
            charRow = 6;
            charXOffset = 103;
            break;
        case 164: //¤
            //Square Button
            charWidth = 24;
            charRow = 6;
            charXOffset = 127;
            break;
        case 179: //³
            //Left Button
            charWidth = 16;
            charRow = 6;
            charXOffset = 151;
            break;
        case 134: //†
            //Right Button
            charWidth = 16;
            charRow = 6;
            charXOffset = 167;
            break;
        case 185: //¹
            //Up Button
            charWidth = 19;
            charRow = 6;
            charXOffset = 183;
            break;
        case 178: //²
            //Down Button
            charWidth = 19;
            charRow = 6;
            charXOffset = 202;
            break;
        case 167: //§
            //PS Symbol
            charWidth = 27;
            charRow = 6;
            charXOffset = 221;
            break;

        //Special
        case 10: //Line Break
            yOffset += fntSprite->h + leadingAdjust;
            xOffset = 0;
            displayChar = 0;
            break;
        case 32: //Space
            xOffset += spaceWidth + trackingAdjust;
            displayChar = 0;
            break;

        default:
            //Do nothing if the character is not supported
            displayChar = 0;
            break;

        }

        if (displayChar) {

            //The character is supported and
            //is not a special case like \n

            fntSprite->x = (-SCREEN_WIDTH/2) + xOffset + x;
            fntSprite->y = (-SCREEN_HEIGHT/2) + yOffset + 8 + y;
            fntSprite->scalex = scale;
            fntSprite->scaley = scale;
            fntSprite->w = charWidth;
            fntSprite->h = charHeight;
            fntSprite->u = (fntTextureX & 0x3f) + charXOffset;
            fntSprite->v = (fntTextureY % 256) + (35 * charRow);
            xOffset += fntSprite->w + trackingAdjust;

            draw_sprite(fntSprite);

        }

        displayChar = 1;

    }

}
