#include "precomp.h"

namespace Tmpl8
{

    Particle_beam::Particle_beam() : min_position(), max_position(), particle_beam_sprite(nullptr), sprite_frame(0), rectangle(), damage(1)
    {
    }

    Particle_beam::Particle_beam(vec2 min, vec2 max, Sprite* particle_beam_sprite, int damage) : particle_beam_sprite(particle_beam_sprite), sprite_frame(0), damage(damage)
    {
        min_position = min;
        max_position = min + max;

        rectangle = Rectangle2D(min_position, max_position);
    }

    void Particle_beam::tick(vector<Tank>& tanks)
    {

        if (++sprite_frame == 30)
        {
            sprite_frame = 0;
        }
    }

    /// <summary>
    /// //Update particle beams
    /// </summary>
    /// <param name="particle_beams">List of particle beams</param>
    /// <param name="tanks">List of tanks</param>
    /// <param name="smokes">List of smokes</param>
    /// <param name="smoke">Smoke sprite</param>
    void Particle_beam::update_particle_beams(vector<Particle_beam>& particle_beams, vector<Tank>& tanks, vector<Smoke>& smokes, Sprite& smoke)
    {
        
        for (Particle_beam& particle_beam : particle_beams)
        {
            particle_beam.tick(tanks);

            for (Tank& tank : tanks)
            {
                if (tank.active && particle_beam.rectangle.intersects_circle(tank.get_position(), tank.get_collision_radius()))
                {
                    if (tank.hit(particle_beam.damage))
                    {
                        smokes.push_back(Smoke(smoke, tank.position - vec2(0, 48)));
                    }
                }
            }
        }
    }

    void Particle_beam::draw(Surface* screen)
    {
        vec2 position = rectangle.min;

        const int offset_x = 23;
        const int offset_y = 137;

        particle_beam_sprite->set_frame(sprite_frame / 10);
        particle_beam_sprite->draw(screen, (int)(position.x - offset_x + HEALTHBAR_OFFSET), (int)(position.y - offset_y));
    }

} // namespace Tmpl8