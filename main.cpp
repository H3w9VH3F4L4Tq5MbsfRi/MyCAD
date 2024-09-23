#define _USE_MATH_DEFINES
#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"
#include"imgui_stdlib.h"
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<vector>
#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<algorithm>

#include"Shader.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Camera.h"

#include"figure.h"
#include"torus.h"
#include"grid.h"
#include"cursor.h"
#include"point.h"
#include"bezierC0.h"
#include"bezierC2.h"
#include"bezierInt.h"
#include"SurfaceC0.h"
#include"SurfaceC2.h"

#include <Serializer.h>
#include <filesystem>
#include "Graph.h"
#include "polyline.h"

const float near = 0.1f;
const float far = 100.0f;

std::vector<Figure*> figures;
std::vector<int> selected;
Grid* grid;
Camera *camera;
Cursor *center;
Cursor *cursor;

glm::mat4 view;
glm::mat4 proj;

glm::mat4 projL;
glm::mat4 projR;
glm::mat4 displacementL;
glm::mat4 displacementR;
float eyeSeparation = 0.1f;
float convergence = 20.0f;
bool anaglyphActive = false;

static int currentMenuItem = 0;
const char *menuItems = "Move camera\0Place cursor\0Add element\0Select point\0Edit Berenstein point";

void window_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void recalculateSelected(bool deleting = false);
void updateCurvesSelectedChange(bool deleting = false);
void updateSurfacesSelectedChange();
std::vector<int> GetClickedFigures(GLFWwindow *window);
void deselectCurve(bool deleting = false);
void curveCreation();
void deselectFigures();;
void deselectSurfaces();
void recalculateCenter();
void loadScene();
void saveScene();
void replaceCpsInCurves(Figure *newCp);
void replaceCpsInSurfaces(Figure *newCp);

glm::vec3 centerTranslation(0.f);
glm::vec3 centerScale(1.f);
glm::vec3 centerAngle(0.f);
int cursorRadius = 5;

std::vector<BezierC0*> curves;
int selectedCurveIdx = -1;
bool clickingOutCurve = false;

const int initXsegments = 3;
const int initZsegments = 2;

const float initParam1 = 3;
const float initParam2 = 2;

std::vector<SurfaceC0*> surfaces;
std::vector<int> selectedSurfaces;
bool checkIfSelectedArePartOfSurface();
void recalculateSelectedSurfaces();

MG1::SceneSerializer serializer;
std::string filePath = "Scenes\\gregory_test_2024.json";

int modelLoc, viewLoc, projLoc, colorLoc, displacementLoc;
int tessModelLoc, tessViewLoc, tessProjLoc, tessColorLoc, tessCpCountLoc,
    tessResolutionLoc, tessSegmentCountLoc, tessSegmentIdxLoc, tessDivisionLoc,
    tessDisplacementLoc;
int tessSurfaceModelLoc, tessSurfaceViewLoc, tessSurfaceProjLoc,
    tessSurfaceColorLoc, tessSurfaceCpCountLoc, tessSurfaceResolutionLoc,
    tessSurfaceDivisionLoc, tessSurfaceSegmentCountLoc,
    tessSurfaceSegmentIdxLoc, tessSurfaceOtherAxisLoc, tessSurfaceBsplineLoc,
    tessSurfaceDisplacementLoc;

bool renderGrid = true;
std::string serializerErrorMsg = "";

std::vector<Polyline*> cycles;

