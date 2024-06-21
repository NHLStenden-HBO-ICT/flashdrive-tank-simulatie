#include "precomp.h"

namespace Tmpl8
{
    std::mutex explosions_mutex;
    std::mutex smokes_mutex; 

Rocket::Rocket(vec2 position, vec2 direction, float collision_radius, allignments allignment, Sprite* rocket_sprite)
    : position(position), speed(direction), collision_radius(collision_radius), allignment(allignment), current_frame(0), rocket_sprite(rocket_sprite), active(true)
{
}

Rocket::~Rocket()
{
}

void Rocket::tick()
{
    position += speed;
    if (++current_frame > 8) current_frame = 0;
}

/// <summary>
/// Draw rocket sprites
/// </summary>
/// <param name="screen">The display screen</param>
void Rocket::draw(Surface* screen)
{
    rocket_sprite->set_frame(((abs(speed.x) > abs(speed.y)) ? ((speed.x < 0) ? 3 : 0) : ((speed.y < 0) ? 9 : 6)) + (current_frame / 3));
    rocket_sprite->draw(screen, (int)position.x - 12 + HEALTHBAR_OFFSET, (int)position.y - 12);
}


/// <summary>
/// Does the given circle collide with this rockets collision circle ?
/// </summary>
/// <param name="position_other">Position of the other object</param>
/// <param name="radius_other">The radius of the other object</param>
/// <returns></returns>
bool Rocket::intersects(vec2 position_other, float radius_other) const
{
    float distance_sqr = (position_other - position).sqr_length();

    if (distance_sqr <= ((collision_radius + radius_other) * (collision_radius + radius_other)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/// <summary>
///Check if rocket collides with enemy tank, spawn explosion, and if tank is destroyed spawn a smoke plume
/// </summary>
/// <param name="rocket"></param>
/// <param name="tanks"></param>
/// <param name="rocket_hit_value"></param>
/// <param name="explosions"></param>
/// <param name="explosion"></param>
/// <param name="smokes"></param>
/// <param name="smoke"></param>
void update_rocket(Rocket& rocket, vector<Tank>& tanks, float rocket_hit_value, vector<Explosion>& explosions, Sprite& explosion, vector<Smoke>& smokes, Sprite& smoke)
{
    rocket.tick();

    for (Tank& tank : tanks)
    {
        if (tank.active && (tank.allignment != rocket.allignment) && rocket.intersects(tank.position, tank.collision_radius))
        {
            explosions_mutex.lock(); 
            explosions.push_back(Explosion(&explosion, tank.position));
            explosions_mutex.unlock();

            if (tank.hit(rocket_hit_value))
            {
                smokes_mutex.lock(); 
                smokes.push_back(Smoke(smoke, tank.position - vec2(7, 24)));
                smokes_mutex.unlock();
            }

            rocket.active = false;
            break;
        }
    }
}

/// <summary>
/// Multithreaded queue to update rockets 
/// </summary>
/// <param name="pool">Thread pool</param>
/// <param name="futures">The threads to wait for</param>
/// <param name="rockets">List of rockets</param>
/// <param name="tanks">List of tanks</param>
/// <param name="rocket_hit_value">How much the the tank is damaged</param>
/// <param name="explosions">List of explosions</param>
/// <param name="explosion">Explosions sprite</param>
/// <param name="smokes">List of smokes</param>
/// <param name="smoke">Smoke sprite</param>
void Rocket::update_rockets(ThreadPool* pool, std::vector<std::future<void>>& futures, vector<Rocket>& rockets, vector<Tank>& tanks, float rocket_hit_value, vector<Explosion>& explosions, Sprite& explosion, vector<Smoke>& smokes, Sprite& smoke)
{
    futures.clear();
    futures.reserve(rockets.size());
   
    for (Rocket& rocket : rockets)
    {
        futures.push_back(pool->enqueue([&rocket, &tanks, rocket_hit_value, &explosions, &explosion, &smokes, &smoke]() { update_rocket(rocket, tanks, rocket_hit_value, explosions, explosion, smokes, smoke); }));
    }

    for (auto& future : futures)
    {
        future.get();
    }
}

/// <summary>
/// Disable rockets if they collide with the "forcefield"
/// </summary>
/// <param name="rockets"></param>
/// <param name="forcefield_hull"></param>
/// <param name="explosions"></param>
/// <param name="explosion"></param>
void Rocket::disable_rockets(vector<Rocket>& rockets, vector<vec2>& forcefield_hull, vector<Explosion> explosions, Sprite& explosion)
{
    for (Rocket& rocket : rockets)
    {
        if (rocket.active)
        {
            for (size_t i = 0; i < forcefield_hull.size(); i++)
            {
                if (circle_segment_intersect(forcefield_hull.at(i), forcefield_hull.at((i + 1) % forcefield_hull.size()), rocket.position, rocket.collision_radius))
                {
                    explosions.push_back(Explosion(&explosion, rocket.position));
                    rocket.active = false;
                }
            }
        }
    }
}
} // namespace Tmpl8
