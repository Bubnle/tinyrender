#include <fstream>
#include <sstream>
#include <algorithm>
#include "model.h"

using namespace std;

Model::Model(const string filename){
    ifstream in;
    // 打开方式
    in.open(filename, ifstream::in);
    string line;
    while(!in.eof()){
        getline(in, line);
        istringstream iss(line.c_str());
        char trash;
        // 顶点 赋值
        if(!line.compare(0, 2, "v ")){
            iss >> trash;
            vec3 v = {0, 0, 0};
            for(int i : {0,1,2}) iss >> v[i];
            verts.push_back(v);
        }
        // 面 赋值
        else if(!line.compare(0, 2, "f ")){
            int f,t,n;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
            }
        }
    }

    
}

int Model::nverts() const{
    return verts.size();
}

int Model::nfaces() const{
    return facet_vrt.size() / 3 ;
}

vec3 Model::vert(const int i) const{
    return verts[i];
}

vec3 Model::vert(const int iface, const int nthvert) const{
    return verts[facet_vrt[ iface* 3 + nthvert]] ;
}