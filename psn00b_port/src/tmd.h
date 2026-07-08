#ifndef _TMD_H_
#define _TMD_H_

#include <stdint.h>
#include <psxgte.h>

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

typedef struct {
    uint8_t olen;
    uint8_t ilen;
    uint8_t flag;
    uint8_t mode;
} TMD_PRIM_HDR;

// Forward declaration of RenderContext to avoid circular includes
typedef struct RenderContext RenderContext;

void load_tmd(uint8_t *data);
void draw_tmd(RenderContext *ctx, SVECTOR *rot, VECTOR *pos);

#endif