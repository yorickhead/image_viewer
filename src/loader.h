#include "raylib.h"

Font load_font();

typedef struct {
  Image img;
  int channels;
} ImageWithChannels;

ImageWithChannels load_image_by_stb(const char *filepath);
