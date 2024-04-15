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
    std::cout << "Kdtree destroyed\n" << std::endl;
}


Kdtree::Kdtree(const std::vector<Tank*>& tanks) {
    TreeRoot = buildTree(tanks, 0);
    printTreeStructure(TreeRoot, 0);
}

Node* Kdtree::buildTree(const std::vector<Tank*>& tanks, int depth) {
    if (tanks.empty()) {
        return nullptr;
    }

    int axis = depth % 2;

    // Sorteer de punten langs de huidige as
    std::vector<Tank*> sorted_tanks = tanks;
    std::sort(sorted_tanks.begin(), sorted_tanks.end(), [axis](const Tank* tank1, const Tank* tank2) {
        return (axis == 0) ? tank1->get_position().x < tank2->get_position().x : tank1->get_position().y < tank2->get_position().y;
        });

    // Bepaal de median index
    int medianIndex = sorted_tanks.size() / 2;

    // Maak een nieuwe knoop aan met het punt in het midden
    Node* newNode = new Node(sorted_tanks[medianIndex]);

    // Bouw recursief de linker- en rechter subtrees
    newNode->left = buildTree(std::vector<Tank*>(sorted_tanks.begin(), sorted_tanks.begin() + medianIndex), depth + 1);
    newNode->right = buildTree(std::vector<Tank*>(sorted_tanks.begin() + medianIndex + 1, sorted_tanks.end()), depth + 1);

    return newNode;
}




// Searches a Point represented by "targetPoint[]" in the K D tree.
// The parameter depth is used to determine current axis.
Tank* Kdtree::searchNearestTankRecursive(Node* root, Tank& targetTank, Tank& closestTank, int depth, double& closestDistance)
{
    if (root == nullptr)
        return &closestTank;

    const int dimension = depth % dimension;

    // Update het dichtstbijzijnde punt en de dichtstbijzijnde afstand.
    double dx = root->tank->get_position().x  - targetTank.get_position().x;
    double dy = root->tank->get_position().y - targetTank.get_position().y;
    double distance = sqrt(dx * dx + dy * dy);

    if (distance < closestDistance) {
        closestDistance = distance;
        closestTank = *root->tank;
    }

    bool nodeIsLeaf = root->left == nullptr && root->right == nullptr;
    if (nodeIsLeaf) { return &closestTank; }

    Node* nextNodeToVisit = (dimension == 0) ? (targetTank.get_position().x > root->tank->get_position().x ? root->right : root->left)
        : (targetTank.get_position().y > root->tank->get_position().y ? root->right : root->left);

    return searchNearestTankRecursive(nextNodeToVisit, targetTank, closestTank, depth + 1, closestDistance);
}


// Searches a Point in the K D tree.
Tank* Kdtree::searchNearestTank(Tank* targetTank)
{
    double closestDistance = std::numeric_limits<double>::max();
    double maxDistance = std::numeric_limits<double>::max();
    Tank* closestTank = nullptr; // TODO Tank meegeven?
    // Pass current depth as 0
    return searchNearestTankRecursive(TreeRoot, *targetTank, *closestTank, 0, closestDistance);
}

void Kdtree::printTreeStructure(Node* root, int depth) {
    if (root == nullptr) {
        return;
    }

    // Recursief afdrukken van de rechter subtree met toename van diepte
    printTreeStructure(root->right, depth + 1);

    // Afdrukken van de huidige knoop (tank) met indentatie op basis van diepte
    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }
    std::cout << "|-- (" << root->tank->get_position().x << ", " << root->tank->get_position().y << ")" << std::endl;

    // Recursief afdrukken van de linker subtree met toename van diepte
    printTreeStructure(root->left, depth + 1);
}



