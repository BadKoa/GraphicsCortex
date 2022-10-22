#pragma once
#include "Config.h"

#include <glm.hpp>
#include <assimp/Importer.hpp> 
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

#include "Buffer.h"
#include "ShaderCompiler.h"
#include "Texture.h"
#include "UnifromQueue.h"

class Graphic {
private:
	glm::vec3 _last_updated_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 _last_updated_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	uniform_update_queue _uniform_update_queue;
public:

	static Assimp::Importer asset_loader;
	ArrayBuffer vertex_buffer;
	IndexBuffer index_buffer;
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);;
	glm::mat4 model_matrix;
	Program* renderer = nullptr;
	Material* material = nullptr;
	unsigned int mode = GL_TRIANGLES;
	Graphic();
	Graphic(const ArrayBuffer& buffer, const IndexBuffer& indicies, Material& material, Program& renderer);
	Graphic(const std::vector<float>& verticies, int data_dim); // legacy
	Graphic(Material& material, Program& renderer);

	void draw(bool show_warnings = true, bool _ignore_default_uniforms = false);
	void update_matrix();

	void load_model(const std::string& file_path);
	void clear_mesh();

	template<typename T>
	void add_uniform_update_queue(uniform_update<T>& uniform_queue) {
		uniform_queue.program = renderer;
		renderer->define_uniform(uniform_queue.uniform_name);
		uniform_queue.uniform_id = renderer->uniforms[uniform_queue.uniform_name];
		_uniform_update_queue.add_uniform_update(uniform_queue);
	}
	template<typename T>
	void add_uniform_update_queue(uniform_update<T>&& uniform_queue) {
		uniform_queue.program = renderer;
		renderer->define_uniform(uniform_queue.uniform_name);
		uniform_queue.uniform_id = renderer->uniforms[uniform_queue.uniform_name];
		_uniform_update_queue.add_uniform_update(uniform_queue);
	}
	
	template<typename T>
	void add_uniform_update_queue(dynamic_uniform_update<T>& dynamic_uniform_queue) {
		dynamic_uniform_queue.program = renderer;
		renderer->define_uniform(dynamic_uniform_queue.uniform_name);
		dynamic_uniform_queue.uniform_id = renderer->uniforms[dynamic_uniform_queue.uniform_name];
		_uniform_update_queue.add_uniform_update(dynamic_uniform_queue);
	}
	template<typename T>
	void add_uniform_update_queue(dynamic_uniform_update<T>&& dynamic_uniform_queue) {
		dynamic_uniform_queue.program = renderer;
		renderer->define_uniform(dynamic_uniform_queue.uniform_name);
		dynamic_uniform_queue.uniform_id = renderer->uniforms[dynamic_uniform_queue.uniform_name];
		_uniform_update_queue.add_uniform_update(dynamic_uniform_queue);
	}

	void remove_uniform_update_queue(const std::string& uniform_name) {
		_uniform_update_queue.remove_uniform_update(uniform_name);
	}

	void remove_uniform_update_queue(unsigned int uniform_id) {
		_uniform_update_queue.remove_uniform_update(uniform_id);
	}

	void set_uniform_upadte_queue(uniform_update_queue& original) {
		_uniform_update_queue.copy(original);
		_uniform_update_queue.link_program(renderer);
		_uniform_update_queue.update_uniform_ids();
	}

	void set_uniform_upadte_queue(uniform_update_queue original) {
		_uniform_update_queue.copy(original);
		_uniform_update_queue.link_program(renderer);
		_uniform_update_queue.update_uniform_ids();
	}

	void update_uniform_queue(bool init);
};

