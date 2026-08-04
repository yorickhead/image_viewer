#include "loader.h"
#include <stddef.h>

ImageWithChannels load_image_by_stb(const char *filepath) {
  ImageWithChannels result = {0};

  Image img = LoadImage(filepath);
  if (img.data == NULL) {
    TraceLog(LOG_ERROR, "Не удалось загрузить изображение: %s", filepath);
    return result;
  }

  result.img = img;

  switch (img.format) {
  case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
    result.channels = 1;
    break;
  case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
    result.channels = 2;
    break;
  case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
    result.channels = 3;
    break;
  case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
    result.channels = 4;
    break;
  default:
    ImageFormat(&result.img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    result.channels = 4;
    break;
  }

  return result;
}

Font load_font() {
  int codepoints[512] = {0};
  int count = 0;

  for (int i = 32; i < 127; i++) {
    codepoints[count++] = i;
  }

  for (int i = 0x0400; i <= 0x04FF; i++) {
    codepoints[count++] = i;
  }

  Font font = LoadFontEx("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 32,
                         codepoints, count);

  return font;
}
