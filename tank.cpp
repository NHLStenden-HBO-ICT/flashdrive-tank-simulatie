#include "precomp.h"
#include "tank.h"


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

void Tank::tick(Terrain& terrain)
{
    vec2 direction = vec2(0, 0);

    if (target != position)
    {
        direction = (target - position).normalized();
    }

    //Update using accumulated force
    speed = direction + force;
    position += speed * max_speed * 0.5f;

    //Update reload time
    if (--reload_time <= 0.0f)
    {
        reloaded = true;
    }

    force = vec2(0.f, 0.f);

    if (++current_frame > 8) current_frame = 0;

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

//Start reloading timer
void Tank::reload_rocket()
{
    reloaded = false;
    reload_time = 200.0f;
}

void Tank::deactivate()
{
    active = false;
}

//Remove health
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

//Draw the sprite with the facing based on this tanks movement direction
void Tank::draw(Surface* screen)
{
    vec2 direction = (target - position).normalized();
    tank_sprite->set_frame(((abs(direction.x) > abs(direction.y)) ? ((direction.x < 0) ? 3 : 0) : ((direction.y < 0) ? 9 : 6)) + (current_frame / 3));
    tank_sprite->draw(screen, (int)position.x - 7 + HEALTHBAR_OFFSET, (int)position.y - 9);
}

//Add some force in a given direction
void Tank::push(vec2 direction, float magnitude)
{
    force += direction * magnitude;
}

void Tank::calculate_tank_routes(vector<Tank> & tanks, Terrain& background_terrain, long long& frame_count)
{
    //Calculate the route to the destination for each tank using dijkstra
    //Initializing routes here so it gets counted for performance..
    if (frame_count == 0)
    {
        for (Tank& t : tanks)
        {
            t.set_route(background_terrain.get_route_dijkstra(t, t.target));
        }
    }
}

void Tank::check_tank_collision(vector<Tank>& tanks)
{
    //Check tank collision and nudge tanks away from each other
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            for (Tank& other_tank : tanks)
            {
                if (&tank == &other_tank || !other_tank.active) continue;

                vec2 dir = tank.get_position() - other_tank.get_position();
                float dir_squared_len = dir.sqr_length();

                float col_squared_len = (tank.get_collision_radius() + other_tank.get_collision_radius());
                col_squared_len *= col_squared_len;

                if (dir_squared_len < col_squared_len)
                {
                    tank.push(dir.normalized(), 1.f);
                }
            }
        }
    }
}

void Tank::check_tank_collision_with_kdtree(vector<Tank>& tanks) {
    vector<Tank*> tankPointers;   

    // Maak een nieuwe vector die pointers naar de Tanks bevat
    tankPointers.reserve(tanks.size()); // Reserveren om reallocation te voorkomen

    // Transformeer de vector naar pointers
    transform(tanks.begin(), tanks.end(), back_inserter(tankPointers), [](Tank& tank) { return &tank; });

    // Maak een Kdtree met de const vector van pointers
    Kdtree kdtree(tankPointers);


    // # Voor elke tank dichtsbijzijnde tank binnen straal zoeken

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

void Tank::update_tanks(vector<Tank>& tanks, Terrain& background_terrain, vector<Rocket>& rockets, float rocket_radius, Sprite& rocket_red, Sprite& rocket_blue)
{
    //Update tanks
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            //Move tanks according to speed and nudges (see above) also reload
            tank.tick(background_terrain);

            //Shoot at closest target if reloaded
            if (tank.rocket_reloaded())
            {
                Tank& target = Tank::find_closest_enemy(tank, tanks);

                rockets.push_back(Rocket(tank.position, (target.get_position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

                tank.reload_rocket();
            }
        }
    }
}

// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given tank
// -----------------------------------------------------------
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