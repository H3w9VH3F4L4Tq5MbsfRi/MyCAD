#include "Graph.h"
#include <iostream>
#include "surfaceC0.h"

Graph::Graph() {
  vertices = std::vector<Figure *>();
  adjList = std::vector<std::vector<int>>();
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
        adjList.push_back({});
      }
    }
    if (currSize != 0 && !cylinder)
      currSize -= 1;
    for (int i = currSize; i < vertices.size() - 1; i++) {
      adjList[i + 1].push_back(i);
      adjList[i].push_back(i + 1);
    }
    if (cylinder) {
      adjList[vertices.size() - 1].push_back(currSize);
      adjList[currSize].push_back(vertices.size() - 1);
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
        adjList.push_back({});
      }
    }
    if (currSize != 0 && !cylinder)
      currSize -= 1;
    for (int i = currSize; i < vertices.size() - 1; i++) {
      adjList[i + 1].push_back(i);
      adjList[i].push_back(i + 1);
    }
    if (cylinder) {
      adjList[vertices.size() - 1].push_back(currSize);
      adjList[currSize].push_back(vertices.size() - 1);
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
        adjList.push_back({});
      }
    }
    if (currSize != 0 && !cylinder)
      currSize -= 1;
    for (int i = currSize; i < vertices.size() - 1; i++) {
      adjList[i + 1].push_back(i);
      adjList[i].push_back(i + 1);
    }
    if (cylinder) {
      adjList[vertices.size() - 1].push_back(currSize);
      adjList[currSize].push_back(vertices.size() - 1);
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
        adjList.push_back({});
      }
    }
    if (currSize != 0 && !cylinder)
      currSize -= 1;
    for (int i = currSize; i < vertices.size() - 1; i++) {
      adjList[i + 1].push_back(i);
      adjList[i].push_back(i + 1);
    }
    if (cylinder) {
      adjList[vertices.size() - 1].push_back(currSize);
      adjList[currSize].push_back(vertices.size() - 1);
    }

    //std::cout << "left: " << (vertices.size() - currSize) << " / "
    //          << leftExpected << std::endl;

    // currSize = vertices.size();
  }
  if (rectangle) {
    adjList[vertices.size() - 1].push_back(0);
    adjList[0].push_back(vertices.size() - 1);
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
        this->adjList.push_back({});
      } else {
        std::cout << "Duplicate found." << std::endl;
      }
    }

    for (int j = 0; j < graphs[i]->adjList.size(); j++) {
      for (int k = 0; k < graphs[i]->adjList[j].size(); k++) {
        this->adjList[newIndices[j]].push_back(
            newIndices[graphs[i]->adjList[j][k]]);
      }
    }
  }
}
