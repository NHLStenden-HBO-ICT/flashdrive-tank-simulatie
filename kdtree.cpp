#include "precomp.h"

Kdtree::~Kdtree() {
	delete TreeRoot;
    TreeRoot = nullptr;
}


Kdtree::Kdtree(const std::vector<Tank*>& tanks) {
    TreeRoot = buildTree(tanks, 0);
}

/// <summary>
/// Build the KD-tree
/// </summary>
/// <param name="tanks">The tanks to sort into the tree</param>
/// <param name="depth">The depth of the tree</param>
/// <returns></returns>
Node* Kdtree::buildTree(const std::vector<Tank*>& tanks, int depth) {
    if (tanks.empty()) {
        return nullptr;
    }

    int axis = depth % 2;

    std::vector<Tank*> sorted_tanks = tanks;

    int medianIndex = 0;

    if (tanks.size() > 1) {
        std::sort(sorted_tanks.begin(), sorted_tanks.end(), [axis](const Tank* tank1, const Tank* tank2) {
            return (axis == 0) ? tank1->get_position().x < tank2->get_position().x : tank1->get_position().y < tank2->get_position().y;
            });
        medianIndex = sorted_tanks.size() / 2;

    }
    
    Node* newNode = new Node(sorted_tanks[medianIndex]);

    if (medianIndex == 1)
    {
        newNode->tank = tanks.at(0);
        return newNode;
    }

    newNode->left = buildTree(std::vector<Tank*>(sorted_tanks.begin(), sorted_tanks.begin() + medianIndex), depth + 1);
    newNode->right = buildTree(std::vector<Tank*>(sorted_tanks.begin() + medianIndex + 1, sorted_tanks.end()), depth + 1);

    return newNode;
}

/// <summary>
/// Search the KD-tree for the nearest tank
/// </summary>
/// <param name="root">The root of the KD-tree</param>
/// <param name="targetTank">The tank to search for</param>
/// <param name="closestTank">The current nearest tank</param>
/// <param name="depth">The current depth of the KD-tree</param>
/// <param name="closestDistance">The current closest distance</param>
/// <returns>The nearest tank</returns>
Tank* Kdtree::searchNearestTankRecursive(Node* root, const Tank* targetTank, Tank* closestTank, int depth, float& closestDistance) 
{
    if (root == nullptr) {
        return nullptr;
    }

    const int dimension = depth % dimensions;

    if (root->tank != nullptr && root->tank->get_position() != targetTank->get_position())
    {
        float dx = root->tank->get_position().x - targetTank->get_position().x;
        float dy = root->tank->get_position().y - targetTank->get_position().y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < closestDistance) {
            closestDistance = distance;
            closestTank = root->tank;
        }
    }
    
    bool nodeIsLeaf = root->left == nullptr && root->right == nullptr;
    if (nodeIsLeaf && root->tank != nullptr) {
        return closestTank;
    }
        
    Node* nextNodeToVisit = getNextNode(root, targetTank, dimension);

    return searchNearestTankRecursive(nextNodeToVisit, targetTank, closestTank, depth + 1, closestDistance);
}

/// <summary>
/// Get the next node to visit
/// </summary>
/// <param name="current">The current node</param>
/// <param name="targetTank">The tank to search for</param>
/// <param name="dimension">The dimension to search in</param>
/// <returns>The next node</returns>
Node* Kdtree::getNextNode(Node* current, const Tank* targetTank, int dimension) 
{
    int x_dimension = 0;

    if (dimension == x_dimension) {
        return (targetTank -> get_position().x > current->tank->get_position().x) ? current->right : current->left;
    }
    
    return (targetTank -> get_position().y > current->tank->get_position().y) ? current->right : current->left;
}

/// <summary>
/// Search the KD-tree for the nearest tank
/// </summary>
/// <param name="targetTank">The tank to search for</param>
/// <returns>The nearest tank</returns>
Tank* Kdtree::searchNearestTank(Tank* targetTank)
{
    float closestDistance = std::numeric_limits<float>::max();
    float constexpr maxDistance = std::numeric_limits<float>::max();
    Tank* closestTank = nullptr; 
    return searchNearestTankRecursive(TreeRoot, targetTank, closestTank, 0, closestDistance);
    delete closestTank;

}



