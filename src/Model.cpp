#include "Model.h"
#include "glad/glad.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <algorithm>
#include <stdexcept>

Model::Model(const std::string& path){
    dir=path.substr(0,path.find_last_of('/'));
    load(path);
    computeBounds();
}
void Model::Draw(const Shader& sh) const{ for(auto& m:meshes)m.Draw(sh); }
void Model::load(const std::string& p){
    Assimp::Importer imp;
    const aiScene* sc=imp.ReadFile(p,aiProcess_Triangulate|aiProcess_FlipUVs);
    if(!sc||sc->mFlags&AI_SCENE_FLAGS_INCOMPLETE||!sc->mRootNode)
        throw std::runtime_error(imp.GetErrorString());
    procNode(sc->mRootNode,sc);
}
void Model::procNode(aiNode* n,const aiScene* s){
    for(unsigned i=0;i<n->mNumMeshes;++i)
        meshes.push_back(procMesh(s->mMeshes[n->mMeshes[i]],s));
    for(unsigned i=0;i<n->mNumChildren;++i)
        procNode(n->mChildren[i],s);
}
Mesh Model::procMesh(aiMesh* m,const aiScene* s){
    std::vector<Vertex> V; std::vector<unsigned> I; std::vector<Texture> T;
    for(unsigned i=0;i<m->mNumVertices;++i){
        Vertex v;
        v.pos={m->mVertices[i].x,m->mVertices[i].y,m->mVertices[i].z};
        v.norm=m->HasNormals()?glm::vec3(m->mNormals[i].x,m->mNormals[i].y,m->mNormals[i].z):glm::vec3(0);
        v.uv=m->mTextureCoords[0]?glm::vec2(m->mTextureCoords[0][i].x,m->mTextureCoords[0][i].y):glm::vec2(0);
        V.push_back(v);
    }
    for(unsigned i=0;i<m->mNumFaces;++i)
        for(unsigned j=0;j<m->mFaces[i].mNumIndices;++j)
            I.push_back(m->mFaces[i].mIndices[j]);
    if(m->mMaterialIndex>=0){
        aiMaterial* mat=s->mMaterials[m->mMaterialIndex];
        auto dif=loadTex(mat,aiTextureType_DIFFUSE,"texture_diffuse");
        T.insert(T.end(),dif.begin(),dif.end());
        auto sp=loadTex(mat,aiTextureType_SPECULAR,"texture_specular");
        T.insert(T.end(),sp.begin(),sp.end());
    }
    return Mesh(V,I,T);
}
std::vector<Texture> Model::loadTex(aiMaterial* mat,aiTextureType tt,const std::string& tn){
    std::vector<Texture> ret;
    for(unsigned i=0;i<mat->GetTextureCount(tt);++i){
        aiString str; mat->GetTexture(tt,i,&str);
        std::string p=dir+"/"+str.C_Str();
        Texture tex; tex.type=tn;
        glGenTextures(1,&tex.id);
        int w,h,nc;
        unsigned char* d=stbi_load("../../resources/models/viking_room.png",&w,&h,&nc,0);
        if(d){ GLenum fmt=nc==3?GL_RGB:GL_RGBA;
            glBindTexture(GL_TEXTURE_2D,tex.id);
            glTexImage2D(GL_TEXTURE_2D,0,fmt,w,h,0,fmt,GL_UNSIGNED_BYTE,d);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            stbi_image_free(d);
            tex.path=p;
            ret.push_back(tex);
        } else stbi_image_free(d);
    }
    return ret;
}
void Model::computeBounds(){
    glm::vec3 mn(FLT_MAX),mx(-FLT_MAX);
    for(auto& M:meshes){
        for(auto& v:M.getVertices()){
            mn=min(mn,v.pos);
            mx=max(mx,v.pos);
        }
    }
    center=(mn+mx)*0.5f;
    radius=glm::length(mx-center);
}
