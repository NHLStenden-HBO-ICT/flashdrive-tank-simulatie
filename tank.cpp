#include "precomp.h"

namespace Tmpl8
{

Tank::Tank(
    float pos_x,
    float pos_y,
    allignments allignment,
    Sprite* tank_sprite,
    Sprite* smoke_sprite,
    float tar_x,
    float tar_y,
    float collision_radius,
    int health,
    float max_speed)
    : position(pos_x, pos_y),
      allignment(allignment),
      target(tar_x, tar_y),
      health(health),
      collision_radius(collision_radius),
      max_speed(max_speed),
      force(0, 0),
      reload_time(1),
      reloaded(false),
      speed(0),
      active(true),
      current_frame(0),
      tank_sprite(tank_sprite),
      smoke_sprite(smoke_sprite)
{

}

Tank::~Tank()
{
}

/// <summary>
/// Update tanks every tick
/// </summary>
/// <param name="terrain">Terrain of the game</param>
void Tank::tick(Terrain& terrain)
{
    vec2 direction = vec2(0, 0);

    if (target != position)
    {
        direction = (target - position).normalized();
    }

    speed = direction + force;
    float position_multiplier = 0.5f;
    position += speed * max_speed * position_multiplier;

    if (--reload_time <= 0.0f)
    {
        reloaded = true;
    }

    force = vec2(0.f, 0.f);

    int number_of_animation_frames = 8;
    int start_frame = 0;
    if (++current_frame > number_of_animation_frames) current_frame = start_frame;

    //Target reached?
    if (current_route.size() > 0)
    {
        if (std::abs(position.x - target.x) < 8.f && std::abs(position.y - target.y) < 8.f)
        {
            target = current_route.at(0);
            current_route.erase(current_route.begin());
        }
    }
}

/// <summary>
/// Set the route of current tank
/// </summary>
/// <param name="route">The route the tank is following</param>
void Tank::set_route(const std::vector<vec2>& route)
{
    if (route.size() > 0)
    {
        current_route = route;
        target = current_route.at(0);
        current_route.erase(current_route.begin());
    }
    else
    {
        target = position;
    }
}

/// <summary>
/// Start reloading timer
/// </summary>
void Tank::reload_rocket()
{
    reloaded = false;
    reload_time = 200.0f;
}

/// <summary>
/// Deactivate the tank
/// </summary>
void Tank::deactivate()
{
    active = false;
}

/// <summary>
/// Remove health
/// </summary>
/// <param name="hit_value">The amount of health to be removed</param>
/// <returns>Has the tank run out of health?</returns>
bool Tank::hit(int hit_value)
{
    health -= hit_value;

    if (health <= 0)
    {
        this->deactivate();
        return true;
    }

    return false;
}

/// <summary>
/// Draw the sprite with the facing based on this tanks movement direction
/// </summary>
/// <param name="screen">The game screen</param>
void Tank::draw(Surface* screen)
{
    vec2 direction = (target - position).normalized();
    tank_sprite->set_frame(((abs(direction.x) > abs(direction.y)) ? ((direction.x < 0) ? 3 : 0) : ((direction.y < 0) ? 9 : 6)) + (current_frame / 3));
    tank_sprite->draw(screen, (int)position.x - 7 + HEALTHBAR_OFFSET, (int)position.y - 9);
}

/// <summary>
/// Add some force in a given direction
/// </summary>
/// <param name="direction">The direction to push towards</param>
/// <param name="magnitude">How strongly to push</param>
void Tank::push(vec2 direction, float magnitude)
{
    force += direction * magnitude;
}


/// <summary>
/// Calculate the route to the destination for each tank using dijkstra
/// </summary>
/// <param name="tanks"></param>
/// <param name="background_terrain"></param>
/// <param name="frame_count"></param>
void Tank::calculate_tank_routes(vector<Tank> & tanks, Terrain& background_terrain, long long& frame_count)
{
    if (frame_count == 0)
    {
        for (Tank& t : tanks)
        {
            t.set_route(background_terrain.get_route_dijkstra(t, t.target));
        }
    }
}

/// <summary>
/// Check if the tank collides with another
/// </summary>
/// <param name="tanks">The tank to check collision for</param>
void Tank::check_tank_collision_with_kdtree(vector<Tank>& tanks) {
    vector<Tank*> tankPointers;   

    tankPointers.reserve(tanks.size());

    transform(tanks.begin(), tanks.end(), back_inserter(tankPointers), [](Tank& tank) { return &tank; });

    Kdtree kdtree(tankPointers);

    Tank* nearest_tank = nullptr;

    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            nearest_tank =  kdtree.searchNearestTank(&tank);

            if (nearest_tank == nullptr)
			{
				continue;
			}

            vec2 dir = tank.get_position() - nearest_tank -> get_position();
            float dir_squared_len = dir.sqr_length();

            float col_squared_len = (tank.get_collision_radius() + nearest_tank -> get_collision_radius());
            col_squared_len *= col_squared_len;

            if (dir_squared_len < col_squared_len)
            {
                tank.push(dir.normalized(), 1.f);
            }
        }
    }
}

/// <summary>
/// Update the tanks
/// </summary>
/// <param name="tanks">List of tanks</param>
/// <param name="background_terrain">Terrain on the screen</param>
/// <param name="rockets">The rockets fired by tanks</param>
/// <param name="rocket_radius">The radius of a rocket</param>
/// <param name="rocket_red">Sprite for a red rocket</param>
/// <param name="rocket_blue">Sprite for a blue rocket</param>
void Tank::update_tanks(vector<Tank>& tanks, Terrain& background_terrain, vector<Rocket>& rockets, float rocket_radius, Sprite& rocket_red, Sprite& rocket_blue)
{
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            tank.tick(background_terrain);

            if (tank.rocket_reloaded())
            {
                Tank& target = Tank::find_closest_enemy(tank, tanks);

                rockets.push_back(Rocket(tank.position, (target.get_position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

                tank.reload_rocket();
            }
        }
    }
}

/// <summary>
/// Find the closest enemy
/// </summary>
/// <param name="current_tank">The tank to check</param>
/// <param name="tanks">List of tanks</param>
/// <returns>The closest enemy</returns>
Tank& Tank::find_closest_enemy(Tank& current_tank, vector<Tank>& tanks)
{
    float closest_distance = numeric_limits<float>::infinity();
    int closest_index = 0;

    for (int i = 0; i < tanks.size(); i++)
    {
        if (tanks.at(i).allignment != current_tank.allignment && tanks.at(i).active)
        {
            float sqr_dist = fabsf((tanks.at(i).get_position() - current_tank.get_position()).sqr_length());
            if (sqr_dist < closest_distance)
            {
                closest_distance = sqr_dist;
                closest_index = i;
            }
        }
    }

    return tanks.at(closest_index);
}

} // namespace Tmpl8