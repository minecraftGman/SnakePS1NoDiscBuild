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
extern unsigned char snd_bite1[];
extern unsigned int  snd_bite1_len;

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
	int pmode = tim->mode & 0x3;
	int pix_w = tim->prect->w;
	if (pmode == 0) pix_w *= 4;
	else if (pmode == 1) pix_w *= 2;
	if (pix_w > 255) pix_w = 255;
	int pix_h = tim->prect->h > 255 ? 255 : tim->prect->h;

	setPolyFT4(poly);
	setXYWH(poly, 0, 0, SCREEN_XRES, SCREEN_YRES);
	setUVWH(poly, 0, 0, pix_w, pix_h);
	setRGB0(poly, 128, 128, 128);
	poly->tpage = getTPage(pmode, 0, tim->prect->x, tim->prect->y);
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

static SVECTOR cube_verts[] = {
	{-50, -50, -50}, { 50, -50, -50}, { 50,  50, -50}, {-50,  50, -50},
	{-50, -50,  50}, { 50, -50,  50}, { 50,  50,  50}, {-50,  50,  50}
};

static int cube_faces[][4] = {
	{0, 1, 2, 3}, {1, 5, 6, 2}, {5, 4, 7, 6},
	{4, 0, 3, 7}, {4, 5, 1, 0}, {3, 2, 6, 7}
};

static void draw_cube(RenderContext *ctx, SVECTOR *rotation, VECTOR *translation, int r, int g, int b) {
	MATRIX mtx;
	RotMatrix(rotation, &mtx);
	TransMatrix(&mtx, translation);
	gte_SetRotMatrix(&mtx);
	gte_SetTransMatrix(&mtx);

	for (int i = 0; i < 6; i++) {
		long p, otz;
		POLY_F4 *poly;

		gte_ldv3(&cube_verts[cube_faces[i][0]], &cube_verts[cube_faces[i][1]], &cube_verts[cube_faces[i][2]]);
		gte_rtpt();
		gte_nclip();
		gte_stopz(&p);

		if (p <= 0) continue;

		gte_avsz3();
		gte_stotz(&otz);

		if (otz > 0 && otz < OT_LEN) {
			poly = (POLY_F4 *)alloc_packet(ctx, otz, sizeof(POLY_F4));
			setPolyF4(poly);
			gte_stsxy0(&poly->x0);
			gte_stsxy1(&poly->x1);
			gte_stsxy2(&poly->x2);
			gte_ldv0(&cube_verts[cube_faces[i][3]]);
			gte_rtps();
			gte_stsxy(&poly->x3);
			setRGB0(poly, r, g, b);
		}
	}
}

#define MAX_SNAKE 100
#define GRID_SIZE 100

int main(void) {
	RenderContext ctx;
	TIM_IMAGE tim;
	
	int s_x[MAX_SNAKE], s_y[MAX_SNAKE];
	int s_len = 3;
	int dx = 1, dy = 0;
	int f_x = 5, f_y = 5;
	int frames = 0;
	int dead = 0;

	for(int i = 0; i < s_len; i++) {
		s_x[i] = -i;
		s_y[i] = 0;
	}

	ResetGraph(0);
	FntLoad(960, 0);
	FntOpen(0, 10, 320, 224, 0, 100);
	setup_context(&ctx);
	initializePad();
	spu_init();

	loadTexture(tex_loading, &tim);
	draw_fullscreen_sprite(&ctx, &tim, 1);
	flip_buffers(&ctx);
	draw_fullscreen_sprite(&ctx, &tim, 1);
	flip_buffers(&ctx);

	for (;;) {
		padUpdate();
		if (SysPadT & Pad1Cross) {
			play_sample(snd_bite1, snd_bite1_len, 0);
			break;
		}
		draw_fullscreen_sprite(&ctx, &tim, 1);
		flip_buffers(&ctx);
	}

	loadTexture(tex_control, &tim);
	for (;;) {
		padUpdate();
		if (SysPadT & Pad1Cross) break;
		draw_fullscreen_sprite(&ctx, &tim, 1);
		flip_buffers(&ctx);
	}

	SVECTOR rot = {400, 0, 0}; 

	for (;;) {
		padUpdate();
		
		if (!dead) {
			if ((SysPadT & Pad1Up) && dy == 0) { dx = 0; dy = -1; }
			if ((SysPadT & Pad1Down) && dy == 0) { dx = 0; dy = 1; }
			if ((SysPadT & Pad1Left) && dx == 0) { dx = -1; dy = 0; }
			if ((SysPadT & Pad1Right) && dx == 0) { dx = 1; dy = 0; }

			frames++;
			if (frames > 10) {
				frames = 0;
				for (int i = s_len - 1; i > 0; i--) {
					s_x[i] = s_x[i - 1];
					s_y[i] = s_y[i - 1];
				}
				s_x[0] += dx;
				s_y[0] += dy;

				if (s_x[0] == f_x && s_y[0] == f_y) {
					if (s_len < MAX_SNAKE) s_len++;
					f_x = (rand() % 10) - 5;
					f_y = (rand() % 10) - 5;
					play_sample(snd_bite1, snd_bite1_len, 0);
				}

				for (int i = 1; i < s_len; i++) {
					if (s_x[0] == s_x[i] && s_y[0] == s_y[i]) dead = 1;
				}
			}
		} else {
			FntPrint(0, "GAME OVER\nPRESS X TO RESTART\n");
			FntFlush(0);
			if (SysPadT & Pad1Cross) {
				s_len = 3;
				dx = 1; dy = 0;
				for(int i = 0; i < s_len; i++) { s_x[i] = -i; s_y[i] = 0; }
				dead = 0;
			}
		}

		VECTOR pos;
		pos.vz = 1800;

		pos.vx = f_x * GRID_SIZE;
		pos.vy = f_y * GRID_SIZE;
		draw_cube(&ctx, &rot, &pos, 255, 0, 0);

		for (int i = 0; i < s_len; i++) {
			pos.vx = s_x[i] * GRID_SIZE;
			pos.vy = s_y[i] * GRID_SIZE;
			draw_cube(&ctx, &rot, &pos, 0, 255, 0);
		}

		flip_buffers(&ctx);
	}
	return 0;
}
