/*

Thumpa demo for the Pebble watch
================================

Copyright (C) 2013 Michael Toksvig

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#pragma GCC optimize ("O3")

#define MY_UUID { 0x92, 0x8E, 0x6B, 0xB2, 0xAD, 0x56, 0x49, 0x82, 0xB5, 0x88, 0xA0, 0x1B, 0xCD, 0x64, 0x6D, 0x48 }

PBL_APP_INFO(
  MY_UUID,
  "Thumpa", "Michael Toksvig",
  1, 0, /* App version */
  DEFAULT_MENU_ICON, APP_INFO_STANDARD_APP
);

#define SCREEN_TOTAL_WIDTH 144
#define SCREEN_TOTAL_HEIGHT 168
#define SCREEN_MARGIN 10
#define SCREEN_LEFT 0
#define SCREEN_TOP SCREEN_MARGIN
#define SCREEN_RIGHT SCREEN_TOTAL_WIDTH
#define SCREEN_BOTTOM (SCREEN_TOTAL_HEIGHT-SCREEN_MARGIN)
#define SCREEN_CENTER_X ((SCREEN_LEFT+SCREEN_RIGHT)/2)
#define SCREEN_CENTER_Y ((SCREEN_TOP+SCREEN_BOTTOM)/2)

#define PLASMA true
#define PLASMA_SCALE 20
#define PLASMA_SPEED 13
#define PLASMA_STRIDE 2
#define PLASMA_MASK ((1<<PLASMA_STRIDE)-1)

#define MODEL true
#define MODEL_SPEED 13

#define THUMPA true

#define LOG PBL_LOG_APP

Window window;

BmpContainer bitmap_container;

#include "cobra_model.h"
#include "vec_mat.h"

Layer line_layer;

int model_scale = MODEL_SCALE;

// 33 patterns, each 8w x 4h x 1 bit

// to render grayscale value g at (x, y), you write e.g.:
//   framebuffer |= dither[g][y%4] & (0x1 << (x%8))

// notice that you can render e.g. 4 bits at once:
//   framebuffer |= dither[g][y%4] & (0xF << (x%8))

uint8_t dither[33][4] = { { 0 } };

// extract bit at position position in byte byte
uint8_t bit(uint8_t byte, uint8_t position) {
  return (byte>>position) & 1;
}

// initalize the dither table
void dither_init() {
  for (uint8_t level = 1; level < ARRAY_LENGTH(dither); ++level) {
    // copy previous level
    for (uint8_t y = 0; y < ARRAY_LENGTH(dither[level]); ++y)
      dither[level][y] = dither[level-1][y];
    // we want to distribute the increments as much as possible
    // so put every other bit into x and every other into y
    // and put the lsbs in the msbs and vice versa
    uint8_t i = level-1;
    uint8_t x =
      bit(i, 0) << 2 |
      bit(i, 2) << 1 |
      bit(i, 4) << 0;
    uint8_t y =
      bit(i, 1) << 1 |
      bit(i, 3) << 0;
    // we want a pair of bits to spell out an X, not a Z
    // so xor y into x
    // yeah, head hurts, but it does work, i think
    x ^= y;
    // set the bit
    dither[level][y] |= 1<<x;
  }
}

int8_t sin_table[256] = { 0 };

void sin_table_init() {
  for (unsigned i = 0; i<ARRAY_LENGTH(sin_table); ++i)
    sin_table[i] = sin_lookup(i<<8)>>9;
}

int8_t fast_sin(uint16_t i) {
  return sin_table[i>>8];
}

void line_layer_update_callback(Layer *me, GContext* ctx) {
  // the rotation around z
  static int32_t anglez = 0;
  static int32_t anglex = 0;
  unsigned i;
  
  (void)me;

  graphics_context_set_stroke_color(ctx, GColorBlack);

  // rotate unit matrix around z
  int32_t m0[3][3]; mrotz(m0, munit, anglez); mshift(m0, m0, 8);
  int32_t m[3][3]; mrotx(m, m0, anglex); mshift(m, m, 8);

  // now draw each edge
  for (i = 0; i < ARRAY_LENGTH(edges); ++i) {
    GPoint a, b;

    screen_transform(&a, m, verts[edges[i][0]], model_scale);
    screen_transform(&b, m, verts[edges[i][1]], model_scale);

    graphics_draw_line(ctx, a, b);
  }

  anglex += MODEL_SPEED*37;
  anglez += MODEL_SPEED*50;
}

