#pragma once

#ifndef ONDERZOEK_NODE_H
#define ONDERZOEK_NODE_H


#include "tank.h"

class Node {

public:
    explicit Node(Tank* tank, Node* left = nullptr, Node* right = nullptr, int axis = 0);

    // Geen kopieerconstructeur en toewijzingsoperator implementeren
    Node(const Node& other) = delete;
    Node& operator=(const Node& other) = delete;

    ~Node() {
        delete left;
        delete right;
    }

    Node* left;
    Node* right;
    Tank* tank;
    int axis;
};


#endif //ONDERZOEK_NODE_H
