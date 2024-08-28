#include "patchC0.h"
#include "point.h"

std::vector<Figure*> PatchC0::CalculatePlane(int cpCount, int segmentCountLoc, int segmentIdxLoc, int divisionLoc, int xSegments, int zSegments, float length, float width)
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

			patches.push_back(new BicubicPatch(cpCount, segmentCountLoc, segmentIdxLoc, divisionLoc, cps, &this->division));
		}
	}
	return newPoints;
}

PatchC0::PatchC0(glm::vec3 position) 
	: Figure(std::make_tuple(std::vector<GLfloat>(), std::vector<GLuint>()),"Patch C0",position, true){}

void PatchC0::RenderTess(int colorLoc, int modelLoc)
{
	for (int i = 0; i < patches.size(); i++) {
		patches[i]->selected = selected;
		patches[i]->RenderTess(colorLoc, modelLoc);
	}
}

bool PatchC0::CreateImgui()
{
	bool change = false;

	int prevValue = division;
	if (ImGui::InputInt("Division", &division)) {
		if (division < 1) {
			division = 1;
		}
		change = division != prevValue;
	}

	if (ImGui::Checkbox("Bezier mesh", &showMesh)) {
		change = true;
	}

	return change;
}

std::vector<Figure*> PatchC0::GetControlPoints()
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

void PatchC0::RefreshBuffers()
{
	for (int i = 0; i < patches.size(); i++) {
		patches[i]->RefreshBuffers();
	}
}

void PatchC0::Render(int colorLoc, int modelLoc)
{
	if (!showMesh)
		return;

	for (int i = 0; i < patches.size(); i++) {
		patches[i]->Render(colorLoc, modelLoc);
	}
}
