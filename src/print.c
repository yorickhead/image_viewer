#include "print.h"

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

void get_formated_size(long size, char *size_buf[]) {
  long bites = size / 8;
  if (bites < 1024) {
    sprintf(*size_buf, "Size: %ld Bites", bites);

    return;
  }

  long kbites = bites / 1024;
  if (kbites < 1000) {
    sprintf(*size_buf, "Size: %ld Kb", kbites);

    return;
  }

  int mbites = kbites / 1000;

  sprintf(*size_buf, "Size: %d Mb", mbites);
}

void print_image_info(long size, int w, int h, int channels) {
  char *size_buf = calloc(32, sizeof(char));
  get_formated_size(size, &size_buf);

  char w_buff[16];
  sprintf(w_buff, "Width: %d", w);

  char h_buff[16];
  sprintf(h_buff, "Height: %d", h);

  char ch_buff[16];
  sprintf(ch_buff, "Channels: %d", channels);

  int x = 600;
  int y = 10;

  DrawText(size_buf, x, y, 20, WHITE);
  DrawText(w_buff, x, y + 25, 20, WHITE);
  DrawText(h_buff, x, y + 50, 20, WHITE);
  DrawText(ch_buff, x, y + 75, 20, WHITE);

  free(size_buf);
}

void print_cursor_cords(int width, int height, Texture texture,
                        Camera2D camera) {
  Vector2 mouseScreen = GetMousePosition();
  Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);

  float texX = width / 2.0f - texture.width / 2.0f;
  float texY = height / 2.0f - texture.height / 2.0f;

  int imgX = (int)(mouseWorld.x - texX);
  int imgY = (int)(mouseWorld.y - texY);

  char x_buff[16];
  sprintf(x_buff, "X: %d", imgX);

  char y_buff[16];
  sprintf(y_buff, "Y: %d", imgY);

  DrawText(x_buff, 400, 20, 24, YELLOW);
  DrawText(y_buff, 400, 45, 24, YELLOW);
}
