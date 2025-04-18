#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Shader.h"
#include "Model.h"

int main() {
    // Init GLFW+OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "3D Slicer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*,int w,int h){glViewport(0,0,w,h);});

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui::StyleColorsDark();

    Shader shader("../resources/shaders/shader.vert","../resources/shaders/shader.frag");
    Model model("../resources/models/viking_room.obj");

    // White texture for untextured meshes
    unsigned int whiteTex;
    glGenTextures(1,&whiteTex);
    glBindTexture(GL_TEXTURE_2D,whiteTex);
    unsigned char whitePixel[3]={255,255,255};
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,1,1,0,GL_RGB,GL_UNSIGNED_BYTE,whitePixel);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    // FBO
    unsigned int fbo,colorTex,rbo;
    glGenFramebuffers(1,&fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    glGenTextures(1,&colorTex);
    glBindTexture(GL_TEXTURE_2D,colorTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,800,600,0,GL_RGB,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,colorTex,0);
    glGenRenderbuffers(1,&rbo);
    glBindRenderbuffer(GL_RENDERBUFFER,rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,800,600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,rbo);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr,"FBO incomplete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    glEnable(GL_DEPTH_TEST);
    float sliceHeight=0.0f;

    while(!glfwWindowShouldClose(window)){
        // Render to FBO
        glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glViewport(0,0,800,600);
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        shader.use();
        // bind white texture to slot0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,whiteTex);
        shader.setInt("texture_diffuse1",0);

        glm::mat4 proj=glm::perspective(glm::radians(45.0f),800.0f/600.0f,0.1f,100.0f);
        glm::mat4 view=glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-5));
        shader.setMat4("projection",proj);
        shader.setMat4("view",view);
        shader.setMat4("model",glm::mat4(1.0f));
        shader.setVec4("slicingPlane",glm::vec4(0,1,0,-sliceHeight));
        model.Draw(shader);

        glBindFramebuffer(GL_FRAMEBUFFER,0);

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Slicer");
        ImGui::SliderFloat("Slice Height",&sliceHeight,-1.0f,1.0f);
        ImGui::Image((ImTextureID)colorTex,ImVec2(800,600),ImVec2(0,1),ImVec2(1,0));
        ImGui::End();
        ImGui::Render();

        int dw,dh;glfwGetFramebufferSize(window,&dw,&dh);
        glViewport(0,0,dw,dh);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
