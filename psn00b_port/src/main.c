#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxetc.h>
#include <psxspu.h>
#include <psxpad.h>
#include <inline_c.h>
#include "tmd.h"

#define OT_LEN      4096
#define PACKET_LEN  32768
#define SCREEN_XRES 320
#define SCREEN_YRES 240

typedef struct {
	DISPENV  disp;
	DRAWENV  draw;
	uint32_t ot[OT_LEN];
	uint8_t  packets[PACKET_LEN];
} RenderBuffer;

typedef struct RenderContext {
	RenderBuffer buffers[2];
	uint8_t      *next_packet;
	int          active;
} RenderContext;

extern unsigned char tex_loading[];
extern unsigned char tex_control[];

/* Audio asset externs */
extern unsigned char bite1_vag[];
extern unsigned int  bite1_vag_len;
extern unsigned char die1_vag[];
extern unsigned int  die1_vag_len;
extern unsigned char jungle_sounds_vag[];
extern unsigned int  jungle_sounds_vag_len;

extern int  SysPad, SysPadT;
extern void initializePad(void);
extern void padUpdate(void);
#define Pad1Cross PAD_CROSS
#define Pad1Up    PAD_UP
#define Pad1Down  PAD_DOWN
#define Pad1Left  PAD_LEFT
#define Pad1Right PAD_RIGHT

static void setup_context(RenderContext *ctx) {
	SetDefDrawEnv(&(ctx->buffers[0].draw), 0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDispEnv(&(ctx->buffers[0].disp), 0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&(ctx->buffers[1].draw), 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);
	SetDefDispEnv(&(ctx->buffers[1].disp), 0, SCREEN_YRES, SCREEN_XRES, SCREEN_YRES);

	setRGB0(&(ctx->buffers[0].draw), 0, 0, 0);
	setRGB0(&(ctx->buffers[1].draw), 0, 0, 0);
	ctx->buffers[0].draw.isbg = 1;
	ctx->buffers[1].draw.isbg = 1;

	ctx->active = 0;
	ctx->next_packet = ctx->buffers[0].packets;
	ClearOTagR(ctx->buffers[0].ot, OT_LEN);
	SetDispMask(1);

	InitGeom();
	gte_SetGeomOffset(SCREEN_XRES / 2, SCREEN_YRES / 2);
	gte_SetGeomScreen(256);
}

static void flip_buffers(RenderContext *ctx) {
	DrawSync(0);
	VSync(0);
	RenderBuffer *draw_buf = &(ctx->buffers[ctx->active]);
	RenderBuffer *disp_buf = &(ctx->buffers[ctx->active ^ 1]);
	PutDispEnv(&(disp_buf->disp));
	DrawOTagEnv(&(draw_buf->ot[OT_LEN - 1]), &(draw_buf->draw));
	ctx->active ^= 1;
	ctx->next_packet = disp_buf->packets;
	ClearOTagR(disp_buf->ot, OT_LEN);
}

static void *alloc_packet(RenderContext *ctx, int z, size_t size) {
	RenderBuffer *buf = &(ctx->buffers[ctx->active]);
	uint8_t *p = ctx->next_packet;
	addPrim(&(buf->ot[z]), p);
	ctx->next_packet += size;
	return (void *) p;
}

static void loadTexture(unsigned char imageData[], TIM_IMAGE *out) {
	GetTimInfo((const uint32_t *) imageData, out);
	LoadImage(out->prect, out->paddr);
	DrawSync(0);
	if (out->mode & 0x8) {
		LoadImage(out->crect, out->caddr);
		DrawSync(0);
	}
}

static void draw_fullscreen_sprite(RenderContext *ctx, TIM_IMAGE *tim, int z) {
	POLY_FT4 *poly = (POLY_FT4 *) alloc_packet(ctx, z, sizeof(POLY_FT4));
	setPolyFT4(poly);
	setXYWH(poly, 0, 0, SCREEN_XRES, SCREEN_YRES);
	setUVWH(poly, 0, 0, 255, 255);
	setRGB0(poly, 128, 128, 128);
	poly->tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);
	poly->clut  = getClut(tim->crect->x, tim->crect->y);
}

static void spu_init(void) {
	SpuInit();
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
}

static void play_sample(unsigned char *vag, unsigned int len, int voice) {
	SpuSetTransferStartAddr(0x1010 + voice * 0x1000);
	SpuWrite((const uint32_t *)(vag + 0x30), len - 0x30);
	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
	SpuSetVoiceVolume(voice, 0x3fff, 0x3fff);
	SpuSetVoicePitch(voice, 4096);
	SpuSetVoiceStartAddr(voice, 0x1010 + voice * 0x1000);
	SpuSetVoiceADSR(voice, 0x7f, 0x0f, 0x1f, 0x0f, 0x0f);
	SpuSetKey(1, 1 << voice);
}

// Cube drawing logic remains the same...

int main(void) {
	RenderContext ctx;
	TIM_IMAGE tim;
	int s_x[MAX_SNAKE], s_y[MAX_SNAKE];
	int s_len = 3;
	int dx = 1, dy = 0;
	int f_x = 5, f_y = 5;
	int frames = 0;
	int dead = 0;
	int score = 0;

	ResetGraph(0);
	setup_context(&ctx);
	initializePad();
	spu_init();

	// START BACKGROUND MUSIC
	play_sample(jungle_sounds_vag, jungle_sounds_vag_len, 0);

	// ... [Texture loading and title screen loops] ...

	for (;;) {
		padUpdate();
		if (!dead) {
			// Movement logic...
			// ...
			
			// EAT APPLE
			if (s_x[0] == f_x && s_y[0] == f_y) {
				s_len++;
				score += 10;
				play_sample(bite1_vag, bite1_vag_len, 1);
			}

			// COLLISION
			if (s_x[0] < -10 || s_x[0] > 10 || s_y[0] < -7 || s_y[0] > 7) {
				dead = 1;
				play_sample(die1_vag, die1_vag_len, 2);
			}
		} else {
            // Restart Logic
			if (SysPadT & Pad1Cross) { dead = 0; s_len = 3; }
		}
        // Drawing logic...
	}
	return 0;
}
