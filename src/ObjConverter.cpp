#include "ObjConverter.h"
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stdexcept>

void ObjConverter::Convert(const std::string &inObjPath, const std::string &outStlPath)
{
    if (inObjPath.empty() || outStlPath.empty()) {
        throw std::invalid_argument("Input or output path is empty");
    }
    if (inObjPath == outStlPath) {
        throw std::invalid_argument("Input and output paths must be different");
    }
    if (inObjPath.substr(inObjPath.find_last_of('.') + 1) != "obj") {
        return; // Nothing to do if not an OBJ file
    }

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(inObjPath,
                                             aiProcess_Triangulate |
                                             aiProcess_GenSmoothNormals |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error(std::string("Assimp load error: ") + importer.GetErrorString());
    }

    Assimp::Exporter exporter;
    aiReturn exportRet = exporter.Export(scene, "stl", outStlPath, 0);
    if (exportRet != aiReturn_SUCCESS) {
        throw std::runtime_error(std::string("Assimp export error: ") + exporter.GetErrorString());
    }
}
