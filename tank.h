#pragma once

namespace Tmpl8
{
    class Terrain;
    class Rocket;

enum allignments
{
    BLUE,
    RED
};

class Tank
{
public:
    Tank(float pos_x, float pos_y, allignments allignment, Sprite* tank_sprite, Sprite* smoke_sprite, float tar_x, float tar_y, float collision_radius, int health, float max_speed);

    ~Tank();

    void tick(Terrain& terrain);

    vec2 get_position() const { return position; };
    float get_collision_radius() const { return collision_radius; };
    bool rocket_reloaded() const { return reloaded; };

    void set_route(const std::vector<vec2>& route);
    void reload_rocket();

    void deactivate();
    bool hit(int hit_value);

    void draw(Surface* screen);

    void push(vec2 direction, float magnitude);

    vec2 position;
    vec2 speed;
    vec2 target;

    vector<vec2> current_route;

    int health;

    float collision_radius;
    vec2 force;

    float max_speed;
    float reload_time;

    bool reloaded;
    bool active;

    allignments allignment;

    int current_frame;
    Sprite* tank_sprite;
    Sprite* smoke_sprite;

    static void calculate_tank_routes(vector<Tank>& tanks, Terrain& background_terrain, long long& frame_count);
    static void check_tank_collision_with_kdtree(vector<Tank>& tanks);
    static Tank& find_closest_enemy(Tank& current_tank, vector<Tank>& tanks);
    static void update_tanks(vector<Tank>& tanks, Terrain& background_terrain, vector<Rocket>& rockets, float rocket_radius, Sprite& rocket_red, Sprite& rocket_blue);
};

} 