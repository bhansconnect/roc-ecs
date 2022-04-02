#include <cstdint>
#include <iostream>
#include <random>

extern "C" {
void* roc_alloc(size_t size, unsigned int alignment) { return malloc(size); }

void* roc_realloc(void* ptr, size_t new_size, size_t old_size,
                  unsigned int alignment) {
  return realloc(ptr, new_size);
}

void roc_dealloc(void* ptr, unsigned int alignment) { free(ptr); }

void roc_panic(void* ptr, unsigned int alignment) {
  char* msg = (char*)ptr;
  fprintf(stderr,
          "Application crashed with message\n\n    %s\n\nShutting down\n", msg);
  exit(0);
}

void* roc_memcpy(void* dest, const void* src, size_t n) {
  return memcpy(dest, src, n);
}

void* roc_memset(void* str, int c, size_t n) { return memset(str, c, n); }
}

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

struct RandConstants32 {
  uint32_t permuteMultiplier;
  uint32_t permuteRandomXorShift;
  uint32_t permuteRandomXorShiftIncrement;
  uint32_t permuteXorShift;
  uint32_t updateIncrement;
  uint32_t updateMultiplier;
};

struct RandState32 {
  RandConstants32 c;
  uint32_t s;
};

struct RocModel {
  RandState32 rng;
};

struct StepReturn {
  RocModel* model;
  uint32_t value;
};

extern "C" {
RocModel* roc__initForHost_1_exposed(uint32_t seed);
// Roc expects this to be passed by value and thus as a chain of uint32_t
// because it does not properly support c-abi.
StepReturn roc__stepForHost_1_exposed(
    RocModel* model
    // int32_t current_frame, float spawn_rate,
    // int32_t explosion_particles,
);
}

class ECS {
 public:
  explicit ECS(int32_t max) {
    // TODO: call roc main store closures and model.
    uint32_t seed = std::random_device{}();
    model_ = roc__initForHost_1_exposed(seed);
    std::cout << "S: " << model_->rng.s << '\n';
  }

  // This will clear all current entities.
  void SetMaxEntities(int32_t max) {
    // TODO: call roc closure to resize.
  }

  std::vector<ToDraw> Step(int32_t current_frame, float spawn_rate,
                           int32_t explosion_particles) {
    // TODO: call roc closure to run all closures.
    StepReturn ret = roc__stepForHost_1_exposed(model_);
    model_ = ret.model;
    std::cout << "rand: " << ret.value << '\n';
    return {};
  }

  int32_t size() {
    // TODO: call roc closure to get size.
    return {};
  }

 private:
  RocModel* model_;
};