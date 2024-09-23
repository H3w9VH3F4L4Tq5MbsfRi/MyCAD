#include "Graph.h"
#include <iostream>
#include "surfaceC0.h"

std::vector<int> Graph::getCanonicalForm(const std::vector<int> &cycle) {
  std::vector<int> minCycle = cycle;

  std::vector<int> reversedCycle = cycle;
  std::reverse(reversedCycle.begin(), reversedCycle.end());

  for (size_t i = 0; i < cycle.size(); ++i) {
    std::vector<int> rotatedCycle(cycle.begin() + i, cycle.end());
    rotatedCycle.insert(rotatedCycle.end(), cycle.begin(), cycle.begin() + i);

    std::vector<int> rotatedReversed(reversedCycle.begin() + i,
                                     reversedCycle.end());
    rotatedReversed.insert(rotatedReversed.end(), reversedCycle.begin(),
                           reversedCycle.begin() + i);

    minCycle = std::min(minCycle, rotatedCycle);
    minCycle = std::min(minCycle, rotatedReversed);
  }

  return minCycle;
}

void Graph::dfs(int start, int current, std::vector<int> &path,
                std::vector<bool> &visited,
                std::set<std::vector<int>> &uniqueCycles, int N) {
  visited[current] = true;
  path.push_back(current);
  std::vector<EdgeStruct*> neighborsFull = adjList.getNeighbours(current);
  std::vector<int> neighbors;
  for (EdgeStruct *neighbor : neighborsFull) {
    neighbors.push_back(neighbor->v);
  }

  if (path.size() == N) {
    if (std::find(neighbors.begin(), neighbors.end(), start) !=
        neighbors.end()) {
      std::vector<int> cycle = getCanonicalForm(path);
      uniqueCycles.insert(cycle);
    }
  } else {
    for (int neighbor : neighbors) {
      if (!visited[neighbor]) {
        dfs(start, neighbor, path, visited, uniqueCycles, N);
      }
    }
  }

  path.pop_back();
  visited[current] = false;
}

Graph::Graph() {
  vertices = std::vector<Figure *>();
  adjList = AdjecencyList();
}

Graph::Graph(SurfaceC0 &surface) {
  int sizeU = surface.CalcSizeU();
  int sizeV = surface.CalcSizeV();
  bool wrappedU = surface.IsWrappedU();
  bool wrappedV = surface.IsWrappedV();

  std::vector<Figure *> cps = surface.GetControlPoints();

  // bottom row (u increasing)
  if (!wrappedV) {
    for (int i = 0; i < sizeU; i++) {
      vertices.push_back(cps[16 * i]);
      adjList.addVertex();
    }
  }
  // right column (v increasing)
  if (!wrappedU) {
    for (int i = 0; i < sizeV; i++) {
      vertices.push_back(cps[16 * sizeU * i + 16 * (sizeU - 1) + 3]);
      adjList.addVertex();
    }
  }
  // top row (u decreasing)
  if (!wrappedV) {
    for (int i = 0; i < sizeU; i++) {
      vertices.push_back(
          cps[16 * (sizeV - 1) * sizeU + 16 * (sizeU - 1 - i) + 15]);
      adjList.addVertex();
    }
  }
  // left column (v decreasing)
  if (!wrappedU) {
    for (int i = 0; i < sizeV; i++) {
      vertices.push_back(cps[16 * sizeU * (sizeV - 1 - i) + 12]);
      adjList.addVertex();
    }
  }

  bool cylinder = wrappedU xor wrappedV;
  bool rectangle = !wrappedU && !wrappedV;

  if (rectangle) {
    for (int i = 0; i < vertices.size() - 1; i++) {
      adjList.addEdge(i, i + 1, &surface);
    }
    adjList.addEdge(vertices.size() - 1, 0, &surface);
  } else if (cylinder) {
    for (int i = 0; i < (vertices.size() / 2) - 1; i++) {
      adjList.addEdge(i, i + 1, &surface);
    }
    adjList.addEdge((vertices.size() / 2) - 1, 0, &surface);
    for (int i = vertices.size() / 2; i < vertices.size() - 1; i++) {
      adjList.addEdge(i, i + 1, &surface);
    }
    adjList.addEdge(vertices.size() - 1, vertices.size() / 2, &surface);
  }
}

Graph::Graph(std::vector<Graph *> &graphs) {
  if (graphs.size() == 0)
    return;

  this->vertices = graphs[0]->vertices;
  this->adjList = graphs[0]->adjList;

  for (int i = 1; i < graphs.size(); i++) {
    std::vector<int> newIndices = {};

    for (int j = 0; j < graphs[i]->vertices.size(); j++) {
      bool found = false;
      for (int k = 0; k < this->vertices.size(); k++) {
        if (graphs[i]->vertices[j] == this->vertices[k]) {
          found = true;
          newIndices.push_back(k);
          break;
        }
      }
      if (!found) {
        newIndices.push_back(this->vertices.size());
        this->vertices.push_back(graphs[i]->vertices[j]);
        this->adjList.addVertex();
      } else {
        //std::cout << "Duplicate found." << std::endl;
      }
    }

    for (int j = 0; j < graphs[i]->adjList.getVertexCount(); j++) {
      for (int k = 0; k < graphs[i]->adjList.getNeighbours(j).size(); k++) {
        this->adjList.addEdge(
            newIndices[j],
            newIndices[graphs[i]->adjList.getNeighbours(j)[k]->v],
            graphs[i]->adjList.getNeighbours(j)[k]->baseSurface);
      }
    }
  }
}

Graph::Graph(Graph &graph, std::vector<int> &cycleVertices) {
  for (int i = 0; i < cycleVertices.size(); i++) {
    vertices.push_back(graph.vertices[cycleVertices[i]]);
    adjList.addVertex();
  }
  for (int i = 1; i < cycleVertices.size(); i++) {
    std::vector<EdgeStruct *> neighbours =
        graph.adjList.getNeighbours(cycleVertices[i - 1]);
    for (int j = 0; j < neighbours.size(); j++) {
      if (neighbours[j]->v == cycleVertices[i]) {
        adjList.addEdge(i - 1, i, neighbours[j]->baseSurface);
        break;
      }
    }
  }

  std::vector<EdgeStruct *> neighbours =
      graph.adjList.getNeighbours(cycleVertices[cycleVertices.size() - 1]);
  for (int i = 0; i < neighbours.size(); i++) {
    if (neighbours[i]->v == cycleVertices[0]) {
      adjList.addEdge(cycleVertices.size() - 1, 0, neighbours[i]->baseSurface);
      break;
    }
  }
}

std::set<std::vector<int>> Graph::findCyclesWithNVertices(int N) {
  std::set<std::vector<int>> uniqueCycles;
  int vertexCount = adjList.getVertexCount();

  for (int i = 0; i < vertexCount; ++i) {
    std::vector<bool> visited(vertexCount, false);
    std::vector<int> path;
    dfs(i, i, path, visited, uniqueCycles, N);
  }

  return uniqueCycles;
}

AdjecencyList::AdjecencyList() : adjList() {}

int AdjecencyList::addVertex() {
  adjList.push_back({});
  return 0;
}

void AdjecencyList::addEdge(int u, int v, SurfaceC0* surf) {
  adjList[u].push_back(new EdgeStruct(v,surf));
  adjList[v].push_back(new EdgeStruct(u,surf));
}

std::vector<EdgeStruct*> AdjecencyList::getNeighbours(int u) {
  return adjList[u];
}

int AdjecencyList::getVertexCount() { return adjList.size(); }
