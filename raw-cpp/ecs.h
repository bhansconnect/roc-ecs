#include <algorithm>
#include <bitset>
#include <cstdint>
#include <random>
#include <vector>

#include "pcg_random.hpp"

// A lot of this library is just done the way it is for simplicity.
// That is the reason for one header file and systems just built into the
// ECS struct.

const float TWO_PI = 2.0f * std::acosf(-1);

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

struct CompDeathTime {
  int32_t dead_frame;
};

struct CompFades {
  uint8_t r_rate;
  uint8_t r_min;
  uint8_t g_rate;
  uint8_t g_min;
  uint8_t b_rate;
  uint8_t b_min;
  uint8_t a_rate;
  uint8_t a_min;
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
  explicit ECS(int32_t max)
      : rng_(pcg_extras::seed_seq_from<std::random_device>()) {
    SetMaxEntities(max);
  }

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

  std::vector<ToDraw> Step(int32_t current_frame, float spawn_rate,
                           int32_t explosion_particles) {
    RunDeathSystem(current_frame);
    RunExplodesSystem(current_frame);
    RunFadeSystem();
    RunMoveSystem();
    RunGravitySystem();
    RunSpawnSystem(current_frame, spawn_rate, explosion_particles);
    Refresh();
    return RunGraphicsSystem();
  }

  int32_t size() { return size_; }

 private:
  // This actually adds all of the new entities into the active list.
  // It also remove old dead entites.
  void Refresh() {
    int32_t i = 0;
    int32_t j = new_size_ - 1;
    while (i <= j) {
      while (i < max_ && entities_[i].IsAlive()) ++i;
      if (i == max_ || i >= j) break;
      while (!entities_[j].IsAlive()) --j;
      if (i >= j) break;
      std::swap(entities_[i], entities_[j]);
    }
    size_ = i;
    new_size_ = i;
    // Having entities sorted seems to have no difference on performance.
    // std::sort(entities_.begin(), entities_.begin() + size_,
    //                    [](Entity a, Entity b) { return a.id < b.id; });
  }

  Entity* AddEntity() {
    if (new_size_ < max_) {
      Entity* e = &entities_[new_size_];
      e->signiture = Signiture();
      ++new_size_;
      return e;
    }
    return nullptr;
  }

