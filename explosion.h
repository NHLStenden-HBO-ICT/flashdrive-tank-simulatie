#pragma once

namespace Tmpl8
{

class Explosion
{
  public:
    Explosion(Sprite* explosion_sprite, vec2 position) : current_frame(0), explosion_sprite(explosion_sprite), position(position) {}

    void draw(Surface* screen);
    bool isDone() const;

    vec2 position;

    int current_frame;
    Sprite* explosion_sprite;

    static void update_explosions(vector<Explosion>& explosions);
private:
    void tick();
};

}
