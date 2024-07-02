#include "precomp.h" // include (only) this in every .cpp file

constexpr auto NUM_TANKS_BLUE = 2048;
constexpr auto NUM_TANKS_RED = 2048;

constexpr auto TANK_MAX_HEALTH = 1000;
constexpr auto ROCKET_HIT_VALUE = 60;
constexpr auto PARTICLE_BEAM_HIT_VALUE = 50;

constexpr auto TANK_MAX_SPEED = 1.0;

constexpr auto HEALTH_BAR_WIDTH = 70;

constexpr auto MAX_FRAMES = 2000;

//Global performance timer
//constexpr auto REF_PERFORMANCE = 91947.5; // Release reference performance Joël

constexpr auto REF_PERFORMANCE = 136400; // Release reference performance Yvonne, 2,2 speedup with all algorithms

static timer perf_timer;
static float duration;

//Load sprite files and initialize sprites
static Surface* tank_red_img = new Surface("assets/Tank_Proj2.png");
static Surface* tank_blue_img = new Surface("assets/Tank_Blue_Proj2.png");
static Surface* rocket_red_img = new Surface("assets/Rocket_Proj2.png");
static Surface* rocket_blue_img = new Surface("assets/Rocket_Blue_Proj2.png");
static Surface* particle_beam_img = new Surface("assets/Particle_Beam.png");
static Surface* smoke_img = new Surface("assets/Smoke.png");
static Surface* explosion_img = new Surface("assets/Explosion.png");

static Sprite tank_red(tank_red_img, 12);
static Sprite tank_blue(tank_blue_img, 12);
static Sprite rocket_red(rocket_red_img, 12);
static Sprite rocket_blue(rocket_blue_img, 12);
static Sprite smoke(smoke_img, 4);
static Sprite explosion(explosion_img, 9);
static Sprite particle_beam_sprite(particle_beam_img, 3);

const static vec2 TANK_SIZE(7, 9);
const static vec2 ROCKET_SIZE(6, 6);

const static float TANK_RADIUS = 3.f;
const static float ROCKET_RADIUS = 5.f;

size_t num_threads = std::thread::hardware_concurrency();


/// <summary>
///  Initialize the simulation state
/// </summary>
void Game::init()
{
    pool = new ThreadPool(num_threads);

    frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

    tanks.reserve(NUM_TANKS_BLUE + NUM_TANKS_RED);

    uint max_rows = 24;

    float start_blue_x = TANK_SIZE.x + 40.0f;
    float start_blue_y = TANK_SIZE.y + 30.0f;

    float start_red_x = 1088.0f;
    float start_red_y = TANK_SIZE.y + 30.0f;

    float spacing = 7.5f;
    int tank_offset = 16;
    float start_x_blue_tanks = 1100.f;
    float start_x_red_tanks = 100.f;

    //Spawn blue tanks
    for (int i = 0; i < NUM_TANKS_BLUE; i++)
    {
        vec2 position{ start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing) };
        tanks.push_back(Tank(position.x, position.y, BLUE, &tank_blue, &smoke, start_x_blue_tanks, position.y + tank_offset, TANK_RADIUS, TANK_MAX_HEALTH, TANK_MAX_SPEED));

    }
    //Spawn red tanks
    for (int i = 0; i < NUM_TANKS_RED; i++)
    {
        vec2 position{ start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing) };
        tanks.push_back(Tank(position.x, position.y, RED, &tank_red, &smoke, start_x_red_tanks, position.y + tank_offset, TANK_RADIUS, TANK_MAX_HEALTH, TANK_MAX_SPEED));
    }

    
    particle_beams.push_back(Particle_beam(vec2(590, 327), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(Particle_beam(vec2(64, 64), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));

 }


/// <summary>
/// Close down application
/// </summary>
void Game::shutdown()
{
}

/// <summary>
/// Checks if a point lies on the left of an arbitrary angled line
/// </summary>
/// <param name="line_start">Starting point of the line</param>
/// <param name="line_end">Ending point of the line</param>
/// <param name="point">Coordinate point</param>
/// <returns></returns>
bool Tmpl8::Game::left_of_line(vec2 line_start, vec2 line_end, vec2 point)
{
    return ((line_end.x - line_start.x) * (point.y - line_start.y) - (line_end.y - line_start.y) * (point.x - line_start.x)) < 0;
}
// -----------------------------------------------------------
// Update the game state:
// Move all objects
// Update sprite frames
// Collision detection
// Targeting etc..
// -----------------------------------------------------------

/// <summary>
/// Update the game state by moving al objects, collision detection and update sprites
/// </summary>
void Game::update()
{
    Tank::calculate_tank_routes(tanks, background_terrain, frame_count);
    Tank::check_tank_collision_with_kdtree(tanks);
    Tank::update_tanks(tanks.size(), num_threads, pool, futures, tanks, background_terrain, rockets, ROCKET_RADIUS, rocket_red, rocket_blue);

    Smoke::update(smokes);

    forcefield_hull.clear();

    int first_active = 0;
    find_first_active_tank(first_active);
    vec2 point_on_hull = tanks.at(first_active).position;

    find_most_left_tank(point_on_hull);

    calculate_rockets_convex_hull(point_on_hull, first_active);

    Rocket::update_rockets(pool, futures, rockets, tanks, ROCKET_HIT_VALUE, explosions, explosion, smokes, smoke);

    Rocket::disable_rockets(rockets, forcefield_hull, explosions, explosion);

    rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }), rockets.end());

    Particle_beam::update_particle_beams(particle_beams, tanks, smokes, smoke);
    Explosion::update_explosions(explosions);

    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.isDone(); }), explosions.end());
}


