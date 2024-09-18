#pragma once

#include <vector>
#include <figure.h>
#include <surfaceC0.h>

class Graph {
private:
  std::vector<Figure*> vertices;
  std::vector<std::vector<int>> adjList;

public:
  Graph();
  Graph(SurfaceC0 &surface);
};