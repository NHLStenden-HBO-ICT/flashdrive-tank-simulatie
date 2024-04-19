#include "precomp.h"
//
// Created by joels on 5-3-2024.
//

#include <algorithm>
#include <iostream>
#include <limits>
#include <valarray>


Kdtree::~Kdtree() {
	delete TreeRoot;
    TreeRoot = nullptr;
}




Kdtree::Kdtree(const std::vector<Tank*>& tanks) {
    TreeRoot = buildTree(tanks, 0);
}

Node* Kdtree::buildTree(const std::vector<Tank*>& tanks, int depth) {
    if (tanks.empty()) {
        return nullptr;
    }

    int axis = depth % 2;

    // Sorteer de punten langs de huidige as
    std::vector<Tank*> sorted_tanks = tanks;

    int medianIndex = 0;

    if (tanks.size() > 1) {
        std::sort(sorted_tanks.begin(), sorted_tanks.end(), [axis](const Tank* tank1, const Tank* tank2) {
            return (axis == 0) ? tank1->get_position().x < tank2->get_position().x : tank1->get_position().y < tank2->get_position().y;
            });
        // Bepaal de median index
        medianIndex = sorted_tanks.size() / 2;

    }
    
    // Maak een nieuwe knoop aan met het punt in het midden
    Node* newNode = new Node(sorted_tanks[medianIndex]);

    if (medianIndex == 1)
    {
        newNode->tank = tanks.at(0);
        return newNode;
    }

    // Bouw recursief de linker- en rechter subtrees
    newNode->left = buildTree(std::vector<Tank*>(sorted_tanks.begin(), sorted_tanks.begin() + medianIndex), depth + 1);
    newNode->right = buildTree(std::vector<Tank*>(sorted_tanks.begin() + medianIndex + 1, sorted_tanks.end()), depth + 1);

    return newNode;
}

// Searches a Point represented by "targetPoint[]" in the K D tree.
// The parameter depth is used to determine current axis.
Tank* Kdtree::searchNearestTankRecursive(Node* root, const Tank* targetTank, Tank* closestTank, int depth, float& closestDistance) 
{
    if (root == nullptr) {
        return nullptr;
    }

    const int dimension = depth % dimensions;

    if (root->tank != nullptr && root->tank->get_position() != targetTank->get_position())
    {
        // Update het dichtstbijzijnde punt en de dichtstbijzijnde afstand.
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

Node* Kdtree::getNextNode(Node* current, const Tank* targetTank, int dimension) 
{

    if (dimension == 0) {
        return (targetTank -> get_position().x > current->tank->get_position().x) ? current->right : current->left;
    }
    
    return (targetTank -> get_position().y > current->tank->get_position().y) ? current->right : current->left;
}

Tank* Kdtree::searchNearestTank(Tank* targetTank)
{
    float closestDistance = std::numeric_limits<float>::max();
    float constexpr maxDistance = std::numeric_limits<float>::max();
    Tank* closestTank = nullptr; 
    return searchNearestTankRecursive(TreeRoot, targetTank, closestTank, 0, closestDistance);
    delete closestTank;

}



