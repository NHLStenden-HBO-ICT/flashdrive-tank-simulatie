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
//constexpr auto REF_PERFORMANCE = 430730; // Debug reference performance Joël
//constexpr auto REF_PERFORMANCE = 91947.5; // Release reference performance Joël

//constexpr auto REF_PERFORMANCE = 537033; // Debug reference performance Yvonne, 1.1 speedup with quick sort
constexpr auto REF_PERFORMANCE = 143674; // Release reference performance Yvonne

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

// -----------------------------------------------------------
// Initialize the simulation state
// This function does not count for the performance multiplier
// (Feel free to optimize anyway though ;) )
// -----------------------------------------------------------
void Game::init()
{
    cout << "Number of threads is " << num_threads << "!" << endl;

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

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::shutdown()
{
}
//Checks if a point lies on the left of an arbitrary angled line
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
void Game::update(float deltaTime)
{
    Tank::calculate_tank_routes(tanks, background_terrain, frame_count);
    Tank::check_tank_collision_with_kdtree(tanks);
    Tank::update_tanks(tanks, background_terrain, rockets, ROCKET_RADIUS, rocket_red, rocket_blue);

    Smoke::update(smokes);

    //Calculate "forcefield" around active tanks
    forcefield_hull.clear();

    //Find first active tank (this loop is a bit disgusting, fix?)
    int first_active = 0;
    find_first_active_tank(first_active);
    vec2 point_on_hull = tanks.at(first_active).position;
    //Find left most tank position
    find_most_left_tank(point_on_hull);

    calculate_rockets_convex_hull(point_on_hull, first_active);

    Rocket::update_rockets(rockets, tanks, ROCKET_HIT_VALUE, explosions, explosion, smokes, smoke);
    Rocket::disable_rockets(rockets, forcefield_hull, explosions, explosion);

    //Remove exploded rockets with remove erase idiom
    rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }), rockets.end());


    Particle_beam::update_particle_beams(particle_beams, tanks, smokes, smoke);
    Explosion::update_explosions(explosions);

    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.isDone(); }), explosions.end());


}

void Tmpl8::Game::calculate_rockets_convex_hull(Tmpl8::vec2& point_on_hull, int first_active)
{
    //Calculate convex hull for 'rocket barrier'
    while (true)
    {
        //Add last found point
        forcefield_hull.push_back(point_on_hull);

        //Loop through all points replacing the endpoint with the current iteration every time 
        //it lies left of the current segment formed by point_on_hull and the current endpoint.
        //By the end we have a segment with no points on the left and thus a point on the convex hull.
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

        //Set the starting point of the next segment to the found endpoint.
        point_on_hull = endpoint;

        //If we went all the way around we are done.
        if (endpoint == forcefield_hull.at(0))
        {
            break;
        }
    }
}
void Tmpl8::Game::find_most_left_tank(Tmpl8::vec2& point_on_hull)
{
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            if (tank.position.x <= point_on_hull.x)
            {
                point_on_hull = tank.position;
            }
        }
    }
}
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
// -----------------------------------------------------------
// Draw all sprites to the screen
// (It is not recommended to multi-thread this function)
// -----------------------------------------------------------
void Game::draw()
{
    // clear the graphics window
    screen->clear(0);

    //Draw background
    background_terrain.draw(screen);

    //Draw sprites
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

    //Draw forcefield (mostly for debugging, its kinda ugly..)
    for (size_t i = 0; i < forcefield_hull.size(); i++)
    {
        vec2 line_start = forcefield_hull.at(i);
        vec2 line_end = forcefield_hull.at((i + 1) % forcefield_hull.size());
        line_start.x += HEALTHBAR_OFFSET;
        line_end.x += HEALTHBAR_OFFSET;
        screen->line(line_start, line_end, 0x0000ff);
    }

    //Draw sorted health bars
    for (int t = 0; t < 2; t++)
    {
        const int NUM_TANKS = ((t < 1) ? NUM_TANKS_BLUE : NUM_TANKS_RED);

        const int begin = ((t < 1) ? 0 : NUM_TANKS_BLUE);
        std::vector<const Tank*> sorted_tanks;

        //insertion_sort_tanks_health(tanks, sorted_tanks, begin, begin + NUM_TANKS);
        quick_sort_init(tanks, sorted_tanks, begin, begin + NUM_TANKS);

        sorted_tanks.erase(std::remove_if(sorted_tanks.begin(), sorted_tanks.end(), [](const Tank* tank) { return !tank->active; }), sorted_tanks.end());

        draw_health_bars(sorted_tanks, t);
    }
}
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

        // Partition
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

        // Recursive
        quick_sort(sorted_tanks, begin, j + 1);
        quick_sort(sorted_tanks, i, end);
    }
}
void Tmpl8::Game::quick_sort_init(const std::vector<Tank>& tanks, vector<const Tank*>& sorted_tanks, int begin, int end)
{

    sorted_tanks.reserve(end - begin);

    // Add all tanks to sorted tanks
    std::transform(tanks.begin() + begin, tanks.begin() + end, std::back_inserter(sorted_tanks),
        [](const Tank& tank) { return &tank; });

    quick_sort(sorted_tanks, begin, end);
}
// -----------------------------------------------------------
// Draw the health bars based on the given tanks health values
// -----------------------------------------------------------
void Tmpl8::Game::draw_health_bars(const vector<const Tank*>& sorted_tanks, const int team)
{
    int health_bar_start_x = (team < 1) ? 0 : (SCRWIDTH - HEALTHBAR_OFFSET) - 1;
    int health_bar_end_x = (team < 1) ? HEALTH_BAR_WIDTH : health_bar_start_x + HEALTH_BAR_WIDTH - 1;

    for (int i = 0; i < SCRHEIGHT - 1; i++)
    {
        //Health bars are 1 pixel each
        int health_bar_start_y = i * 1;
        int health_bar_end_y = health_bar_start_y + 1;

        screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
    }

    //Draw the <SCRHEIGHT> least healthy tank health bars
    int draw_count = std::min(SCRHEIGHT, (int)sorted_tanks.size());
    for (int i = 0; i < draw_count - 1; i++)
    {
        //Health bars are 1 pixel each
        int health_bar_start_y = i * 1;
        int health_bar_end_y = health_bar_start_y + 1;

        float health_fraction = (1 - ((double)sorted_tanks.at(i)->health / (double)TANK_MAX_HEALTH));

        if (team == 0) { screen->bar(health_bar_start_x + (int)((double)HEALTH_BAR_WIDTH * health_fraction), health_bar_start_y, health_bar_end_x, health_bar_end_y, GREENMASK); }
        else { screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x - (int)((double)HEALTH_BAR_WIDTH * health_fraction), health_bar_end_y, GREENMASK); }
    }
}
// -----------------------------------------------------------
// When we reach max_frames print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
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
// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::tick(float deltaTime)
{
    if (!lock_update)
    {
        update(deltaTime);
    }
    draw();

    measure_performance();

    frame_count++;
    string frame_count_string = "FRAME: " + std::to_string(frame_count);
    frame_count_font->print(screen, frame_count_string.c_str(), 350, 580);
}
