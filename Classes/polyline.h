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
  int counter = 0;
  const int step = 250;
  const float minIntensity = 0.1f;
  const glm::vec4 color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

  std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate(Graph *graph) const;
  glm::vec4 GetAnimatedColor(bool grayscale);
};