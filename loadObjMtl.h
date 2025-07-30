#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>

struct Mesh {
    std::vector<float> vertices; // x,y,z, u,v, nx,ny,nz
    std::vector<unsigned int> indices;
};

struct Material {
    float kd[3] = {1,1,1};
    std::string texPath;
};

inline Mesh loadObjMtl(const char* objPath, std::unordered_map<std::string, Material>& materials, const char* baseDir = "") {
    Mesh mesh;
    std::vector<Material> usedMaterials;
    std::vector<float> pos, uv, norm;
    std::vector<std::tuple<int,int,int>> faceData;
    std::vector<int> faceMat;

    std::ifstream in(objPath);
    std::string line, currentMat;
    while (std::getline(in,line)) {
        std::istringstream iss(line);
        std::string type; iss >> type;
        if (type=="mtllib") {
            std::string mtl; iss >> mtl;
            std::ifstream m(std::string(baseDir) + mtl);
            std::string l;
            Material mat;
            std::string name;
            while (std::getline(m,l)) {
                std::istringstream im(l);
                std::string t; im >> t;
                if (t=="newmtl") { if (!currentMat.empty()) materials[currentMat] = mat;
                    im >> name; currentMat = name; mat = Material();
                } else if (t=="Kd") { im >> mat.kd[0] >> mat.kd[1] >> mat.kd[2]; }
                else if (t=="map_Kd") { im >> mat.texPath; }
            }
            if (!currentMat.empty()) materials[currentMat] = mat;
        } else if (type=="usemtl") {
            iss >> currentMat;
        } else if (type=="v") {
            float x,y,z; iss>>x>>y>>z;
            pos.insert(pos.end(),{x,y,z});
        } else if (type=="vt") {
            float u,v; iss>>u>>v;
            uv.insert(uv.end(),{u,v});
        } else if (type=="vn") {
            float nx,ny,nz; iss>>nx>>ny>>nz;
            norm.insert(norm.end(),{nx,ny,nz});
        } else if (type=="f") {
            std::string a,b,c;
            iss>>a>>b>>c;
            auto parseIdx=[&](const std::string& s){
                int v=-1, t=-1, n=-1;
                sscanf(s.c_str(), "%d/%d/%d",&v,&t,&n);
                return std::make_tuple(v-1, t-1, n-1);
            };
            faceData.push_back(parseIdx(a));
            faceData.push_back(parseIdx(b));
            faceData.push_back(parseIdx(c));
            faceMat.push_back(currentMat.empty()? -1 : std::distance(materials.begin(), materials.find(currentMat)));
        }
    }

    // build interleaved buffer
    std::unordered_map<std::string,int> matIdMap;
    int idx=0;
    std::map<std::tuple<int,int,int>, int> uniqueMap;
int idx=0;
for (size_t i=0; i<faceData.size(); ++i) {
    auto t = faceData[i];
    if (uniqueMap.count(t) == 0) {
        uniqueMap[t] = idx++;
        auto [vi, ti, ni] = t;
        mesh.vertices.push_back(pos[vi*3+0]);
        mesh.vertices.push_back(pos[vi*3+1]);
        mesh.vertices.push_back(pos[vi*3+2]);
        if (ti >= 0) {
            mesh.vertices.push_back(uv[ti*2+0]);
            mesh.vertices.push_back(uv[ti*2+1]);
        } else mesh.vertices.insert(mesh.vertices.end(), {0,0});
        if (ni >= 0) {
            mesh.vertices.push_back(norm[ni*3+0]);
            mesh.vertices.push_back(norm[ni*3+1]);
            mesh.vertices.push_back(norm[ni*3+2]);
        } else mesh.vertices.insert(mesh.vertices.end(), {0,0,1});
    }
    mesh.indices.push_back(uniqueMap[t]);
}


    return mesh;
}
