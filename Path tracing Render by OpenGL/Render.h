#pragma once
#include "Common.h"

#include <vector>
#include <string>

class Render {

};

class RenderPass : Render {
public:
    unsigned int FBO = 0;
    unsigned int vao, vbo;
    std::vector<unsigned int> colorAttachments;
    unsigned int program;
    int width = 960;
    int height = 540;
public:
    RenderPass(const unsigned int& width, const unsigned int& height) 
        :width(width), height(height) {}
    
    void bindData(bool finalPass = false) {
        if (!finalPass) glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        std::vector<vec3> square = { vec3(-1, -1, 0), vec3(1, -1, 0), vec3(-1, 1, 0), vec3(1, 1, 0), vec3(-1, 1, 0), vec3(1, -1, 0) };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * square.size(), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * square.size(), &square[0]);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);   // layout (location = 0) 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
        // ���� finalPass ������֡�������ɫ����
        if (!finalPass) {
            std::vector<unsigned int> attachments;
            for (int i = 0; i < colorAttachments.size(); i++) {
                glBindTexture(GL_TEXTURE_2D, colorAttachments[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachments[i], 0);// ����ɫ����󶨵� i ����ɫ����
                attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
            }
            glDrawBuffers(attachments.size(), &attachments[0]);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void draw(std::vector<unsigned int> texPassArray = {}) {
        glUseProgram(program);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindVertexArray(vao);
        // ����һ֡��֡������ɫ����
        for (int i = 0; i < texPassArray.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texPassArray[i]);
            std::string uName = "texPass" + std::to_string(i);
            glUniform1i(glGetUniformLocation(program, uName.c_str()), i);
        }
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }
};