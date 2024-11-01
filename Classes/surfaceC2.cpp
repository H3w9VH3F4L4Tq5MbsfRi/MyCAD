#include "surfaceC2.h"
#include "point.h"

SurfaceC2::SurfaceC2(glm::vec3 position, std::string name) : SurfaceC0(position, name) {}

std::vector<Figure*> SurfaceC2::CalculatePlane(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int xSegments, int zSegments, float length, float width, int gregoryLoc)
{
	int lengthSegmentCount = xSegments + 2;
	float segmentLength = length / lengthSegmentCount;
	int widthSegmentCount = zSegments + 2;
	float segmentWidth = width / widthSegmentCount;

	glm::vec3 position = this->GetPosition();
	std::vector<Figure*> newPoints = std::vector<Figure*>();

	for (int i = 0; i < zSegments; i++) {
		for (int j = 0; j < xSegments; j++) {
			bool firstInRow = (j == 0);
			bool firstInColumn = (i == 0);

			std::vector<Figure*> cps = std::vector<Figure*>();
			for (int k = 0; k < 16; k++) {
				if (!firstInRow && k % 4 != 3) {
					cps.push_back(patches[i * xSegments + (j - 1)]->GetControlPoints()[k + 1]);
					continue;
				}
				if (!firstInColumn && glm::floor(k / 4) != 3) {
					cps.push_back(patches[(i - 1) * xSegments + j]->GetControlPoints()[k + 4]);
					continue;
				}

				float x = position.x + (j + (k % 4) - lengthSegmentCount / 2.f) * segmentLength;
				float z = position.z + (i + glm::floor(k / 4) - widthSegmentCount / 2.f) * segmentWidth;
				Point* p = new Point(glm::vec3(x, position.y, z), 0.02F);
				cps.push_back(p);
				newPoints.push_back(p);
			}
            this->patches.push_back(new BicubicPatch(
                cpCount, segmentCountLoc, segmentIdxLoc,
                divisionLoc, otherAxisLoc, bsplineLoc, true, cps,
                &this->division, gregoryLoc));
		}
	}

	return newPoints;
}

std::vector<Figure*> SurfaceC2::CalculateCylinder(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int xSegments, int zSegments, float radius, float height, int gregoryLoc)
{
	float segmentRadius = M_PI * 2.f / xSegments;
	int heightSegmentCount = zSegments + 2;
	float segmentHeight = height / heightSegmentCount;

	glm::vec3 position = this->GetPosition();
	std::vector<Figure*> newPoints = std::vector<Figure*>();

	std::vector<glm::vec2> xzPositions = std::vector<glm::vec2>();
	for (int i = 0; i < xSegments; i++) {
		float x = position.x + radius * glm::cos(i * segmentRadius);
		float z = position.z + radius * glm::sin(i * segmentRadius);
		xzPositions.push_back(glm::vec2(x, z));
	}

	for (int i = 0; i < zSegments; i++) {
		for (int j = 0; j < xSegments; j++) {
			bool firstInRow = (j == 0);
			bool firstInColumn = (i == 0);

			std::vector<Figure*> cps = std::vector<Figure*>();
			for (int k = 0; k < 16; k++) {
				int columnIdx = j + k % 4;
				if (columnIdx >= xSegments) {
					if (j == 0)
						cps.push_back(cps[k - 3]);
					else
						cps.push_back(patches[i * xSegments]->GetControlPoints()[columnIdx - xSegments + glm::floor(k / 4) * 4]);
					continue;
				}
				if (!firstInRow && k % 4 != 3) {
					cps.push_back(patches[i * xSegments + (j - 1)]->GetControlPoints()[k + 1]);
					continue;
				}
				if (!firstInColumn && glm::floor(k / 4) != 3) {
					cps.push_back(patches[(i - 1) * xSegments + j]->GetControlPoints()[k + 4]);
					continue;
				}

				float x = xzPositions[columnIdx].x;
				float y = position.y + (i + glm::floor(k / 4) - heightSegmentCount / 2.f) * segmentHeight;
				float z = xzPositions[columnIdx].y;
				Point* p = new Point(glm::vec3(x, y, z), 0.02F);
				cps.push_back(p);
				newPoints.push_back(p);
			}
            this->patches.push_back(new BicubicPatch(
                cpCount, segmentCountLoc, segmentIdxLoc,
                divisionLoc, otherAxisLoc, bsplineLoc, true, cps,
                &this->division, gregoryLoc));
		}
	}

	return newPoints;
}

int SurfaceC2::Serialize(MG1::Scene &scene, std::vector<uint32_t> cpsIdxs) {
  MG1::BezierSurfaceC2 s;
  s.name = name;
  s.uWrapped = this->IsWrappedU();
  s.vWrapped = this->IsWrappedV();
  s.size.x = this->CalcSizeU();
  s.size.y = this->CalcSizeV();

  for (int i = 0; i < patches.size(); i++) {
    std::vector<uint32_t> cpsIdxsPatch(cpsIdxs.begin() + i * 16,
                                       cpsIdxs.begin() + i * 16 + 16);
    MG1::BezierPatchC2 p;
    p.samples.x = division;
    p.samples.y = division;
    for (int j = 0; j < cpsIdxsPatch.size(); j++)
      p.controlPoints.push_back(cpsIdxsPatch[j]);
    ;
    s.patches.push_back(p);
  }
  scene.surfacesC2.push_back(s);
  return -1;
}

void SurfaceC2::CreateFromControlPoints(int cpCount, int segmentCountLoc,
                                        int segmentIdxLoc, int divisionLoc,
                                        int otherAxisLoc, int bsplineLoc,
                                        std::vector<Figure*> cps, int gregoryLoc) 
{
  if (cps.size() % 16 != 0)
    return;

  int patchCount = cps.size() / 16;
  for (int i = 0; i < patchCount; i++) {
    std::vector<Figure *> cpsPatch(cps.begin() + i * 16,
                                   cps.begin() + i * 16 + 16);
    this->patches.push_back(new BicubicPatch(
        cpCount, segmentCountLoc, segmentIdxLoc, divisionLoc, otherAxisLoc,
        bsplineLoc, true, cpsPatch, &this->division, gregoryLoc));
  }
}

int SurfaceC2::CalcSizeU() { return CalcSize(1, 0); }

int SurfaceC2::CalcSizeV() { return CalcSize(4, 0); }

bool SurfaceC2::IsWrappedU() { return CheckWrappedU(1, 0); }

bool SurfaceC2::IsWrappedV() { return CheckWrappedV(4, 0); }
