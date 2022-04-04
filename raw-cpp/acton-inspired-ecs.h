#include <algorithm>
#include <bitset>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

#include "pcg_random.hpp"

// A lot of this library is just done the way it is for simplicity.
// That is the reason for one header file and systems just built into the
// ECS struct.

const float TWO_PI = 2.0f * std::acos(-1);

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
  static constexpr int32_t DEATH_TIME_INDEX = 0;
  static constexpr int32_t FADES_INDEX = 1;
  static constexpr int32_t EXPLODES_INDEX = 2;
  static constexpr int32_t GRAPHICS_INDEX = 3;
  static constexpr int32_t POSITION_INDEX = 4;
  static constexpr int32_t VELOCITY_INDEX = 5;
  static constexpr int32_t FEELS_GRAVITY_INDEX = 6;
  static constexpr int32_t COUNT = 7;

  struct Initializer {
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

  bool IsAlive() const { return true; }

  bool Matches(Signiture other) const {
    return (data_ & other.data_) == other.data_;
  }

  friend bool operator==(const Signiture& lhs, const Signiture& rhs) {
    return lhs.data_ == rhs.data_;
  }

 private:
  std::bitset<COUNT> data_;
};

struct KilledEntity {
  const Signiture signiture;
  std::optional<CompDeathTime> death_time;
  std::optional<CompFades> fades;
  std::optional<CompExplodes> explodes;
  std::optional<CompGraphics> graphics;
  std::optional<CompPosition> position;
  std::optional<CompVelocity> velocity;

  bool Matches(Signiture other) const { return signiture.Matches(other); }
};

// This ECS is sorted into architypes.
// Essentially entities with the same signiture are grouped together.
struct Architype {
  Architype(Signiture sig, int32_t cap) : signiture(sig) {
    auto reserve = [&](int sig_index, auto& vec) {
      if (signiture[sig_index]) {
        vec.reserve(cap);
      }
    };
    reserve(Signiture::DEATH_TIME_INDEX, death_time);
    reserve(Signiture::FADES_INDEX, fades);
    reserve(Signiture::EXPLODES_INDEX, explodes);
    reserve(Signiture::GRAPHICS_INDEX, graphics);
    reserve(Signiture::POSITION_INDEX, position);
    reserve(Signiture::VELOCITY_INDEX, velocity);
  }

  void addEntity(std::optional<CompDeathTime> dt, std::optional<CompFades> f,
                 std::optional<CompExplodes> e, std::optional<CompGraphics> g,
                 std::optional<CompPosition> p, std::optional<CompVelocity> v) {
    if (dt) death_time.push_back(*dt);
    if (f) fades.push_back(*f);
    if (e) explodes.push_back(*e);
    if (g) graphics.push_back(*g);
    if (p) position.push_back(*p);
    if (v) velocity.push_back(*v);
    ++size;
  }

  KilledEntity removeEntity(int i) {
    KilledEntity e{.signiture = signiture};
    auto swap_and_remove = [&](int sig_index, auto& vec, auto& data) {
      if (signiture[sig_index]) {
        std::iter_swap(vec.begin() + i, vec.end() - 1);
        data = vec.back();
        vec.pop_back();
      }
    };
    swap_and_remove(Signiture::DEATH_TIME_INDEX, death_time, e.death_time);
    swap_and_remove(Signiture::FADES_INDEX, fades, e.fades);
    swap_and_remove(Signiture::EXPLODES_INDEX, explodes, e.explodes);
    swap_and_remove(Signiture::GRAPHICS_INDEX, graphics, e.graphics);
    swap_and_remove(Signiture::POSITION_INDEX, position, e.position);
    swap_and_remove(Signiture::VELOCITY_INDEX, velocity, e.velocity);
    --size;
    return e;
  }

  bool Matches(Signiture other) const { return signiture.Matches(other); }

  const Signiture signiture;
  int32_t size = 0;

  // Unused vectors will remain empty.
  std::vector<CompDeathTime> death_time;
  std::vector<CompFades> fades;
  std::vector<CompExplodes> explodes;
  std::vector<CompGraphics> graphics;
  std::vector<CompPosition> position;
  std::vector<CompVelocity> velocity;
};

struct Entity {
  Architype& architype;
  int32_t id;
};

class ECS {
 public:
  static constexpr int32_t DEFAULT_CAP = 128;

  explicit ECS(int32_t max)
      : max_(max), rng_(pcg_extras::seed_seq_from<std::random_device>()) {}