// take values in the +/-1<<9 range
// remap them to the 0..32 range somehow
int32_t remap(int32_t x) {
  x = (x>>7)+(1<<2); // 0..7
  int32_t result[] = { 0, 4, 2, 6, 1, 5, 3, 7 };
  if (MODEL)
    return result[x]*2+18; // pale or you can't see model
  return result[x]*4+2; // darker
}

void plasma_effect(GBitmap *bitmap) {
  // int16_t w = bitmap->bounds.size.w;
  int16_t h = bitmap->bounds.size.h;
  uint32_t stride = bitmap->row_size_bytes;
  uint8_t *bytes = bitmap->addr;

  static uint16_t fx0, fx1, fy0, fy1;
  uint16_t y0 = fy0, y1 = fy1;
  for (int y = 0; y < h; ++y) {
    int32_t plasma0 = fast_sin(y0) + fast_sin(y1);

    uint16_t x0 = fx0, x1 = fx1;
    for (uint16_t i = 0; i < stride; ++i) {
      uint8_t buf = 0;
      for (uint16_t j = 0; j < 8; j += PLASMA_STRIDE) {
        // compute plasma
        int32_t plasma = plasma0 + fast_sin(x0) + fast_sin(x1);
        // remap into bands
        plasma = remap(plasma);
        // or PLASMA_MASK bits of dither into buf
        buf |= dither[plasma][y%4] & (PLASMA_MASK << j);

        x0 += PLASMA_SCALE*PLASMA_STRIDE*47;
        x1 += PLASMA_SCALE*PLASMA_STRIDE*26;
      }
      bytes[y*stride+i] = buf;
    }
    y0 += PLASMA_SCALE*9;
    y1 += PLASMA_SCALE*33;
  }

  fx0 += PLASMA_SPEED*77;
  fy0 += PLASMA_SPEED*93;
  fx1 += PLASMA_SPEED*-12;
  fy1 += PLASMA_SPEED*17;
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
  (void)handle;

  if (MODEL)
    layer_mark_dirty(&line_layer);

  if (PLASMA) {
    plasma_effect(&bitmap_container.bmp);
    layer_mark_dirty(&bitmap_container.layer.layer);
  }

  if (THUMPA) {
    static int scales[] = {
      MODEL_SCALE-2, MODEL_SCALE-2, MODEL_SCALE-2, MODEL_SCALE-2,
      MODEL_SCALE-2, MODEL_SCALE-2, MODEL_SCALE-2, MODEL_SCALE-2,
      MODEL_SCALE-1, MODEL_SCALE+0, MODEL_SCALE+1, MODEL_SCALE+1,
      MODEL_SCALE+0, MODEL_SCALE-1,
    };

    static int frame = 0;

    ++frame;
    if (frame==ARRAY_LENGTH(scales))
      frame = 0;

    model_scale = scales[frame];

    if (frame==7) {
      uint32_t durations[] = { 50, 50 };
      static VibePattern pattern;

      pattern.durations = durations;
      pattern.num_segments = 2;

      vibes_enqueue_custom_pattern(pattern);
    }
  }

  // if milliseconds is 0 or 1, bad things happen
  app_timer_send_event(ctx, 3 /* milliseconds */, cookie);
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Demo");
  window_stack_push(&window, false /* Not animated */);

  resource_init_current_app(&APP_RESOURCES);

  dither_init();
  sin_table_init();

  if (PLASMA) {
    bmp_init_container(RESOURCE_ID_IMAGE_TOX, &bitmap_container);

    layer_add_child(&window.layer, &bitmap_container.layer.layer);
  }

  if (MODEL) {
    layer_init(&line_layer, window.layer.frame);
    line_layer.update_proc = &line_layer_update_callback;
    layer_add_child(&window.layer, &line_layer);
  }

  // Start the animation.
  app_timer_send_event(ctx, 50 /* ms */, 0 /* Not cookie */);
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  if (PLASMA)
    bmp_deinit_container(&bitmap_container);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .timer_handler = &handle_timer
  };
  app_event_loop(params, &handlers);
}
