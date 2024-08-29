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
void deselectFigures();
void deselectSurface(bool deleting = false);
void recalculateCenter();

glm::vec3 centerTranslation(0.f);
glm::vec3 centerScale(1.f);
glm::vec3 centerAngle(0.f);
int cursorRadius = 5;

std::vector<BezierC0*> curves;
int selectedCurveIdx = -1;
bool clickingOutCurve = false;

const int initXsegments = 2;
const int initZsegments = 2;

const float initParam1 = 3;
const float initParam2 = 2;

std::vector<SurfaceC0*> surfaces;
int selectedSurfaceIdx = -1;
bool checkIfSelectedArePartOfSurface();

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
    Shader shaderProgram("default.vert", "default.frag");
    int modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
    int viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
    int projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
    int colorLoc = glGetUniformLocation(shaderProgram.ID, "color");

    Shader tessShaderProgram("tessellation.vert", "default.frag",
                             "tessellation.tesc", "tessellation.tese");
    int tessModelLoc = glGetUniformLocation(tessShaderProgram.ID, "model");
    int tessViewLoc = glGetUniformLocation(tessShaderProgram.ID, "view");
    int tessProjLoc = glGetUniformLocation(tessShaderProgram.ID, "proj");
    int tessColorLoc = glGetUniformLocation(tessShaderProgram.ID, "color");
    int tessCpCountLoc = glGetUniformLocation(tessShaderProgram.ID, "cpCount");
    int tessResolutionLoc =
        glGetUniformLocation(tessShaderProgram.ID, "resolution");
    int tessSegmentCountLoc =
        glGetUniformLocation(tessShaderProgram.ID, "segmentCount");
    int tessSegmentIdxLoc =
        glGetUniformLocation(tessShaderProgram.ID, "segmentIdx");
	int tessDivisionLoc =
		glGetUniformLocation(tessShaderProgram.ID, "division");

	Shader tessSurfaceShaderProgram("tessellation.vert", "default.frag",
		"tessellation.tesc", "tessellationSurface.tese");
    int tessSurfaceModelLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "model");
    int tessSurfaceViewLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "view");
    int tessSurfaceProjLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "proj");
    int tessSurfaceColorLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "color");
    int tessSurfaceCpCountLoc = glGetUniformLocation(tessSurfaceShaderProgram.ID, "cpCount");
    int tessSurfaceResolutionLoc =
        glGetUniformLocation(tessSurfaceShaderProgram.ID, "resolution");
    int tessSurfaceDivisionLoc =
        glGetUniformLocation(tessSurfaceShaderProgram.ID, "division");
    int tessSurfaceSegmentCountLoc =
        glGetUniformLocation(tessSurfaceShaderProgram.ID, "segmentCount");
    int tessSurfaceSegmentIdxLoc =
        glGetUniformLocation(tessSurfaceShaderProgram.ID, "segmentIdx");
	int tessSurfaceOtherAxisLoc =
		glGetUniformLocation(tessSurfaceShaderProgram.ID, "otherAxis");

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

    // matrices locations
    camera->PrepareMatrices(view, proj);

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

        // default shader activation
        shaderProgram.Activate();

        // matrices for default shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        // objects rendering with default shader
        grid->Render(colorLoc, modelLoc);
        for (int i = 0; i < figures.size(); i++) {
          figures[i]->Render(colorLoc, modelLoc);
        }
        cursor->Render(colorLoc, modelLoc);
        if (selected.size() > 0) {
          center->Render(colorLoc, modelLoc);
        }
        // render curves polyline with default shader (still)
        for (int i = 0; i < curves.size(); i++) {
          if (curves[i]->selected) {
            curves[i]->RenderPolyline(colorLoc, modelLoc);
          }
        }
		// render surfaces with default shader
		for (int i = 0; i < surfaces.size(); i++) 
			surfaces[i]->Render(colorLoc, modelLoc);

        // tessellation shader activation
        tessShaderProgram.Activate();

        // matrices for tessellation shader (with workaround)
        glm::mat4 tessView = glm::mat4(view);
        glm::mat4 tessProj = glm::mat4(proj);
        glUniformMatrix4fv(tessViewLoc, 1, GL_FALSE, glm::value_ptr(tessView));
        glUniformMatrix4fv(tessProjLoc, 1, GL_FALSE, glm::value_ptr(tessProj));
        glUniform2i(tessResolutionLoc, camera->GetWidth(), camera->GetHeight());

        // curves rendering with tessellation shader
        for (int i = 0; i < curves.size(); i++) {
          curves[i]->Render(tessColorLoc, tessModelLoc);
        }

		// surface tessellation shader activation
		tessSurfaceShaderProgram.Activate();

		// matrices for surface tessellation shader (with workaround)
		glm::mat4 tessSurfaceView = glm::mat4(view);
		glm::mat4 tessSurfaceProj = glm::mat4(proj);
		glUniformMatrix4fv(tessSurfaceViewLoc, 1, GL_FALSE, glm::value_ptr(tessSurfaceView));
		glUniformMatrix4fv(tessSurfaceProjLoc, 1, GL_FALSE, glm::value_ptr(tessSurfaceProj));
		glUniform2i(tessSurfaceResolutionLoc, camera->GetWidth(), camera->GetHeight());

		// surfaces rendering with surface tessellation shader
		for (int i = 0; i < surfaces.size(); i++)
			surfaces[i]->RenderTess(tessColorLoc, tessModelLoc);

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
                    std::vector<Figure*> newFigures = plane->CalculatePlane(tessCpCountLoc, tessSegmentCountLoc, tessSegmentIdxLoc, tessDivisionLoc, tessSurfaceOtherAxisLoc, PxSegments, PzSegments, Pparam1, Pparam2);
                    for (int i = 0; i < newFigures.size(); i++) {
                        figures.push_back(newFigures[i]);
                    }
                    surfaces.push_back(plane);
                    selectedSurfaceIdx = surfaces.size() - 1;
                    surfaces[selectedSurfaceIdx]->selected = true;

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
                    std::vector<Figure*> newFigures = cylinder->CalculateCylinder(tessCpCountLoc, tessSegmentCountLoc, tessSegmentIdxLoc, tessDivisionLoc, tessSurfaceOtherAxisLoc, PxSegments, PzSegments, Pparam1, Pparam2);
                    for (int i = 0; i < newFigures.size(); i++) {
                        figures.push_back(newFigures[i]);
                    }
                    surfaces.push_back(cylinder);
                    selectedSurfaceIdx = surfaces.size() - 1;
                    surfaces[selectedSurfaceIdx]->selected = true;

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
				      &surfaces[i]->selected)) {
				      bool temp = surfaces[i]->selected;
				      std::for_each(surfaces.begin(), surfaces.end(), [](Figure* f) {
					      f->selected = false;; });
				      surfaces[i]->selected = temp;

				      selectedSurfaceIdx = temp ? i : -1;
                      recalculateSelected();
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
                    clickingOutCurve = temp;
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
          if (selected.size() > 0 || selectedCurveIdx != -1 || selectedSurfaceIdx != -1) {
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
				  if (selectedSurfaceIdx != -1) {
					  surfaces.erase(surfaces.begin() + selectedSurfaceIdx);
                      deselectSurface(true);
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

          if (((selectedCurveIdx != -1) != (selectedSurfaceIdx != -1)) && selected.size() == 0) {
              // delete complex figure with all its control points
			  ImGui::SameLine();
			  if (ImGui::Button("Delete with cps")) {
                  std::vector<Figure*> cpsToDelete;
				  if (selectedCurveIdx != -1) {
					  cpsToDelete = curves[selectedCurveIdx]->GetControlPoints();
					  curves.erase(curves.begin() + selectedCurveIdx);
					  deselectCurve(true);
				  }
				  if (selectedSurfaceIdx != -1) {
					  cpsToDelete = surfaces[selectedSurfaceIdx]->GetControlPoints();
					  surfaces.erase(surfaces.begin() + selectedSurfaceIdx);
                      deselectSurface(true);
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
				  if (selectedSurfaceIdx != -1) {
					  std::vector<Figure*> cps = surfaces[selectedSurfaceIdx]->GetControlPoints();
					  for (int i = 0; i < cps.size(); i++) {
						  cps[i]->selected = true;
					  }
                      deselectSurface();
				  }
				  recalculateSelected();
			  }
		  }

		  if (selected.size() != 0 || selectedCurveIdx != -1 || selectedSurfaceIdx != -1) {
              ImGui::SameLine();
			  if (ImGui::Button("Deselect all")) {
				  deselectFigures();
                  deselectCurve();
                  deselectSurface();
			  }
		  }

          // add points to curve button
          if (selected.size() > 0 && selectedCurveIdx != -1 && selectedSurfaceIdx == -1) {
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
          if (selected.size() == 1 && selectedCurveIdx == -1 && selectedSurfaceIdx == -1) 
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
          if (selected.size() == 0 && selectedCurveIdx != -1 && selectedSurfaceIdx == -1 && currentMenuItem != 4) 
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

          if (selected.size() == 0 && selectedCurveIdx == -1 && selectedSurfaceIdx != -1) {
              ImGui::Separator();
              // change name window
              ImGui::InputText("Change name", &surfaces[selectedSurfaceIdx]->name);
              // display selected surface menu
              surfaces[selectedSurfaceIdx]->CreateImgui();
          }

          // multiple figures manipulation
          if (selected.size() > 1 && selectedCurveIdx == -1 && selectedSurfaceIdx == -1) {
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
                    return;
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

void deselectSurface(bool deleting)
{
	if (selectedSurfaceIdx != -1 && !deleting) {
		surfaces[selectedSurfaceIdx]->selected = false;
	}
	selectedSurfaceIdx = -1;
	recalculateSelected();
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
