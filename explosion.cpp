#include "precomp.h"
#include "explosion.h"


namespace Tmpl8
{
    const int MAX_EXPLOSION_FRAMES = 18;

    bool Explosion::isDone() const
    {
        return current_frame == MAX_EXPLOSION_FRAMES;
    }

    void Explosion::tick()
    {
        if (current_frame < MAX_EXPLOSION_FRAMES) current_frame++;
    }

    void Explosion::draw(Surface* screen)
    {
        explosion_sprite->set_frame(current_frame / 2);
        explosion_sprite->draw(screen, (int)position.x + HEALTHBAR_OFFSET, (int)position.y);
    }

    void Explosion::update_explosions(vector<Explosion>& explosions)
    {
        //Update explosion sprites and remove when done with remove erase idiom
        for (Explosion& explosion : explosions)
        {
            explosion.tick();
        }


    }
}
