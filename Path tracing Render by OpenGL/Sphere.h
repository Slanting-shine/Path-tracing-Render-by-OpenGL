#pragma once
#include<vector>
#include"Material.h"
#include "Common.h"
#include "Triangle.h"

class Sphere 
{
public:
    //segement is for LOD
	Sphere(const glm::vec3& position, const float& radius, const unsigned int& segment);

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, segment * segment * 6, GL_UNSIGNED_INT, 0);
    }
    void loadTriangles(std::vector<Triangle>& triangles, Material material);
public:
    const unsigned int segment;
	const float PI = 3.14159265358979323846f;
    unsigned int VBO, VAO;
    std::vector<float> sphereVertices;
    std::vector<int> sphereIndices;
};


Sphere::Sphere(const glm::vec3& position, const float& radius, const unsigned int& segment)
    :segment(segment) 
{
   
    /*2-计算球体顶点*/
    //生成球的顶点
    for (int y = 0; y <= segment; y++)
    {
        for (int x = 0; x <= segment; x++)
        {
            float xSegment = (float)x / (float)segment;
            float ySegment = (float)y / (float)segment;
            float xPos = position.x + radius * std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = position.y + radius * std::cos(ySegment * PI);
            float zPos = position.z + radius * std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);


            /*生成顶点法向量*/
            float A = 2 * pow(PI, 2) * pow(radius, 2) * std::sin(PI * ySegment);
            float xNormal = A * std::cos(xSegment * 2. * PI) * std::sin(ySegment * PI);
            float yNormal = A * std::cos(ySegment * PI);
            float zNormal = A * sin(xSegment * 2 * PI) * std::sin(ySegment * PI);
            
            sphereVertices.push_back(xNormal);
            sphereVertices.push_back(yNormal);
            sphereVertices.push_back(zNormal);
        }
    }
    

    //生成球的Indices
    for (int i = 0; i < segment; i++)
    {
        for (int j = 0; j < segment; j++)
        {
            sphereIndices.push_back(i * (segment + 1) + j);
            sphereIndices.push_back((i + 1) * (segment + 1) + j);
            sphereIndices.push_back((i + 1) * (segment + 1) + j + 1);
            sphereIndices.push_back(i * (segment + 1) + j);
            sphereIndices.push_back((i + 1) * (segment + 1) + j + 1);
            sphereIndices.push_back(i * (segment + 1) + j + 1);

            //for (int k = 0; k < sphereIndices.size(); k += 3) {
            //    std::cout << sphereIndices[k] << " " << sphereIndices[k+1] << " " << sphereIndices[k+2] << std::endl;
            //}
            //std::cout<<std::endl;
        }
    }


    //for (int k = 0; k < sphereindices.size(); k += 3) {
    //    std::cout << sphereindices[k] << " " << sphereindices[k + 1] << " " << sphereindices[k + 2] << std::endl;
    //}

    /*3-数据处理*/
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //生成并绑定球体的VAO和VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //将顶点数据绑定至当前默认的缓冲中
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

    GLuint element_buffer_object;//EBO
    glGenBuffers(1, &element_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

    //设置顶点 position 属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal 属性指针
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //解绑VAO和VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Sphere::loadTriangles(std::vector<Triangle>& triangles, Material material) {
    bool smoothNormal = false;
    std::vector<vec3> vertices;
    std::vector<GLuint> indices;
    std::vector<vec3> normals;

    float x, y, z;
    float u, v, w;
    for (int i = 0; i < sphereVertices.size(); i += 6) {
        x = sphereVertices[i];
        y = sphereVertices[i+1];
        z = sphereVertices[i+2];
        vertices.push_back(vec3(x, y, z));
        u = sphereVertices[i + 3];
        v = sphereVertices[i + 4];
        w = sphereVertices[i + 5];
        normals.push_back(vec3(u, v, w));
    }

    for (int i = 0; i < sphereIndices.size(); i += 3) {
        indices.push_back(sphereIndices[i]);
        indices.push_back(sphereIndices[i+1]);
        indices.push_back(sphereIndices[i+2]);
    }


   

    // 生成法线
    
    for (int i = 0; i < indices.size(); i += 3) {
        vec3 p1 = vertices[indices[i]];
        vec3 p2 = vertices[indices[i + 1]];
        vec3 p3 = vertices[indices[i + 2]];
        vec3 n = normalize(cross(p2 - p1, p3 - p1));
        normals[indices[i]] = +n;
        normals[indices[i + 1]] = +n;
        normals[indices[i + 2]] = +n;
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



    /*for (int i = 0; i < sphereIndices.size();i+=3) {
        Triangle& t = triangles[offset + i / 3];
        t.p1 = vec3(sphereVertices[sphereIndices[i]], sphereVertices[sphereIndices[i] + 1], sphereVertices[sphereIndices[i] + 2]);
        t.n1 = vec3(sphereVertices[sphereIndices[i] + 3], sphereVertices[sphereIndices[i] + 4], sphereVertices[sphereIndices[i] + 5]);

        t.p2 = vec3(sphereVertices[sphereIndices[i + 1]], sphereVertices[sphereIndices[i + 1] + 1], sphereVertices[sphereIndices[i + 1] + 2]);
        t.n2 = vec3(sphereVertices[sphereIndices[i + 1] + 3], sphereVertices[sphereIndices[i + 1] + 4], sphereVertices[sphereIndices[i + 1] + 5]);

        t.p3 = vec3(sphereVertices[sphereIndices[i + 2]], sphereVertices[sphereIndices[i + 2] + 1], sphereVertices[sphereIndices[i + 2] + 2]);
        t.n3 = vec3(sphereVertices[sphereIndices[i + 2] + 3], sphereVertices[sphereIndices[i + 2] + 4], sphereVertices[sphereIndices[i + 2] + 5]);
        t.material = material;
    }*/

}