#include "Grid2.h"

#include <glm/gtc/type_ptr.hpp>

std::string const Grid2::GetClassName() {
    return "Grid";
}

Grid2::Grid2(float size, int division, glm::vec3 position, GLint modelLoc, GLint colorLoc, bool numerate)
    : size(size), division((int)pow(2, round(log2(division)))), SimpleFigure(position, modelLoc, GetClassName(), colorLoc, numerate) {
    Calculate(false);
}

void Grid2::Calculate(bool recalculate)
{
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    float mSize = size / division;

    // main grid
    for (int i = 0; i <= division; i++) {
        float x = -size / 2 + mSize * i;
        for (int j = 0; j <= division; j++) {
            float z = size / 2 - mSize * j;
            vertices.push_back(x);
            vertices.push_back(0.f);
            vertices.push_back(z);

            // edge to the right
            if (i < division && z != 0) {
                indices.push_back(i * (division + 1) + j);
                indices.push_back((i + 1) * (division + 1) + j);
            }
            // down edge
            if (j < division && x != 0) {
                indices.push_back(i * (division + 1) + j);
                indices.push_back(i * (division + 1) + j + 1);
            }
        }
    }

    size_t verticesCount = vertices.size() / 3;

    // X axis
    for (int i = 0; i <= division; i++) {
        float x = -size / 2 + mSize * i;
        vertices.push_back(x);
        vertices.push_back(0.f);
        vertices.push_back(0.f);

        // edge to the right
        if (i < division) {
            indices.push_back(verticesCount + i);
            indices.push_back(verticesCount + i + 1);
        }
    }

    verticesCount = vertices.size() / 3;

    // Z axis
    for (int i = 0; i <= division; i++) {
        float z = size / 2 - mSize * i;
        vertices.push_back(0.f);
        vertices.push_back(0.f);
        vertices.push_back(z);

        // down edge
        if (i < division) {
            indices.push_back(verticesCount + i);
            indices.push_back(verticesCount + i + 1);
        }
    }

    InitAndFillBuffers(vertices, indices);
}

void Grid2::Render()
{
    vao.Bind();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // main grid
    glLineWidth(1.0f);
    glUniform4fv(colorLoc, 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    glDrawElements(GL_LINES, indices_count - 4 * division, GL_UNSIGNED_INT, 0);
    // X axis
    glLineWidth(3.0f);
    glUniform4fv(colorLoc, 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
    glDrawElements(GL_LINES, 2 * division, GL_UNSIGNED_INT,
        (void*)((indices_count - 4 * division) * sizeof(GLuint)));
    // Y axis
    glUniform4fv(colorLoc, 1, glm::value_ptr(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));
    glDrawElements(GL_LINES, 2 * division, GL_UNSIGNED_INT,
        (void*)((indices_count - 2 * division) * sizeof(GLuint)));

    vao.Unbind();
}
