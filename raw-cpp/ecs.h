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
  int32_t dead_frame;
};

struct CompFades {
  int8_t color_rate;
  int8_t color_min;
  int8_t opacity_rate;
  int8_t opacity_min;
};

struct CompExplodes {
  int32_t num_particles;
};

struct CompGraphics {
  Color color;
  float radius;
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

  bool IsAlive() const { return data_[IS_ALIVE_INDEX]; }

  bool Matches(Signiture other) const {
    return (data_ & other.data_) == other.data_;
  }

 private:
  std::bitset<COUNT> data_;
};

struct Entity {
  int32_t id;
  Signiture signiture;

  bool IsAlive() { return signiture.IsAlive(); }
  bool Matches(Signiture other) const { return signiture.Matches(other); }
};

class ECS {
 public:
  explicit ECS(int32_t max) { SetMaxEntities(max); }

  // This will clear all current entities.
  void SetMaxEntities(int32_t max) {
    entities_.resize(max);
    death_time_.resize(max);
    fades_.resize(max);
    explodes_.resize(max);
    graphics_.resize(max);
    position_.resize(max);
    velocity_.resize(max);
    size_ = 0;
    new_size_ = 0;
    max_ = max;
    for (int32_t i = 0; i < max; ++i) {
      entities_[i].id = i;
      entities_[i].signiture = Signiture();
    }
  }

  std::vector<ToDraw> Step(int32_t current_frame, int32_t spawn_interval,
                           int32_t explosion_particles) {
    RunDeathSystem(current_frame);
    RunFadeSystem();
    RunMoveSystem();
    RunSpawnSystem(current_frame, spawn_interval, explosion_particles);
    Refresh();
    return RunGraphicsSystem();
  }

 private:
  // This actually adds all of the new entities into the active list.
  // It also remove old dead entites.
  void Refresh() {
    int32_t i = 0;
    int32_t j = new_size_ - 1;
    while (i <= j) {
      while (entities_[i].IsAlive()) ++i;
      if (i >= j) break;
      while (!entities_[j].IsAlive()) --j;
      if (i >= j) break;
      std::swap(entities_[i], entities_[j]);
    }
    size_ = i;
    new_size_ = i;
  }

  Entity* AddEntity() {
    if (new_size_ < max_) {
      Entity* e = &entities_[new_size_];
      ++new_size_;
      return e;
    }
    return nullptr;
  }

  void RunSpawnSystem(int32_t current_frame, int32_t spawn_interval,
                      int32_t explosion_particles) {
    if (current_frame % spawn_interval == 0) {
      Entity* e = AddEntity();
      if (e == nullptr) return;

      int32_t id = e->id;
      death_time_[id] = {.dead_frame = current_frame + 60};
      explodes_[id] = {.num_particles = explosion_particles};
      graphics_[id] = {.color = {.r = 255, .g = 255, .b = 255, .a = 255},
                       .radius = 0.02};
      position_[id] = {.x = 0.5, .y = 0.0};
      velocity_[id] = {.dy = 0.01};
      e->signiture = Signiture({.isAlive = true,
                                .hasDeathTime = true,
                                .hasExplodes = true,
                                .hasGraphics = true,
                                .hasPosition = true,
                                .hasVelocity = true});
    }
  }

  void RunDeathSystem(int32_t current_frame) {
    Signiture sig({.isAlive = true, .hasDeathTime = true});
    for (int32_t i = 0; i < size_; ++i) {
      Entity& e = entities_[i];
      if (e.Matches(sig)) {
        e.signiture[Signiture::IS_ALIVE_INDEX] =
            current_frame < death_time_[e.id].dead_frame;
      }
    }
  }

  void RunMoveSystem() {
    Signiture sig({.isAlive = true, .hasPosition = true, .hasVelocity = true});
    for (int32_t i = 0; i < size_; ++i) {
      Entity& e = entities_[i];
      if (e.Matches(sig)) {
        CompPosition& p = position_[e.id];
        CompVelocity v = velocity_[e.id];
        p.x += v.dx;
        p.y += v.dy;
      }
    }
  }

  void RunFadeSystem() {
    Signiture sig({.isAlive = true, .hasFades = true, .hasGraphics = true});
    for (int32_t i = 0; i < size_; ++i) {
      Entity& e = entities_[i];
      if (e.Matches(sig)) {
        Color& color = graphics_[e.id].color;
        CompFades& f = fades_[e.id];
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
    out.reserve(size_);
    for (int32_t i = 0; i < size_; ++i) {
      Entity& e = entities_[i];
      if (e.Matches(sig)) {
        CompGraphics g = graphics_[e.id];
        CompPosition p = position_[e.id];
        out.push_back(
            {.color = g.color, .radius = g.radius, .x = p.x, .y = p.y});
      }
    }
    return out;
  }

  std::vector<Entity> entities_;
  std::vector<CompDeathTime> death_time_;
  std::vector<CompFades> fades_;
  std::vector<CompExplodes> explodes_;
  std::vector<CompGraphics> graphics_;
  std::vector<CompPosition> position_;
  std::vector<CompVelocity> velocity_;

  // This is the number of active entities.
  int32_t size_;
  // This is the number of active + newly created entities.
  int32_t new_size_;
  int32_t max_;
};
