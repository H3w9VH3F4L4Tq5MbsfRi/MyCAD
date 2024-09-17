#pragma once
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "helpers.h"

#include <glm/glm.hpp>
#include <tuple>
#include <string>
#include"imgui.h"

#include <Serializer.h>

class Figure
{
private:
  glm::vec3 scale;
  glm::vec3 angle;
  glm::vec3 translation;
  glm::vec3 center;

  static int counter;

  glm::vec3 cScale;
  glm::vec3 cAngle;
  glm::vec3 cTranslation;

protected:
  VAO vao;
  VBO vbo;
  EBO ebo;

  glm::mat4 model;
  size_t indices_count;
  bool is3D = false;

  glm::vec4 GetColor(bool grayscale) const {
    glm::vec4 color = selected ? glm::vec4(0.89f, 0.29f, 0.15f, 1.0f)
                               : glm::vec4(1.0f, 0.73f, 0.31f, 1.0f);

    if (grayscale) {
      float gray = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
      color = glm::vec4(gray, gray, gray, color.a);
    }

    return color;
  };

  glm::vec4 GetPolylineColor(bool grayscale) const {
    return grayscale ? glm::vec4(0.587f, 0.587f, 0.587f, 1.f)
                     : glm::vec4(0.f, 1.f, 0.f, 1.f);
  };

public:
  std::string name;
  bool selected = false;

