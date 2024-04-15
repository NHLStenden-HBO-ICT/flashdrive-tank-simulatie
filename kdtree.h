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

    void printTreeStructure(Node* root, int depth);

private:
    Tank* searchNearestTankRecursive(Node* root, Tank& targetTank, Tank& closestTank, int depth, double& closestDistance);
    static Node* buildTree(const std::vector<Tank*>& tanks, int depth);
};


