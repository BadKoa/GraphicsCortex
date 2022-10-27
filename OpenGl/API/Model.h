#pragma once

#include "Buffer.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
public:
	static Assimp::Importer asset_loader;
	std::vector<float> vertex_data;
	std::vector<unsigned int> index_data;
	
	ArrayBuffer vertex_buffer;
	IndexBuffer index_buffer;

	Model();
	Model(ArrayBuffer& verticies, IndexBuffer& indirices);
	Model(ArrayBuffer&& verticies, IndexBuffer&& indicies);

	void load_model(const std::string& file_path);
	
	std::vector<float> get_partial_data(const std::string& mask = "11100000");

	void clear_ram();

};