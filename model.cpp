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

    // 建立映射关系
    vector<int> idx(nfaces());
    for(int i = 0; i< nfaces() ;i++){
        idx[i] = i;
    }

    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << std::endl;

    // 按照z深度对三角形排序；按照三角形当中z的最小值；升序排列
    sort(idx.begin(), idx.end(),
              [&](const int& a, const int& b) { // given two triangles, compare their min z coordinate
                  float aminz = std::min(vert(a, 0).z,
                                         std::min(vert(a, 1).z,
                                                  vert(a, 2).z));
                  float bminz = std::min(vert(b, 0).z,
                                         std::min(vert(b, 1).z,
                                                  vert(b, 2).z));
                  return aminz < bminz;
              });
    
    // 存储排序后的顶点坐标；因为面按照z最小值排序了
    vector<int> facet_vrt2(nfaces()*3); 
    for(int i = 0;i<nfaces();i++){
        for(int j = 0;j <3 ;j++){
            facet_vrt2[i*3 + j] = facet_vrt[idx[i]*3 + j];
        }
    }
    facet_vrt = std::move(facet_vrt2);
    
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