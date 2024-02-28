#include "precomp.h" // include (only) this in every .cpp file
#include "smoke.h"
#include "tank.h"
#include "rocket.h"
#include "explosion.h"
#include "particle_beam.h"

constexpr auto num_tanks_blue = 2048;
constexpr auto num_tanks_red = 2048;

constexpr auto tank_max_health = 1000;
constexpr auto rocket_hit_value = 60;
constexpr auto particle_beam_hit_value = 50;

constexpr auto tank_max_speed = 1.0;

constexpr auto health_bar_width = 70;

constexpr auto max_frames = 2000;

//Global performance timer
//constexpr auto REF_PERFORMANCE = 430730; // Debug reference performance Joël
//constexpr auto REF_PERFORMANCE = 68916; // Release reference performance Joël

constexpr auto REF_PERFORMANCE = 537033; // Debug reference performance Yvonne
//constexpr auto REF_PERFORMANCE = 132409; // Release reference performance Yvonne
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

const static vec2 tank_size(7, 9);
const static vec2 rocket_size(6, 6);

const static float tank_radius = 3.f;
const static float rocket_radius = 5.f;

static int recursion_count = 0;

// -----------------------------------------------------------
// Initialize the simulation state
// This function does not count for the performance multiplier
// (Feel free to optimize anyway though ;) )
// -----------------------------------------------------------
void Game::init()
{
    frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

    tanks.reserve(num_tanks_blue + num_tanks_red);

    uint max_rows = 24;

    float start_blue_x = tank_size.x + 40.0f;
    float start_blue_y = tank_size.y + 30.0f;

    float start_red_x = 1088.0f;
    float start_red_y = tank_size.y + 30.0f;

    float spacing = 7.5f;

    //Spawn blue tanks
    for (int i = 0; i < num_tanks_blue; i++)
    {
        vec2 position{ start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing) };
        tanks.push_back(Tank(position.x, position.y, BLUE, &tank_blue, &smoke, 1100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
    }
    //Spawn red tanks
    for (int i = 0; i < num_tanks_red; i++)
    {
        vec2 position{ start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing) };
        tanks.push_back(Tank(position.x, position.y, RED, &tank_red, &smoke, 100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
    }

    particle_beams.push_back(Particle_beam(vec2(590, 327), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
    particle_beams.push_back(Particle_beam(vec2(64, 64), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
    particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
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
    Tank::check_tank_collision(tanks);
    Tank::update_tanks(tanks, background_terrain, rockets, rocket_radius, rocket_red, rocket_blue);

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

    update_rockets();
    disable_rockets();

    //Remove exploded rockets with remove erase idiom
    rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }), rockets.end());


    update_particle_beams();
    Explosion::update_explosions(explosions);

    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.done(); }), explosions.end());


}



void Tmpl8::Game::update_particle_beams()
{
    //Update particle beams
    for (Particle_beam& particle_beam : particle_beams)
    {
        particle_beam.tick(tanks);

        //Damage all tanks within the damage window of the beam (the window is an axis-aligned bounding box)
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

void Tmpl8::Game::disable_rockets()
{
    //Disable rockets if they collide with the "forcefield"
    //Hint: A point to convex hull intersection test might be better here? :) (Disable if outside)
    for (Rocket& rocket : rockets)
    {
        if (rocket.active)
        {
            for (size_t i = 0; i < forcefield_hull.size(); i++)
            {
                if (circle_segment_intersect(forcefield_hull.at(i), forcefield_hull.at((i + 1) % forcefield_hull.size()), rocket.position, rocket.collision_radius))
                {
                    explosions.push_back(Explosion(&explosion, rocket.position));
                    rocket.active = false;
                }
            }
        }
    }

}

void Tmpl8::Game::update_rockets()
{
    //Update rockets
    for (Rocket& rocket : rockets)
    {
        rocket.tick();

        //Check if rocket collides with enemy tank, spawn explosion, and if tank is destroyed spawn a smoke plume
        for (Tank& tank : tanks)
        {
            if (tank.active && (tank.allignment != rocket.allignment) && rocket.intersects(tank.position, tank.collision_radius))
            {
                explosions.push_back(Explosion(&explosion, tank.position));

                if (tank.hit(rocket_hit_value))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(7, 24)));
                }

                rocket.active = false;
                break;
            }
        }
    }
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
    for (int i = 0; i < num_tanks_blue + num_tanks_red; i++)
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
        const int NUM_TANKS = ((t < 1) ? num_tanks_blue : num_tanks_red);

        const int begin = ((t < 1) ? 0 : num_tanks_blue);
        std::vector<const Tank*> sorted_tanks;

        //insertion_sort_tanks_health(tanks, sorted_tanks, begin, begin + NUM_TANKS);
        quick_sort_init(tanks, sorted_tanks, begin, begin + NUM_TANKS);

        sorted_tanks.erase(std::remove_if(sorted_tanks.begin(), sorted_tanks.end(), [](const Tank* tank) { return !tank->active; }), sorted_tanks.end());

        draw_health_bars(sorted_tanks, t);
    }
}

