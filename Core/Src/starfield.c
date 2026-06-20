#include "oled.h"
#include "main.h"
#include <stdlib.h>
#include <string.h>


#define NUM_STARS     80
#define MAX_DEPTH     64      // "distance" — star starts far, approaches 0
#define SPEED          2      // depth units consumed per frame
#define CX            64      // screen center X
#define CY            32      // screen center Y

// star
typedef struct {
    int16_t x;   // 3D x  (-64 to +63)
    int16_t y;   // 3D y  (-32 to +31)
    uint8_t z;   // depth (1 = closest, MAX_DEPTH = farthest)
} Star;

static Star stars[NUM_STARS];

// helpers

/* cheap pseudo-random: good enough for star positions */
static uint16_t rng_state = 0xACE1;
static uint16_t fast_rand(void)
{
    rng_state ^= rng_state << 7;
    rng_state ^= rng_state >> 9;
    rng_state ^= rng_state << 8;
    return rng_state;
}

static void spawn_star(Star *s)
{
    /* place star at a random 3D position far away */
    s->x = (int16_t)(fast_rand() % 128) - 64;   // -64..+63
    s->y = (int16_t)(fast_rand() %  64) - 32;   // -32..+31
    s->z = MAX_DEPTH;                             // start far
}

static void init_stars(void)
{
    for (uint8_t i = 0; i < NUM_STARS; i++) {
        spawn_star(&stars[i]);
        /* scatter depths so the field looks full from frame 1 */
        stars[i].z = (uint8_t)((fast_rand() % MAX_DEPTH) + 1);
    }
}

/*
 * Perspective projection:
 *   screen_x = (3D_x / z) * SCALE + CENTER
 *
 * We use MAX_DEPTH as the scale factor — when z == MAX_DEPTH the star
 * sits almost at the center; as z → 1 it flies to the edge.
 */
static inline int16_t project_x(int16_t x3d, uint8_t z)
{
    return (int16_t)((x3d * MAX_DEPTH) / z + CX);
}

static inline int16_t project_y(int16_t y3d, uint8_t z)
{
    return (int16_t)((y3d * MAX_DEPTH) / z + CY);
}

// ─── Public entry point ───────────────────────────────────────────────────────
void Starfield_Run(I2C_HandleTypeDef *hi2c)
{
    init_stars();

    while (1)
    {
        OLED_Fill(0);   // clear buffer

        for (uint8_t i = 0; i < NUM_STARS; i++)
        {
            Star *s = &stars[i];

            /* move star closer */
            if (s->z <= SPEED) {
                spawn_star(s);   // flew past us — recycle
                continue;
            }
            s->z -= SPEED;

            /* project to 2D */
            int16_t sx = project_x(s->x, s->z);
            int16_t sy = project_y(s->y, s->z);

            /* clip — star left the screen */
            if (sx < 0 || sx >= 128 || sy < 0 || sy >= 64) {
                spawn_star(s);
                continue;
            }

            /*
             * Star brightness / size by depth:
             *   far  (z > MAX_DEPTH*2/3) → single dim pixel
             *   mid  (z > MAX_DEPTH*1/3) → single bright pixel
             *   near (z ≤ MAX_DEPTH*1/3) → 2×2 blob so it "blooms"
             */
            if (s->z > (MAX_DEPTH * 2 / 3)) {
                /* dim — only draw every other star to thin them out */
                if (i & 1) OLED_DrawPixel(sx, sy, 1);
            } else if (s->z > (MAX_DEPTH / 3)) {
                OLED_DrawPixel(sx, sy, 1);
            } else {
                /* close — 2×2 bloom */
                OLED_DrawPixel(sx,   sy,   1);
                OLED_DrawPixel(sx+1, sy,   1);
                OLED_DrawPixel(sx,   sy+1, 1);
                OLED_DrawPixel(sx+1, sy+1, 1);
            }
        }

        OLED_UpdateScreen(hi2c);
        HAL_Delay(30);   // ~33 fps — tweak to taste
    }
}
