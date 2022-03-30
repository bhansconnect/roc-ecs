#include <cstdint>
#include <iostream>
#include <random>

struct Color {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;
};

struct ToDraw {
  Color color;
  float radius;
  float x;
  float y;
};

extern "C" {
void roc__initForHost_1_exposed_generic(int64_t& x);
}

class ECS {
 public:
  explicit ECS(int32_t max) {
    // TODO: call roc main store closures and model.
    int64_t x = 0;
    roc__initForHost_1_exposed_generic(x);
    std::cout << "X: " << x << '\n';
  }

  // This will clear all current entities.
  void SetMaxEntities(int32_t max) {
    // TODO: call roc closure to resize.
  }

  std::vector<ToDraw> Step(int32_t current_frame, float spawn_rate,
                           int32_t explosion_particles) {
    // TODO: call roc closure to run all closures.
    return {};
  }

  int32_t size() {
    // TODO: call roc closure to get size.
    return {};
  }
};