// -----------------------------------------------------------
// Sort tanks by health value using insertion sort
// -----------------------------------------------------------
void Tmpl8::Game::insertion_sort_tanks_health(const std::vector<Tank>& original, std::vector<const Tank*>& sorted_tanks, int begin, int end)
{
    const int NUM_TANKS = end - begin;
    sorted_tanks.reserve(NUM_TANKS);
    sorted_tanks.emplace_back(&original.at(begin));

    for (int i = begin + 1; i < (begin + NUM_TANKS); i++)
    {
        const Tank& current_tank = original.at(i);

        for (int s = (int)sorted_tanks.size() - 1; s >= 0; s--)
        {
            const Tank* current_checking_tank = sorted_tanks.at(s);

            if ((current_checking_tank->compare_health(current_tank) <= 0))
            {
                sorted_tanks.insert(1 + sorted_tanks.begin() + s, &current_tank);
                break;
            }

            if (s == 0)
            {
                sorted_tanks.insert(sorted_tanks.begin(), &current_tank);
                break;
            }
        }
    }
}

int Tmpl8::Game::get_median(const std::vector<const Tank*>& input, int begin, int end)
{

    if (end - begin == 2)
    {
        return (int)input.at(begin)->health;
    }

    const int center = (end + begin) / 2;
    const int tank_health_at_begin = (int)input.at(begin)->health;
    const int tank_health_at_center = (int)input.at(center)->health;
    const int tank_health_at_end =  (int)input.at(end - 1)->health;

    std::vector<int> index_values = { tank_health_at_begin, tank_health_at_center, tank_health_at_end };

    // Sort the vector to get the middle value
    for (int i = 0; i < index_values.size(); i++) {
        for (int j = 0; j < index_values.size() - 1; j++)
        {
            if (index_values.at(j) > index_values.at(j + 1))
            {
                std::swap(index_values.at(j), index_values.at(j + 1));
            }
        }
    }
    return index_values.at(1);
}



void Tmpl8::Game::quick_sort(std::vector<const Tank*>& sorted_tanks, int begin, int end)
{
    if (begin < end)
    {

        if (begin == num_tanks_blue)
        {
            begin = 0;
            end = end - num_tanks_blue;
        }

        //Get pivot
        int pivot = get_median(sorted_tanks, begin, end);
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

        // Recursive calls
        quick_sort(sorted_tanks, begin, j + 1);
        quick_sort(sorted_tanks, i, end);
    }
}

void Tmpl8::Game::quick_sort_init(const std::vector<Tank>& tanks, std::vector<const Tank*>& sorted_tanks, int begin, int end)
{

    //Add everything from tanks to sorted tanks
    sorted_tanks.reserve(end - begin);

    // Add all tanks to sorted tanks
    std::transform(tanks.begin() + begin, tanks.begin() + end, std::back_inserter(sorted_tanks),
        [](const Tank& tank) { return &tank; });

    //Use sorted tanks to call quick sort
    quick_sort(sorted_tanks, begin, end);
}


// -----------------------------------------------------------
// Draw the health bars based on the given tanks health values
// -----------------------------------------------------------
void Tmpl8::Game::draw_health_bars(const std::vector<const Tank*>& sorted_tanks, const int team)
{
    int health_bar_start_x = (team < 1) ? 0 : (SCRWIDTH - HEALTHBAR_OFFSET) - 1;
    int health_bar_end_x = (team < 1) ? health_bar_width : health_bar_start_x + health_bar_width - 1;

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

        float health_fraction = (1 - ((double)sorted_tanks.at(i)->health / (double)tank_max_health));

        if (team == 0) { screen->bar(health_bar_start_x + (int)((double)health_bar_width * health_fraction), health_bar_start_y, health_bar_end_x, health_bar_end_y, GREENMASK); }
        else { screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x - (int)((double)health_bar_width * health_fraction), health_bar_end_y, GREENMASK); }
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
    if (frame_count >= max_frames)
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

    // print something in the graphics window
    //screen->Print("hello world", 2, 2, 0xffffff);

    // print something to the text window
    //cout << "This goes to the console window." << std::endl;

    //Print frame count
    frame_count++;
    string frame_count_string = "FRAME: " + std::to_string(frame_count);
    frame_count_font->print(screen, frame_count_string.c_str(), 350, 580);
}
