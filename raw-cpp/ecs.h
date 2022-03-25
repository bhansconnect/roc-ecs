#include <algorithm>
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

struct ToDraw {
  Color color;
  float radius;
  float x;
  float y;
};

struct CompDeathTime {
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

class Signiture {
 public:
  static constexpr int32_t IS_ALIVE_INDEX = 0;
  static constexpr int32_t DEATH_TIME_INDEX = 1;
  static constexpr int32_t FADES_INDEX = 2;
  static constexpr int32_t EXPLODES_INDEX = 3;
  static constexpr int32_t GRAPHICS_INDEX = 4;
  static constexpr int32_t POSITION_INDEX = 5;
  static constexpr int32_t VELOCITY_INDEX = 6;
  static constexpr int32_t FEELS_GRAVITY_INDEX = 7;
  static constexpr int32_t COUNT = 8;

  struct Initializer {
    bool isAlive = false;
    bool hasDeathTime = false;
    bool hasFades = false;
    bool hasExplodes = false;
    bool hasGraphics = false;
    bool hasPosition = false;
    bool hasVelocity = false;
    bool feelsGravity = false;
  };

  Signiture() {}
  Signiture(Initializer init) {
    data_[IS_ALIVE_INDEX] = init.isAlive;
    data_[DEATH_TIME_INDEX] = init.hasDeathTime;
    data_[FADES_INDEX] = init.hasFades;
    data_[EXPLODES_INDEX] = init.hasExplodes;
    data_[GRAPHICS_INDEX] = init.hasGraphics;
    data_[POSITION_INDEX] = init.hasPosition;
    data_[VELOCITY_INDEX] = init.hasVelocity;
    data_[FEELS_GRAVITY_INDEX] = init.feelsGravity;
  }

  bool operator[](size_t x) const { return data_[x]; }
  std::bitset<COUNT>::reference operator[](size_t x) { return data_[x]; }

  bool matches(Signiture other) const {
    return (data_ & other.data_) == other.data_;
  }

 private:
  std::bitset<COUNT> data_;
};

struct Entity {
  int32_t id;
  Signiture signiture;

  bool matches(Signiture other) const { return signiture.matches(other); }
};

struct ECS {
  std::vector<Entity> entities;
  std::vector<CompDeathTime> death_time;
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
    death_time.resize(max);
    fades.resize(max);
    explodes.resize(max);
    graphics.resize(max);
    position.resize(max);
    velocity.resize(max);
  }

  void Step(uint32_t current_frame) {
    RunFadeSystem();
    RunGraphicsSystem();
    RunDeathSystem(current_frame);
  }

  void RunDeathSystem(uint32_t current_frame) {
    Signiture sig({.isAlive = true, .hasDeathTime = true});
    for (uint32_t i = 0; i < size; ++i) {
      Entity& e = entities[i];
      if (e.matches(sig)) {
        e.signiture[Signiture::IS_ALIVE_INDEX] =
            current_frame < death_time[e.id].dead_frame;
      }
    }
  }

  void RunFadeSystem() {
    Signiture sig({.isAlive = true, .hasFades = true, .hasGraphics = true});
    for (uint32_t i = 0; i < size; ++i) {
      Entity& e = entities[i];
      if (e.matches(sig)) {
        Color& color = graphics[e.id].color;
        CompFades& f = fades[e.id];
        auto updateColor = [](uint8_t& c, uint8_t min, uint8_t rate) {
          c = std::max(min, static_cast<uint8_t>(std::min(c - rate, 0)));
        };
        updateColor(color.r, f.color_min, f.color_rate);
        updateColor(color.g, f.color_min, f.color_rate);
        updateColor(color.b, f.color_min, f.color_rate);
        updateColor(color.a, f.opacity_min, f.opacity_rate);
      }
    }
  }

  std::vector<ToDraw> RunGraphicsSystem() {
    Signiture sig({.isAlive = true, .hasGraphics = true, .hasPosition = true});
    std::vector<ToDraw> out;
    out.reserve(size);
    for (uint32_t i = 0; i < size; ++i) {
      Entity& e = entities[i];
      if (e.matches(sig)) {
        CompGraphics g = graphics[e.id];
        CompPosition p = position[e.id];
        out.push_back(
            {.color = g.color, .radius = g.radius, .x = p.x, .y = p.y});
      }
    }
    return out;
  }
};
