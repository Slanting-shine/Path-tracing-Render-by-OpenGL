#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <ctime>

#include "lib/hdrloader.h"
#include "Common.h"
#include "Sphere.h"
#include "Render.h"
#include "Camera.h"
#include "Shader.h"
#include "Material.h"
#include "Triangle.h"
#include "BVH.h"

//extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

using namespace glm;

unsigned int trianglesTextureBuffer;
unsigned int nodesTextureBuffer;
unsigned int lastFrame;
unsigned int hdrMap;
float fov = 45;
bool firstMouse;
// 绘制
clock_t t1, t2;
double dt, fps;
unsigned int frameCounter = 0;
float lastTime;
// 鼠标运动函数
double lastX = 0.0, lastY = 0.0;

const float aspect_ratio = 16 / 9;
// timing
float deltaTime = 0.0f;
// Window dimensions
const unsigned int WIDTH = 512, HEIGHT = 512;

Camera camera(fov, vec3(0.0, 0.0, 4.0), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
RenderPass pass1(WIDTH, HEIGHT);
RenderPass pass2(WIDTH, HEIGHT);
RenderPass pass3(WIDTH, HEIGHT);

struct Triangle_encoded {
    vec3 p1, p2, p3;    // 顶点坐标
    vec3 n1, n2, n3;    // 顶点法线
    vec3 emissive;      // 自发光参数
    vec3 baseColor;     // 颜色
    vec3 param1;        // (subsurface, metallic, specular)
    vec3 param2;        // (specularTint, roughness, anisotropic)
    vec3 param3;        // (sheen, sheenTint, clearcoat)
    vec3 param4;        // (clearcoatGloss, IOR, transmission)
    vec3 param5;        // (dielectric , reserve, reserve)
};

struct BVHNode_encoded {
    vec3 childs;        // (left, right, reserve)
    vec3 leafInfo;      // (n, index, reserve)
    vec3 AA, BB;
};

unsigned int getTextureRGB32F(int width, int height) {
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

// 模型变换矩阵
mat4 transformMatrix(vec3 rotateCtrl, vec3 translateCtrl, vec3 scaleCtrl) {
    glm::mat4 unit(    // 单位矩阵
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 0, 0, 1)
    );
    mat4 scale = glm::scale(unit, scaleCtrl);
    mat4 translate = glm::translate(unit, translateCtrl);
    mat4 rotate = unit;
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.x), glm::vec3(1, 0, 0));
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.y), glm::vec3(0, 1, 0));
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.z), glm::vec3(0, 0, 1));

    mat4 model = translate * rotate * scale;
    return model;
}

