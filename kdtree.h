#pragma once


#include "Node.h"
#include <vector>


class Kdtree {

public:
    explicit Kdtree(const std::vector<Tank*>& tanks);
    ~Kdtree();
    Node* TreeRoot{};

    int dimensions = 2;

    Node* getRoot() const {
        return TreeRoot;
    }

    Tank* searchNearestTank(Tank* targetTank);

private:
    Node* getNextNode(Node* current, const Tank* targetTank, int dimension);
    Tank* searchNearestTankRecursive(Node* root, const Tank* targetTank, Tank* closestTank, int depth, float& closestDistance);
    static Node* buildTree(const std::vector<Tank*>& tanks, int depth);
};


