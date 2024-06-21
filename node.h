#pragma once

class Node {

public:
    explicit Node(Tank* tank, Node* left = nullptr, Node* right = nullptr, int axis = 0);

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
