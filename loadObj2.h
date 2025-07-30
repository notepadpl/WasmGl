#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

struct Mesh {
    std::vector<float> vertices; // x,y,z
    std::vector<unsigned int> indices;
};

Mesh loadObjSimple(const std::string& path) {
    Mesh mesh;
    std::vector<float> positions;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Nie udało się otworzyć pliku: " << path << std::endl;
        return mesh;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        else if (prefix == "f") {
            std::string token;
            std::vector<unsigned int> faceIndices;

            while (iss >> token) {
                std::istringstream tokenStream(token);
                std::string vertexIndexStr;
                std::getline(tokenStream, vertexIndexStr, '/');

                unsigned int vertexIndex = std::stoi(vertexIndexStr);
                faceIndices.push_back(vertexIndex - 1); // OBJ is 1-indexed
            }

            if (faceIndices.size() == 3) {
                mesh.indices.push_back(faceIndices[0]);
                mesh.indices.push_back(faceIndices[1]);
                mesh.indices.push_back(faceIndices[2]);
            }
            else if (faceIndices.size() == 4) {
                // fan triangulation for quads
                mesh.indices.push_back(faceIndices[0]);
                mesh.indices.push_back(faceIndices[1]);
                mesh.indices.push_back(faceIndices[2]);

                mesh.indices.push_back(faceIndices[0]);
                mesh.indices.push_back(faceIndices[2]);
                mesh.indices.push_back(faceIndices[3]);
            }
        }
    }

    mesh.vertices = std::move(positions);
    std::cout << "Wczytano " << mesh.vertices.size() / 3 << " wierzchołków i "
              << mesh.indices.size() / 3 << " trójkątów." << std::endl;
    return mesh;
}
