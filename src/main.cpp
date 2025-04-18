#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "ImGuizmo.h"
#include "Shader.h"
#include "Model.h"
#include <iostream>

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main() {
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "3D Slicer", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui::StyleColorsDark();

    // Load resources
    Shader shader("../resources/shaders/shader.vert", "../resources/shaders/shader.frag");
    Model model("../resources/models/viking_room.obj");

    // Create white fallback texture
    unsigned int whiteTex;
    glGenTextures(1, &whiteTex);
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    unsigned char whitePixel[3] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Setup FBO objects
    unsigned int fbo = 0, colorTex = 0, rbo = 0;
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &colorTex);
    glGenRenderbuffers(1, &rbo);

    glEnable(GL_DEPTH_TEST);

    // State
    glm::vec3 rotation(0.0f);
    float scale = 1.0f;

    // ImGuizmo state
    static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
    static ImGuizmo::MODE currentMode = ImGuizmo::WORLD;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        // Main DockSpace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("MainDockHost", nullptr, host_flags);
        ImGui::PopStyleVar(2);
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        // Slicer panel
        ImGui::Begin("Slicer");
        ImGui::SliderFloat3("Rotation (°)", glm::value_ptr(rotation), -180.0f, 180.0f);
        ImGui::SliderFloat("Scale", &scale, 0.1f, 5.0f);
        ImGui::End();

        // Scene panel (no padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene");
        ImVec2 avail = ImGui::GetContentRegionAvail();
        int width = (int)avail.x;
        int height = (int)avail.y;
        static int prevW = 0, prevH = 0;
        if (width != prevW || height != prevH) {
            prevW = width; prevH = height;
            glBindTexture(GL_TEXTURE_2D, colorTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Render scene into FBO
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteTex);
        shader.setInt("texture_diffuse1", 0);

        float dist = model.radius * 2.5f;
        glm::mat4 view = glm::lookAt(glm::vec3(0, model.radius, dist), glm::vec3(0), glm::vec3(0, 1, 0));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, dist * 3.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", proj);

        glm::mat4 M(1.0f);
        M = glm::translate(M, -model.center);
        M = glm::scale(M, glm::vec3(scale));
        M = glm::rotate(M, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        M = glm::rotate(M, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        M = glm::rotate(M, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        shader.setMat4("model", M);
        model.Draw(shader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Draw the rendered texture
        ImGui::Image((ImTextureID)(intptr_t)colorTex, ImVec2(avail.x, avail.y), ImVec2(0, 1), ImVec2(1, 0));

        // ImGuizmo integration:
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();
        ImGuizmo::SetRect(winPos.x, winPos.y, winSize.x, winSize.y);

        float viewMat[16], projMat[16], modelMat[16];
        memcpy(viewMat, glm::value_ptr(view),  sizeof(viewMat));
        memcpy(projMat, glm::value_ptr(proj),  sizeof(projMat));
        memcpy(modelMat, glm::value_ptr(M),     sizeof(modelMat));

        // Operation shortcuts
        if (ImGui::IsKeyPressed(ImGuiKey_T, false)) currentOp = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R, false)) currentOp = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_S, false)) currentOp = ImGuizmo::SCALE;

        // Manipulate and apply
        if (ImGuizmo::Manipulate(viewMat, projMat, currentOp, currentMode, modelMat, nullptr, nullptr)) {
            float t[3], r[3], s[3];
            ImGuizmo::DecomposeMatrixToComponents(modelMat, t, r, s);
            rotation.x = r[0];
            rotation.y = r[1];
            rotation.z = r[2];
            scale      = s[0];
        }

        ImGui::PopStyleVar();
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}