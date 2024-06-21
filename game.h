#pragma once

namespace Tmpl8
{
    //forward declarations
    class Tank;
    class Rocket;
    class Smoke;
    class Particle_beam;
    class Median;

    class Game
    {
    public:
        void set_target(Surface* surface) { screen = surface; }
        void init();
        void shutdown();
        void update();
        void calculate_rockets_convex_hull(Tmpl8::vec2& point_on_hull, int first_active);

        void find_most_left_tank(Tmpl8::vec2& point_on_hull);
        void find_first_active_tank(int& first_active);

        void draw();
        void tick();

        void quick_sort(std::vector<const Tank*>& sorted_tanks, int begin, int end);
        void quick_sort_init(const std::vector<Tank>& tanks, std::vector<const Tank*>& sorted_tanks, int begin, int end);
        
        void draw_health_bars(const std::vector<const Tank*>& sorted_tanks, const int team);
        void measure_performance();

        void mouse_up(int button)
        { /* implement if you want to detect mouse button presses */
        }

        void mouse_down(int button)
        { /* implement if you want to detect mouse button presses */
        }

        void mouse_move(int x, int y)
        { /* implement if you want to detect mouse movement */
        }

        void key_up(int key)
        { /* implement if you want to handle keys */
        }

        void key_down(int key)
        { /* implement if you want to handle keys */
        }

    private:
        Surface* screen;
        ThreadPool* pool;
        vector<Tank> tanks;
        vector<Smoke> smokes;
        vector<Rocket> rockets;
        vector<Explosion> explosions;
        vector<Particle_beam> particle_beams;
        std::vector<std::future<void>> futures;

        Terrain background_terrain;
        std::vector<vec2> forcefield_hull;

        Font* frame_count_font;
        long long frame_count = 0;

        bool lock_update = false;

        //Checks if a point lies on the left of an arbitrary angled line
        bool left_of_line(vec2 line_start, vec2 line_end, vec2 point);
    };

}; // namespace Tmpl8