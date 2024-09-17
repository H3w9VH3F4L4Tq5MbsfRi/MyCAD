#include "Graph.h"
Graph::Graph(SurfaceC0 &surface) 
{ 
  int sizeU = surface.CalcSizeU();
  int sizeV = surface.CalcSizeV();
  bool wrappedU = surface.IsWrappedU();
  bool wrappedV = surface.IsWrappedV();

  std::vector<Figure *> cps = surface.GetControlPoints();

  int currSize = 0;

  // bottom (u increasing)
  if (!wrappedV) {
    for (int i = 0; i < sizeU; i++) {
      for (int j = 0; j < 4; j++) {
        if (j == 0 && i != 0)
          continue;

        int idx = i * 16 + j;
        vertices.push_back(cps[idx]);
        adjList.push_back({});
      }
    }
    for (int i = 1; i < vertices.size(); i++) {
      adjList[i - 1].push_back(i);
      adjList[i].push_back(i - 1);
    }
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
        adjList.push_back({});
      }
    }
    for (int i = currSize; i < vertices.size(); i++) {
      adjList[i - 1].push_back(i);
      adjList[i].push_back(i - 1);
    }
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
        adjList.push_back({});
      }
    }
    for (int i = currSize; i < vertices.size(); i++) {
      adjList[i - 1].push_back(i);
      adjList[i].push_back(i - 1);
    }
    currSize = vertices.size();
  }

  if (!wrappedU) {
    // left (v decreasing)
    for (int i = sizeV - 1; i >= 0; i--) {
      for (int j = 3; j >= 0; j--) {
        if (j == 3)
          continue;
        if (j == 0 && i == 0)
          continue;

        int idx = i * sizeU * 16 + j * 4;
        vertices.push_back(cps[idx]);
        adjList.push_back({});
      }
    }
    for (int i = currSize; i < vertices.size(); i++) {
      adjList[i - 1].push_back(i);
      adjList[i].push_back(i - 1);
    }
    // currSize = vertices.size();
  }

  adjList[vertices.size() - 1].push_back(0);
  adjList[0].push_back(vertices.size() - 1);
}