#pragma once
#include "figure.h"
#include "Graph.h"

class Polyline : public Figure {
public:
  Polyline();
  Polyline(Graph *graph);

  void Render(int colorLoc, int modelLoc, bool grayscale = false);
  bool GetBoundingSphere(CAD::Sphere &sphere) { return false; };

private:
  std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate(Graph *graph) const;
};