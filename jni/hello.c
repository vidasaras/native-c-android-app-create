#include <android_native_app_glue.h>
#include <android/log.h>
#include <android/native_window.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define LOG_TAG "NativeHello"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Complete 5x7 font array for printable ASCII characters
static const uint8_t font5x7[128][5] = {
  [' '] = {0x00,0x00,0x00,0x00,0x00},
  ['!'] = {0x00,0x00,0x5F,0x00,0x00},
  [','] = {0x00,0x05,0x06,0x00,0x00},
  ['H'] = {0x7F,0x08,0x08,0x08,0x7F},
  ['W'] = {0x7C,0x02,0x01,0x02,0x7C},
  ['a'] = {0x20,0x54,0x54,0x54,0x78},
  ['d'] = {0x08,0x14,0x14,0x14,0x7F},
  ['e'] = {0x38,0x54,0x54,0x54,0x18},
  ['l'] = {0x00,0x41,0x7F,0x40,0x00},
  ['o'] = {0x38,0x44,0x44,0x44,0x38},
  ['r'] = {0x7C,0x08,0x04,0x04,0x08},
};

static void draw_char_with_stride(uint32_t *pixels, int w, int h, int stride, int x, int y, char c, uint32_t color) {
    if (c < 0 || c >= 128) return;

    for (int dx = 0; dx < 5; ++dx) {
        uint8_t col = font5x7[(int)c][dx];
        for (int dy = 0; dy < 7; ++dy) {
            if (col & (1 << dy)) {
                int px = x + dx, py = y + dy;
                if (px >= 0 && px < w && py >= 0 && py < h)
                    pixels[py * stride + px] = color;
            }
        }
    }
}

static void draw_text_with_stride(uint32_t *pixels, int w, int h, int stride, int x, int y, const char *text, uint32_t color) {
    for (int i = 0; text[i]; ++i)
        draw_char_with_stride(pixels, w, h, stride, x + i * 6, y, text[i], color);
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      LOGI("Window initialized");
      if (app->window) {
        // Set window format and make fullscreen
        ANativeWindow_setBuffersGeometry(app->window, 0, 0, WINDOW_FORMAT_RGBA_8888);

        // Get actual window dimensions
        int32_t width = ANativeWindow_getWidth(app->window);
        int32_t height = ANativeWindow_getHeight(app->window);
        LOGI("Window size: %dx%d", width, height);
      }
      break;
    case APP_CMD_TERM_WINDOW:
      LOGI("Window terminated");
      break;
    case APP_CMD_GAINED_FOCUS:
      LOGI("Gained focus");
      break;
    case APP_CMD_LOST_FOCUS:
      LOGI("Lost focus");
      break;
  }
}

static float rotation = 0.0f;

static void draw_rectangle_with_stride(uint32_t *pixels, int w, int h, int stride, int cx, int cy, int width, int height, float angle, uint32_t color) {
  float cos_a = cosf(angle);
  float sin_a = sinf(angle);

  // Draw filled rectangle
  for (int y = -height/2; y <= height/2; y++) {
    for (int x = -width/2; x <= width/2; x++) {
      int rx = (int)(x * cos_a - y * sin_a) + cx;
      int ry = (int)(x * sin_a + y * cos_a) + cy;

      if (rx >= 0 && rx < w && ry >= 0 && ry < h) {
        pixels[ry * stride + rx] = color;  // Use stride here
      }
    }
  }
}

void android_main(struct android_app* app) {
  app->onAppCmd = handle_cmd;

  // Prevent stripping of native_app_glue
  app_dummy();

  LOGI("Starting main loop");

  // Main loop
  while (1) {
    int events;
    struct android_poll_source* source;

    // Poll events with timeout 0 to process input and lifecycle events
    while (ALooper_pollOnce(0, NULL, &events, (void**)&source) >= 0) {
      if (source) {
        source->process(app, source);
      }

      if (app->destroyRequested != 0) {
        LOGI("App destroy requested, exiting");
        return;
      }
    }

    // Only draw if window is valid
    if (app->window) {
      ANativeWindow_Buffer buffer;
      int32_t result = ANativeWindow_lock(app->window, &buffer, NULL);
      if (result == 0) {
        uint32_t *pixels = (uint32_t*)buffer.bits;
        int w = buffer.width;
        int h = buffer.height;
        int stride = buffer.stride;  // This is the key fix

        LOGI("Buffer: %dx%d, stride: %d", w, h, stride);

        // Clear screen using stride
        for (int y = 0; y < h; y++) {
          for (int x = 0; x < w; x++) {
            pixels[y * stride + x] = 0xFF000000;  // Use stride instead of w
          }
        }

        // Draw rectangle using stride
        int rect_width = w / 4;  // Make it proportional to screen
        int rect_height = h / 4;
        draw_rectangle_with_stride(pixels, w, h, stride, w/2, h/2, rect_width, rect_height, rotation, 0xFFFFFFFF);

        rotation += 0.02f;  // Slower rotation

        const char *msg = "Hello, World!";
        int text_width = strlen(msg) * 6;
        int x = (w - text_width) / 2;
        int y = 20;  // 20 pixels from top

        draw_text_with_stride(pixels, w, h, stride, x, y, msg, 0xFFFFFFFF);

        ANativeWindow_unlockAndPost(app->window);
      }
    }

    // Sleep a bit to reduce CPU usage
    usleep(16667); // ~60 FPS
  }
}