int main() { 
    // initial values
    int width = 1500;
    int height = 800;
    glm::vec3 cameraPosition = glm::vec3(3.0f, 3.0f, 3.0f);
    float fov = M_PI / 4.0f;
    int guiWidth = 300;

    #pragma region gl_boilerplate
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, "MKMG", NULL, NULL);
    if (window == NULL) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, width - guiWidth, height);
    glEnable(GL_DEPTH_TEST);

    GLFWimage icon;
    icon.pixels = stbi_load("icon.png", &icon.width, &icon.height, 0, 4);
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);
    #pragma endregion

    // shaders and uniforms
    Shader shaderProgram("Shaders\\default.vert", "Shaders\\default.frag");
    modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
    viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
    projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
    colorLoc = glGetUniformLocation(shaderProgram.ID, "color");
    displacementLoc = glGetUniformLocation(shaderProgram.ID, "displacement");

    Shader tessShaderProgram("Shaders\\tessellation.vert", "Shaders\\default.frag",
                             "Shaders\\tessellation.tesc", "Shaders\\tessellation.tese");
    tessModelLoc = glGetUniformLocation(tessShaderProgram.ID, "model");
    tessViewLoc = glGetUniformLocation(tessShaderProgram.ID, "view");
    tessProjLoc = glGetUniformLocation(tessShaderProgram.ID, "proj");
    tessColorLoc = glGetUniformLocation(tessShaderProgram.ID, "color");
    tessCpCountLoc = glGetUniformLocation(tessShaderProgram.ID, "cpCount");
    tessResolutionLoc = glGetUniformLocation(tessShaderProgram.ID, "resolution");
    tessSegmentCountLoc = glGetUniformLocation(tessShaderProgram.ID, "segmentCount");
    tessSegmentIdxLoc = glGetUniformLocation(tessShaderProgram.ID, "segmentIdx");
	tessDivisionLoc = glGetUniformLocation(tessShaderProgram.ID, "division");
    tessDisplacementLoc = glGetUniformLocation(tessShaderProgram.ID, "displacement");

	Shader tessSurfaceShaderProgram("Shaders\\tessellation.vert", "Shaders\\default.frag",
		"Shaders\\tessellation.tesc", "Shaders\\tessellationSurface.tese");
    tessSurfaceModelLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "model");
    tessSurfaceViewLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "view");
    tessSurfaceProjLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "proj");
    tessSurfaceColorLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "color");
    tessSurfaceCpCountLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "cpCount");
    tessSurfaceResolutionLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "resolution");
    tessSurfaceDivisionLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "division");
    tessSurfaceSegmentCountLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "segmentCount");
    tessSurfaceSegmentIdxLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "segmentIdx");
	tessSurfaceOtherAxisLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "otherAxis");
	tessSurfaceBsplineLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "bspline");
    tessSurfaceDisplacementLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "displacement");

    // callbacks
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // init figures
    grid = new Grid();
    cursor = new Cursor();
    center = new Cursor();
    camera = new Camera(width, height, cameraPosition, fov, near, far, guiWidth);
    cycles = std::vector<Polyline*>();

    // matrices locations
    camera->PrepareMatrices(view, proj);
    camera->PrepareAnaglyphMatrices(convergence, eyeSeparation, projL, projR);
    CAD::displacemt(eyeSeparation, displacementL, displacementR);

    int PxSegments = initXsegments;
    int PzSegments = initZsegments;
    float Pparam1 = initParam1;
    float Pparam2 = initParam2;

    #pragma region imgui_boilerplate
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    #pragma endregion

    auto renderFigures = [&](bool grayscale, glm::mat4 displacement, glm::mat4 projection) {
      // default shader activation
      shaderProgram.Activate();

      // matrices for default shader
      glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
      glUniformMatrix4fv(displacementLoc, 1, GL_FALSE,
                         glm::value_ptr(displacement));

      // objects rendering with default shader
      for (int i = 0; i < figures.size(); i++) {
        figures[i]->Render(colorLoc, modelLoc, grayscale);
      }
      // render curves polyline with default shader (still)
      for (int i = 0; i < curves.size(); i++) {
        if (curves[i]->selected) {
          curves[i]->RenderPolyline(colorLoc, modelLoc, grayscale);
        }
      }
      // render surfaces with default shader
      for (int i = 0; i < surfaces.size(); i++)
        surfaces[i]->Render(colorLoc, modelLoc, grayscale);

      if (selectedSurfaces.size() > 0 && cycles.size() > 0) {
        int selecredCycleIndex = -1;
        for (int i = 0; i < cycles.size(); i++) {
          if (cycles[i]->selected) {
            selecredCycleIndex = i;
            break;
          }
        }
        cycles[selecredCycleIndex]->Render(colorLoc, modelLoc, grayscale);

        for (int i = 0; i < cycles.size(); i++) {
          if (i == selecredCycleIndex)
            continue;
          cycles[i]->Render(colorLoc, modelLoc, grayscale);
        }
      }

      // tessellation shader activation
      tessShaderProgram.Activate();

      // matrices for tessellation shader (with workaround)
      glm::mat4 tessView = glm::mat4(view);
      glm::mat4 tessProj = glm::mat4(projection);
      glUniformMatrix4fv(tessViewLoc, 1, GL_FALSE, glm::value_ptr(tessView));
      glUniformMatrix4fv(tessProjLoc, 1, GL_FALSE, glm::value_ptr(tessProj));
      glUniform2i(tessResolutionLoc, camera->GetWidth(), camera->GetHeight());
      glUniformMatrix4fv(tessDisplacementLoc, 1, GL_FALSE,
                         glm::value_ptr(displacement));

      // curves rendering with tessellation shader
      for (int i = 0; i < curves.size(); i++) {
        curves[i]->Render(tessColorLoc, tessModelLoc, grayscale);
      }

      // surface tessellation shader activation
      tessSurfaceShaderProgram.Activate();

      // matrices for surface tessellation shader (with workaround)
      glm::mat4 tessSurfaceView = glm::mat4(view);
      glm::mat4 tessSurfaceProj = glm::mat4(projection);
      glUniformMatrix4fv(tessSurfaceViewLoc, 1, GL_FALSE,
                         glm::value_ptr(tessSurfaceView));
      glUniformMatrix4fv(tessSurfaceProjLoc, 1, GL_FALSE,
                         glm::value_ptr(tessSurfaceProj));
      glUniform2i(tessSurfaceResolutionLoc, camera->GetWidth(),
                  camera->GetHeight());
      glUniformMatrix4fv(tessSurfaceDisplacementLoc, 1, GL_FALSE,
                         glm::value_ptr(displacement));

      // surfaces rendering with surface tessellation shader
      for (int i = 0; i < surfaces.size(); i++)
        surfaces[i]->RenderTess(tessSurfaceColorLoc, tessSurfaceModelLoc,
                                grayscale);
    };

    while (!glfwWindowShouldClose(window)) 
    {
        #pragma region init
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(camera->guiWidth, camera->GetHeight()));
        ImGui::SetNextWindowPos(ImVec2(camera->GetWidth(), 0));
        #pragma endregion

        if (currentMenuItem == 0){
          camera->HandleInputs(window);
          camera->PrepareMatrices(view, proj);
        }
        
        // render non-grayscaleable objects
        shaderProgram.Activate();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(displacementLoc, 1, GL_FALSE,
                           glm::value_ptr(glm::mat4(1.0f)));
        if (renderGrid) grid->Render(colorLoc, modelLoc);
        cursor->Render(colorLoc, modelLoc);
        if (selected.size() > 0) {
          center->Render(colorLoc, modelLoc);
        }

        // render grayscaleable objects
        if (!anaglyphActive) {
          renderFigures(false, glm::mat4(1.f), proj);
        } else {
          glColorMask(true, false, false, false);
          renderFigures(true, displacementL, projL);
          glClear(GL_DEPTH_BUFFER_BIT);
          glColorMask(false, true, true, false);
          renderFigures(true, displacementR, projR);
          glColorMask(true, true, true, true);
        }

        // imgui rendering
        if (ImGui::Begin("Menu", 0,
                         ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
          // mode selection
          if (ImGui::Combo(" ", &currentMenuItem, menuItems)) {
            if (currentMenuItem != 2) {
              clickingOutCurve = false;
            }
            if (currentMenuItem == 4) {
              deselectFigures();
            }
          }

          // cursor position & radius slider
          {
            cursor->CreateImgui();
            ImGui::SliderInt("Radius", &cursorRadius, glm::max((int)near, 1),
                             glm::max((int)far, 1));
          }

          // add object buttons for object add mode
          if (currentMenuItem == 2) {
            ImGui::Separator();
            // torus
            if (ImGui::Button("Torus")) {
              figures.push_back(new Torus(cursor->GetPosition()));
            }
            // point
            ImGui::SameLine();
            if (ImGui::Button("Point")) {
              figures.push_back(new Point(cursor->GetPosition()));
            }
            // bezier C0
            if (ImGui::Button("Bezier C0")) {
              curves.push_back(new BezierC0(tessCpCountLoc, tessSegmentCountLoc,
                                            tessSegmentIdxLoc, tessDivisionLoc));
              curveCreation();
            }
            // bezier C2
            ImGui::SameLine();
            if (ImGui::Button("Bezier C2")) {
              curves.push_back(new BezierC2(tessCpCountLoc, tessSegmentCountLoc,
                                            tessSegmentIdxLoc, tessDivisionLoc));
              curveCreation();
            }
            // bezier int
            ImGui::SameLine();
            if (ImGui::Button("Bezier Int")) {
              curves.push_back(new BezierInt(tessCpCountLoc, tessSegmentCountLoc,
                                            tessSegmentIdxLoc, tessDivisionLoc));
              curveCreation();
            } 
            // plane C0 
            if (ImGui::Button("Plane C0")) {
                ImGui::OpenPopup("planeC0popup");
            }
            if (ImGui::BeginPopup("planeC0popup")) {
                ImGui::SeparatorText("Plane C0 params:");
                if (ImGui::InputInt("x segments", &PxSegments)) {
                    PxSegments = PxSegments >= 1 ? PxSegments : 1;
                }
                if (ImGui::InputInt("z segments", &PzSegments)) {
                    PzSegments = PzSegments >= 1 ? PzSegments : 1;
                }
                if (ImGui::InputFloat("length", &Pparam1, 0.01f, 1.f, "%.2f")) {
                    Pparam1 = Pparam1 >= 0.01f ? Pparam1 : 0.01f;
                }
                if (ImGui::InputFloat("width", &Pparam2, 0.01f, 1.f, "%.2f")) {
                    Pparam2 = Pparam2 >= 0.01f ? Pparam2 : 0.01f;
                }
                if (ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();

                    SurfaceC0* plane = new SurfaceC0(cursor->GetPosition());
                    std::vector<Figure*> newFigures = plane->CalculatePlane(tessSurfaceCpCountLoc, tessSurfaceSegmentCountLoc, tessSurfaceSegmentIdxLoc, tessSurfaceDivisionLoc, 
                        tessSurfaceOtherAxisLoc, tessSurfaceBsplineLoc, PxSegments, PzSegments, Pparam1, Pparam2);
                    for (int i = 0; i < newFigures.size(); i++) {
                        figures.push_back(newFigures[i]);
                    }
                    surfaces.push_back(plane);
                    deselectSurfaces();
                    plane->selected = true;
                    recalculateSelectedSurfaces();
                    PxSegments = initXsegments;
                    PzSegments = initZsegments;
                    Pparam1 = initParam1;
                    Pparam2 = initParam2;
                }

                ImGui::EndPopup();
            }
            // cylinder C0
            ImGui::SameLine();
            if (ImGui::Button("Cylinder C0")) {
                ImGui::OpenPopup("cylinderC0popup");
            }
            if (ImGui::BeginPopup("cylinderC0popup")) {
                ImGui::SeparatorText("Cylinder C0 params:");
                if (ImGui::InputInt("ambit segments", &PxSegments)) {
                    PxSegments = PxSegments >= 1 ? PxSegments : 1;
                }
                if (ImGui::InputInt("y segments", &PzSegments)) {
                    PzSegments = PzSegments >= 1 ? PzSegments : 1;
                }
                if (ImGui::InputFloat("radius", &Pparam1, 0.01f, 1.f, "%.2f")) {
                    Pparam1 = Pparam1 >= 0.01f ? Pparam1 : 0.01f;
                }
                if (ImGui::InputFloat("height", &Pparam2, 0.01f, 1.f, "%.2f")) {
                    Pparam2 = Pparam2 >= 0.01f ? Pparam2 : 0.01f;
                }
                if (ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();

                    SurfaceC0* cylinder = new SurfaceC0(cursor->GetPosition());
                    std::vector<Figure*> newFigures = cylinder->CalculateCylinder(tessSurfaceCpCountLoc, tessSurfaceSegmentCountLoc, tessSurfaceSegmentIdxLoc, tessSurfaceDivisionLoc,
                        tessSurfaceOtherAxisLoc, tessSurfaceBsplineLoc, PxSegments, PzSegments, Pparam1, Pparam2);
                    for (int i = 0; i < newFigures.size(); i++) {
                        figures.push_back(newFigures[i]);
                    }
                    surfaces.push_back(cylinder);
                    deselectSurfaces();
                    cylinder->selected = true;
                    recalculateSelectedSurfaces();

                    PxSegments = initXsegments;
                    PzSegments = initZsegments;
                    Pparam1 = initParam1;
                    Pparam2 = initParam2;
                }

                ImGui::EndPopup();
            }
            // plane C2
            if (ImGui::Button("Plane C2")) {
                ImGui::OpenPopup("planeC2popup");
            }
            if (ImGui::BeginPopup("planeC2popup")) {
                ImGui::SeparatorText("Plane C2 params:");
                if (ImGui::InputInt("x segments", &PxSegments)) {
                    PxSegments = PxSegments >= 1 ? PxSegments : 1;
                }
                if (ImGui::InputInt("z segments", &PzSegments)) {
                    PzSegments = PzSegments >= 1 ? PzSegments : 1;
                }
                if (ImGui::InputFloat("length", &Pparam1, 0.01f, 1.f, "%.2f")) {
                    Pparam1 = Pparam1 >= 0.01f ? Pparam1 : 0.01f;
                }
                if (ImGui::InputFloat("width", &Pparam2, 0.01f, 1.f, "%.2f")) {
                    Pparam2 = Pparam2 >= 0.01f ? Pparam2 : 0.01f;
                }
                if (ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();

                    SurfaceC0* plane = new SurfaceC2(cursor->GetPosition());
                    std::vector<Figure*> newFigures = plane->CalculatePlane(tessSurfaceCpCountLoc, tessSurfaceSegmentCountLoc, tessSurfaceSegmentIdxLoc, tessSurfaceDivisionLoc,
                        tessSurfaceOtherAxisLoc, tessSurfaceBsplineLoc, PxSegments, PzSegments, Pparam1, Pparam2);
                    for (int i = 0; i < newFigures.size(); i++) {
                        figures.push_back(newFigures[i]);
                    }
                    surfaces.push_back(plane);
                    deselectSurfaces();
                    plane->selected = true;
                    recalculateSelectedSurfaces();

                    PxSegments = initXsegments;
                    PzSegments = initZsegments;
                    Pparam1 = initParam1;
                    Pparam2 = initParam2;
                }

                ImGui::EndPopup();
            }
            // cylinder C2
            ImGui::SameLine();
            if (ImGui::Button("Cylinder C2")) {
                ImGui::OpenPopup("cylinderC2popup");
            }
            if (ImGui::BeginPopup("cylinderC2popup")) {
                ImGui::SeparatorText("Cylinder C2 params:");
                if (ImGui::InputInt("ambit segments", &PxSegments)) {
                    PxSegments = PxSegments >= 3 ? PxSegments : 3;
                }
                if (ImGui::InputInt("y segments", &PzSegments)) {
                    PzSegments = PzSegments >= 1 ? PzSegments : 1;
                }
                if (ImGui::InputFloat("radius", &Pparam1, 0.01f, 1.f, "%.2f")) {
                    Pparam1 = Pparam1 >= 0.01f ? Pparam1 : 0.01f;
                }
                if (ImGui::InputFloat("height", &Pparam2, 0.01f, 1.f, "%.2f")) {
                    Pparam2 = Pparam2 >= 0.01f ? Pparam2 : 0.01f;
                }
                if (ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();

                    SurfaceC0* cylinder = new SurfaceC2(cursor->GetPosition());
                    std::vector<Figure*> newFigures = cylinder->CalculateCylinder(tessSurfaceCpCountLoc, tessSurfaceSegmentCountLoc, tessSurfaceSegmentIdxLoc, tessSurfaceDivisionLoc,
                        tessSurfaceOtherAxisLoc, tessSurfaceBsplineLoc, PxSegments, PzSegments, Pparam1, Pparam2);
                    for (int i = 0; i < newFigures.size(); i++) {
                        figures.push_back(newFigures[i]);
                    }
                    surfaces.push_back(cylinder);
                    deselectSurfaces();
                    cylinder->selected = true;
                    recalculateSelectedSurfaces();

                    PxSegments = initXsegments;
                    PzSegments = initZsegments;
                    Pparam1 = initParam1;
                    Pparam2 = initParam2;
                }

                ImGui::EndPopup();
            }
          }


          ImGui::BeginChild("figures", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.5f), ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar);
           
              // surface selection
		      if (surfaces.size() > 0) {
			      ImGui::SeparatorText("Surfaces");
		      }
		      for (int i = 0; i < surfaces.size(); i++) {
                  if (ImGui::Selectable(surfaces[i]->name.c_str(),
                                        &surfaces[i]->selected)) 
                  {
                    if (!ImGui::GetIO().KeyShift) 
                    {
                      bool temp = surfaces[i]->selected;
                      std::for_each(surfaces.begin(),
                                    surfaces.end(),
                                    [](SurfaceC0 *s) {
                                      s->selected = false;
                                      ;
                                    });
                      surfaces[i]->selected = temp;
                    }
                    recalculateSelectedSurfaces();
                  }
		      }
          
              // curves selection
              if (curves.size() > 0) {
                ImGui::SeparatorText("Curves");
              }
              for (int i = 0; i < curves.size(); i++) {
                if (ImGui::Selectable(curves[i]->name.c_str(),
                                      &curves[i]->selected)) {
                  bool temp = curves[i]->selected;
                  std::for_each(curves.begin(), curves.end(), [](Figure *f) {
                    f->selected = false;;});
                  curves[i]->selected = temp;

                  if (!temp) {
                    deselectCurve();
                  } else {
                    selectedCurveIdx = i;
                  }

                  recalculateSelected();
                }
              }
              // other figures selection
              if (figures.size() > 0 && currentMenuItem != 4) {
                ImGui::SeparatorText("Other figures");

                for (int i = 0; i < figures.size(); i++) {
                  if (ImGui::Selectable(figures[i]->name.c_str(),
                                        &figures[i]->selected)) {
                    if (!ImGui::GetIO().KeyShift) {
                      bool temp = figures[i]->selected;
                      std::for_each(figures.begin(), figures.end(), [](Figure *f) {
                        f->selected = false;
                        ;
                      });
                      figures[i]->selected = temp;
                    }
                    recalculateSelected();
                  }
                }
              }

		  ImGui::EndChild();

          // delete button
          if (selected.size() > 0 || selectedCurveIdx != -1 || selectedSurfaces.size() > 0) {
            ImGui::Separator();
            if (ImGui::Button("Delete selected")) 
            {
              if (!checkIfSelectedArePartOfSurface()) 
              {
                  updateCurvesSelectedChange(true);
                  for (int i = selected.size() - 1; i >= 0; i--) {
                      figures.erase(figures.begin() + selected[i]);
                  }
                  if (selectedCurveIdx != -1) {
                      curves.erase(curves.begin() + selectedCurveIdx);
                      deselectCurve(true);
                  }
                  for (int i = selectedSurfaces.size() - 1; i >= 0; i--) {
                    surfaces.erase(surfaces.begin() + selectedSurfaces[i]);
                    recalculateSelectedSurfaces();
                  }
                  recalculateSelected(true);
              }
              else 
              {
                  ImGui::OpenPopup("FailedToDelete");
              }
            }
          }
          if (ImGui::BeginPopup("FailedToDelete")) {
              ImGui::Text("At least one of the selected points is part of a surface!");
              if (ImGui::Button("OK")) {
                  ImGui::CloseCurrentPopup();
              }
              ImGui::EndPopup();
          }

          if (((selectedCurveIdx != -1) != (selectedSurfaces.size() == 1)) && selected.size() == 0) {
              // delete complex figure with all its control points
			  ImGui::SameLine();
			  if (ImGui::Button("Delete with cps")) {
                  std::vector<Figure*> cpsToDelete;
				  if (selectedCurveIdx != -1) {
					  cpsToDelete = curves[selectedCurveIdx]->GetControlPoints();
					  curves.erase(curves.begin() + selectedCurveIdx);
					  deselectCurve(true);
				  }
                  if (selectedSurfaces.size() == 1) {
					  cpsToDelete = surfaces[selectedSurfaces[0]]->GetControlPoints();
                      surfaces.erase(surfaces.begin() + selectedSurfaces[0]);
                      recalculateSelectedSurfaces();
				  }
				  for (int i = 0; i < cpsToDelete.size(); i++) 
					  for (int j = 0; j < figures.size(); j++) 
                          if (cpsToDelete[i] == figures[j]) 
                          {
                              figures.erase(figures.begin() + j);
                              break;
                          }

				  recalculateSelected(true);
			  }
			  // select all control points
			  if (ImGui::Button("Select all cps")) {
				  if (selectedCurveIdx != -1) {
					  std::vector<Figure*> cps = curves[selectedCurveIdx]->GetControlPoints();
					  for (int i = 0; i < cps.size(); i++) {
						  cps[i]->selected = true;
					  }
                      deselectCurve();
				  }
				  if (selectedSurfaces.size() == 1) {
					  std::vector<Figure*> cps = surfaces[selectedSurfaces[0]]->GetControlPoints();
					  for (int i = 0; i < cps.size(); i++) {
						  cps[i]->selected = true;
					  }
                      deselectSurfaces();
				  }
				  recalculateSelected();
			  }
		  }

		  if (selected.size() != 0 || selectedCurveIdx != -1 || selectedSurfaces.size() != 0) {
              ImGui::SameLine();
			  if (ImGui::Button("Deselect all")) {
				  deselectFigures();
                  deselectCurve();
                  deselectSurfaces();
			  }
		  }

          // add points to curve button
          if (selected.size() > 0 && selectedCurveIdx != -1 &&
              selectedSurfaces.size() == 0) {
            if (ImGui::Button("Add points to curve")) {
              for (int i = 0; i < selected.size(); i++) {
                curves[selectedCurveIdx]->AddControlPoint(figures[selected[i]]);
                clickingOutCurve = false;
              }
            }
          }
          // clicking out curve checkbox
          if (selectedCurveIdx != -1 && currentMenuItem != 4) {
            ImGui::Checkbox("Click-out curve", &clickingOutCurve);
          }

          // selected item menu
          if (selected.size() == 1 && selectedCurveIdx == -1 &&
              selectedSurfaces.size() == 0) 
          {
              ImGui::Separator();
              // change name window
              ImGui::InputText("Change name", &figures[selected[0]]->name);
              // display selected item position
              center->SetPosition(figures[selected[0]]->GetPosition());
              // display selected item menu
              if (figures[selected[0]]->CreateImgui()) {
                updateCurvesSelectedChange();
				updateSurfacesSelectedChange();
              }
          }
          if (selected.size() == 0 && selectedCurveIdx != -1 &&
              selectedSurfaces.size() == 0 && currentMenuItem != 4) 
          {
            ImGui::Separator();
            // change name window
            ImGui::InputText("Change name", &curves[selectedCurveIdx]->name);
            // display selected curve menu
            if (curves[selectedCurveIdx]->CreateImgui()) 
            {
              if (curves[selectedCurveIdx]->GetControlPoints().size() == 0) 
              {
                curves.erase(curves.begin() + selectedCurveIdx);
                deselectCurve(true);
                recalculateSelected(true);
              }
            }
          }

          if (selected.size() == 0 && selectedCurveIdx == -1 &&
              selectedSurfaces.size() == 1) {
              ImGui::Separator();
              // change name window
              ImGui::InputText("Change name", &surfaces[selectedSurfaces[0]]->name);
              // display selected surface menu
              surfaces[selectedSurfaces[0]]->CreateImgui();
          }

          // multiple figures merging & manipulation
          if (selected.size() > 1 && selectedCurveIdx == -1 &&
              selectedSurfaces.size() == 0) {
            if (ImGui::Button("Merge selected")) 
            {
              Figure *mergeResult = new Point(center->GetPosition());
              replaceCpsInCurves(mergeResult);
              replaceCpsInSurfaces(mergeResult);
              for (int i = selected.size() - 1; i >= 0; i--) {
                figures.erase(figures.begin() + selected[i]);
              }
              mergeResult->selected = true;
              figures.push_back(mergeResult);
              recalculateSelected(true);
            }

            bool change = false;
            // translation
			ImGui::SeparatorText("Center translation");
			if (ImGui::InputFloat("cX", &centerTranslation.x, 0.01f, 1.f, "%.2f")) {
				change = true;
                recalculateCenter();
				for (int i = 0; i < selected.size(); i++) {
                    figures[selected[i]]->CalculatePivotTransformation(
                        center->GetPosition(), centerScale, centerAngle, centerTranslation);
				}
			}
			if (ImGui::InputFloat("cY", &centerTranslation.y, 0.01f, 1.f, "%.2f")) {
				change = true;
                recalculateCenter();
				for (int i = 0; i < selected.size(); i++) {
					figures[selected[i]]->CalculatePivotTransformation(
                        center->GetPosition(), centerScale, centerAngle, centerTranslation);
				}
			}
			if (ImGui::InputFloat("cZ", &centerTranslation.z, 0.01f, 1.f, "%.2f")) {
				change = true;
                recalculateCenter();
				for (int i = 0; i < selected.size(); i++) {
					figures[selected[i]]->CalculatePivotTransformation(
                        center->GetPosition(), centerScale, centerAngle, centerTranslation);
				}
			}
			if (ImGui::Button("Reset center translation")) {
				change = true;
				centerTranslation = glm::vec3(0.f);
				recalculateCenter();
				for (int i = 0; i < selected.size(); i++) {
					figures[selected[i]]->CalculatePivotTransformation(
						center->GetPosition(), centerScale, centerAngle, centerTranslation);
				}
			}

            // scaling manipulation
            ImGui::SeparatorText("Center scale");
            if (ImGui::InputFloat("cSx", &centerScale.x, 0.01f, 1.f, "%.2f")) {
              change = true;
              for (int i = 0; i < selected.size(); i++) {
                figures[selected[i]]->CalculatePivotTransformation(
                    center->GetPosition(), centerScale, centerAngle, centerTranslation);
              }
            }
            if (ImGui::InputFloat("cSy", &centerScale.y, 0.01f, 1.f, "%.2f")) {
              change = true;
              for (int i = 0; i < selected.size(); i++) {
                figures[selected[i]]->CalculatePivotTransformation(
                    center->GetPosition(), centerScale, centerAngle, centerTranslation);
              }
            }
            if (ImGui::InputFloat("cSz", &centerScale.z, 0.01f, 1.f, "%.2f")) {
              change = true;
              for (int i = 0; i < selected.size(); i++) {
                figures[selected[i]]->CalculatePivotTransformation(
                    center->GetPosition(), centerScale, centerAngle, centerTranslation);
              }
            }
            if (ImGui::Button("Reset center scale")) {
              change = true;
              centerScale = glm::vec3(1.f);
              for (int i = 0; i < selected.size(); i++) {
                figures[selected[i]]->CalculatePivotTransformation(
                    center->GetPosition(), centerScale, centerAngle, centerTranslation);
              }
            }
            // rotation manipulation
            ImGui::SeparatorText("Center rotation");
            if (ImGui::SliderAngle("cX axis", &centerAngle.x, -180.f, 180.f)) {
              change = true;
              for (int i = 0; i < selected.size(); i++) {
                figures[selected[i]]->CalculatePivotTransformation(
                    center->GetPosition(), centerScale, centerAngle, centerTranslation);
              }
            }
            if (ImGui::SliderAngle("cY axis", &centerAngle.y, -180.f, 180.f)) {
              change = true;
              for (int i = 0; i < selected.size(); i++) {
                figures[selected[i]]->CalculatePivotTransformation(
                    center->GetPosition(), centerScale, centerAngle, centerTranslation);
              }
            }
            if (ImGui::SliderAngle("cZ axis", &centerAngle.z, -180.f, 180.f)) {
              change = true;
              for (int i = 0; i < selected.size(); i++) {
                figures[selected[i]]->CalculatePivotTransformation(
                    center->GetPosition(), centerScale, centerAngle, centerTranslation);
              }
            }
            if (ImGui::Button("Reset center rotation")) {
              change = true;
              centerAngle = glm::vec3(0.f);
              for (int i = 0; i < selected.size(); i++) {
                figures[selected[i]]->CalculatePivotTransformation(
                    center->GetPosition(), centerScale, centerAngle, centerTranslation);
              }
            }
            if (change) {
              updateCurvesSelectedChange();
              updateSurfacesSelectedChange();
            }
          }

          //bspline menu
          if (currentMenuItem == 4 && selectedCurveIdx != -1) {
            curves[selectedCurveIdx]->CreateBsplineImgui();
          }

          //anaglyph menu
          ImGui::Separator();
          ImGui::Text("Anaglyph parameters");
          ImGui::Checkbox("Active", &anaglyphActive);
          if (ImGui::InputFloat("Convergence", &convergence, 0.01f, 1.f,
                                "%.2f")) {
            if (convergence <= 0.f) convergence = 0.01f;

            camera->PrepareAnaglyphMatrices(convergence, eyeSeparation, projL,
                                            projR);
          };
          if (ImGui::InputFloat("Eye separation", &eyeSeparation, 0.01f, 1.f,
              "%.2f")) {
            if (eyeSeparation < 0.f) eyeSeparation = 0.f;
            
            camera->PrepareAnaglyphMatrices(convergence, eyeSeparation, projL,
                                            projR);
            CAD::displacemt(eyeSeparation, displacementL, displacementR);
          }

          // save/load scene
          ImGui::Separator();
          ImGui::Text("Scene serialization");
          ImGui::InputText("Target path", &filePath);
          if (ImGui::Button("Load")) {
            try {
            if (!std::filesystem::exists(filePath))
                throw std::exception("File does not exist!");
            serializer.LoadScene(filePath);
            loadScene();
            ImGui::OpenPopup("LoadSuccessPopup");
            } catch (const std::exception &e) {
              serializerErrorMsg = e.what();
              ImGui::OpenPopup("LoadSaveErrorPopup");
            }
          }
          ImGui::SameLine();
          if (ImGui::Button("Save")) {
            try {
            if (std::filesystem::exists(filePath))
                throw std::exception("File already exists!");
            saveScene();
            serializer.SaveScene(filePath);
            ImGui::OpenPopup("SaveSuccessPopup");
            } catch (const std::exception &e) {
              serializerErrorMsg = e.what();
              ImGui::OpenPopup("LoadSaveErrorPopup");
            }
          }

          if (ImGui::BeginPopup("LoadSaveErrorPopup")) {
            ImGui::Text(serializerErrorMsg.c_str());
            if (ImGui::Button("OK")) {
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }
          if (ImGui::BeginPopup("LoadSuccessPopup")) {
            ImGui::Text("Scene loaded from");
            ImGui::SameLine();
            ImGui::Text(filePath.c_str());
            if (ImGui::Button("OK")) {
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }
          if (ImGui::BeginPopup("SaveSuccessPopup")) {
            ImGui::Text("Scene saved to");
            ImGui::SameLine();
            ImGui::Text(filePath.c_str());
            if (ImGui::Button("OK")) {
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }

          if (selectedSurfaces.size() > 0) 
          {
            ImGui::SeparatorText("Gregory patch options:");
            ImGui::Text("Places for patch: ");
            ImGui::SameLine();
            ImGui::Text(std::to_string(cycles.size()).c_str());

            if (cycles.size() > 1) {
              int selectedCycle = -1;
              for (int i = 0; i < cycles.size(); i++) {
                if (cycles[i]->selected) {
                  selectedCycle = i;
                  break;
                }
              }
              selectedCycle += 1;

              if (ImGui::SliderInt("Selection", &selectedCycle, 1,
                                   cycles.size())) {
                for (int i = 0; i < cycles.size(); i++) {
                  cycles[i]->selected = false;
                }
                cycles[selectedCycle - 1]->selected = true;
              }
            }

            if (cycles.size() > 0) {
              if (ImGui::Button("Create Gregory patch")) {
                // TODO: implement
                std::cout << "Creating Gregory patch" << std::endl;
              }
            }
          }
        }

        ImGui::End();
        #pragma region rest
        ImGui::Render();
        //std::cout << ImGui::GetIO().Framerate << std::endl;
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
        #pragma endregion
    }
    #pragma region exit
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    grid->Delete();
    cursor->Delete();
    center->Delete();
	std::for_each(figures.begin(), figures.end(),
                  [](Figure* f) { f->Delete(); });
    std::for_each(curves.begin(), curves.end(),
                  [](Figure *c) { c->Delete(); });
	std::for_each(surfaces.begin(), surfaces.end(),
		          [](Figure* p) { p->Delete(); });
    shaderProgram.Delete();
    tessShaderProgram.Delete();
	tessSurfaceShaderProgram.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
    #pragma endregion
}

// callbacks
void window_size_callback(GLFWwindow *window, int width, int height) {
  camera->SetWidth(width);
  camera->SetHeight(height);
  camera->PrepareMatrices(view, proj);
  glViewport(0, 0, width - camera->guiWidth, height);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) 
{
  if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
    cursorRadius = glm::clamp((int)(cursorRadius + yoffset),
                              glm::max((int)near, 1), glm::max((int)far, 1));
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
      currentMenuItem = 0;
      clickingOutCurve = false;
    } else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
      currentMenuItem = 1;
      clickingOutCurve = false;
    } else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
      currentMenuItem = 2;
    } else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
      currentMenuItem = 3;
      clickingOutCurve = false;
    } else if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
      currentMenuItem = 4;
      clickingOutCurve = false;
      deselectFigures();
    }

    if (key == GLFW_KEY_G && action == GLFW_PRESS)
      renderGrid = !renderGrid;
  }
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    clickingOutCurve = false;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
    {
      glm::vec3 newCursorPos =
          CAD::calculateNewCursorPos(window, proj, view, camera, cursorRadius);
      std::vector<int> clickedFigures = GetClickedFigures(window);

      // cursor movement
      if (currentMenuItem == 1) {
        cursor->SetPosition(newCursorPos);
      } 
      // point selection
      else if (currentMenuItem == 3) {
        for (int i = 0; i < clickedFigures.size(); i++) {
          figures[clickedFigures[i]]->selected = !figures[clickedFigures[i]]->selected;
          recalculateSelected();
        }
      }
      // add point to curve being created
      if (clickingOutCurve) {
        std::vector<Figure*> newCPs;
        // if any points have been clicked - add them
        if (clickedFigures.size() > 0) {
          for (int i = 0; i < clickedFigures.size(); i++) {
            newCPs.push_back(figures[clickedFigures[i]]);
          }
        } 
        // if none - add *only* newCursorPos
        else {
          newCPs.push_back(new Point(newCursorPos));
          figures.push_back(newCPs[newCPs.size() - 1]);
        }
        
        for (int i = 0; i < newCPs.size(); i++) {
          curves[selectedCurveIdx]->AddControlPoint(newCPs[i]);
        }
      }
    }
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    clickingOutCurve = false;
}

