#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

// 加载模型就直接用教程的代码了！
Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Eigen::Vector3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "顶点个数： " << verts_.size() << " 三角形数："  << faces_.size() << std::endl;

    for (const auto &item: faces_){
        std::vector<Eigen::Vector3f> coordinate;
        for (int i = 0; i < 3; ++i){
            coordinate.push_back(verts_[item[i]]);
        }
        triangles.push_back(new Triangle(coordinate))
    }
}

Model::~Model() {
}
