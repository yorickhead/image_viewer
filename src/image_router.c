#include "image_router.h"
#include "raylib.h"
#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stdlib.h"

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