// other functions
void recalculateSelected(bool deleting) {
  // resolve center model matrix
  if (selected.size() > 1 && !deleting) {
    for (int i = 0; i < selected.size(); i++) {
      figures[selected[i]]->SavePivotTransformations();
    }
  }
  centerTranslation = glm::vec3(0.f);
  centerScale = glm::vec3(1.f);
  centerAngle = glm::vec3(0.f);
  // resolve vertex deletion in regards to bezier curve
  updateCurvesSelectedChange();
  // find selected figures
  selected.clear();
  for (int i = 0; i < figures.size(); i++) {
    if (figures[i]->selected)
      selected.push_back(i);
  }
  recalculateCenter();
}

void updateCurvesSelectedChange(bool deleting) {
  for (int i = 0; i < selected.size(); i++) {
    for (int j = 0; j < curves.size(); j++) {
      std::vector<Figure*> points = curves[j]->GetControlPoints();
      int deleted = 0;
      for (int k = 0; k < points.size(); k++) {
        if (figures[selected[i]] == points[k]) {
          if (deleting) {
            curves[j]->RemoveControlPoint(k + deleted);
          } else {
            // 'else' because otherwise we would refresh buffors twice
            curves[j]->RefreshBuffers();
          }
        }
      }
    }
  }
}

