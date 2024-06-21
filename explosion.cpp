#include "precomp.h"
#include "explosion.h"


namespace Tmpl8
{
    const int MAX_EXPLOSION_FRAMES = 18;

    /// <summary>
    /// is the explosion done
    /// </summary>
    /// <returns>if the explosion is done</returns>
    bool Explosion::isDone() const
    {
        return current_frame == MAX_EXPLOSION_FRAMES;
    }

    /// <summary>
    /// Update explosion animation with one tick
    /// </summary>
    void Explosion::tick()
    {
        if (current_frame < MAX_EXPLOSION_FRAMES) current_frame++;
    }

    /// <summary>
    /// Draw the explosion
    /// </summary>
    /// <param name="screen">Screen to draw to</param>
    void Explosion::draw(Surface* screen)
    {
        explosion_sprite->set_frame(current_frame / 2);
        explosion_sprite->draw(screen, (int)position.x + HEALTHBAR_OFFSET, (int)position.y);
    }

    /// <summary>
    /// Update all explosions
    /// </summary>
    /// <param name="explosions">Explosions to update</param>
    void Explosion::update_explosions(vector<Explosion>& explosions)
    {
        //Update explosion sprites and remove when done with remove erase idiom
        for (Explosion& explosion : explosions)
        {
            explosion.tick();
        }


    }
}
