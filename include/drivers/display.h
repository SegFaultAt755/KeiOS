#pragma once

#include <stdint.h>
#include <stdbool.h>

struct display_info {
    uint32_t *lfb_addr;
    uint32_t  flags;
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;
    uint8_t   bpp;
};

/* Core operations */
int  display_initialize(struct display_info info);
void display_clear(uint32_t color);
void display_draw_pixel(int32_t x, int32_t y, uint32_t color);

/* Shape engine */
void display_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
void display_draw_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);
void display_fill_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);

/* Getters */
uint32_t  display_get_width(void);
uint32_t  display_get_height(void);
uint32_t  display_get_pitch(void);
uint8_t   display_get_bpp(void);
uint32_t  display_get_flags(void);
uint32_t* display_get_lfb_addr(void);

/**
* Reads a pixel from the framebuffer safely
* @return 0 on success, 1 if the coordinates are out of bounds or unitialized
*/
int32_t display_get_pixel(int32_t x, int32_t y, uint32_t *out_color);

/* Setters */
void display_set_lfb_addr(uint32_t *new_lfb);
void display_set_resolution(uint32_t width, uint32_t height, uint32_t pitch);

/* Bitmap engine */
bool display_draw_bitmap(const uint8_t *bmp_data, int32_t x, int32_t y);
bool display_draw_bitmap_ex(const uint8_t *bmp_data, int32_t dst_x, int32_t dst_y,
                            uint32_t scale_num, uint32_t scale_den,
                            int32_t rotation_deg, uint8_t alpha);
bool display_draw_bitmap_scaled(const uint8_t *bmp_data, int32_t x, int32_t y,
                                uint32_t scale_num, uint32_t scale_den);
bool display_draw_bitmap_rotated(const uint8_t *bmp_data, int32_t x, int32_t y,
                                 int32_t degrees);
bool display_draw_bitmap_alpha(const uint8_t *bmp_data, int32_t x, int32_t y,
                               uint8_t alpha);