void updateSurfacesSelectedChange()
{
	for (int i = 0; i < selected.size(); i++) {
		for (int j = 0; j < surfaces.size(); j++) {
			std::vector<Figure*> points = surfaces[j]->GetControlPoints();

			for (int k = 0; k < points.size(); k++) {
				if (figures[selected[i]] == points[k]) {
					surfaces[j]->RefreshBuffers();
                    break;
				}
			}
		}
	}
}

std::vector<int> GetClickedFigures(GLFWwindow *window) {
  std::vector<int> result;

  CAD::Sphere sphere;
  for (int i = 0; i < figures.size(); i++) {
    if (figures[i]->GetBoundingSphere(sphere)) {
      if (CAD::circleIntersections(
              sphere, camera->Position,
              CAD::calculateCameraRay(window, proj, view, camera))
              .size() > 0) {
        result.push_back(i);
      }
    }
  }
  return result;
}

void deselectCurve(bool deleting) {
  if (selectedCurveIdx != -1 && !deleting) {
    curves[selectedCurveIdx]->selected = false;
    if (curves[selectedCurveIdx]->GetControlPoints().size() == 0) {
      curves.erase(curves.begin() + selectedCurveIdx);
    }
  }
  selectedCurveIdx = -1;
  clickingOutCurve = false;
}