/// <summary>
/// Loop through all points replacing the endpoint with the current iteration every time 
/// it lies left of the current segment formed by point_on_hull and the current endpoint.
/// By the end we have a segment with no points on the left and thus a point on the convex hull.
/// </summary>
/// <param name="point_on_hull">The point on the convex hull to check</param>
/// <param name="first_active">First active rocket that is found</param>
void Tmpl8::Game::calculate_rockets_convex_hull(Tmpl8::vec2& point_on_hull, int first_active)
{
    while (true)
    {
        forcefield_hull.push_back(point_on_hull);
       
        vec2 endpoint = tanks.at(first_active).position;

        for (Tank& tank : tanks)
        {
            if (tank.active)
            {
                if ((endpoint == point_on_hull) || left_of_line(point_on_hull, endpoint, tank.position))
                {
                    endpoint = tank.position;
                }
            }
        }

        point_on_hull = endpoint;

        if (endpoint == forcefield_hull.at(0))
        {
            break;
        }
    }
}

/// <summary>
/// Find the tank that is most left from point of the convex hull
/// </summary>
/// <param name="point_on_hull">The point where to check from</param>
void Tmpl8::Game::find_most_left_tank(Tmpl8::vec2& point_on_hull)
{
    for (Tank& tank : tanks)
    {
        if (tank.active && tank.position.x <= point_on_hull.x)
        {
            point_on_hull = tank.position;
        }
    }
}

/// <summary>
/// Find the first active tank
/// </summary>
/// <param name="first_active">The current first active tank</param>
void Tmpl8::Game::find_first_active_tank(int& first_active)
{
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            break;
        }

        first_active++;
    }
}

/// <summary>
/// Draw all sprites to the screen 
/// </summary>
void Game::draw()
{
    screen->clear(0);

    background_terrain.draw(screen);

    for (int i = 0; i < NUM_TANKS_BLUE + NUM_TANKS_RED; i++)
    {
        tanks.at(i).draw(screen);

        vec2 tank_pos = tanks.at(i).get_position();
    }

    for (Rocket& rocket : rockets)
    {
        rocket.draw(screen);
    }

    for (Smoke& smoke : smokes)
    {
        smoke.draw(screen);
    }

    for (Particle_beam& particle_beam : particle_beams)
    {
        particle_beam.draw(screen);
    }

    for (Explosion& explosion : explosions)
    {
        explosion.draw(screen);
    }

    // Draw forcefield
    for (size_t i = 0; i < forcefield_hull.size(); i++)
    {
        vec2 line_start = forcefield_hull.at(i);
        vec2 line_end = forcefield_hull.at((i + 1) % forcefield_hull.size());
        line_start.x += HEALTHBAR_OFFSET;
        line_end.x += HEALTHBAR_OFFSET;
        screen->line(line_start, line_end, 0x0000ff);
    }

    // Draw sorted health bars for blue and red team
    for (int t = 0; t < 2; t++)
    {
        const int NUM_TANKS = ((t < 1) ? NUM_TANKS_BLUE : NUM_TANKS_RED);

        const int begin = ((t < 1) ? 0 : NUM_TANKS_BLUE);
        std::vector<const Tank*> sorted_tanks;

        quick_sort_init(tanks, sorted_tanks, begin, begin + NUM_TANKS);

        sorted_tanks.erase(std::remove_if(sorted_tanks.begin(), sorted_tanks.end(), [](const Tank* tank) { return !tank->active; }), sorted_tanks.end());

        draw_health_bars(sorted_tanks, t);
    }
}

