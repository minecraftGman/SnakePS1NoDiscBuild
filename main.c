/*
 * Snake (No-CD, PSn00bSDK build) -- milestone 1
 * ------------------------------------------------
 * This is NOT the full 3D game yet. PSn00bSDK does not provide an
 * equivalent to Sony's high-level "Gs" object/scene library that the
 * original 3D.c/Snake.c rendering was built on, nor a .TMD model loader,
 * so that part still needs to be written as a small custom engine (see
 * the project README for the concrete plan -- the primitive types the
 * models actually use are already identified).
 *
 * What this milestone proves out end-to-end, for real, in CI:
 * - screen/double-buffer setup (psxgpu.h)
 * - the embedded (no-CD) texture assets displaying as 2D sprites
 * (the game's original loading screen + controls screen)
 * - controller input (Controller.c, reused completely unmodified --
 * PadInit()/PadRead() is identical between PsyQ and PSn00bSDK)
 * - SPU sound effect playback from the embedded VAG data
 *
 * Once this builds and boots correctly, the 3D gameplay layer gets added
 * on top of this same foundation.
 */

#include <stdint.h>
#include <stddef.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxetc.h>
#include <psxspu.h>
#include <psxpad.h>
#include <inline_c.h>

#define OT_LEN      8
#define PACKET_LEN  4096

#define SCREEN_XRES 320
#define SCREEN_YRES 240

typedef struct {
	DISPENV  disp;
	DRAWENV  draw;
	uint32_t ot[OT_LEN];
	uint8_t  packets[PACKET_LEN];
} RenderBuffer;

typedef struct {
	RenderBuffer buffers[2];
	uint8_t      *next_packet;
	int          active;
} RenderContext;

/* Embedded assets (no CD needed) -- see AssetsTextures.c / AssetsAudio.c */
extern unsigned char tex_loading[];
extern unsigned char tex_control[];
extern unsigned char snd_bite1[];
extern unsigned int  snd_bite1_len;

/* --- Rendering setup (adapted from PSn00bSDK's beginner/hello example) --- */

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

/* --- Texture loading (drop-in equivalent of the original loadTexture(),
 * using PSn00bSDK's GetTimInfo() in place of Sony Gs library's
 * GsGetTimInfo()) --- */

static TIM_IMAGE loaded_tim;

static void loadTexture(unsigned char imageData[], TIM_IMAGE *out) {
	GetTimInfo((const uint32_t *) imageData, out);
	LoadImage(out->prect, out->paddr);
	DrawSync(0);
	if (out->mode & 0x8) {
		LoadImage(out->crect, out->caddr);
		DrawSync(0);
	}
}

/* Draws a loaded TIM as a full 320x240 background sprite. */
static void draw_fullscreen_sprite(RenderContext *ctx, TIM_IMAGE *tim, int z) {
	POLY_FT4 *poly = (POLY_FT4 *) alloc_packet(ctx, z, sizeof(POLY_FT4));

	setPolyFT4(poly);
	setXYWH(poly, 0, 0, SCREEN_XRES, SCREEN_YRES);
	setUVWH(poly, 0, 0, tim->prect->w, tim->prect->h);
	setRGB0(poly, 128, 128, 128);
	poly->tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);
	poly->clut  = getClut(tim->crect->x, tim->crect->y);
}

/* --- Controller input: Controller.c is reused completely unmodified. --- */
extern int  SysPad, SysPadT;
extern void initializePad(void);
extern void padUpdate(void);
#define Pad1Cross PAD_CROSS

/* --- SPU: minimal one-shot playback of an embedded VAG sample. --- */

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
	SpuSetVoiceADSR(voice, 0x00ff, 0x0000);
	SpuSetKey(1, 1 << voice);
}

int main(void) {
	RenderContext ctx;
	TIM_IMAGE tim;

	ResetGraph(0);
	FntLoad(960, 0);
	setup_context(&ctx);

	initializePad();
	spu_init();

	/* Loading screen, drawn twice so it lands in both render buffers,
	 * same as the original PreRender();PreRender(); double-call. */
	loadTexture(tex_loading, &tim);
	draw_fullscreen_sprite(&ctx, &tim, 1);
	FntPrint(-1, "Snake (PSn00bSDK build)\n");
	FntPrint(-1, "Press X to continue\n");
	FntFlush(-1);
	flip_buffers(&ctx);

	draw_fullscreen_sprite(&ctx, &tim, 1);
	flip_buffers(&ctx);

	/* Wait for X, playing the bite sound as a smoke test that SPU works. */
	for (;;) {
		padUpdate();
		if (SysPadT & Pad1Cross) {
			play_sample(snd_bite1, snd_bite1_len, 0);
			break;
		}

		draw_fullscreen_sprite(&ctx, &tim, 1);
		flip_buffers(&ctx);
	}

	/* Controls screen. */
	loadTexture(tex_control, &tim);
	for (;;) {
		padUpdate();
		draw_fullscreen_sprite(&ctx, &tim, 1);
		if (SysPadT & Pad1Cross) {
			FntPrint(-1, "3D gameplay layer not yet ported -- see README\n");
			FntFlush(-1);
		}
		flip_buffers(&ctx);
	}

	return 0;
}