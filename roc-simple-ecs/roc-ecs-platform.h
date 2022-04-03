#include <cstdint>
#include <iostream>
#include <random>

extern "C" {
void* roc_alloc(size_t size, unsigned int alignment) {
  (void)alignment;
  return malloc(size);
}

void* roc_realloc(void* ptr, size_t new_size, size_t old_size,
                  unsigned int alignment) {
  (void)old_size;
  (void)alignment;
  return realloc(ptr, new_size);
}

void roc_dealloc(void* ptr, unsigned int alignment) {
  (void)alignment;
  free(ptr);
}

void roc_panic(void* ptr, unsigned int alignment) {
  (void)alignment;
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
std::ostream& operator<<(std::ostream& os, const Color& c) {
  return os << "{ b: " << (int)c.b << ", g: " << (int)c.g << ", r: " << (int)c.r
            << ", a: " << (int)c.a << " }";
}

struct ToDraw {
  float radius;
  float x;
  float y;
  Color color;
};
std::ostream& operator<<(std::ostream& os, const ToDraw& td) {
  return os << "{ color: " << td.color << ", radius: " << td.radius
            << ", x: " << td.x << ", y: " << td.y << " }";
}

template <typename T>
class RocList {
 public:
  RocList() {}
  RocList(RocList&& old) {
    elements_ = old.elements_;
    size_ = old.size_;
    old.elements_ = nullptr;
    old.size_ = 0;
  }
  // Remove copy construtor, it has refcount implications.
  RocList(const RocList&) = delete;

  ~RocList() {
    if (size_ > 0) {
      ssize_t* rc = refcount_ptr();
      if (*rc == std::numeric_limits<ssize_t>::min()) {
        // Note: this may be wrong based off of the element alignment.
        // But looks to be correct for our current use case.
        free(rc);
      } else if (*rc < 0) {
        *rc -= 1;
      }
    }
  }

  constexpr T operator[](size_t i) const noexcept { return *(elements_ + i); }
  constexpr T* begin() const noexcept { return elements_; }
  constexpr T* end() const noexcept { return elements_ + size_; }

  size_t size() { return size_; }

 private:
  ssize_t* refcount_ptr() const {
    return reinterpret_cast<ssize_t*>(elements_) - 1;
  }

  T* elements_ = nullptr;
  size_t size_ = 0;
};

using RocModel = void*;

struct StepReturn {
  RocModel model;
  RocList<ToDraw> to_draw;
};

extern "C" {
RocModel roc__initForHost_1_exposed(uint32_t seed, int32_t max);
RocModel roc__setMaxForHost_1_exposed(RocModel model, int32_t max);
int32_t roc__sizeForHost_1_exposed(RocModel model);
// Roc expects this to be passed by value and thus as a chain of uint32_t
// because it does not properly support c-abi.
void roc__stepForHost_1_exposed_generic(RocModel model, int32_t current_frame,
                                        float spawn_rate,
                                        int32_t explosion_particles,
                                        StepReturn& ret);
}

class ECS {
 public:
  explicit ECS(int32_t max) {
    uint32_t seed = std::random_device{}();
    model_ = roc__initForHost_1_exposed(seed, max);
  }

  // This will clear all current entities.
  void SetMaxEntities(int32_t max) {
    model_ = roc__setMaxForHost_1_exposed(model_, max);
  }

  RocList<ToDraw> Step(int32_t current_frame, float spawn_rate,
                       int32_t explosion_particles) {
    StepReturn ret;
    roc__stepForHost_1_exposed_generic(model_, current_frame, spawn_rate,
                                       explosion_particles, ret);
    model_ = ret.model;
    return std::move(ret.to_draw);
  }

  int32_t size() { return roc__sizeForHost_1_exposed(model_); }

 private:
  RocModel model_;
};