  // This will clear all current entities.
  void SetMaxEntities(int32_t max) {
    max_ = max;
    size_ = 0;
    architypes_.clear();
  }

  // Runs systems that only modify a single entity at a time.
  // They can not change the components the entity has.
  inline void RunSimpleSystem(Signiture signiture,
                              std::function<void(Entity)> f) {
    for (auto& architype : architypes_) {
      if (architype.Matches(signiture)) {
        for (int32_t i = 0; i < architype.size; ++i) {
          f({architype, i});
        }
      }
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
    return RunGraphicsSystem();
  }

  int32_t size() const { return size_; }

 private:
  inline bool CanAddEntity() const { return size_ < max_; }

  inline void AddEntity(Signiture signiture,
                        std::optional<CompDeathTime> death_time,
                        std::optional<CompFades> fades,
                        std::optional<CompExplodes> explodes,
                        std::optional<CompGraphics> graphics,
                        std::optional<CompPosition> position,
                        std::optional<CompVelocity> velocity) {
    if (size_ < max_) {
      ++size_;
      auto iter = std::find_if(architypes_.begin(), architypes_.end(),
                               [signiture](const Architype& at) {
                                 return at.signiture == signiture;
                               });
      if (iter == architypes_.end()) {
        architypes_.emplace_back(signiture, DEFAULT_CAP);
        architypes_.back().addEntity(death_time, fades, explodes, graphics,
                                     position, velocity);
      } else {
        iter->addEntity(death_time, fades, explodes, graphics, position,
                        velocity);
      }
    }
  }

  void RunSpawnSystem(int32_t current_frame, float spawn_rate,
                      int32_t explosion_particles) {
    const Signiture spawn_signiture({.hasDeathTime = true,
                                     .hasExplodes = true,
                                     .hasGraphics = true,
                                     .hasPosition = true,
                                     .hasVelocity = true});
    auto spawn_entity = [&]() -> bool {
      if (!CanAddEntity()) return false;

      float rise_speed =
          std::uniform_real_distribution<float>(0.01, 0.025)(rng_);
      float frames_to_cross_screen = 1.0 / rise_speed;
      int32_t life_in_frames = std::uniform_int_distribution<int>(
          frames_to_cross_screen * 0.6, frames_to_cross_screen * 0.95)(rng_);
      CompDeathTime death_time = {.dead_frame = current_frame + life_in_frames};
      CompExplodes explodes = {.num_particles = explosion_particles};

      int color = std::uniform_int_distribution<int>(0, 2)(rng_);
      CompGraphics graphics = {
          .color =
              {
                  .b = static_cast<uint8_t>(color == 2 ? 255 : 0),
                  .g = static_cast<uint8_t>(color == 1 ? 255 : 0),
                  .r = static_cast<uint8_t>(color == 0 ? 255 : 0),
                  .a = 255,
              },
          .radius = 0.02};

      float x = std::uniform_real_distribution<float>(0.05, 0.95)(rng_);
      CompPosition position = {.x = x, .y = 0.0};
      CompVelocity velocity = {.dy = rise_speed};

      AddEntity(spawn_signiture, death_time, /*fades=*/std::nullopt, explodes,
                graphics, position, velocity);
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
    const Signiture death_signiture({.hasDeathTime = true});

    newly_dead_entities.clear();
    for (auto& architype : architypes_) {
      if (architype.Matches(death_signiture)) {
        for (int32_t i = 0; i < architype.size; ++i) {
          if (current_frame >= architype.death_time[i].dead_frame) {
            newly_dead_entities.push_back(architype.removeEntity(i));
            --i;
            --size_;
          }
        }
      }
    }
  }

  void RunExplodesSystem(int32_t current_frame) {
    const Signiture explodes_signiture(
        {.hasExplodes = true, .hasGraphics = true, .hasPosition = true});
    for (const auto& e : newly_dead_entities) {
      if (e.Matches(explodes_signiture)) {
        CompPosition position = *e.position;
        Color color = (*e.graphics).color;
        if (!CanAddEntity()) return;

        int32_t life_in_frames =
            std::uniform_int_distribution<int>(10, 30)(rng_);
        float frame_scale = (10.0f / life_in_frames);
        CompDeathTime death_time = {.dead_frame =
                                        current_frame + life_in_frames};

        CompFades fades = {
            .a_rate = static_cast<uint8_t>(30.0f * frame_scale),
            .a_min = 50,
        };
        if (color.r > color.g && color.r > color.b) {
          fades.g_min = 100;
          fades.g_rate = static_cast<uint8_t>(40.0f * frame_scale);
          color = {.b = 0, .g = 255, .r = 255, .a = 255};
        } else if (color.g > color.b) {
          fades.b_min = 100;
          fades.b_rate = static_cast<uint8_t>(40.0f * frame_scale);
          color = {.b = 255, .g = 255, .r = 0, .a = 255};
        } else {
          fades.r_min = 100;
          fades.r_rate = static_cast<uint8_t>(40.0f * frame_scale);
          color = {.b = 255, .g = 0, .r = 255, .a = 255};
        }
        CompGraphics graphics = {
            .color = color,
            .radius = 0.03f / frame_scale,
        };

        const Signiture signiture({
            .hasDeathTime = true,
            .hasFades = true,
            .hasGraphics = true,
            .hasPosition = true,
        });
        AddEntity(signiture, death_time, fades, /*explodes=*/std::nullopt,
                  graphics, position,
                  /*velocity=*/std::nullopt);

        int32_t generated_particles =
            std::min((*e.explodes).num_particles, max_ - size_);
        if (generated_particles == 0) return;

        fades.r_rate >>= 2;
        fades.g_rate >>= 2;
        fades.b_rate >>= 2;
        fades.a_rate >>= 1;

        Signiture particle_signiture({
            .hasDeathTime = true,
            .hasFades = true,
            .hasGraphics = true,
            .hasPosition = true,
            .hasVelocity = true,
            .feelsGravity = true,
        });
        float chunk_size = (TWO_PI / generated_particles);
        const float vel_scale = 0.01;
        for (int i = 0; i < generated_particles; ++i) {
          float min = i * chunk_size;
          float max = (i + 1) * chunk_size;
          float direction =
              std::uniform_real_distribution<float>(min, max)(rng_);
          float unit_dx = std::cos(direction);
          float unit_dy = std::sin(direction);

          CompVelocity velocity = {.dx = unit_dx * vel_scale,
                                   .dy = unit_dy * vel_scale};
          CompGraphics graphics = {
              .color = color,
              .radius = 0.015f / frame_scale,
          };
          CompDeathTime death_time = {
              .dead_frame = current_frame +
                            static_cast<int>(1.5f * life_in_frames) +
                            std::uniform_int_distribution<int>(0, 10)(rng_)};
          AddEntity(particle_signiture, death_time, fades,
                    /*explodes=*/std::nullopt, graphics, position, velocity);
        }
      }
    }
  }

  void RunMoveSystem() {
    const Signiture move_signiture({.hasPosition = true, .hasVelocity = true});
    RunSimpleSystem(move_signiture, [](Entity e) {
      CompPosition& p = e.architype.position[e.id];
      CompVelocity v = e.architype.velocity[e.id];
      p.x += v.dx;
      p.y += v.dy;
    });
  }

  void RunGravitySystem() {
    const Signiture gravity_signiture(
        {.hasVelocity = true, .feelsGravity = true});
    RunSimpleSystem(gravity_signiture, [](Entity e) {
      CompVelocity& v = e.architype.velocity[e.id];
      // TODO: Tune the gravity constant.
      v.dy -= 0.0003;
    });
  }

  void RunFadeSystem() {
    const Signiture fade_signiture({.hasFades = true, .hasGraphics = true});
    RunSimpleSystem(fade_signiture, [](Entity e) {
      Color& color = e.architype.graphics[e.id].color;
      CompFades f = e.architype.fades[e.id];
      auto updateColor = [](uint8_t& c, uint8_t min, uint8_t rate) {
        c = std::max(c - rate, static_cast<int32_t>(min));
      };
      updateColor(color.r, f.r_min, f.r_rate);
      updateColor(color.g, f.g_min, f.g_rate);
      updateColor(color.b, f.b_min, f.b_rate);
      updateColor(color.a, f.a_min, f.a_rate);
    });
  }

  std::vector<ToDraw> RunGraphicsSystem() {
    const Signiture graphics_signiture(
        {.hasGraphics = true, .hasPosition = true});
    std::vector<ToDraw> out;
    out.reserve(size_);
    RunSimpleSystem(graphics_signiture, [&out](Entity e) {
      CompGraphics g = e.architype.graphics[e.id];
      CompPosition p = e.architype.position[e.id];
      out.push_back({.color = g.color, .radius = g.radius, .x = p.x, .y = p.y});
    });
    return out;
  }

  std::vector<KilledEntity> newly_dead_entities;
  std::vector<Architype> architypes_;
  int32_t size_;
  int32_t max_;

  pcg32 rng_;
};
