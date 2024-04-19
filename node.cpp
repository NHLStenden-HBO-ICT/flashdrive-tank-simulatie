#include "precomp.h"

//
// Created by joels on 5-3-2024.
//

#include <iostream>

Node::Node(Tank* tank, Node* left, Node* right, int axis) : tank(tank), left(left), right(right), axis(axis) {}
