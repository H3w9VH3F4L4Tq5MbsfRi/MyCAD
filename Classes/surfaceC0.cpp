#include "SurfaceC0.h"
#include "point.h"

std::vector<Figure*> SurfaceC0::CalculatePlane(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int xSegments, int zSegments, float length, float width)
{
	float patchLength = length / xSegments;
	float patchLengthStep = patchLength / 3.f;
	float patchWidth = width / zSegments;
	float patchWidthStep = patchWidth / 3.f;

	glm::vec3 position = this->GetPosition();
	std::vector<Figure*> newPoints = std::vector<Figure*>();

	for (int i = 0; i < zSegments; i++) {
		for (int j = 0; j < xSegments; j++) {
			bool noBottom = (i == 0);
			bool noLeft = (j == 0);

			std::vector<Figure*> cps = std::vector<Figure*>();
			for (int k = 0; k < 16; k++) {
				if (!noBottom && glm::floor(k / 4) == 0) {
					cps.push_back((patches[(i - 1) * xSegments + j]->GetControlPoints())[k + 12]);
					continue;
				}
				if (!noLeft && k % 4 == 0) {
					cps.push_back((patches[i * xSegments + (j - 1)]->GetControlPoints())[k + 3]);
					continue;
				}

				float x = position.x + patchLength * (j - xSegments / 2.f) + (k % 4) * patchLengthStep;
				float z = position.z + patchWidth * (i - zSegments / 2.f) + glm::floor(k / 4) * patchWidthStep;
				Point* p = new Point(glm::vec3(x, position.y, z), 0.02F);
				cps.push_back(p);
				newPoints.push_back(p);
			}
            this->patches.push_back(new BicubicPatch(
                cpCount, segmentCountLoc, segmentIdxLoc,
                divisionLoc, otherAxisLoc, bsplineLoc, false, cps,
                &this->division));
		}
	}
    this->ambit = new Graph(*this);
	return newPoints;
}

std::vector<Figure*> SurfaceC0::CalculateCylinder(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int otherAxisLoc, int bsplineLoc, int xSegments, int zSegments, float radius, float height) 
{
	float patchRadius = 2 * M_PI / xSegments;
	float patchRadiusStep = patchRadius / 3.f;
	float patchHeight = height / zSegments;
	float patchHeightStep = patchHeight / 3.f;

	glm::vec3 position = this->GetPosition();
	std::vector<Figure*> newPoints = std::vector<Figure*>();

	std::vector<glm::vec2> xzPositions = std::vector<glm::vec2>();
	for (int i = 0; i < xSegments * 3; i++) {
		float x = position.x + radius * glm::cos(i * patchRadiusStep);
		float z = position.z + radius * glm::sin(i * patchRadiusStep);
		xzPositions.push_back(glm::vec2(x, z));
	}

	for (int i = 0; i < zSegments; i++) {
		for (int j = 0; j < xSegments; j++) {
			bool noBottom = (i == 0);
			bool noLeft = (j == 0);

			std::vector<Figure*> cps = std::vector<Figure*>();
			for (int k = 0; k < 16; k++) {
				if (!noBottom && glm::floor(k / 4) == 0) {
					cps.push_back((patches[(i - 1) * xSegments + j]->GetControlPoints())[k + 12]);
					continue;
				}
				if (!noLeft && k % 4 == 0) {
					cps.push_back((patches[i * xSegments + (j - 1)]->GetControlPoints())[k + 3]);
					continue;
				}
				if (j * 3 + k % 4 == xzPositions.size()) {
					if (j == 0)
						cps.push_back(cps[k - 3]);
					else
						cps.push_back((patches[i * xSegments]->GetControlPoints())[k - 3]);
					continue;
				}

				float x = xzPositions[j * 3 + k % 4].x;
				float y = position.y + patchHeight * (i - zSegments / 2.f) + glm::floor(k / 4) * patchHeightStep;
				float z = xzPositions[j * 3 + k % 4].y;
				Point* p = new Point(glm::vec3(x, y, z), 0.02F);
				cps.push_back(p);
				newPoints.push_back(p);
			}
            this->patches.push_back(new BicubicPatch(
                cpCount, segmentCountLoc, segmentIdxLoc,
                divisionLoc, otherAxisLoc, bsplineLoc, false, cps,
                &this->division));
		}
	}
    this->ambit = new Graph(*this);
	return newPoints;
}

