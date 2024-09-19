#pragma once

#include <vector>
#include <figure.h>

class SurfaceC0;

class Graph {
private:
  std::vector<Figure*> vertices;
  std::vector<std::vector<int>> adjList;

public:
  Graph();
  Graph(SurfaceC0 &surface);
  Graph(std::vector<Graph*> &graphs);
};