#pragma once
#include <vector>
#include <string>

struct Mesh {
    std::vector<float> vertices; // x,y,z
    std::vector<unsigned int> indices;
};

Mesh loadObjSimple(const std::string& path) {
    Mesh mesh;
    std::vector<float> positions;
    std::vector<unsigned int> faces;

    FILE* file = fopen(path.c_str(), "r");
    if (!file) return mesh;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            unsigned int i1, i2, i3;
            sscanf(line, "f %u %u %u", &i1, &i2, &i3);
            // OBJ jest 1-indexed
            mesh.indices.push_back(i1 - 1);
            mesh.indices.push_back(i2 - 1);
            mesh.indices.push_back(i3 - 1);
        }
    }
    fclose(file);

    mesh.vertices = std::move(positions);
    return mesh;
}
