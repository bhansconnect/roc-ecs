#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

#include "SDL.h"
#if defined(SIMPLE_ECS)
#include "simple-ecs.h"
const char *NAME = "simple-ecs";
#elif defined(ACTON_ECS)
#include "acton-inspired-ecs.h"
const char *NAME = "acton-ecs";
#endif

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

void draw_circle(Color *pixels, float raw_center_x, float raw_center_y,
                 float raw_radius, Color color) {
  int center_x = std::round(raw_center_x * WIDTH);
  int center_y = std::round((1.0 - raw_center_y) * HEIGHT);
  int radius = std::max(
      static_cast<int>(std::round(raw_radius * (HEIGHT + WIDTH) / 2.0)), 1);

  uint8_t new_a_flip = (255 - color.a);
  uint16_t new_ra = static_cast<uint16_t>(color.r * color.a) / 255;
  uint16_t new_ga = static_cast<uint16_t>(color.g * color.a) / 255;
  uint16_t new_ba = static_cast<uint16_t>(color.b * color.a) / 255;

  int r2 = radius * radius;
  for (int x = std::max(center_x - radius, 0);
       x < std::min(center_x + radius, WIDTH); ++x) {
    int x2 = (center_x - x) * (center_x - x);
    for (int y = std::max(center_y - radius, 0);
         y < std::min(center_y + radius, HEIGHT); ++y) {
      int y2 = (center_y - y) * (center_y - y);
      if (y2 + x2 <= r2) {
        Color &pixel_color = pixels[x + WIDTH * y];

        Color new_color;
        new_color.a = 255;
        new_color.r =
            new_ra + static_cast<uint16_t>(pixel_color.r * new_a_flip) / 255;
        new_color.g =
            new_ga + static_cast<uint16_t>(pixel_color.g * new_a_flip) / 255;
        new_color.b =
            new_ba + static_cast<uint16_t>(pixel_color.b * new_a_flip) / 255;
        pixel_color = new_color;
      }
    }
  }
}

int main() {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event event;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    return 3;
  }

  window = SDL_CreateWindow(NAME, SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0);
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

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  if (texture == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s",
                 SDL_GetError());
    return 3;
  }

  int max_entities = 512;
  ECS ecs(max_entities);
  int32_t frames = 0, current_frame = 0;
  int32_t entity_count = 0;
  float spawn_rate = 1.0f / 15.0f;
  bool render = true;
  Uint32 last_print = SDL_GetTicks();
  while (true) {
    Uint64 start = SDL_GetPerformanceCounter();

    bool has_events = SDL_PollEvent(&event);
    if (has_events) {
      if (event.type == SDL_QUIT) {
        break;
      } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
          case SDLK_LEFT:
            max_entities = std::max(1, max_entities / 2);
            std::cout << "Max Entities: " << max_entities << '\n';
            ecs.SetMaxEntities(max_entities);
            break;
          case SDLK_RIGHT:
            max_entities *= 2;
            std::cout << "Max Entities: " << max_entities << '\n';
            ecs.SetMaxEntities(max_entities);
            break;
          case SDLK_DOWN:
            spawn_rate *= 1.0f / 1.1f;
            std::cout << "Spawn Rate: " << spawn_rate << '\n';
            break;
          case SDLK_UP:
            spawn_rate *= 1.1f;
            std::cout << "Spawn Rate: " << spawn_rate << '\n';
            break;
          case SDLK_x:
            render = !render;
            break;
          default:
            break;
        }
      }
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    auto particles = ecs.Step(current_frame, spawn_rate, 16);
    if (render) {
      void *pixels = nullptr;
      int pitch;
      SDL_LockTexture(texture, NULL, &pixels, &pitch);
      memset(pixels, 0, WIDTH * HEIGHT * sizeof(Uint32));
      for (const auto &to_draw : particles) {
        draw_circle(reinterpret_cast<Color *>(pixels), to_draw.x, to_draw.y,
                    to_draw.radius, to_draw.color);
      }
      SDL_UnlockTexture(texture);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, NULL, NULL);
      SDL_RenderPresent(renderer);
    }
    entity_count = std::max(entity_count, ecs.size());
    ++current_frame;
    ++frames;

    if (SDL_GetTicks() - last_print > 1000) {
      std::cout << "Current FPS: " << std::to_string(frames) << "\t\t";
      std::cout << "Max Entities: " << std::to_string(entity_count) << '\n';
      entity_count = 0;
      frames = 0;
      last_print = SDL_GetTicks();
    }

    // Cap to 60 FPS
    Uint64 end = SDL_GetPerformanceCounter();
    float elapsedMS =
        (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
    float delay = floor(1000.0f / 60.0f - elapsedMS);
    if (delay > 0) {
      SDL_Delay(delay);
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
