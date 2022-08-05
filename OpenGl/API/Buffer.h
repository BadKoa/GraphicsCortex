#pragma once
#include "Config.h"

#include <vector>

#include "GL/glew.h"
#include "Debuger.h"

class ArrayBuffer {
private:
	static unsigned int current_binded_buffer;
public:
	unsigned int id;
	int data_count;
	std::vector<unsigned int> vertex_attribute_structure;
	ArrayBuffer();
	ArrayBuffer(float verticies[], int data_count);
	ArrayBuffer(std::vector<float> verticies);
	void push_attribute(unsigned int count);
	void initialize_buffer(const std::vector<float>& verticies);
	void initialize_buffer(float verticies[], int data_count);
	void bind();
	void unbind();
};

class IndexBuffer {
private:
	static unsigned int current_binded_buffer;
public:
	unsigned int id;
	int vertex_dim;
	int data_count;
	IndexBuffer();

	IndexBuffer(unsigned int verticies[], int vertex_dim, int data_count);
	IndexBuffer(std::vector<unsigned int> verticies, int vertex_dim);
	void initialize_buffer(std::vector<unsigned int>& verticies, int vertex_dim);
	void initialize_buffer(unsigned int verticies[], int vertex_dim, int data_count);
	void bind();
	void unbind();
};