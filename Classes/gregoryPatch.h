#pragma once
#include "figure.h"

#include <vector>

class GregoryPatch : public Figure {
private:
  std::vector<Figure *> controlPoints;
  int division = 4;
  bool showMesh = false;

  std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
  InitializeAndCalculate(std::vector<Figure *> controlPoints);
  std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;

public:
  GregoryPatch(std::vector<Figure *> controlPoints);

  void Render(int colorLoc, int modelLoc, bool grayscale);
  void RenderTess(int colorLoc, int modelLoc, bool grayscale);
  bool CreateImgui();
  bool GetBoundingSphere(CAD::Sphere &sphere) { return false; };
  std::vector<Figure *> GetControlPoints() { return controlPoints; };
  void RefreshBuffers();
  bool ReplaceControlPoint(int idx, Figure *cp);
};