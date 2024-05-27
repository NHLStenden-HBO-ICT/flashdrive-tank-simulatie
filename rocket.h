#pragma once
#include "Explosion.h"
#include "Smoke.h"

namespace Tmpl8
{

class Rocket
{
  public:
    Rocket(vec2 position, vec2 direction, float collision_radius, allignments allignment, Sprite* rocket_sprite);
    ~Rocket();

    void tick();
    void draw(Surface* screen);

    bool intersects(vec2 position_other, float radius_other) const;

    vec2 position;
    vec2 speed;

    float collision_radius;

    bool active;

    allignments allignment;

    int current_frame;
    Sprite* rocket_sprite;
    static void update_rockets(ThreadPool* pool, std::vector<std::future<void>>& futures, vector<Rocket>& rockets, vector<Tank>& tanks, float rocket_hit_value, vector<Explosion>& explosions, Sprite& explosion, vector<Smoke>& smokes, Sprite& smoke);
    static void disable_rockets(vector<Rocket>& rockets, vector<vec2>& forcefield_hull, vector<Explosion> explosions, Sprite& explosion);

};

} // namespace Tmpl8