#include "color_picker.h"
#include <stdio.h>

void get_color_under_cursor(int width, int height, Texture texture, Image image,
                            Camera2D camera) {
  Vector2 mouseScreen = GetMousePosition();
  Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);

  float texX = width / 2.0f - texture.width / 2.0f;
  float texY = height / 2.0f - texture.height / 2.0f;

  int imgX = (int)(mouseWorld.x - texX);
  int imgY = (int)(mouseWorld.y - texY);

  Color pixelColor = BLANK;
  bool validPixel = false;

  if (imgX >= 0 && imgX < texture.width && imgY >= 0 && imgY < texture.height) {
    pixelColor = GetImageColor(image, imgX, imgY);
    validPixel = true;
  }

  if (validPixel) {
    DrawRectangle(190, 50, 40, 40, pixelColor);
    DrawRectangleLines(190, 50, 40, 40, WHITE);

    char colorText[64];
    sprintf(colorText, "RGB: %d, %d, %d\nHEX: #%02X%02X%02X", pixelColor.r,
            pixelColor.g, pixelColor.b, pixelColor.r, pixelColor.g,
            pixelColor.b);

    DrawText(colorText, 250, 50, 1, WHITE);
  }
}
