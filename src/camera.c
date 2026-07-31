#include "camera.h"

#include "raylib.h"
#include "raymath.h"

void route_keys(Camera2D *camera, int *zoomMode) {
  if (IsKeyPressed(KEY_R)) {
    camera->target =
        (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera->offset =
        (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera->rotation = 0.0f;
    camera->zoom = 1.0f;
  }

  if (IsKeyDown(KEY_EQUAL)) {
    float scale = 0.008f;
    camera->zoom = camera->zoom + scale;
  }

  if (IsKeyDown(KEY_MINUS)) {
    float scale = 0.008f;
    camera->zoom = camera->zoom - scale;
  }

  int count = 3;

  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
    camera->target.y -= count;
  }

  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
    camera->target.y += count;
  }

  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
    camera->target.x -= count;
  }

  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
    camera->target.x += count;
  }

  if (IsKeyPressed(KEY_ONE))
    *zoomMode = 0;
  else if (IsKeyPressed(KEY_TWO))
    *zoomMode = 1;

  // Translate based on mouse right click
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 delta = GetMouseDelta();
    delta = Vector2Scale(delta, -1.0f / camera->zoom);
    camera->target = Vector2Add(camera->target, delta);
  }
}

void route_camera(Camera2D *camera, int *zoomMode, int w, int h) {

  route_keys(camera, zoomMode);

  if (*zoomMode == 0) {
    // Zoom based on mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      // Get the world point that is under the mouse
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

      // Set the offset to where the mouse is
      camera->offset = GetMousePosition();

      // Set the target to match, so that the camera maps the world space
      // point under the cursor to the screen space point under the cursor at
      // any zoom
      camera->target = mouseWorldPos;

      // Zoom increment
      // Uses log scaling to provide consistent zoom speed
      float scale = 0.2f * wheel;
      camera->zoom = Clamp(expf(logf(camera->zoom) + scale), 0.125f, 64.0f);
    }
  } else {
    // Zoom based on mouse right click
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      // Get the world point that is under the mouse
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);
      // Set the target to match, so that the camera maps the world space
      // point under the cursor to the screen space point under the cursor at
      // any zoom
      camera->target = mouseWorldPos;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      // Zoom increment
      // Uses log scaling to provide consistent zoom speed
      float deltaX = GetMouseDelta().x;
      float scale = 0.005f * deltaX;
      camera->zoom = Clamp(expf(logf(camera->zoom) + scale), 0.125f, 64.0f);
    }
  }
}
