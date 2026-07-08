#ifndef _TMD_H_
#define _TMD_H_

#include <stdint.h>
#include <psxgte.h>
#include <psxgpu.h>

#define OT_LEN      4096
#define PACKET_LEN  32768

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

typedef struct {
    uint32_t id;
    uint32_t flags;
    uint32_t bck;
    uint32_t num_objs;
} TMD_HEADER;

typedef struct {
    uint32_t vert_top;
    uint32_t num_verts;
    uint32_t normal_top;
    uint32_t num_normals;
    uint32_t prim_top;
    uint32_t num_prims;
    int32_t  scale;
} TMD_OBJ;

void load_tmd(uint8_t *data);
void draw_tmd(RenderContext *ctx, SVECTOR *rot, VECTOR *pos);

#endif
