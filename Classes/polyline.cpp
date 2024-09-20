#include "polyline.h"
#include <glm/gtc/type_ptr.hpp>

Polyline::Polyline()
    : Figure(std::tuple<std::vector<GLfloat>, std::vector<GLuint>>(),
             "Polyline,", glm::vec3(0.f)) {}

Polyline::Polyline(Graph *graph)
    : Figure(Calculate(graph), "Polyline", glm::vec3(0.f)) {}

void Polyline::Render(int colorLoc, int modelLoc, bool grayscale) {
  vao.Bind();
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  glLineWidth(5.0f);
  glUniform4fv(colorLoc, 1, glm::value_ptr(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));

  glDrawElements(GL_LINES, indices_count, GL_UNSIGNED_INT, 0);
  vao.Unbind();
}

std::tuple<std::vector<GLfloat>, std::vector<GLuint>>
Polyline::Calculate(Graph *graph) const {
  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;

  for (int i = 0; i < graph->vertices.size(); i++) {
    vertices.push_back(graph->vertices[i]->GetPosition().x);
    vertices.push_back(graph->vertices[i]->GetPosition().y);
    vertices.push_back(graph->vertices[i]->GetPosition().z);
  }

  for (int i = 0; i < graph->adjList.size(); i++) {
    for (int j = 0; j < graph->adjList[i].size(); j++) {
      indices.push_back(i);
      indices.push_back(graph->adjList[i][j]);
    }
  }

  return std::make_tuple(vertices, indices);
}
