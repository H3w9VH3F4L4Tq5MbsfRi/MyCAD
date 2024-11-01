#pragma once
#include "point.h"
#include "figure.h"

#include<vector>

class BicubicPatch : public Figure 
{
private:
	std::vector<Figure*> controlPoints;
	int* division;

	int cpCountLoc;
	int segmentCountLoc;
	int segmentIdxLoc;
	int divisionLoc;
	int otherAxisLoc;

	bool bspline;
	int bsplineLoc;
	int gregoryLoc;

	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> InitializeAndCalculate
	(int cpCountLoc, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, bool bspline, std::vector<Figure*> controlPoints, int* division, int gregoryLoc);
	std::tuple<std::vector<GLfloat>, std::vector<GLuint>> Calculate() const;

public:
	BicubicPatch(int cpCountLoc, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, bool bspline, std::vector<Figure*> controlPoints, int* division, int gregoryLoc);

	void Render(int colorLoc, int modelLoc, bool grayscale);
    void RenderTess(int colorLoc, int modelLoc, bool grayscale);
	bool CreateImgui() { return false; };
	bool GetBoundingSphere(CAD::Sphere& sphere) { return false; };

	std::vector<Figure*> GetControlPoints() { return controlPoints; };
	void RefreshBuffers();

	bool ReplaceControlPoint(int idx, Figure *cp);
};