// 读取 obj
void readObj(std::string filepath, std::vector<Triangle>& triangles, Material material, mat4 trans, bool smoothNormal) {

    // 顶点位置，索引
    std::vector<vec3> vertices;
    std::vector<unsigned int> indices;

    // 打开文件流
    std::ifstream fin(filepath);
    std::string line;
    if (!fin.is_open()) {
        std::cout << "文件 " << filepath << " 打开失败" << std::endl;
        exit(-1);
    }

    // 计算 AABB 盒，归一化模型大小
    float maxx = -11451419.19;
    float maxy = -11451419.19;
    float maxz = -11451419.19;
    float minx = 11451419.19;
    float miny = 11451419.19;
    float minz = 11451419.19;

    // 按行读取
    while (std::getline(fin, line)) {
        std::istringstream sin(line);   // 以一行的数据作为 string stream 解析并且读取
        std::string type;
        GLfloat x, y, z;
        int v0, v1, v2;
        int vn0, vn1, vn2;
        int vt0, vt1, vt2;
        char slash;

        // 统计斜杆数目，用不同格式读取
        int slashCnt = 0;
        for (int i = 0; i < line.length(); i++) {
            if (line[i] == '/') slashCnt++;
        }

        // 读取obj文件
        sin >> type;
        if (type == "v") {
            sin >> x >> y >> z;
            vertices.push_back(vec3(x, y, z));
            maxx = max(maxx, x); maxy = max(maxx, y); maxz = max(maxx, z);
            minx = min(minx, x); miny = min(minx, y); minz = min(minx, z);
        }
        if (type == "f") {
            if (slashCnt == 6) {
                sin >> v0 >> slash >> vt0 >> slash >> vn0;
                sin >> v1 >> slash >> vt1 >> slash >> vn1;
                sin >> v2 >> slash >> vt2 >> slash >> vn2;
            }
            else if (slashCnt == 3) {
                sin >> v0 >> slash >> vt0;
                sin >> v1 >> slash >> vt1;
                sin >> v2 >> slash >> vt2;
            }
            else {
                sin >> v0 >> v1 >> v2;
            }
            indices.push_back(v0 - 1);
            indices.push_back(v1 - 1);
            indices.push_back(v2 - 1);
        }
    }

    // 模型大小归一化
    float lenx = maxx - minx;
    float leny = maxy - miny;
    float lenz = maxz - minz;
    float maxaxis = max(lenx, max(leny, lenz));

    /*for (auto& v : vertices) {
        v.x /= maxaxis;
        v.y /= maxaxis;
        v.z /= maxaxis;
    }*/

    // 通过矩阵进行坐标变换
    for (auto& v : vertices) {
        vec4 vv = vec4(v.x, v.y, v.z, 1);
        vv = trans * vv;
        v = vec3(vv.x, vv.y, vv.z);
    }

    // 生成法线
    std::vector<vec3> normals(vertices.size(), vec3(0, 0, 0));
    for (int i = 0; i < indices.size(); i += 3) {
        vec3 p1 = vertices[indices[i]];
        vec3 p2 = vertices[indices[i + 1]];
        vec3 p3 = vertices[indices[i + 2]];
        vec3 n = normalize(cross(p2 - p1, p3 - p1));
        normals[indices[i]] = n;
        normals[indices[i + 1]] = n;
        normals[indices[i + 2]] = n;
    }

    // 构建 Triangle 对象数组
    int offset = triangles.size();  // 增量更新
    triangles.resize(offset + indices.size() / 3);
    for (int i = 0; i < indices.size(); i += 3) {
        Triangle& t = triangles[offset + i / 3];
        // 传顶点属性
        t.p1 = vertices[indices[i]];
        t.p2 = vertices[indices[i + 1]];
        t.p3 = vertices[indices[i + 2]];
        if (!smoothNormal) {
            vec3 n = normalize(cross(t.p2 - t.p1, t.p3 - t.p1));
            t.n1 = n; t.n2 = n; t.n3 = n;
        }
        else {
            t.n1 = normalize(normals[indices[i]]);
            t.n2 = normalize(normals[indices[i + 1]]);
            t.n3 = normalize(normals[indices[i + 2]]);
        }

        // 传材质
        t.material = material;
    }
}

inline float randomFloat() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline float randomFloat(float min, float max) {
    // Returns a random real in [min,max).
    return min + (max - min) * randomFloat();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void GLCheckError() {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL error] (" << error << ")" << std::endl;
    }
    return;
}

