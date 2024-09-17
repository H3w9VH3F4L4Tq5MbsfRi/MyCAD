#include "bezierC0.h"

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

const int renderSegments = 10;

void BezierC0::RefreshBuffers() {
  std::tuple<std::vector<GLfloat>, std::vector<GLuint>> data = Calculate();
  indices_count = std::get<1>(data).size();
  vbo.ReplaceBufferData(std::get<0>(data).data(),
                        std::get<0>(data).size() * sizeof(GLfloat));
  ebo.ReplaceBufferData(std::get<1>(data).data(),
                        std::get<1>(data).size() * sizeof(GLuint));
}

void BezierC0::RenderPolyline(int colorLoc, int modelLoc,
                              bool grayscale) {
  vao.Bind();
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  glLineWidth(1.0f);

  glUniform4fv(colorLoc, 1, glm::value_ptr(GetPolylineColor(grayscale)));
  glDrawElements(GL_LINE_STRIP, indices_count, GL_UNSIGNED_INT, 0);

  vao.Unbind();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
BezierC0::Calculate() const {
  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;

  for (int i = 0; i < controlPoints.size(); i++) {
    glm::vec3 pos = controlPoints[i]->GetPosition();
    vertices.push_back(pos.x);
    vertices.push_back(pos.y);
    vertices.push_back(pos.z);

    indices.push_back(i);
  }

  return std::make_tuple(vertices, indices);
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
BezierC0::InitializeAndCalculate(int cpCountLoc, int segmentCountLoc,
                                 int segmentIdxLoc, int divisionLoc) {
  this->cpCountLoc = cpCountLoc;
  this->segmentCountLoc = segmentCountLoc;
  this->segmentIdxLoc = segmentIdxLoc;
  this->divisionLoc = divisionLoc;

  //return Calculate();
  return std::make_tuple(std::vector<GLfloat>(), std::vector<GLuint>());
}

BezierC0::BezierC0(int cpCountLoc, int segmentCountLoc, int segmentIdxLoc, int divisionLoc,
                   const char *name, bool numerate)
    : Figure(InitializeAndCalculate(cpCountLoc, segmentCountLoc, segmentIdxLoc, divisionLoc),
             name, glm::vec3(0.f), numerate) {}

void BezierC0::Render(int colorLoc, int modelLoc, bool grayscale) {
  vao.Bind();

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  glUniform4fv(colorLoc, 1, glm::value_ptr(GetColor(grayscale)));
  glUniform1i(segmentCountLoc, renderSegments);
  glLineWidth(3.0f);
  glUniform1i(divisionLoc, 1);

  int count = glm::ceil((indices_count - 1) / 3.0f);
  for (int i = 0; i < count; i++) {
    int offset = 3 * i;
    int size = glm::min(4, (int)indices_count - offset);
    glUniform1i(cpCountLoc, size);
    glPatchParameteri(GL_PATCH_VERTICES, size);

    for (int j = 0; j < renderSegments; j++) {
      glUniform1i(segmentIdxLoc, j);
      glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_INT,
                     (void *)(offset * sizeof(GLuint)));
    }
  }

  vao.Unbind();
}

bool BezierC0::CreateImgui() {
  bool change = false;

  if (ImGui::BeginListBox("Control points")) {
    for (int i = 0; i < controlPoints.size(); i++) {
      if (ImGui::Selectable((controlPoints[i]->name + ' ').c_str())) {
        RemoveControlPoint(i);
        change = true;
      }
    }
    ImGui::EndListBox();
  }
  return change;
}

void BezierC0::AddControlPoint(Figure *cp) {
  controlPoints.push_back(cp);
  RefreshBuffers();
}

bool BezierC0::RemoveControlPoint(int idx) {
  if (idx >= controlPoints.size())
    return false;
  controlPoints.erase(controlPoints.begin() + idx);
  RefreshBuffers();
  return true;
}

bool BezierC0::ReplaceControlPoint(int idx, Figure *cp) 
{
  if (idx >= controlPoints.size())
    return false;
  controlPoints[idx] = cp;
  RefreshBuffers();
  return true;
}

void BezierC0::ClearControlPoints() {
  controlPoints.clear();
  RefreshBuffers();
}

int BezierC0::Serialize(MG1::Scene &scene, std::vector<uint32_t> cpsIdxs) 
{ 
    MG1::BezierC0 b;
    b.name = name;
    for (auto cpIdx : cpsIdxs) {
      b.controlPoints.push_back(cpIdx);
    }
    scene.bezierC0.push_back(b);
    return -1; 
}
