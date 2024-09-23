#include "polyline.h"
#include <glm/gtc/type_ptr.hpp>

Polyline::Polyline()
    : Figure(std::tuple<std::vector<GLfloat>, std::vector<GLuint>>(),
             "Polyline,", glm::vec3(0.f)) {
  this->graph = new Graph();
}

Polyline::Polyline(Graph *graph)
    : Figure(InitializeAndCalculate(graph), "Polyline", glm::vec3(0.f)) {
  this->graph = graph;
}

void Polyline::Render(int colorLoc, int modelLoc, bool grayscale) {
  vao.Bind();
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  glLineWidth(5.0f);
  glUniform4fv(colorLoc, 1, glm::value_ptr(GetAnimatedColor(grayscale)));

  glDrawElements(GL_LINES, indices_count, GL_UNSIGNED_INT, 0);
  vao.Unbind();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
Polyline::InitializeAndCalculate(Graph *graph) {
  this->graph = graph;
  return Calculate();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
Polyline::Calculate() const {
  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;

  for (int i = 0; i < graph->vertices.size(); i++) {
    vertices.push_back(graph->vertices[i]->GetPosition().x);
    vertices.push_back(graph->vertices[i]->GetPosition().y);
    vertices.push_back(graph->vertices[i]->GetPosition().z);
  }

  for (int i = 0; i < graph->adjList.getVertexCount(); i++) {
    for (int j = 0; j < graph->adjList.getNeighbours(i).size(); j++) {
      indices.push_back(i);
      indices.push_back(graph->adjList.getNeighbours(i)[j]);
    }
  }

  return std::make_tuple(vertices, indices);
}

glm::vec4 Polyline::GetAnimatedColor(bool grayscale) {
  if (!selected) {
    // counter = 0;
    return color * minIntensity;
  }

  counter++;
  int coef = counter % (step * 2);
  coef -= step;
  float intensity = (abs(coef) / (float)step);
  intensity = intensity < minIntensity ? minIntensity : intensity;
  glm::vec4 resultColor = color * intensity;

  if (grayscale) {
    float gray = 0.299f * resultColor.r + 0.587f * resultColor.g +
                 0.114f * resultColor.b;
    resultColor = glm::vec4(gray, gray, gray, resultColor.a);
  }

  return resultColor;
}
