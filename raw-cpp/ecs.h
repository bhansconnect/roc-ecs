#include <bitset>
#include <cstdint>
#include <vector>

// A lot of this library is just done the way it is for simplicity.
// That is the reason for one header file and systems just built into the
// ECS struct.

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

struct CompLife {
  uint32_t dead_frame;
};

struct CompFades {
  int8_t color_rate;
  int8_t color_min;
  int8_t opacity_rate;
  int8_t opacity_min;
};

struct CompExplodes {
  uint32_t num_particles;
};

struct CompGraphics {
  float radius;
  Color color;
};

struct CompPosition {
  float x;
  float y;
};

struct CompVelocity {
  float dx;
  float dy;
};

struct Signiture {
  static constexpr int32_t LIFE_INDEX = 0;
  static constexpr int32_t FADES_INDEX = 1;
  static constexpr int32_t EXPLODES_INDEX = 2;
  static constexpr int32_t GRAPHICS_INDEX = 3;
  static constexpr int32_t POSITION_INDEX = 4;
  static constexpr int32_t VELOCITY_INDEX = 5;
  static constexpr int32_t HAS_GRAVITY_INDEX = 6;
  static constexpr int32_t COUNT = 7;

  std::bitset<COUNT> data;

  bool operator[](size_t x) { return data[x]; }
};

struct Entity {
  int32_t id;
  bool dead;
  Signiture signiture;
};

struct ECS {
  std::vector<Entity> entities;
  std::vector<CompLife> life;
  std::vector<CompFades> fades;
  std::vector<CompExplodes> explodes;
  std::vector<CompGraphics> graphics;
  std::vector<CompPosition> position;
  std::vector<CompVelocity> velocity;

  uint32_t size;
  uint32_t new_size;

  explicit ECS(uint32_t max) { SetMaxEntities(max); }

  void SetMaxEntities(uint32_t max) {
    entities.resize(max);
    life.resize(max);
    fades.resize(max);
    explodes.resize(max);
    graphics.resize(max);
    position.resize(max);
    velocity.resize(max);
  }

  void Step(uint32_t current_frame) { RunLifeSystem(current_frame); }

  void RunLifeSystem(uint32_t current_frame) {
    for (uint32_t i = 0; i < size; ++i) {
      Entity& e = entities[i];
      if (e.signiture[Signiture::LIFE_INDEX]) {
        e.dead = current_frame == life[e.id].dead_frame;
      }
    }
  }
};
