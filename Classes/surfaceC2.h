#pragma once
#include "figure.h"
#include "surfaceC0.h"

#include <vector>
#include <string>

class SurfaceC2 : public SurfaceC0
{
public:
	SurfaceC2(glm::vec3 position, std::string name = "Surface C2");

	std::vector<Figure*> CalculatePlane(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int xSegments, int zSegments, float length, float width) override;
	std::vector<Figure*> CalculateCylinder(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int xSegments, int zSegments, float radius, float height) override;
    int addToMG1Scene(MG1::Scene &scene, std::vector<uint32_t> cpsIdxs) override;
};