/// <summary>
/// Quick sort healthbar values
/// </summary>
/// <param name="sorted_tanks">The list of already sorted tanks</param>
/// <param name="begin">Start position to sort</param>
/// <param name="end">End position to sort</param>
void Tmpl8::Game::quick_sort(vector<const Tank*>& sorted_tanks, int begin, int end)
{
    if (begin < end)
    {

        if (begin == NUM_TANKS_BLUE)
        {
            begin = 0;
            end = end - NUM_TANKS_BLUE;
        }

        //Get pivot
        int pivot = Median::get_median_health(sorted_tanks, begin, end);
        const int* pivot_ptr = &pivot;

        int i = begin;
        int j = end - 1;

        while (i <= j)
        {
            while ((int)sorted_tanks.at(i)->health < *pivot_ptr)
                i++;

            while ((int)sorted_tanks.at(j)->health > *pivot_ptr)
                j--;

            if (i <= j)
            {
                std::swap(sorted_tanks.at(i), sorted_tanks.at(j));
                i++;
                j--;
            }
        }

        quick_sort(sorted_tanks, begin, j + 1);
        quick_sort(sorted_tanks, i, end);
    }
}

/// <summary>
/// Quick sort initializer 
/// </summary>
/// <param name="tanks">List of tanks to sort</param>
/// <param name="sorted_tanks">List to put sorted tanks into</param>
/// <param name="begin">Start position</param>
/// <param name="end">End position</param>
void Tmpl8::Game::quick_sort_init(const std::vector<Tank>& tanks, vector<const Tank*>& sorted_tanks, int begin, int end)
{
    sorted_tanks.reserve(end - begin);

    std::transform(tanks.begin() + begin, tanks.begin() + end, std::back_inserter(sorted_tanks),
        [](const Tank& tank) { return &tank; });

    quick_sort(sorted_tanks, begin, end);
}

/// <summary>
/// Draw the health bars based on the given tanks health values
/// </summary>
/// <param name="sorted_tanks">List of sorted tanks</param>
/// <param name="team">Team number to represent red (1) or blue (0)</param>
void Tmpl8::Game::draw_health_bars(const vector<const Tank*>& sorted_tanks, const int team)
{
    int healthbar_height = 1;
    int health_bar_start_x = (team < 1) ? 0 : (SCRWIDTH - HEALTHBAR_OFFSET) - 1;
    int health_bar_end_x = (team < 1) ? HEALTH_BAR_WIDTH : health_bar_start_x + HEALTH_BAR_WIDTH - 1;

    for (int i = 0; i < SCRHEIGHT - 1; i++)
    {
        int health_bar_start_y = i * healthbar_height;
        int health_bar_end_y = health_bar_start_y + healthbar_height;

        screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
    }

    int draw_count = std::min(SCRHEIGHT, (int)sorted_tanks.size());
    for (int i = 0; i < draw_count - healthbar_height; i++)
    {

        int health_bar_start_y = i * healthbar_height;
        int health_bar_end_y = health_bar_start_y + healthbar_height;

        float health_fraction = (1 - ((double)sorted_tanks.at(i)->health / (double)TANK_MAX_HEALTH));

        if (team == 0) { screen->bar(health_bar_start_x + (int)((double)HEALTH_BAR_WIDTH * health_fraction), health_bar_start_y, health_bar_end_x, health_bar_end_y, GREENMASK); }
        else { screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x - (int)((double)HEALTH_BAR_WIDTH * health_fraction), health_bar_end_y, GREENMASK); }
    }
}

/// <summary>
/// When we reach max_frames print the duration and speedup multiplier 
/// </summary>
void Tmpl8::Game::measure_performance()
{
    char buffer[128];
    if (frame_count >= MAX_FRAMES)
    {
        if (!lock_update)
        {
            duration = perf_timer.elapsed();

            cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
            lock_update = true;
        }

        frame_count--;
    }

    if (lock_update)
    {
        screen->bar(420 + HEALTHBAR_OFFSET, 170, 870 + HEALTHBAR_OFFSET, 430, 0x030000);
        int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60, min = ((int)duration / 60000);
        sprintf(buffer, "%02i:%02i:%03i", min, sec, ms);
        frame_count_font->centre(screen, buffer, 200);
        sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
        frame_count_font->centre(screen, buffer, 340);
    }
}

/// <summary>
/// Every tick the game updates
/// </summary>
void Game::tick()
{
    if (!lock_update)
    {
        update();
    }
    draw();

    measure_performance();

    frame_count++;
    string frame_count_string = "FRAME: " + std::to_string(frame_count);
    frame_count_font->print(screen, frame_count_string.c_str(), 350, 580);
}