void curveCreation() {
  if (selectedCurveIdx != -1) {
    deselectCurve();
  }
  selectedCurveIdx = curves.size() - 1;
  curves[selectedCurveIdx]->selected = true;
  for (int i = 0; i < selected.size(); i++) {
    curves[selectedCurveIdx]->AddControlPoint(figures[selected[i]]);
  }
  if (selected.size() == 0) {
    // if curve was created from selected figures then
    // it doesnt get activated
    clickingOutCurve = true;
  } else {
    deselectFigures();
  }
}

void deselectFigures() {
  std::for_each(figures.begin(), figures.end(), [](Figure *f) {
    f->selected = false;
    ;
  });
  recalculateSelected();
}

void deselectSurfaces() 
{
  std::for_each(surfaces.begin(), surfaces.end(), [](Figure *p) {
    p->selected = false;;});
  selectedSurfaces.clear();
}

void recalculateCenter()
{
    glm::vec3 centerVec(0.f);
    for (int i = 0; i < selected.size(); i++) {
        centerVec += figures[selected[i]]->GetPosition();
    }
    centerVec /= selected.size();
    center->SetPosition(centerVec);
}

bool checkIfSelectedArePartOfSurface()
{
    for (int i = 0; i < selected.size(); i++) {
        for (int j = 0; j < surfaces.size(); j++) {
            std::vector<Figure*> points = surfaces[j]->GetControlPoints();

            for (int k = 0; k < points.size(); k++) {
                if (figures[selected[i]] == points[k]) {
                    return true;
                }
            }
        }
    }
    return false;
}