int main()
{
    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }


    /*  scene config  */
    std::vector<Triangle> triangles;

    Material m;
    m.baseColor = vec3(0.48, 0.83, 0.35);
    m.roughness = 0.1;
    m.metallic = 0.5;
    m.clearcoat = 1.0;
    m.subsurface = 1.0;
    //m.clearcoatGloss = 0.05;
    //readObj("models/Stanford Bunny.obj", triangles, m, transformMatrix(vec3(0, 0, 0), vec3(0, 1.5, 0), vec3(2.5, 2.5, 2.5)), true);


    m.baseColor = vec3(0.8, 0.8, 0.8);
    m.roughness = 0.3;
    m.specular = 0.7;
    m.metallic = 0.3;
    m.clearcoat = 0.0;
    m.dielectric = 1.5;
    //Sphere sphere(vec3(0.0f, 0.85f, 0.0f), 0.8f, 500);
   // sphere.loadTriangles(triangles, m);

    m.emissive = vec3(100.0f);
    m.dielectric = 0.0;
    //XZRect xz_rect(-0.5, 0.5, -0.5, 0.5, 3.85);
    //xz_rect.loadTriangles(triangles, m);
    //Box box(vec3(-0.5,0.85,-0.5), vec3(0.5,1.85,0.5), triangles, m);

    m.emissive = vec3(0);
    m.baseColor = vec3(0.48, 0.83, 0.35);
    m.roughness = 0.3;
    m.metallic = 0.2;
    m.clearcoat = 1.0;
    //m.subsurface = 1.0;

    /*const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 1.0;
            auto x0 = -10.0 + i * w;
            auto z0 = -10.0 + j * w;
            auto y0 = -2.0;
            auto x1 = x0 + w;
            auto y1 = randomFloat(-1, 0);
            auto z1 = z0 + w;

            Box box(vec3(x0, y0, z0), vec3(x1, y1, z1), triangles, m);
        }
    }*/
    //readObj("models/teapot.obj", triangles, m, transformMatrix(vec3(0, 0, 0), vec3(0, -0.4, 0), vec3(1.75, 1.75, 1.75)), true);
    //readObj("models/Stanford Bunny.obj", triangles, m, transformMatrix(vec3(0, 0, 0), vec3(0.4, -1.5, 0), vec3(2.5, 2.5, 2.5)), true);

    m.baseColor = vec3(0.5, 0.5, 1);
    m.metallic = 0.0;
    m.roughness = 0.1;
    m.clearcoat = 1.0;
    //m.emissive = vec3(10, 10, 20);
    //readObj("models/Stanford Bunny.obj", triangles, m, transformMatrix(vec3(0, 0, 0), vec3(3.4, -1.5, 0), vec3(2.5, 2.5, 2.5)), true);

    m.emissive = vec3(0, 0, 0);
    m.baseColor = vec3(0.725, 0.71, 0.68);
    m.baseColor = vec3(1, 1, 1);

    float len = 13.0;
    m.metallic = 0.7;
    m.roughness = 0.3;
    //readObj("models/quad.obj", triangles, m, transformMatrix(vec3(0, 0, 0), vec3(0, -1.5, 0), vec3(len, 0.01, len)), false);

    m.baseColor = vec3(1, 1, 1);
    m.emissive = vec3(20, 20, 20);
    //readObj("models/quad.obj", triangles, m, transformMatrix(vec3(0, 0, 0), vec3(0.0, 1.48, -0.0), vec3(1.7, 0.01, 1.7)), false);


    int nTriangles = triangles.size();
    std::cout << "模型读取完成: 共 " << nTriangles << " 个三角形" << std::endl;

    // 建立 bvh
    BVHNode testNode;
    testNode.left = 255;
    testNode.right = 128;
    testNode.n = 30;
    testNode.AA = vec3(1, 1, 0);
    testNode.BB = vec3(0, 1, 0);
    //std::vector<BVHNode> nodes{ testNode };
    BVH m_bvh;
    m_bvh.nodes.push_back(testNode);
    //buildBVH(triangles, nodes, 0, triangles.size() - 1, 8);
    m_bvh.buildBVHwithSAH(triangles, 0, triangles.size() - 1, 8);
    int nNodes = m_bvh.nodes.size();
    std::cout << "BVH 建立完成: 共 " << nNodes << " 个节点" << std::endl;

    // 编码 三角形, 材质
    std::vector<Triangle_encoded> triangles_encoded(nTriangles);
    for (int i = 0; i < nTriangles; i++) {
        Triangle& t = triangles[i];
        Material& m = t.material;
        // 顶点位置
        triangles_encoded[i].p1 = t.p1;
        triangles_encoded[i].p2 = t.p2;
        triangles_encoded[i].p3 = t.p3;
        // 顶点法线
        triangles_encoded[i].n1 = t.n1;
        triangles_encoded[i].n2 = t.n2;
        triangles_encoded[i].n3 = t.n3;
        // 材质
        triangles_encoded[i].emissive = m.emissive;
        triangles_encoded[i].baseColor = m.baseColor;
        triangles_encoded[i].param1 = vec3(m.subsurface, m.metallic, m.specular);
        triangles_encoded[i].param2 = vec3(m.specularTint, m.roughness, m.anisotropic);
        triangles_encoded[i].param3 = vec3(m.sheen, m.sheenTint, m.clearcoat);
        triangles_encoded[i].param4 = vec3(m.clearcoatGloss, m.IOR, m.transmission);
        triangles_encoded[i].param5 = vec3(m.dielectric, 0, 0);
    }

    // 编码 BVHNode, aabb
    std::vector<BVHNode_encoded> nodes_encoded(nNodes);
    for (int i = 0; i < nNodes; i++) {
        nodes_encoded[i].childs = vec3(m_bvh.nodes[i].left, m_bvh.nodes[i].right, 0);
        nodes_encoded[i].leafInfo = vec3(m_bvh.nodes[i].n, m_bvh.nodes[i].index, 0);
        nodes_encoded[i].AA = m_bvh.nodes[i].AA;
        nodes_encoded[i].BB = m_bvh.nodes[i].BB;
    }

    // 设置纹理缓冲写入三角形数据
    unsigned int tbo0;
    glGenBuffers(1, &tbo0);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo0);
    glBufferData(GL_TEXTURE_BUFFER, triangles_encoded.size() * sizeof(Triangle_encoded), &triangles_encoded[0], GL_STATIC_DRAW);
    glGenTextures(1, &trianglesTextureBuffer);
    glBindTexture(GL_TEXTURE_BUFFER, trianglesTextureBuffer);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo0);

    // 设置纹理缓冲写入BVHNode数据
    unsigned int tbo1;
    glGenBuffers(1, &tbo1);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo1);
    glBufferData(GL_TEXTURE_BUFFER, nodes_encoded.size() * sizeof(BVHNode_encoded), &nodes_encoded[0], GL_STATIC_DRAW);
    glGenTextures(1, &nodesTextureBuffer);
    glBindTexture(GL_TEXTURE_BUFFER, nodesTextureBuffer);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo1);

    // hdr 全景图
    HDRLoaderResult hdrRes;
    bool r = HDRLoader::load("./HDR/cayley_interior_4k.hdr", hdrRes);
    hdrMap = getTextureRGB32F(hdrRes.width, hdrRes.height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, hdrRes.width, hdrRes.height, 0, GL_RGB, GL_FLOAT, hdrRes.cols);

    // 管线配置
    Shader pass1_shader("./shaders/pass1.shader");
    pass1.program = pass1_shader.getProgramID();
    pass1.colorAttachments.push_back(getTextureRGB32F(pass1.width, pass1.height));
    pass1.colorAttachments.push_back(getTextureRGB32F(pass1.width, pass1.height));
    pass1.colorAttachments.push_back(getTextureRGB32F(pass1.width, pass1.height));
    pass1.bindData();

    pass1_shader.use();
    pass1_shader.setInt("nTriangles", triangles.size());
    pass1_shader.setInt("nNodes", m_bvh.nodes.size());
    pass1_shader.setInt("width", pass1.width);
    pass1_shader.setInt("height", pass1.height);
   
    Shader pass2_shader("./shaders/pass2.shader");
    pass2.program = pass2_shader.getProgramID();
    lastFrame = getTextureRGB32F(pass2.width, pass2.height);
    pass2.colorAttachments.push_back(lastFrame);
    pass2.bindData();

    Shader pass3_shader("./shaders/pass3.shader");
    pass3.program = pass3_shader.getProgramID();
    pass3.bindData(true);
    std::cout << "开始..." << std::endl << std::endl;

    glEnable(GL_DEPTH_TEST);  // 开启深度测试
    glClearColor(0.1, 0.1, 0.1, 1.0);   // 背景颜色 -- 黑
     // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        //processInput(window);
        glClear(GL_COLOR_BUFFER_BIT);
       
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        // 帧计时
        t2 = clock();
        dt = (double)(t2 - t1) / CLOCKS_PER_SEC;
        fps = 1.0 / dt;
        std::cout << '\r';
        std::cout << std::fixed << std::setprecision(2) << "FPS : " << fps << "    迭代次数: " << frameCounter;
        t1 = t2;
        
        mat4 cameraView = inverse(camera.GetViewMatrix());   // lookat 的逆矩阵将光线方向进行转换
      
        //set uniform for pass1
        glUseProgram(pass1.program);
        glUniform3fv(glGetUniformLocation(pass1.program, "eye"), 1, value_ptr(camera.position));
        glUniformMatrix4fv(glGetUniformLocation(pass1.program, "cameraRotate"), 1, GL_FALSE, value_ptr(cameraView));
        glUniform1ui(glGetUniformLocation(pass1.program, "frameCounter"), frameCounter++);// 传计数器用作随机种子

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, trianglesTextureBuffer);
        glUniform1i(glGetUniformLocation(pass1.program, "triangles"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, nodesTextureBuffer);
        glUniform1i(glGetUniformLocation(pass1.program, "nodes"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, lastFrame);
        glUniform1i(glGetUniformLocation(pass1.program, "lastFrame"), 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, hdrMap);
        glUniform1i(glGetUniformLocation(pass1.program, "hdrMap"), 3);

        // 绘制
        pass1.draw();
        pass2.draw(pass1.colorAttachments);
        pass3.draw(pass2.colorAttachments);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminates GLFW, clearing any resources allocated by GLFW.
    glDeleteProgram(pass1.program);
    glDeleteProgram(pass2.program);
    glDeleteProgram(pass3.program);
    GLCheckError();

    glfwTerminate();
    
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    frameCounter = 0;
    //std::cout << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)        
        camera.ProcessKeyboard(FORWARD, deltaTime);
  
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.mouse_sensitivity = 10.0;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera.mouse_sensitivity = 2.5;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    frameCounter = 0;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
