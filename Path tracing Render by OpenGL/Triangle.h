#pragma once
#include"Common.h"

struct Triangle {
    vec3 p1, p2, p3;    // 顶点坐标
    vec3 n1, n2, n3;    // 顶点法线
    Material material;  // 材质
};


class XZRect {
public:
    float x0,x1, z0, z1,k;
    std::vector<vec3> rect_vertices;
    std::vector<int> rect_indices;
public:
    //use two point on that plane
    XZRect(const float& x0, const float& x1, const float& z0, const float& z1, const float& k)
        :x0(x0), x1(x1), z0(z0), z1(z1), k(k)
    {
        rect_vertices.push_back(vec3(x0, k, z0));
        rect_vertices.push_back(vec3(0, 1, 0));

        rect_vertices.push_back(vec3(x1, k, z0));
        rect_vertices.push_back(vec3(0, 1, 0));

        rect_vertices.push_back(vec3(x1, k, z1));
        rect_vertices.push_back(vec3(0, 1, 0));

        rect_vertices.push_back(vec3(x0, k, z1));
        rect_vertices.push_back(vec3(0, 1, 0));
        rect_indices = { 0,1,2,0,2,3 };
    }

    void loadTriangles(std::vector<Triangle>& triangles, Material material);
};

class XYRect {
public:
    float x0, x1, y0, y1,k;
    std::vector<vec3> rect_vertices;
    std::vector<int> rect_indices;
public:
    XYRect(const float& x0, const float& x1, const float& y0, const float& y1, const float& k)
        :x0(x0), x1(x1), y0(y0), y1(y1), k(k)
    {
        rect_vertices.push_back(vec3(x0, y0, k));
        rect_vertices.push_back(vec3(0, 0, 1));

        rect_vertices.push_back(vec3(x1, y0, k));
        rect_vertices.push_back(vec3(0, 0, 1));

        rect_vertices.push_back(vec3(x1, y1, k));
        rect_vertices.push_back(vec3(0, 0, 1));

        rect_vertices.push_back(vec3(x0, y1, k));
        rect_vertices.push_back(vec3(0, 0, 1));
        rect_indices = { 0,1,2,0,2,3 };
    }

    void loadTriangles(std::vector<Triangle>& triangles, Material material);
};

class YZRect {
public:
    float y0, y1, z0, z1,k;
    std::vector<vec3> rect_vertices;
    std::vector<int> rect_indices;
public:
    YZRect(const float& y0, const float& y1, const float& z0, const float& z1, const float& k)
        :y0(y0), y1(y1), z0(z0), z1(z1), k(k)
    {
        rect_vertices.push_back(vec3(k, y0, z0));
        rect_vertices.push_back(vec3(1, 0, 0));

        rect_vertices.push_back(vec3(k, y1, z0));
        rect_vertices.push_back(vec3(1, 0, 0));

        rect_vertices.push_back(vec3(k, y1, z1));
        rect_vertices.push_back(vec3(1, 0, 0));

        rect_vertices.push_back(vec3(k, y0, z1));
        rect_vertices.push_back(vec3(1, 0, 0));
        rect_indices = { 0,1,2,0,2,3 };
    }

    void loadTriangles(std::vector<Triangle>& triangles, Material material);
};

class Box {

public:
    Box(const vec3& p0, const vec3& p1, std::vector<Triangle>& triangles, Material material) {
        XZRect xz_rect(p0.x, p1.x, p0.z, p1.z, p0.y);
        XZRect xz_rect1(p0.x, p1.x, p0.z, p1.z, p1.y);
        
        XYRect xy_rect(p0.x, p1.x, p0.y, p1.y, p1.z);
        XYRect xy_rect1(p0.x, p1.x, p0.y, p1.y, p0.z);

        YZRect yz_rect(p0.y, p1.y, p0.z, p1.z, p1.x);
        YZRect yz_rect1(p0.y, p1.y, p0.z, p1.z, p0.x);

        xz_rect.loadTriangles(triangles, material);
        xz_rect1.loadTriangles(triangles, material);

        xy_rect.loadTriangles(triangles, material);
        xy_rect1.loadTriangles(triangles, material);

        yz_rect.loadTriangles(triangles, material);
        yz_rect1.loadTriangles(triangles, material);
    }
    
};

void XZRect::loadTriangles(std::vector<Triangle>& triangles, Material material) {
    bool smoothNormal = false;
    std::vector<vec3> vertices;
    std::vector<GLuint> indices;
    std::vector<vec3> normals;

    float x, y, z;
    float u, v, w;
    for (int i = 0; i < rect_vertices.size(); i += 2) {
        
        vertices.push_back(rect_vertices[i]);
        normals.push_back(rect_vertices[i + 1]);
    }

    for (int i = 0; i < rect_indices.size(); i += 3) {
        indices.push_back(rect_indices[i]);
        indices.push_back(rect_indices[i + 1]);
        indices.push_back(rect_indices[i + 2]);
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

        //std::cout << indices[i] << " " << indices[i+1] << " " << indices[i+2] << std::endl;
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

void XYRect::loadTriangles(std::vector<Triangle>& triangles, Material material) {
    bool smoothNormal = false;
    std::vector<vec3> vertices;
    std::vector<GLuint> indices;
    std::vector<vec3> normals;

    float x, y, z;
    float u, v, w;
    for (int i = 0; i < rect_vertices.size(); i += 2) {

        vertices.push_back(rect_vertices[i]);
        normals.push_back(rect_vertices[i + 1]);
    }

    for (int i = 0; i < rect_indices.size(); i += 3) {
        indices.push_back(rect_indices[i]);
        indices.push_back(rect_indices[i + 1]);
        indices.push_back(rect_indices[i + 2]);
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

        //std::cout << indices[i] << " " << indices[i+1] << " " << indices[i+2] << std::endl;
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

void YZRect::loadTriangles(std::vector<Triangle>& triangles, Material material) {
    bool smoothNormal = false;
    std::vector<vec3> vertices;
    std::vector<GLuint> indices;
    std::vector<vec3> normals;

    float x, y, z;
    float u, v, w;
    for (int i = 0; i < rect_vertices.size(); i += 2) {

        vertices.push_back(rect_vertices[i]);
        normals.push_back(rect_vertices[i + 1]);
    }

    for (int i = 0; i < rect_indices.size(); i += 3) {
        indices.push_back(rect_indices[i]);
        indices.push_back(rect_indices[i + 1]);
        indices.push_back(rect_indices[i + 2]);
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

        //std::cout << indices[i] << " " << indices[i+1] << " " << indices[i+2] << std::endl;
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