void recalculateSelectedSurfaces() {
  selectedSurfaces.clear();
  for (int i = 0; i < surfaces.size(); i++) {
    if (surfaces[i]->selected) {
      selectedSurfaces.push_back(i);
    }
  }
  std::vector<Graph*> graphs = {};
  for (int i = 0; i < selectedSurfaces.size(); i++) {
    graphs.push_back(surfaces[selectedSurfaces[i]]->ambit);
  }

  Graph *commonGraph = new Graph(graphs);
  std::set<std::vector<int>> foundCycles = commonGraph->findCyclesWithNVertices(3);
  cycles.clear();
  for (auto cycle : foundCycles) {
    cycles.push_back(new Polyline(new Graph(*commonGraph, cycle)));
  }
  if (cycles.size() > 0) {
    cycles[0]->selected = true;
  }
}

void loadScene() {
  // clear selection
  deselectFigures();
  deselectCurve();
  deselectSurfaces();
  recalculateCenter();

  // remove figures
  std::for_each(figures.begin(), figures.end(), [](Figure *f) { f->Delete(); });
  std::for_each(curves.begin(), curves.end(), [](Figure *c) { c->Delete(); });
  std::for_each(surfaces.begin(), surfaces.end(),
                [](Figure *p) { p->Delete(); });
  figures.clear();
  curves.clear();
  surfaces.clear();

  Figure::ZeroCounter();

  // load scene
  auto &scene = MG1::Scene::Get();

  std::vector<uint32_t> pointIds;

  auto findPointById = [&](MG1::PointRef ref) {
    bool found = false;
    for (int i = 0; i < pointIds.size(); i++) {
      if (pointIds[i] == ref.GetId()) {
        return figures[i];
        found = true;
        break;
      }
    }
    if (!found) {
      throw std::runtime_error("Invalid JSON");
    }
  };

  // import points
  for (auto &point : scene.points) {
    Point *p = new Point(CAD::deserializeVec3(point.position));
    p->name = point.name;
    figures.push_back(p);
    pointIds.push_back(point.GetId());
  }

  // import toruses
  for (auto &torus : scene.tori) {
    Torus *t =
        new Torus(CAD::deserializeVec3(torus.position), torus.largeRadius,
                  torus.smallRadius, torus.samples.x, torus.samples.y);
    t->name = torus.name;
    t->SetAngle(CAD::deserializeVec3(torus.rotation));
    t->SetScale(CAD::deserializeVec3(torus.scale));
    figures.push_back(t);
  }

  // import bezierC0
  for (auto &bezC0 : scene.bezierC0) {
    BezierC0 *b = new BezierC0(tessCpCountLoc, tessSegmentCountLoc,
                               tessSegmentIdxLoc, tessDivisionLoc);
    b->name = bezC0.name;
    for (auto &idx : bezC0.controlPoints) {
      b->AddControlPoint(findPointById(idx));
    }
    curves.push_back(b);
  }

  // import bezierC2
  for (auto &bezC2 : scene.bezierC2) {
    BezierC2 *b = new BezierC2(tessCpCountLoc, tessSegmentCountLoc,
                               tessSegmentIdxLoc, tessDivisionLoc);
    b->name = bezC2.name;
    for (auto &idx : bezC2.controlPoints) {
      b->AddControlPoint(findPointById(idx));
    }
    curves.push_back(b);
  }

  // import bezierInt
  for (auto &bezInt : scene.interpolatedC2) {
    BezierInt *b = new BezierInt(tessCpCountLoc, tessSegmentCountLoc,
                                 tessSegmentIdxLoc, tessDivisionLoc);
    b->name = bezInt.name;
    for (auto &idx : bezInt.controlPoints) {
      b->AddControlPoint(findPointById(idx));
    }
    curves.push_back(b);
  }

  // import surface C0
  for (auto &surfC0 : scene.surfacesC0) {
    SurfaceC0 *s = new SurfaceC0(glm::vec3(0.f));
    s->name = surfC0.name;

    std::vector<Figure*> cps = std::vector<Figure*>();
    for (auto &patch : surfC0.patches)
      for (auto &idx : patch.controlPoints)
        cps.push_back(findPointById(idx));

    s->CreateFromControlPoints(
        tessSurfaceCpCountLoc, tessSurfaceSegmentCountLoc,
        tessSurfaceSegmentIdxLoc, tessSurfaceDivisionLoc,
        tessSurfaceOtherAxisLoc, tessSurfaceBsplineLoc, cps);
    surfaces.push_back(s);
  }

  // import surface C2
  for (auto &surfC2 : scene.surfacesC2) {
    SurfaceC2 *s = new SurfaceC2(glm::vec3(0.f));
    
     std::vector<Figure*> cps = std::vector<Figure*>();
    for (auto &patch : surfC2.patches)
      for (auto &idx : patch.controlPoints)
        cps.push_back(findPointById(idx));

    s->CreateFromControlPoints(
        tessSurfaceCpCountLoc, tessSurfaceSegmentCountLoc,
        tessSurfaceSegmentIdxLoc, tessSurfaceDivisionLoc,
        tessSurfaceOtherAxisLoc, tessSurfaceBsplineLoc, cps);
    surfaces.push_back(s);
  }
}

