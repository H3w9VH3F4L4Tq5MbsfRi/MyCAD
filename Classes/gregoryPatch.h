#pragma once
#include "figure.h"

#include <vector>

class GregoryPatch : public Figure {
private:
  std::vector<Figure *> controlPoints;
  bool showMesh = false;
  int division = 4;

  int cpCountLoc;
  int segmentCountLoc;
  int segmentIdxLoc;
  int divisionLoc;
  int otherAxisLoc;
  int bsplineLoc;
  int gregoryLoc;

  std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
  InitializeAndCalculate(std::vector<Figure*> controlPoints, int cpCountLoc, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int gregoryLoc);
  std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;

public:
  GregoryPatch(std::vector<Figure*> controlPoints, int cpCountLoc, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int gregoryLoc);

  void Render(int colorLoc, int modelLoc, bool grayscale);
  void RenderTess(int colorLoc, int modelLoc, bool grayscale);
  bool CreateImgui();
  bool GetBoundingSphere(CAD::Sphere &sphere) { return false; };
  std::vector<Figure *> GetControlPoints() { return controlPoints; };
  void RefreshBuffers();
  bool ReplaceControlPoint(int idx, Figure *cp);
};