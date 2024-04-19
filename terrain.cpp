#include "precomp.h"
#include "terrain.h"

namespace fs = std::filesystem;
namespace Tmpl8
{
    Terrain::Terrain()
    {
        //Load in terrain sprites
        grass_img = std::make_unique<Surface>("assets/tile_grass.png");
        forest_img = std::make_unique<Surface>("assets/tile_forest.png");
        rocks_img = std::make_unique<Surface>("assets/tile_rocks.png");
        mountains_img = std::make_unique<Surface>("assets/tile_mountains.png");
        water_img = std::make_unique<Surface>("assets/tile_water.png");


        tile_grass = std::make_unique<Sprite>(grass_img.get(), 1);
        tile_forest = std::make_unique<Sprite>(forest_img.get(), 1);
        tile_rocks = std::make_unique<Sprite>(rocks_img.get(), 1);
        tile_water = std::make_unique<Sprite>(water_img.get(), 1);
        tile_mountains = std::make_unique<Sprite>(mountains_img.get(), 1);


        //Load terrain layout file and fill grid based on tiletypes
        fs::path terrain_file_path{ "assets/terrain.txt" };
        std::ifstream terrain_file(terrain_file_path);

        if (!terrain_file.is_open()) {
            std::cout << "Could not open terrain file! Is the path correct? Defaulting to grass.." << std::endl;
            std::cout << "Path was: " << terrain_file_path << std::endl;
        }

        std::string terrain_line;
        std::getline(terrain_file, terrain_line);
        std::istringstream lineStream(terrain_line);

        int rows;

        lineStream >> rows;

        for (size_t row = 0; row < rows; row++)
        {
            std::getline(terrain_file, terrain_line);

            for (size_t collumn = 0; collumn < terrain_line.size(); collumn++)
            {
                switch (std::toupper(terrain_line.at(collumn)))
                {
                case 'G':
                    tiles.at(row).at(collumn).tile_type = TileType::GRASS;
                    break;
                case 'F':
                    tiles.at(row).at(collumn).tile_type = TileType::FORREST;
                    break;
                case 'R':
                    tiles.at(row).at(collumn).tile_type = TileType::ROCKS;
                    break;
                case 'M':
                    tiles.at(row).at(collumn).tile_type = TileType::MOUNTAINS;
                    break;
                case 'W':
                    tiles.at(row).at(collumn).tile_type = TileType::WATER;
                    break;
                default:
                    tiles.at(row).at(collumn).tile_type = TileType::GRASS;
                    break;
                }
            }
        }

        //Instantiate tiles for path planning
        for (size_t y = 0; y < tiles.size(); y++)
        {
            for (size_t x = 0; x < tiles.at(y).size(); x++)
            {
                tiles.at(y).at(x).position_x = x;
                tiles.at(y).at(x).position_y = y;

                if (is_accessible(y, x + 1)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y).at(x + 1)); }
                if (is_accessible(y, x - 1)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y).at(x - 1)); }
                if (is_accessible(y + 1, x)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y + 1).at(x)); }
                if (is_accessible(y - 1, x)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y - 1).at(x)); }
            }
        }
    }

    
    void Terrain::update()
    {
        //Pretend there is animation code here.. next year :)
    }

    void Terrain::draw(Surface* target) const
    {

        for (size_t y = 0; y < tiles.size(); y++)
        {
            for (size_t x = 0; x < tiles.at(y).size(); x++)
            {
                int posX = (x * sprite_size) + HEALTHBAR_OFFSET;
                int posY = y * sprite_size;

                switch (tiles.at(y).at(x).tile_type)
                {
                case TileType::GRASS:
                    tile_grass->draw(target, posX, posY);
                    break;
                case TileType::FORREST:
                    tile_forest->draw(target, posX, posY);
                    break;
                case TileType::ROCKS:
                    tile_rocks->draw(target, posX, posY);
                    break;
                case TileType::MOUNTAINS:
                    tile_mountains->draw(target, posX, posY);
                    break;
                case TileType::WATER:
                    tile_water->draw(target, posX, posY);
                    break;
                default:
                    tile_grass->draw(target, posX, posY);
                    break;
                }
            }
        }
    }

    /**
 * Calculate a route from the current position of the tank to the specified target.
 *
 * @param tank The tank for which the route is calculated.
 * @param target The target position where the tank needs to move.
 * @return A vector of vec2 containing the coordinates of the tiles on the route.
 *         If a valid route cannot be found, an empty vector is returned.
 */
    vector<vec2> Terrain::get_route_dijkstra(const Tank& tank, const vec2& target)
    {
        // Determine the x- and y-coordinates of the tank position and the target based on the sprite size
        const size_t pos_x = tank.position.x / sprite_size;
        const size_t pos_y = tank.position.y / sprite_size;
        const size_t target_x = target.x / sprite_size;
        const size_t target_y = target.y / sprite_size;

        // Initialize the distance matrix and the visited array
        std::vector<std::vector<int>> distances(tiles.size(), std::vector<int>(tiles[0].size(), std::numeric_limits<int>::max()));
        std::vector<std::vector<TerrainTile*>> previous(tiles.size(), std::vector<TerrainTile*>(tiles[0].size(), nullptr));
        std::vector<std::vector<bool>> visited(tiles.size(), std::vector<bool>(tiles[0].size(), false));

        // Set the distance of the starting tile to 0
        distances[pos_y][pos_x] = 0;

        // Create a priority queue with distance as the sorting criterion
        std::priority_queue<std::pair<int, TerrainTile*>, std::vector<std::pair<int, TerrainTile*>>, std::greater<>> prio_queue;
        prio_queue.emplace(0, &tiles[pos_y][pos_x]);

        // Perform Dijkstra's algorithm to find the shortest route
        while (!prio_queue.empty()) {
            TerrainTile* current_tile = prio_queue.top().second;
            prio_queue.pop();

            // If the current tile is the target, stop searching
            if (current_tile->position_x == target_x && current_tile->position_y == target_y)
                break;

            // If the current tile has already been visited, continue with the next iteration
            if (visited[current_tile->position_y][current_tile->position_x])
                continue;

            // Mark the current tile as visited
            visited[current_tile->position_y][current_tile->position_x] = true;

            // Loop through the exits of the current tile and update the distances
            for (TerrainTile* exit : current_tile->exits) {
                int exit_x = exit->position_x;
                int exit_y = exit->position_y;

                int new_distance = distances[current_tile->position_y][current_tile->position_x] + 1;

                if (new_distance < distances[exit_y][exit_x]) {
                    distances[exit_y][exit_x] = new_distance;
                    previous[exit_y][exit_x] = current_tile;
                    prio_queue.emplace(new_distance, exit);
                }
            }
        }

        std::vector<vec2> route;
        bool no_route_found = previous[target_y][target_x] == nullptr;

        if (no_route_found) {
            return route;
        }

        TerrainTile* current_tile = &tiles[target_y][target_x];

        while (current_tile != nullptr) {
            // Add the current tile position to the route
            float x = static_cast<float>(current_tile->position_x) * sprite_size;
            float y = static_cast<float>(current_tile->position_y) * sprite_size;
            route.push_back(vec2(x, y));

            // Move to the next tile in the route
            current_tile = previous[current_tile->position_y][current_tile->position_x];
        }

        std::reverse(route.begin(), route.end());
        return route;
    }


    //TODO: Function not used and take speed into account next year :)
    float Terrain::get_speed_modifier(const vec2& position) const
    {
        const size_t pos_x = position.x / sprite_size;
        const size_t pos_y = position.y / sprite_size;

        switch (tiles.at(pos_y).at(pos_x).tile_type)
        {
        case TileType::GRASS:
            return 1.0f;
            break;
        case TileType::FORREST:
            return 0.5f;
            break;
        case TileType::ROCKS:
            return 0.75f;
            break;
        case TileType::MOUNTAINS:
            return 0.0f;
            break;
        case TileType::WATER:
            return 0.0f;
            break;
        default:
            return 1.0f;
            break;
        }
    }

    bool Terrain::is_accessible(int y, int x)
    {
        //Bounds check
        if ((x >= 0 && x < terrain_width) && (y >= 0 && y < terrain_height))
        {
            //Inaccessible terrain check
            if (tiles.at(y).at(x).tile_type != TileType::MOUNTAINS && tiles.at(y).at(x).tile_type != TileType::WATER)
            {
                return true;
            }
        }

        return false;
    }
}