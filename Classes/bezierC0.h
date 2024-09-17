#pragma once
#include "point.h"
#include "figure.h"

#include<vector>

class BezierC0 : public Figure
{
private:
  int cpCountLoc;
  int segmentCountLoc;
  int segmentIdxLoc;
  int divisionLoc;

  std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
  InitializeAndCalculate(int cpCountLoc, int segmentCountLoc,
                         int segmentIdxLoc, int divisionLoc);

protected:
  std::vector<Figure *> controlPoints;

  std::tuple<std::vector<GLfloat>, std::vector<GLuint>> virtual Calculate() const;

public:
  BezierC0(int cpCountLoc, int segmentCountLoc, int segmentIdxLoc, int divisionLoc,
           const char *name = "Bezier C0", bool numerate = true);

  void virtual Render(int colorLoc, int modelLoc, bool grayscale);
  bool virtual CreateImgui();
  bool GetBoundingSphere(CAD::Sphere &sphere) { return false; };

  std::vector<Figure*> GetControlPoints() { return controlPoints; };
  void AddControlPoint(Figure* cp);
  bool RemoveControlPoint(int idx);
  bool ReplaceControlPoint(int idx, Figure *cp);
  void RefreshBuffers();
  void virtual RenderPolyline(int colorLoc, int modelLoc, bool grayscale);
  void ClearControlPoints();
  void virtual CreateBsplineImgui() { ImGui::Text("That ain't Bezier C2"); };
  int Serialize(MG1::Scene &scene, std::vector<uint32_t> cpsIdxs) override;
};