  virtual void Render(int colorLoc, int modelLoc, bool grayscale) = 0;
  virtual void RenderTess(int colorLoc, int modelLoc, bool grayscale){};
  virtual bool GetBoundingSphere(CAD::Sphere& sphere) = 0;
  virtual bool CreateImgui(){
    bool change = false;
    ImGui::SeparatorText((name + " options:").c_str());

    // display position
    ImGui::Text(CAD::printPosition(GetPosition(), "Position: \n").c_str());

    // translation manipulation
    ImGui::SeparatorText("Translation");
    if (ImGui::InputFloat("X", &translation.x, 0.01f, 1.f, "%.2f")) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::InputFloat("Y", &translation.y, 0.01f, 1.f, "%.2f")) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::InputFloat("Z", &translation.z, 0.01f, 1.f, "%.2f")) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::Button("Reset translation")) {
      SetTranslation(glm::vec3(0.0f));
      change = true;
    }

    if (!is3D)
      return change;

    // scaling manipulation
    ImGui::SeparatorText("Scale");
    if (ImGui::InputFloat("Sx", &scale.x, 0.01f, 1.f, "%.2f")) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::InputFloat("Sy", &scale.y, 0.01f, 1.f, "%.2f")) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::InputFloat("Sz", &scale.z, 0.01f, 1.f, "%.2f")) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::Button("Reset scale")) {
      SetScale(glm::vec3(1.0f));
      change = true;
    }

    // rotation manipulation
    ImGui::SeparatorText("Rotation");
    if (ImGui::SliderAngle("X axis", &angle.x, -180.f, 180.f)) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::SliderAngle("Y axis", &angle.y, -180.f, 180.f)) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::SliderAngle("Z axis", &angle.z, -180.f, 180.f)) {
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::Button("Reset rotation")) {
      SetAngle(glm::vec3(0.0f));
      change = true;
    }
     
    return change;
  }
  bool CreatePositionImgui() {
    bool change = false;
    ImGui::SeparatorText((name + " position:").c_str());

    glm::vec3 pos = GetPosition();

    if (ImGui::InputFloat("X", &pos.x, 0.01f, 1.f, "%.2f")) {
      SetPosition(pos);
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::InputFloat("Y", &pos.y, 0.01f, 1.f, "%.2f")) {
      SetPosition(pos);
      CalculateModelMatrix();
      change = true;
    }
    if (ImGui::InputFloat("Z", &pos.z, 0.01f, 1.f, "%.2f")) {
      SetPosition(pos);
      CalculateModelMatrix();
      change = true;
    }

    return change;
  }

  void Delete() {
    vao.Delete();
    vbo.Delete();
    ebo.Delete();
  }
  void virtual CalculateModelMatrix() {
    glm::mat4 translateM =
        CAD::translate(glm::mat4(1.0f), GetPosition());
    glm::mat4 rotateM = CAD::rotate(glm::mat4(1.0f), GetAngle());
    glm::mat4 scaleM = CAD::scaling(glm::mat4(1.0f), GetScale());

    model = translateM * rotateM * scaleM;
  }

  Figure(std::tuple<std::vector<GLfloat>, std::vector<GLuint>> data, std::string type, glm::vec3 center, bool numerate = false) {
    scale = glm::vec3(1.0f);
    angle = glm::vec3(0.0f);
    translation = glm::vec3(0.0f);
    this->center = center;
    indices_count = std::get<1>(data).size();
    model = glm::mat4(1.0f);

    cScale = glm::vec3(1.0f);
    cAngle = glm::vec3(0.0f);
    cTranslation = glm::vec3(0.0f);

    vao.Bind();
    vbo = VBO(std::get<0>(data).data(),
              std::get<0>(data).size() * sizeof(GLfloat));
    ebo =
        EBO(std::get<1>(data).data(), std::get<1>(data).size() * sizeof(GLint));

    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 0, (void *)0);
    vao.Unbind();
    vbo.Unbind();
    ebo.Unbind();

    name = type;
    if (numerate)
      name = name + " #" + std::to_string(++counter);

    CalculateModelMatrix();
  }

  glm::vec3 GetScale() const { return scale + cScale - glm::vec3(1.f); }
  glm::vec3 GetAngle() const { return angle + cAngle; }
  glm::vec3 GetPosition() const { return center + translation + cTranslation; }

  void SetScale(glm::vec3 nScale) {
    scale = nScale;
    CalculateModelMatrix();
  }
  void SetAngle(glm::vec3 nAngle) {
    angle = nAngle;

    while (angle.x > M_PI)
      angle.x -= M_PI * 2;
    while (angle.x < -M_PI)
      angle.x += M_PI * 2;

    while (angle.y > M_PI)
      angle.y -= M_PI * 2;
    while (angle.y < -M_PI)
      angle.y += M_PI * 2;

    while (angle.z > M_PI)
      angle.z -= M_PI * 2;
    while (angle.z < -M_PI)
      angle.z += M_PI * 2;

    CalculateModelMatrix();
  }
  void SetTranslation(glm::vec3 nTranslation) {
    translation = nTranslation;
    CalculateModelMatrix();
  }
  void SetPosition(glm::vec3 nPosition) {
    center = nPosition;
    translation = glm::vec3(0.f);
    CalculateModelMatrix();
  }
  void CalculatePivotTransformation(glm::vec3 centerPosition, glm::vec3 centerScale,
                       glm::vec3 centerAngle, glm::vec3 centerTranslation) { 

    glm::vec4 pos = glm::vec4(center + translation + centerTranslation, 1.f);

    cTranslation = glm::vec3(
        CAD::translate(CAD::scaling(CAD::rotate(CAD::translate(glm::mat4(1.f),
            -centerPosition),
            centerAngle),
            centerScale),
            centerPosition) *
        pos -
        pos) + centerTranslation;

      cAngle = centerAngle;
      cScale = centerScale;

      CalculateModelMatrix();
  }
  void SavePivotTransformations() {
    SetTranslation(translation + cTranslation);
    cTranslation = glm::vec3(0.f);

    SetAngle(angle + cAngle);
    cAngle = glm::vec3(0.f);

    SetScale(scale + cScale - glm::vec3(1.f));
    cScale = glm::vec3(1.f);

    CalculateModelMatrix();
  }
  void static ZeroCounter() { counter = 0; }

  int virtual Serialize(MG1::Scene &scene, std::vector<uint32_t> cpsIdxs =
                                               std::vector<uint32_t>()) {
    return -1;
  };
};