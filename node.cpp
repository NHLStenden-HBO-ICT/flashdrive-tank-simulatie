#include "precomp.h"
#include <iostream>

Node::Node(Tank* tank, Node* left, Node* right, int axis) : tank(tank), left(left), right(right), axis(axis) {}
