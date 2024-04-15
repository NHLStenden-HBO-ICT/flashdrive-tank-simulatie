#include "precomp.h"

#pragma once

#ifndef ONDERZOEK_NODE_H
#define ONDERZOEK_NODE_H


#include "tank.h"

class Node {

public:
    explicit Node(Tank* tank, Node* left = nullptr, Node* right = nullptr, int axis = 0);

    ~Node() {
        delete left;
        delete right;
        delete tank;
    }

    Node* left;
    Node* right;
    Tank* tank;
    int axis;
};


#endif //ONDERZOEK_NODE_H
