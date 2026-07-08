//number of calls to audio_transfer_vag_to_spu
//times 3???
#define SOUND_MALLOC_MAX 36

#define cdSetVol(vol, v) \
	(vol)->val0=(vol)->val2=v,(vol)->val1=(vol)->val3=0,CdMix(vol)

void getReturnStatus ();
void cdMusicUpdate ();

SpuCommonAttr l_c_attr;
SpuVoiceAttr  g_s_attr;
SpuVolume cdMusicVol;
unsigned long l_vag1_spu_addr;

//CD Music
//When using cd music, don't use audio_init(). I don't know why
//but that makes cd music not work.
//track 1 is always game data
//track 2 is Theme Music
//track 3 is Jungle Sounds for the jungle level
//track 4 is a dummy track. This is used to detect the
//end of the last music track. That way all the tracks
//before this one can be looped in-game.

CdlLOC	loc[100];
CdlATV	atv;
int	vol = 0x80;
u_char	param[4], result[8];
int ret, ntoc;
int	track, min, sec, level;

int loopMusic = false;
int trackToLoop = 2; //don't ever set this below 2. track 1 is game data.
int loopWait;

void audio_init() {

    short volume = 0x3fff;

	SpuInit();
	SpuInitMalloc (SOUND_MALLOC_MAX, (char*)(SPU_MALLOC_RECSIZ * (SOUND_MALLOC_MAX + 1)));

	l_c_attr.mask = (
        SPU_COMMON_MVOLL|
        SPU_COMMON_MVOLR|
        SPU_COMMON_CDVOLL|
        SPU_COMMON_CDVOLR|
        SPU_COMMON_CDMIX);

	l_c_attr.mvol.left  = volume; // set master left volume
	l_c_attr.mvol.right = volume; // set master right volume

	cdMusicVol.left = volume;
	cdMusicVol.right = volume;
	l_c_attr.cd.volume = cdMusicVol; // set CDDA music volume

	SpuSetCommonAttr (&l_c_attr);

}

void cdMusicInit () {

    CdSetDebug(0);
    cdSetVol(&atv, vol);

	/* check DiskStatus */
	while( DsSystemStatus() != DslReady ) {
		printf( "Not Ready ...\n" );
	}

	ntoc = CdGetToc(loc);
	if (ntoc == 0) {
		printf("No TOC found: please use CD-DA disc...\n");
	}

#ifdef TOCBUG
	for (i = 1; i < ntoc; i++)
		CdIntToPos(CdPosToInt(&loc[i]) - 74, &loc[i]);
#endif


    param[0] = CdlModeRept|CdlModeDA;	//report ON and CD-DA ON
    CdControl(CdlSetmode, param, 0);

    //the number in loc[] indicates the track to play
    CdControl(CdlPlay, (u_char *)&loc[3], 0);
    trackToLoop = 3;
    loopMusic = true;

}

void repeatTrack (int trackToRepeat) {
    int repeat = false;
    if (track > trackToRepeat) {
        //track 0 is data so make sure not to play track 1
        if (trackToRepeat > 1) {
            //printf( "Repeating Track %d\n", trackToRepeat);
            loopWait++;
            repeat = true;
        }
    }

    //Wait 5 frames until repeating. This fixes an issue in some emulators where
    //CD Audio would stop working if it was replayed too soon after ending. If this
    //bug still happens in your game, change 5 to something higher.
    if (repeat && loopWait >= 5) {
        loopWait = 0;
        CdControl(CdlPlay, (u_char *)&loc[trackToRepeat], 0);
    }

}

void cdMusicUpdate () {

    /* check the report from CD-ROM */
    if ((ret = CdReady(1, result)) == CdlDataReady) {
        if ((result[4]&0x80) == 0) {
            track = btoi(result[1]);
            min   = result[3];
            sec   = result[4];
            level = (result[6]<<8)|result[7];
        }
    } else if (ret == CdlDiskError) {
        //If error is detected, retry to play the first track.
        CdControl(CdlPlay, (u_char *)&loc[2], 0);
    }

    /* error check */
    if ((ret = CdSync(1, 0)) == CdlDiskError) {
        CdControl(CdlPlay, (u_char *)&loc[2], 0);
        FntPrint("CDROM: DiskError. retrying..\n");
    }

    if (loopMusic) {
        repeatTrack(trackToLoop);
    }

    //FntPrint("pos = (%2d:%02x:%02x)\n",track, min, sec);

}

void getReturnStatus () {
    //check the status of the CD drive
    CdControl(CdlNop, 0, result);
    switch (result[0]) {
    case 128: printf("CD-DA playing\n");
        break;
    case 64: printf("seeking... \n");
        break;
    case 32: printf("reading data sector... \n");
        break;
    case 16: printf("shell open \n");
        break;
    case 4: printf("error during seeking/reading \n");
        break;
    case 2: printf("motor rotating...\n");
        break;
    case 1: printf("command issue error\n");
        break;
    default: printf("Result Code: %d\n", result[0]);
    }
}

void audio_transfer_vag_to_spu(char* sound, int sound_size, int voice_channel, int pitchMode) {
	SpuSetTransferMode (SpuTransByDMA); // set transfer mode to DMA
	l_vag1_spu_addr = SpuMalloc(sound_size); // allocate SPU memory for sound 1
	SpuSetTransferStartAddr(l_vag1_spu_addr); // set transfer starting address to malloced area
	SpuWrite (sound + 0x30, sound_size); // perform actual transfer
	SpuIsTransferCompleted (SPU_TRANSFER_WAIT); // wait for DMA to complete
	g_s_attr.mask =
			(
					SPU_VOICE_VOLL |
					SPU_VOICE_VOLR |
					SPU_VOICE_PITCH |
					SPU_VOICE_WDSA |
					SPU_VOICE_ADSR_AMODE |
					SPU_VOICE_ADSR_SMODE |
					SPU_VOICE_ADSR_RMODE |
					SPU_VOICE_ADSR_AR |
					SPU_VOICE_ADSR_DR |
					SPU_VOICE_ADSR_SR |
					SPU_VOICE_ADSR_RR |
					SPU_VOICE_ADSR_SL
			);

	g_s_attr.voice = (voice_channel);

	g_s_attr.volume.left  = 0x1fff;
	g_s_attr.volume.right = 0x1fff;

	if (pitchMode == 0) {
        //4096 = 44100Hz
        g_s_attr.pitch = 4096;
	} else if (pitchMode == 1) {
	    //2048 = 22050Hz
	    g_s_attr.pitch = 2048;
	} else {
	    //1024 = 11025Hz
	    g_s_attr.pitch = 1024;
	}

	g_s_attr.addr         = l_vag1_spu_addr;
	g_s_attr.a_mode       = SPU_VOICE_LINEARIncN;
	g_s_attr.s_mode       = SPU_VOICE_LINEARIncN;
	g_s_attr.r_mode       = SPU_VOICE_LINEARDecN;
	g_s_attr.ar           = 0x0;
	g_s_attr.dr           = 0x0;
	g_s_attr.sr           = 0x0;
	g_s_attr.rr           = 0x0;
	g_s_attr.sl           = 0xf;

	SpuSetVoiceAttr (&g_s_attr);
}

void audio_play(int voice_channel, int action) {
    if (action) {
        //play sound
        SpuSetKey(SpuOn, voice_channel);
    } else {
        //stop sound
        SpuSetKey(SpuOff, voice_channel);
    }
}

void audio_free(unsigned long spu_address) {
	SpuFree(spu_address);
}