void saveScene() 
{ 
    auto &scene = MG1::Scene::Get(); 
    scene.Clear();

    std::vector<std::tuple<int,uint32_t>> pointIds;
    for (int i = 0; i < figures.size(); i++)
    {
      int retVal = figures[i]->Serialize(scene);
      if (retVal != -1)
        pointIds.push_back(std::tuple <int, uint32_t>(i, retVal));
    }

    auto findPointIndexes = [&](std::vector<Figure*> cps) 
    {
      std::vector<uint32_t> cpIdxs;
      for (int i = 0; i < cps.size(); i++) {
        for (int j = 0; j < pointIds.size(); j++) {
          if (cps[i] == figures[std::get<0>(pointIds[j])]) {
            cpIdxs.push_back(std::get<1>(pointIds[j]));
            break;
          }
        }
      }
      return cpIdxs;
    };

    for (BezierC0* curv : curves)
      curv->Serialize(scene, findPointIndexes(curv->GetControlPoints()));
     
    for (SurfaceC0 *surf : surfaces) 
      surf->Serialize(scene, findPointIndexes(surf->GetControlPoints()));
}

void replaceCpsInCurves(Figure *newCp) {
  for (int i = 0; i < selected.size(); i++) {
    for (int j = 0; j < curves.size(); j++) {
      std::vector<Figure *> cps = curves[j]->GetControlPoints();
      for (int k = 0; k < cps.size(); k++) {
        if (figures[selected[i]] == cps[k]) {
          curves[j]->ReplaceControlPoint(k, newCp);
        }
      }
    }
  }
}

void replaceCpsInSurfaces(Figure *newCp) 
{
  for (int i = 0; i < selected.size(); i++) {
    for (int j = 0; j < surfaces.size(); j++) {
      std::vector<Figure *> cps = surfaces[j]->GetControlPoints();
      for (int k = 0; k < cps.size(); k++) {
        if (figures[selected[i]] == cps[k]) {
          surfaces[j]->ReplaceControlPoint(k, newCp);
        }
      }
    }
  }
}
