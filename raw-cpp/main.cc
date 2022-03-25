#include <algorithm>
#include <cmath>
#include <iostream>

#include "SDL.h"
#include "ecs.h"

void draw_circle(SDL_Renderer *renderer, float raw_center_x, float raw_center_y,
                 float raw_radius, Color color, int width, int height) {
  int center_x = std::round(raw_center_x * width);
  int center_y = std::round((1.0 - raw_center_y) * height);
  int radius = std::max(
      static_cast<int>(std::round(raw_radius * (height + width) / 2.0)), 1);

  // Setting the color to be RED with 100% opaque (0% trasparent).
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

  // Drawing circle
  int r2 = radius * radius;
  for (int x = std::max(center_x - radius, 0);
       x <= std::min(center_x + radius, width); ++x) {
    int x2 = (center_x - x) * (center_x - x);
    for (int y = std::max(center_y - radius, 0);
         y <= std::min(center_y + radius, height); ++y) {
      int y2 = (center_y - y) * (center_y - y);
      if (y2 + x2 <= r2) {
        SDL_RenderDrawPoint(renderer, x, y);
      }
    }
  }
}

int main() {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    return 3;
  }

  window = SDL_CreateWindow("ecs-test", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, 800, 600, 0);
  if (window == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s",
                 SDL_GetError());
    return 3;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s",
                 SDL_GetError());
    return 3;
  }

  ECS ecs(100);
  int h, w;
  int32_t frames = 0, current_frame = 0, spawn_interval = 15;
  Uint32 last_print = SDL_GetTicks();
  while (true) {
    Uint64 start = SDL_GetPerformanceCounter();

    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
      break;
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
    SDL_GetWindowSize(window, &w, &h);
    for (const auto &to_draw :
         ecs.Step(current_frame, spawn_interval, spawn_interval)) {
      draw_circle(renderer, to_draw.x, to_draw.y, to_draw.radius, to_draw.color,
                  w, h);
    }
    SDL_RenderPresent(renderer);
    ++current_frame;
    ++frames;

    if (SDL_GetTicks() - last_print > 1000) {
      std::cout << "Current FPS: " << std::to_string(frames) << '\n';
      frames = 0;
      last_print = SDL_GetTicks();
    }

    // Cap to 60 FPS
    Uint64 end = SDL_GetPerformanceCounter();
    float elapsedMS =
        (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
    SDL_Delay(floor(1000.0 / 60.0 - elapsedMS));
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
