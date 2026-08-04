#define _POSIX_C_SOURCE 200809L

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "stb_image.h"
#include "stb_image_resize2.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "camera.h"
#include "color_picker.h"
#include "image_router.h"
#include "loader.h"
#include "print.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

/* ---------- helpers ---------- */

static long get_file_size(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (!fp)
    return -1;

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return -1;
  }

  long size = ftell(fp);
  fclose(fp);
  return size;
}

static char **get_filenames_in_dir(const char *dir, int *out_count) {
  DIR *dr = opendir(dir);
  if (!dr) {
    TraceLog(LOG_ERROR, "Cannot open dir: %s", dir);
    return NULL;
  }

  // count regular files
  int count = 0;
  struct dirent *de;
  while ((de = readdir(dr)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
      continue;

    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, de->d_name);

    struct stat st;
    if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode))
      count++;
  }

  char **filenames = calloc(count + 1, sizeof(char *));
  if (!filenames) {
    closedir(dr);
    return NULL;
  }

  rewinddir(dr);
  int i = 0;
  while ((de = readdir(dr)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
      continue;

    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, de->d_name);

    struct stat st;
    if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {
      filenames[i] = strdup(fullpath);
      if (!filenames[i]) {
        for (int j = 0; j < i; j++)
          free(filenames[j]);
        free(filenames);
        closedir(dr);
        return NULL;
      }
      i++;
    }
  }

  closedir(dr);
  filenames[count] = NULL;
  if (out_count)
    *out_count = count;
  return filenames;
}

static void free_filenames(char **filenames) {
  if (!filenames)
    return;
  for (int i = 0; filenames[i] != NULL; i++)
    free(filenames[i]);
  free(filenames);
}

/* Load image, resize to window width, create texture */
static bool load_view_image(const char *path, ImageWithChannels *out_img,
                            Texture2D *out_tex, int *out_orig_w,
                            int *out_orig_h, long *out_size) {
  ImageWithChannels img = load_image_by_stb(path);
  if (img.img.data == NULL)
    return false;

  *out_orig_w = img.img.width;
  *out_orig_h = img.img.height;
  *out_size = get_file_size(path);

  resize_image(&img.img, WINDOW_WIDTH, img.channels);

  *out_tex = LoadTextureFromImage(img.img);
  *out_img = img;
  return true;
}

static void unload_view_image(ImageWithChannels *img, Texture2D *tex) {
  if (tex && tex->id > 0)
    UnloadTexture(*tex);
  if (img && img->img.data)
    UnloadImage(img->img);

  memset(img, 0, sizeof(*img));
  memset(tex, 0, sizeof(*tex));
}

/* ---------- main ---------- */

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage:\n");
    printf("  %s <image>\n", argv[0]);
    printf("  %s -d <directory>\n", argv[0]);
    return 1;
  }

  bool dir_mode = (strcmp(argv[1], "-d") == 0);

  if (dir_mode && argc < 3) {
    printf("Usage: %s -d <directory>\n", argv[0]);
    return 1;
  }

  const char *path = dir_mode ? argv[2] : argv[1];

  char **filenames = NULL;
  int file_count = 0;
  int index = 0;

  if (dir_mode) {
    filenames = get_filenames_in_dir(path, &file_count);
    if (!filenames || file_count == 0) {
      TraceLog(LOG_ERROR, "No files in directory: %s", path);
      free_filenames(filenames);
      return 1;
    }
    path = filenames[0];
  }

  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Image Viewer");
  SetTargetFPS(60);

  ImageWithChannels imgWChs = {0};
  Texture2D texture = {0};
  int image_width = 0;
  int image_height = 0;
  long file_size = -1;

  if (!load_view_image(path, &imgWChs, &texture, &image_width, &image_height,
                       &file_size)) {
    TraceLog(LOG_ERROR, "Failed to load: %s", path);
    free_filenames(filenames);
    CloseWindow();
    return 1;
  }

  Font font = load_font();

  Camera2D camera = {0};
  camera.zoom = 1.0f;
  int zoomMode = 0;

  while (!WindowShouldClose()) {
    /* ---- input: next / prev image in directory mode ---- */
    if (dir_mode && file_count > 0) {
      int new_index = index;

      if (IsKeyPressed(KEY_RIGHT))
        new_index = (index + 1) % file_count;
      if (IsKeyPressed(KEY_LEFT))
        new_index = (index - 1 + file_count) % file_count;

      if (new_index != index) {
        index = new_index;
        unload_view_image(&imgWChs, &texture);

        if (!load_view_image(filenames[index], &imgWChs, &texture, &image_width,
                             &image_height, &file_size)) {
          TraceLog(LOG_ERROR, "Failed to load: %s", filenames[index]);
        }

        // reset camera on image change
        camera.zoom = 1.0f;
        camera.target = (Vector2){0};
        camera.offset = (Vector2){0};
      }
    }

    if (IsKeyPressed(KEY_F11)) {
      ToggleFullscreen();
    }

    /* ---- rotate ---- */
    if (GuiButton((Rectangle){WINDOW_WIDTH - 60, 30, 50, 50}, "R")) {
      ImageRotate(&imgWChs.img, 90);
      UnloadTexture(texture);
      texture = LoadTextureFromImage(imgWChs.img);

      // after 90° rotation width/height swap
      int tmp = image_width;
      image_width = image_height;
      image_height = tmp;
    }

    /* ---- camera ---- */
    route_camera(&camera, &zoomMode, WINDOW_WIDTH, WINDOW_HEIGHT);

    /* ---- draw ---- */
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(camera);
    DrawTexture(texture, WINDOW_WIDTH / 2 - texture.width / 2,
                WINDOW_HEIGHT / 2 - texture.height / 2, WHITE);
    EndMode2D();

    DrawTextEx(font, "Esc - exit | A/D or Left/Right - prev/next",
               (Vector2){10, 10}, 20, 1, WHITE);

    print_image_info(file_size, image_width, image_height, imgWChs.channels);
    print_cursor_cords(WINDOW_WIDTH, WINDOW_HEIGHT, texture, camera);

    char zoom_buf[32];
    snprintf(zoom_buf, sizeof(zoom_buf), "Zoom: %.0f%%", camera.zoom * 100.0f);
    DrawText(zoom_buf, 10, 70, 26, WHITE);

    get_color_under_cursor(WINDOW_WIDTH, WINDOW_HEIGHT, texture, imgWChs.img,
                           camera);

    EndDrawing();
  }

  /* ---- cleanup ---- */
  unload_view_image(&imgWChs, &texture);
  UnloadFont(font);
  free_filenames(filenames);
  CloseWindow();
  return 0;
}