  void RunSpawnSystem(int32_t current_frame, float spawn_rate,
                      int32_t explosion_particles) {
    auto spawn_entity = [&]() -> bool {
      Entity* e = AddEntity();
      if (e == nullptr) return false;

      int32_t id = e->id;
      float rise_speed =
          std::uniform_real_distribution<float>(0.01, 0.025)(rng_);
      float frames_to_cross_screen = 1.0 / rise_speed;
      int32_t life_in_frames = std::uniform_int_distribution<int>(
          frames_to_cross_screen * 0.6, frames_to_cross_screen * 0.95)(rng_);
      death_time_[id] = {.dead_frame = current_frame + life_in_frames};
      explodes_[id] = {.num_particles = explosion_particles};

      int color = std::uniform_int_distribution<int>(0, 2)(rng_);
      graphics_[id] = {
          .color = {.r = static_cast<uint8_t>(color == 0 ? 255 : 0),
                    .g = static_cast<uint8_t>(color == 1 ? 255 : 0),
                    .b = static_cast<uint8_t>(color == 2 ? 255 : 0),
                    .a = 255},
          .radius = 0.02};

      float x = std::uniform_real_distribution<float>(0.05, 0.95)(rng_);
      position_[id] = {.x = x, .y = 0.0};
      velocity_[id] = {.dy = rise_speed};
      e->signiture = Signiture({.isAlive = true,
                                .hasDeathTime = true,
                                .hasExplodes = true,
                                .hasGraphics = true,
                                .hasPosition = true,
                                .hasVelocity = true});
      return true;
    };
    int guaranteed_spawns = static_cast<int>(spawn_rate);
    for (int i = 0; i < guaranteed_spawns; ++i) {
      if (!spawn_entity()) return;
    }
    float rand_spawn = std::uniform_real_distribution<float>(0.0, 1.0)(rng_);
    if (rand_spawn < spawn_rate - guaranteed_spawns) {
      if (!spawn_entity()) return;
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

  void RunExplodesSystem(int32_t current_frame) {
    Signiture sig(
        {.hasExplodes = true, .hasPosition = true, .hasGraphics = true});
    for (int32_t i = 0; i < size_; ++i) {
      Entity& e = entities_[i];
      if (!e.IsAlive() && e.Matches(sig)) {
        CompPosition pos = position_[e.id];
        Color color = graphics_[e.id].color;
        Entity* flash = AddEntity();
        if (flash == nullptr) return;

        int32_t life_in_frames =
            std::uniform_int_distribution<int>(10, 30)(rng_);
        float frame_scale = (10.0f / life_in_frames);
        death_time_[flash->id] = {.dead_frame = current_frame + life_in_frames};

        CompFades f = {.a_min = 50,
                       .a_rate = static_cast<uint8_t>(30.0f * frame_scale)};
        if (color.r > color.g && color.a > color.b) {
          f.g_min = 100;
          f.g_rate = static_cast<uint8_t>(40.0f * frame_scale);
          color = {.r = 255, .g = 255, .b = 0, .a = 255};
        } else if (color.g > color.b) {
          f.b_min = 100;
          f.b_rate = static_cast<uint8_t>(40.0f * frame_scale);
          color = {.r = 0, .g = 255, .b = 255, .a = 255};
        } else {
          f.r_min = 100;
          f.r_rate = static_cast<uint8_t>(40.0f * frame_scale);
          color = {.r = 255, .g = 0, .b = 255, .a = 255};
        }
        graphics_[flash->id] = {
            .color = color,
            .radius = 0.03f / frame_scale,
        };

        fades_[flash->id] = f;

        position_[flash->id] = pos;
        flash->signiture = Signiture({
            .isAlive = true,
            .hasDeathTime = true,
            .hasFades = true,
            .hasGraphics = true,
            .hasPosition = true,
        });

        int32_t num_particles = explodes_[e.id].num_particles;
        std::vector<Entity*> particles;
        particles.reserve(num_particles);
        for (int i = 0; i < num_particles; ++i) {
          Entity* particle = AddEntity();
          if (particle == nullptr) break;
          particles.push_back(particle);
        }

        f.r_rate >>= 2;
        f.g_rate >>= 2;
        f.b_rate >>= 2;
        f.a_rate >>= 1;

        const int generated_particles = particles.size();
        float chunk_size = (TWO_PI / generated_particles);
        const float vel_scale = 0.01;
        for (int i = 0; i < generated_particles; ++i) {
          Entity* particle = particles[i];
          float min = i * chunk_size;
          float max = (i + 1) * chunk_size;
          float direction =
              std::uniform_real_distribution<float>(min, max)(rng_);
          float unit_dx = std::cosf(direction);
          float unit_dy = std::sinf(direction);

          position_[particle->id] = pos;
          velocity_[particle->id] = {.dx = unit_dx * vel_scale,
                                     .dy = unit_dy * vel_scale};
          graphics_[particle->id] = {
              .color = color,
              .radius = 0.015f / frame_scale,
          };
          fades_[particle->id] = f;
          death_time_[particle->id] = {
              .dead_frame = current_frame +
                            static_cast<int>(1.5f * life_in_frames) +
                            std::uniform_int_distribution<int>(0, 10)(rng_)};
          particle->signiture = Signiture({
              .isAlive = true,
              .hasDeathTime = true,
              .hasFades = true,
              .hasGraphics = true,
              .hasPosition = true,
              .hasVelocity = true,
              .feelsGravity = true,
          });
        }
      }
    }
  }

  void RunGravitySystem() {
    Signiture sig({.isAlive = true, .feelsGravity = true, .hasVelocity = true});
    for (int32_t i = 0; i < size_; ++i) {
      Entity& e = entities_[i];
      if (e.Matches(sig)) {
        CompVelocity& v = velocity_[e.id];
        // TODO: Tune the gravity constant.
        v.dy -= 0.0003;
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
          c = std::max(c - rate, static_cast<int32_t>(min));
        };
        updateColor(color.r, f.r_min, f.r_rate);
        updateColor(color.g, f.g_min, f.g_rate);
        updateColor(color.b, f.b_min, f.b_rate);
        updateColor(color.a, f.a_min, f.a_rate);
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

  // TODO: evaluate if entities_ should really be some for of ordered map or
  // have some other way to remain ordered.
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

  pcg32 rng_;
};
