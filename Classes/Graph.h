#pragma once

#include <vector>
#include <figure.h>

class SurfaceC0;

class AdjecencyList {
private:
  std::vector<std::vector<int>> adjList;

public:
  AdjecencyList();

  int addVertex();
  void addEdge(int u, int v);
  std::vector<int> getNeighbours(int u);
  int getVertexCount();
};

class Graph {
private:
  std::vector<Figure*> vertices;
  AdjecencyList adjList;

  std::vector<int> getCanonicalForm(const std::vector<int> &cycle);
  void dfs(int start, int current, std::vector<int> &path,
           std::vector<bool> &visited, std::set<std::vector<int>> &uniqueCycles,
           int N);

public:
  Graph();
  Graph(SurfaceC0 &surface);
  Graph(std::vector<Graph*> &graphs);
  Graph(Graph &graph, std::vector<int> &cycleVertices);

  std::set<std::vector<int>> findCyclesWithNVertices(int N);

  friend class Polyline;
};