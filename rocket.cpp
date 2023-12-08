#include "precomp.h"
#include "rocket.h"


namespace Tmpl8
{
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

//Draw the sprite with the facing based on this rockets movement direction
void Rocket::draw(Surface* screen)
{
    rocket_sprite->set_frame(((abs(speed.x) > abs(speed.y)) ? ((speed.x < 0) ? 3 : 0) : ((speed.y < 0) ? 9 : 6)) + (current_frame / 3));
    rocket_sprite->draw(screen, (int)position.x - 12 + HEALTHBAR_OFFSET, (int)position.y - 12);
}

//Does the given circle collide with this rockets collision circle?
bool Rocket::intersects(vec2 position_other, float radius_other) const
{
    //Note: Uses squared lengths to remove expensive square roots
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

void Rocket::update_rockets(vector<Rocket>& rockets, vector<Tank>& tanks, float rocket_hit_value, vector<Explosion>& explosions, Sprite& explosion, vector<Smoke>& smokes, Sprite& smoke)
{
    //Update rockets
    for (Rocket& rocket : rockets)
    {
        rocket.tick();

        //Check if rocket collides with enemy tank, spawn explosion, and if tank is destroyed spawn a smoke plume
        for (Tank& tank : tanks)
        {
            if (tank.active && (tank.allignment != rocket.allignment) && rocket.intersects(tank.position, tank.collision_radius))
            {
                explosions.push_back(Explosion(&explosion, tank.position));

                if (tank.hit(rocket_hit_value))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(7, 24)));
                }

                rocket.active = false;
                break;
            }
        }
    }
}

void Rocket::disable_rockets(vector<Rocket>& rockets, vector<vec2>& forcefield_hull, vector<Explosion> explosions, Sprite& explosion)
{
    //Disable rockets if they collide with the "forcefield"
    //Hint: A point to convex hull intersection test might be better here? :) (Disable if outside)
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
