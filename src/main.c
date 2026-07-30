#include "raylib.h"

#include "stb_image.h"

#include "stb_image_resize2.h"

#include <stddef.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "camera.h"
#include "loader.h"

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 800

void print_image_info(long size, int w, int h, int channels) {

  char size_buff[32];
  sprintf(size_buff, "Size: %ld", size);

  char w_buff[16];
  sprintf(w_buff, "Width: %d", w);

  char h_buff[16];
  sprintf(h_buff, "Height: %d", h);

  char ch_buff[16];
  sprintf(ch_buff, "Channels: %d", channels);

  int x = 600;
  int y = 10;

  DrawText(size_buff, x, y, 20, WHITE);
  DrawText(w_buff, x, y + 25, 20, WHITE);
  DrawText(h_buff, x, y + 50, 20, WHITE);
  DrawText(ch_buff, x, y + 75, 20, WHITE);
}

void resize_image(Image *img, int target_width, int channels) {
  if (img->width <= 0 || img->height <= 0)
    return;

  int out_w = target_width;
  int out_h = (int)((float)img->height * target_width / img->width);

  if (out_h < 1)
    out_h = 1;

  unsigned char *output_pixels =
      (unsigned char *)malloc((size_t)out_w * out_h * channels);
  if (!output_pixels) {
    TraceLog(LOG_ERROR, "Не удалось выделить память для ресайза");
    return;
  }

  stbir_pixel_layout layout;
  switch (channels) {
  case 1:
    layout = STBIR_1CHANNEL;
    break;
  case 2:
    layout = STBIR_2CHANNEL;
    break;
  case 3:
    layout = STBIR_RGB;
    break;
  case 4:
    layout = STBIR_RGBA;
    break;
  default:
    free(output_pixels);
    return;
  }

  stbir_resize(img->data, img->width, img->height, 0, output_pixels, out_w,
               out_h, 0, layout, STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP,
               STBIR_FILTER_DEFAULT);

  stbi_image_free(img->data);

  img->data = output_pixels;
  img->width = out_w;
  img->height = out_h;
}

long get_file_size(char *filename) {
  FILE *fp = fopen(filename, "r");

  if (fp == NULL)
    return -1;

  if (fseek(fp, 0, SEEK_END) < 0) {
    fclose(fp);
    return -1;
  }

  long size = ftell(fp);
  // release the resources when not required
  fclose(fp);
  return size;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Использование: %s <путь_к_изображению>\n", argv[0]);
    return 1;
  }

  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Просмотр изображения");

  ImageWithChannels imgWChs = load_image_by_stb(argv[1]);
  if (imgWChs.img.data == NULL) {
    CloseWindow();
    return 1;
  }

  resize_image(&imgWChs.img, WINDOW_WIDTH, imgWChs.channels);

  Texture2D texture = LoadTextureFromImage(imgWChs.img);
  UnloadImage(imgWChs.img);

  long file_size = get_file_size(argv[1]);

  SetTargetFPS(60);

  Font font = load_font();

  // zoom
  Camera2D camera = {0};
  camera.zoom = 1.0f;

  int zoomMode = 0; // 0-Mouse Wheel, 1-Mouse Move

  while (!WindowShouldClose()) {

    // zoom
    route_camera(&camera, &zoomMode, WINDOW_WIDTH, WINDOW_HEIGHT);

    // draw
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(camera);
    DrawTexture(texture, WINDOW_WIDTH / 2 - texture.width / 2,
                WINDOW_HEIGHT / 2 - texture.height / 2, WHITE);

    EndMode2D();
    DrawTextEx(font, "Нажми Esc чтобы выйти", (Vector2){10, 10}, 32, 1, WHITE);
    print_image_info(file_size, imgWChs.img.width, imgWChs.img.height,
                     imgWChs.channels);
    EndDrawing();
  }

  UnloadFont(font);
  UnloadTexture(texture);
  CloseWindow();
  return 0;
}
