#pragma once
#include "geometry.h"
#include <vector>
#include <string>
using namespace std;

class Model{
    private:
        vector<vec3> verts = {};
        vector<int> facet_vrt = {}; // 每个三角形存储的顶点的下标
    public:
        Model(){}
        Model(const string filename);
        int nverts() const ;
        int nfaces() const ;
        vec3 vert(const int i) const;
        vec3 vert(const int iface, const int nthvert) const;
};

