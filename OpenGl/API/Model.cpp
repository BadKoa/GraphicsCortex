#include "Model.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include "Image.h"
#include <vector>
#include <map>

Assimp::Importer Model::asset_loader;

Model::Model() :
	vertex_buffer(ArrayBuffer()), index_buffer(IndexBuffer()) {}

Model::Model(const std::string& file_path) {
	load_model(file_path);
}


Model::Model(ArrayBuffer& verticies, IndexBuffer& indicies) :
	vertex_buffer(std::move(verticies)), index_buffer(std::move(indicies)) {}

Model::Model(ArrayBuffer&& verticies, IndexBuffer&& indicies) :
	vertex_buffer(verticies), index_buffer(indicies) {}

std::string compute_directory(const std::string& file_name) {
	std::string dir = file_name;
	for (int i = dir.size() - 1; i >= 0; i--) {
		if (dir[i] != '/' && dir[i] != '\\')
			dir.pop_back();
		else
			break;
	}
	return dir;
}

std::string compute_filename(const std::string& file_name) {
	std::string name = "";
	for (int i = file_name.size() - 1; i >= 0; i--) {
		if (file_name[i] != '/' && file_name[i] != '\\')
			name = file_name[i] + name;
		else
			break;
	}
	return name;
}

UnorderedMaterial Model::load_model(const std::string& filepath, float scale) {
	
	clear_ram();

	const aiScene* imported_scene = asset_loader.ReadFile(filepath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);

	std::cout << asset_loader.GetErrorString();

	int vertex_count = 0;
	int index_count = 0;
	for (int i = 0; i < imported_scene->mNumMeshes; i++) {
		vertex_count += imported_scene->mMeshes[i]->mNumVertices;
		index_count += imported_scene->mMeshes[i]->mNumFaces;
	}
	index_count *= 3; // if all faces are triangles!

	unsigned int prefix_indicies_sum = 0;

	std::vector<std::string> image_paths;
	std::string dir = compute_directory(filepath);

	for (int i = 0; i < imported_scene->mNumMeshes; i++) {
		// 0: diffuse_index, 1: specular_index, 2: normal_index
		int map_indicies[3] = {-1, -1, -1};
		unsigned int map_types[3] = {aiTextureType_DIFFUSE, aiTextureType_METALNESS, aiTextureType_NORMALS};

		for (int j = 0; j < 3; j++) {
			int material_index = imported_scene->mMeshes[i]->mMaterialIndex;
			aiString image_name;
			if (imported_scene->mMaterials[material_index]->GetTexture((aiTextureType)map_types[j], 0, &image_name) == AI_SUCCESS) {
				map_indicies[j] = image_paths.size();
				//std::string path = dir + std::string(image_name.C_Str());
				std::string path = dir + "textures/" + compute_filename(std::string(image_name.C_Str()));
				std::cout << path << std::endl;
				bool image_exists = false;
				for (int k = 0; k < image_paths.size(); k++) {
					if (image_paths[k] == path) {
						image_exists = true;
						map_indicies[j] = k;
						break;
					}
				}
				if(!image_exists){
					image_paths.push_back(path);
				}
			}
			else { // texture not found
				map_indicies[j] = -1;
			}
		}


		for (int j = 0; j < imported_scene->mMeshes[i]->mNumVertices; j++) {
			aiVector3D vertex = imported_scene->mMeshes[i]->mVertices[j];
			vertex_data.push_back((float)vertex.x * scale);
			vertex_data.push_back((float)vertex.y * scale);
			vertex_data.push_back((float)vertex.z * scale);

			aiVector3D texcoords = imported_scene->mMeshes[i]->mTextureCoords[0][j];
			vertex_data.push_back(texcoords.x);
			vertex_data.push_back(texcoords.y);
			vertex_data.push_back(map_indicies[0]); // diffuse
			vertex_data.push_back(map_indicies[1]); // specular
			vertex_data.push_back(map_indicies[2]); // normals

			aiVector3D normal = imported_scene->mMeshes[i]->mNormals[j];
			normal.Normalize();
			vertex_data.push_back((float)normal.x);
			vertex_data.push_back((float)normal.y);
			vertex_data.push_back((float)normal.z);

		}
		for (int j = 0; j < imported_scene->mMeshes[i]->mNumFaces; j++) {
			const aiFace& Face = imported_scene->mMeshes[i]->mFaces[j];

			index_data.push_back(prefix_indicies_sum + (unsigned int)Face.mIndices[0]);
			index_data.push_back(prefix_indicies_sum + (unsigned int)Face.mIndices[1]);
			index_data.push_back(prefix_indicies_sum + (unsigned int)Face.mIndices[2]);

		}
		prefix_indicies_sum += imported_scene->mMeshes[i]->mNumVertices;
	}

	vertex_buffer.vertex_attribute_structure.clear();
	vertex_buffer.initialize_buffer(vertex_data);
	vertex_buffer.push_attribute(3);	// position
	vertex_buffer.push_attribute(2);	// texture uv
	vertex_buffer.push_attribute(3);	// texture_map index
	vertex_buffer.push_attribute(3);	// normals

	index_buffer.initialize_buffer(index_data, 3);

	UnorderedMaterial material(image_paths.size());
	for (int i = 0; i < image_paths.size(); i++) {
		material.set_texture(image_paths[i], 4, i);
	}
	return material;
}

void Model::clear_ram() {
	vertex_data.clear();
	index_data.clear();
}