#include "precomp.h"
#include "explosion.h"


namespace Tmpl8
{
    bool Explosion::done() const
    {
        return current_frame > 17;
    }

    void Explosion::tick()
    {
        if (current_frame < 18) current_frame++;
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
