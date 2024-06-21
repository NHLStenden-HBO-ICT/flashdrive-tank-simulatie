#include "precomp.h"

namespace Tmpl8
{

/// <summary>
/// Update the game tick
/// </summary>
void Smoke::tick()
{
    if (++current_frame == 60) current_frame = 0;
}

/// <summary>
/// Draw the smokes on the screen
/// </summary>
/// <param name="screen">Screen to display the game on</param>
void Smoke::draw(Surface* screen)
{
    smoke_sprite.set_frame(current_frame / 15);

    smoke_sprite.draw(screen, (int)position.x + HEALTHBAR_OFFSET, (int)position.y);
}

/// <summary>
/// Update smoke plumes
/// </summary>
/// <param name="smokes">List of smokes</param>
void Smoke::update(vector<Smoke>& smokes)
{
    
    for (Smoke& smoke : smokes)
    {
        smoke.tick();
    }
}

} // namespace Tmpl8