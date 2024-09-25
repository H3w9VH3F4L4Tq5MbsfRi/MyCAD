#include "gregoryPatch.h"

const int renderSegments = 10;

bool GregoryPatch::CreateImgui() {
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

void GregoryPatch::RefreshBuffers() {
  std::tuple<std::vector<GLfloat>, std::vector<GLuint>> data = Calculate();
  indices_count = std::get<1>(data).size();
  vbo.ReplaceBufferData(std::get<0>(data).data(),
                        std::get<0>(data).size() * sizeof(GLfloat));
  ebo.ReplaceBufferData(std::get<1>(data).data(),
                        std::get<1>(data).size() * sizeof(GLuint));
}

bool GregoryPatch::ReplaceControlPoint(int idx, Figure *cp) {
  if (idx >= controlPoints.size())
    return false;

  controlPoints[idx] = cp;
  RefreshBuffers();
  return true;
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
GregoryPatch::InitializeAndCalculate(std::vector<Figure *> controlPoints) {
  this->controlPoints = controlPoints;
  return Calculate();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
GregoryPatch::Calculate() const {
  std::vector<glm::vec3> R;
  for (int i = 0; i < 6; i++) {
    for (int j = 1; j < 4; j++) {
      R.push_back((controlPoints[i * 4 + j - 1]->GetPosition() +
                   controlPoints[i * 4 + j]->GetPosition()) /
                  2.f);
    }
  }

  std::vector<glm::vec3> S;
  for (int i = 0; i < 6; i++) {
    for (int j = 1; j < 3; j++) {
      S.push_back((R[i * 3 + j - 1] + R[i * 3 + j]) / 2.f);
    }
  }

  std::vector<glm::vec3> T;
  for (int i = 0; i < 6; i++) {
    for (int j = 1; j < 2; j++) {
      T.push_back((S[i * 2 + j - 1] + S[i * 2 + j]) / 2.f);
    }
  }

  std::vector<glm::vec3> cps = std::vector<glm::vec3>();
  cps.resize(60);
  for (int i = 0; i < 3; i++) {
    int offset = i * 20;
    int nextIdx = (i + 1) % 3;

    cps[0 + offset] = T[i * 2];                                     // 0
    cps[1 + offset] = 2.f * T[i * 2] - T[i * 2 + 1];                // 1
    // 2
    // 3
    cps[4 + offset] = S[i * 4 + 1];                                 // 4
    cps[5 + offset] = 2.f * S[i * 4 + 1] - S[i * 4 + 3];            // 5
    cps[6 + offset] = cps[5 + offset];                              // 6
    // 7
    // 8
    // 9
    cps[10 + offset] = R[i * 6 + 2];                                // 10
    cps[11 + offset] = 2.f * R[i * 6 + 2] - R[i * 6 + 5];           // 11
    cps[12 + offset] = 2.f * R[nextIdx * 6] - R[nextIdx * 6 + 3];   // 12
    cps[13 + offset] = 2.f * S[nextIdx * 4] - S[nextIdx * 4 + 2];   // 13
    cps[14 + offset] = cps[13 + offset];                            // 14
    cps[15 + offset] = 2.f * T[nextIdx * 2] - T[nextIdx * 2 + 1];   // 15
    cps[16 + offset] = controlPoints[nextIdx * 8]->GetPosition();   // 16
    cps[17 + offset] = R[nextIdx * 6];                              // 17
    cps[18 + offset] = S[nextIdx * 4];                              // 18
    cps[19 + offset] = T[nextIdx * 2];                              // 19
  }

  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;

  // TODO

  return std::make_tuple(vertices, indices);
}

GregoryPatch::GregoryPatch(std::vector<Figure *> controlPoints)
    : Figure(InitializeAndCalculate(controlPoints), "Gregory patch", glm::vec3(0.f), true) {
  this->controlPoints = controlPoints;
}

void GregoryPatch::Render(int colorLoc, int modelLoc, bool grayscale) {
    // TODO
}

void GregoryPatch::RenderTess(int colorLoc, int modelLoc, bool grayscale) {
    // TODO
}
