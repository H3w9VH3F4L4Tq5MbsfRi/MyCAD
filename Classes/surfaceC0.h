#pragma once
#include "Graph.h"
#include "figure.h"
#include "bicubicPatch.h"

#include <vector>
#include <string>

class SurfaceC0 : public Figure
{
protected:
	std::vector<BicubicPatch*> patches = std::vector<BicubicPatch*>();
	bool showMesh = false;
	int division = 4;

	int CalcSize(int i, int j);
    bool CheckWrappedU(int i, int j);
    bool CheckWrappedV(int i, int j);

public:
    Graph *ambit = new Graph();

	SurfaceC0(glm::vec3 position, std::string name = "Surface C0");

	void Render(int colorLoc, int modelLoc, bool grayscale);
    void RenderTess(int colorLoc, int modelLoc, bool grayscale);
	bool CreateImgui();
	bool GetBoundingSphere(CAD::Sphere& sphere) { return false; };

	std::vector<Figure*> GetControlPoints();
	void RefreshBuffers();

	std::vector<Figure*> virtual CalculatePlane(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int xSegments, int zSegments, float length, float width);
	std::vector<Figure*> virtual CalculateCylinder(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int xSegments, int zSegments, float radius, float height);

    int Serialize(MG1::Scene &scene, std::vector<uint32_t> cpsIdxs) override;
    void virtual CreateFromControlPoints(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, std::vector<Figure*> cps);

	int virtual CalcSizeU();
    int virtual CalcSizeV();

	bool virtual IsWrappedU();
    bool virtual IsWrappedV();

	bool ReplaceControlPoint(int idx, Figure *cp);
};