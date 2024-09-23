#pragma once

#include <vector>
#include <figure.h>

class SurfaceC0;

struct EdgeStruct {
  int v;
  SurfaceC0 *baseSurface;

  EdgeStruct(int v, SurfaceC0 *baseSurface) {
    this->v = v;
    this->baseSurface = baseSurface;
  }
};

class AdjecencyList {
private:
  std::vector<std::vector<EdgeStruct*>> adjList;

public:
  AdjecencyList();

  int addVertex();
  void addEdge(int u, int v, SurfaceC0 *surf);
  std::vector<EdgeStruct*> getNeighbours(int u);
  int getVertexCount();
};

class Graph {
private:
  std::vector<int> getCanonicalForm(const std::vector<int> &cycle);
  void dfs(int start, int current, std::vector<int> &path,
           std::vector<bool> &visited, std::set<std::vector<int>> &uniqueCycles,
           int N);

public:
  std::vector<Figure*> vertices;
  AdjecencyList adjList;

  Graph();
  Graph(SurfaceC0 &surface);
  Graph(std::vector<Graph*> &graphs);
  Graph(Graph &graph, std::vector<int> &cycleVertices);

  std::set<std::vector<int>> findCyclesWithNVertices(int N);

  friend class Polyline;
};