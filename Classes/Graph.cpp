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
  std::vector<int> neighbors = adjList.getNeighbours(current);

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

  //std::cout << surface.name << std::endl;
  //std::cout << "size (u|v): " << sizeU << " | " << sizeV << std::endl;
  //if (!wrappedU && !wrappedV)
  //  std::cout << "type: Rectangle" << std::endl;
  //else if (wrappedU) 
  //    std::cout << "type: Cylinder U" << std::endl;
  //else if (wrappedV)
  //  std::cout << "type: Cylinder V" << std::endl;
  //else
  //  std::cout << "WTF" << std::endl;

  std::vector<Figure *> cps = surface.GetControlPoints();

  int currSize = 0;
  bool cylinder = wrappedU xor wrappedV;
  bool rectangle = !wrappedU && !wrappedV;

  //int bottomExpected = 0;
  //if (sizeU >= 1)
  //  bottomExpected += 4;
  //if (sizeU >= 2)
  //  bottomExpected += (sizeU - 1) * 3;
  //if (cylinder)
  //  bottomExpected -= 1;

  //int rightExpected = 0;
  //rightExpected += sizeV * 3;

  //int topExpected = 0;
  //topExpected += sizeU * 3;

  //int leftExpected = 0;
  //if (sizeV >= 1)
  //  leftExpected += 2;
  //if (sizeV >= 2)
  //  leftExpected += (sizeV - 1) * 3;
  //if (cylinder)
  //  leftExpected += 1;

  // bottom (u increasing)
  if (!wrappedV) {
    for (int i = 0; i < sizeU; i++) {
      for (int j = 0; j < 4; j++) {
        if (j == 0 && i != 0)
          continue;
        if (i == (sizeU - 1) && j == 3 && cylinder)
          continue;

        int idx = i * 16 + j;
        vertices.push_back(cps[idx]);
        adjList.addVertex();
      }
    }
    if (currSize != 0 && !cylinder)
      currSize -= 1;
    for (int i = currSize; i < vertices.size() - 1; i++) {
      adjList.addEdge(i, i + 1);
    }
    if (cylinder) {
      adjList.addEdge(vertices.size() - 1, currSize);
    }

    //std::cout << "bottom: " << vertices.size() << " / "
    //          << bottomExpected << std::endl;

    currSize = vertices.size();
  }

  if (!wrappedU) {
    // right (v increasing)
    for (int i = 0; i < sizeV; i++) {
      for (int j = 0; j < 4; j++) {
        if (j == 0)
          continue;

        int idx = (sizeU - 1) * 16 + i * sizeU * 16 + j * 4 + 3;
        vertices.push_back(cps[idx]);
        adjList.addVertex();
      }
    }
    if (currSize != 0 && !cylinder)
      currSize -= 1;
    for (int i = currSize; i < vertices.size() - 1; i++) {
      adjList.addEdge(i, i + 1);
    }
    if (cylinder) {
      adjList.addEdge(vertices.size() - 1, currSize);
    }

    //std::cout << "right: " << (vertices.size() - currSize) << " / " << rightExpected
    //          << std::endl;

    currSize = vertices.size();
  }
  
  if (!wrappedV) {
    // top (u decreasing)
    for (int i = sizeU - 1; i >= 0; i--) {
      for (int j = 3; j >= 0; j--) {
        if (j == 3)
          continue;

        int idx = (sizeV - 1) * sizeU * 16 + i * 16 + 12 + j;
        vertices.push_back(cps[idx]);
        adjList.addVertex();
      }
    }
    if (currSize != 0 && !cylinder)
      currSize -= 1;
    for (int i = currSize; i < vertices.size() - 1; i++) {
      adjList.addEdge(i, i + 1);
    }
    if (cylinder) {
      adjList.addEdge(vertices.size() - 1, currSize);
    }

    //std::cout << "top: " << (vertices.size() - currSize) << " / "
    //          << topExpected << std::endl;

    currSize = vertices.size();
  }

  if (!wrappedU) {
    // left (v decreasing)
    for (int i = sizeV - 1; i >= 0; i--) {
      for (int j = 3; j >= 0; j--) {
        if (j == 3)
          continue;
        if (j == 0 && i == 0 && !cylinder)
          continue;

        int idx = i * sizeU * 16 + j * 4;
        vertices.push_back(cps[idx]);
        adjList.addVertex();
      }
    }
    if (currSize != 0 && !cylinder)
      currSize -= 1;
    for (int i = currSize; i < vertices.size() - 1; i++) {
      adjList.addEdge(i, i + 1);
    }
    if (cylinder) {
      adjList.addEdge(vertices.size() - 1, currSize);
    }

    //std::cout << "left: " << (vertices.size() - currSize) << " / "
    //          << leftExpected << std::endl;

    // currSize = vertices.size();
  }
  if (rectangle) {
    adjList.addEdge(vertices.size() - 1, 0);
  }

  //std::cout << std::endl;
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
            newIndices[j], newIndices[graphs[i]->adjList.getNeighbours(j)[k]]);
      }
    }
  }
}

Graph::Graph(Graph &graph, std::vector<int> &cycleVertices) {
  for (int i = 0; i < cycleVertices.size(); i++) {
    vertices.push_back(graph.vertices[cycleVertices[i]]);
    adjList.addVertex();
  }
  for (int i = 1; i < this->vertices.size(); i++) {
    adjList.addEdge(i - 1, i);
  }
  adjList.addEdge(this->vertices.size() - 1, 0);
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

void AdjecencyList::addEdge(int u, int v) {
  adjList[u].push_back(v);
  adjList[v].push_back(u);
}

std::vector<int> AdjecencyList::getNeighbours(int u) { return adjList[u]; }

int AdjecencyList::getVertexCount() { return adjList.size(); }