int SurfaceC0::Serialize(MG1::Scene &scene, std::vector<uint32_t> cpsIdxs) {
  MG1::BezierSurfaceC0 s;
  s.name = name;
  s.uWrapped = this->IsWrappedU();
  s.vWrapped = this->IsWrappedV();
  s.size.x = this->CalcSizeU();
  s.size.y = this->CalcSizeV();

  for (int i = 0; i < patches.size(); i++) 
  {
	std::vector<uint32_t> cpsIdxsPatch(cpsIdxs.begin() + i * 16,
                                       cpsIdxs.begin() + i * 16 + 16);
    MG1::BezierPatchC0 p;
    p.samples.x = division;
    p.samples.y = division;
    for (int j = 0; j < cpsIdxsPatch.size(); j++) 
		p.controlPoints.push_back(cpsIdxsPatch[j]);
;
   s.patches.push_back(p);
  }
  scene.surfacesC0.push_back(s);
  return -1;
}

void SurfaceC0::CreateFromControlPoints(int cpCount, int segmentCountLoc,
                                        int segmentIdxLoc, int divisionLoc,
                                        int otherAxisLoc, int bsplineLoc,
                                        std::vector<Figure*> cps) 
{
  if (cps.size() % 16 != 0) return;

  int patchCount = cps.size() / 16;
  for (int i = 0; i < patchCount; i++) {
    std::vector<Figure *> cpsPatch(cps.begin() + i * 16,
                                   cps.begin() + i * 16 + 16);
    this->patches.push_back(new BicubicPatch(cpCount, segmentCountLoc, segmentIdxLoc,
                              divisionLoc, otherAxisLoc, bsplineLoc, false,
                              cpsPatch, &this->division));
  }
  this->ambit = new Graph(*this);
}

int SurfaceC0::CalcSize(int i, int j) 
{ 
if (patches.size() == 0)
    return 0;

  int counter = 1;
  Figure *cpToCheck = patches[0]->GetControlPoints()[i];

  for (int k = 1; k < patches.size(); k++) {
    Figure *currentCp = patches[k]->GetControlPoints()[j];
    if (cpToCheck == currentCp) {
      counter++;
      cpToCheck = patches[k]->GetControlPoints()[i];
    }
  }

  return counter;
}

bool SurfaceC0::CheckWrappedU(int i, int j) 
{
  if (patches.size() == 0)
    return false;

  return patches[0]->GetControlPoints()[j] ==
         patches[this->CalcSizeU() - 1]->GetControlPoints()[i];
}

bool SurfaceC0::CheckWrappedV(int i, int j) {
  if (patches.size() == 0)
    return false;

  return patches[0]->GetControlPoints()[j] ==
         patches[this->CalcSizeU() * (this->CalcSizeV() - 1)]
             ->GetControlPoints()[i];
}

int SurfaceC0::CalcSizeU() { return CalcSize(3, 0); }

int SurfaceC0::CalcSizeV() { return CalcSize(12, 0); }

bool SurfaceC0::IsWrappedU() { return CheckWrappedU(3, 0); }

bool SurfaceC0::IsWrappedV() { return CheckWrappedV(12, 0); }

bool SurfaceC0::ReplaceControlPoint(int idx, Figure *cp) { 
  if (idx >= patches.size() * 16)
    return false;

  int patchIdx = idx / 16;
  int cpIdx = idx % 16;
  bool result = patches[patchIdx]->ReplaceControlPoint(cpIdx, cp);
  RefreshBuffers();
  return result;
}

SurfaceC0::SurfaceC0(glm::vec3 position, std::string name)
	: Figure(std::make_tuple(std::vector<GLfloat>(), std::vector<GLuint>()), name, position, true) {}

void SurfaceC0::RenderTess(int colorLoc, int modelLoc, bool grayscale)
{
	for (int i = 0; i < patches.size(); i++) {
		patches[i]->selected = selected;
		patches[i]->RenderTess(colorLoc, modelLoc, grayscale);
	}
}

bool SurfaceC0::CreateImgui()
{
	bool change = false;

	int prevValue = division;
	if (ImGui::InputInt("Division", &division)) {
		if (division < 2) {
			division = 2;
		}
		change = division != prevValue;
	}

	if (ImGui::Checkbox("Show mesh", &showMesh)) {
		change = true;
	}

	return change;
}

std::vector<Figure*> SurfaceC0::GetControlPoints()
{
	std::vector<Figure*> cps = std::vector<Figure*>();
	for (int i = 0; i < patches.size(); i++) {
		std::vector<Figure*> tempCps = patches[i]->GetControlPoints();
		for (int j = 0; j < tempCps.size(); j++) {
			cps.push_back(tempCps[j]);
		}
	}
	return cps;
}

void SurfaceC0::RefreshBuffers()
{
	for (int i = 0; i < patches.size(); i++) {
		patches[i]->RefreshBuffers();
	}
    this->ambit = new Graph(*this);
}

void SurfaceC0::Render(int colorLoc, int modelLoc, bool grayscale)
{
	if (!showMesh)
		return;

	for (int i = 0; i < patches.size(); i++) {
		patches[i]->Render(colorLoc, modelLoc, grayscale);
	}
}
