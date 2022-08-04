#include <vector>
#include <string>

#include <iostream>

#include "Buffer.h";

unsigned int ArrayBuffer::current_binded_buffer = 0;

ArrayBuffer::ArrayBuffer()
	: id(0), data_count(0) {};

ArrayBuffer::ArrayBuffer(float verticies[], int data_count)
	: data_count(data_count)
{
	initialize_buffer(verticies, data_count);
}

ArrayBuffer::ArrayBuffer(std::vector<float> verticies)
	: data_count(verticies.size())
{
	initialize_buffer((float*)&verticies[0], data_count);
}

void ArrayBuffer::initialize_buffer(float verticies[], int data_count) {
	GLCall(glGenBuffers(1, &id));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, id));
	GLCall(glBufferData(GL_ARRAY_BUFFER, data_count * sizeof(float), verticies, GL_STATIC_DRAW));

	current_binded_buffer = id;
}

void ArrayBuffer::push_attribute(unsigned int count) {
	this->vertex_attribute_structure.push_back(count);
}

void ArrayBuffer::bind() {
	#ifdef ARRAY_BUFFER_REPEATED_BIND_OPTIMIZATION
	if (id != current_binded_buffer)
	#endif
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, id));
		int sum = 0;
		for (int attrib : vertex_attribute_structure) {
			sum += attrib;
		}
		int prefix_sum = 0;
		for (int i = 0; i < this->vertex_attribute_structure.size(); i++) {
			GLCall(glEnableVertexAttribArray(i));
			GLCall(glVertexAttribPointer(i, vertex_attribute_structure[i], GL_FLOAT, GL_FALSE, sum*sizeof(float), (void*)(prefix_sum*sizeof(float))));
			prefix_sum += vertex_attribute_structure[i];
		}

		current_binded_buffer = id;
	}
}

void ArrayBuffer::unbind() {
	#ifdef ARRAY_BUFFER_REPEATED_BIND_OPTIMIZATION
	if (0 != current_binded_buffer)
	#endif
	{ 
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
	current_binded_buffer = 0;
}

unsigned int IndexBuffer::current_binded_buffer = 0;

IndexBuffer::IndexBuffer() : id(0), vertex_dim(2), data_count(0) {}

IndexBuffer::IndexBuffer(unsigned int verticies[], int vertex_dim, int data_count)
	: vertex_dim(vertex_dim), data_count(data_count)
{
	initialize_buffer(verticies, vertex_dim, data_count);
};

IndexBuffer::IndexBuffer(std::vector<unsigned int> verticies, int vertex_dim)
	: vertex_dim(vertex_dim), data_count(verticies.size())
{
	initialize_buffer((unsigned int*)&verticies[0], vertex_dim, data_count);
}

void IndexBuffer::initialize_buffer(unsigned int verticies[], int vertex_dim, int data_count) {
	GLCall(glGenBuffers(1, &id));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_count * sizeof(unsigned int), verticies, GL_STATIC_DRAW));
}

void IndexBuffer::bind() {
	#ifdef INDEX_BUFFER_REPEATED_BIND_OPTIMIZATION
	if (id != current_binded_buffer)
	#endif
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
	current_binded_buffer = id;
}

void IndexBuffer::unbind() {
	#ifdef INDEX_BUFFER_REPEATED_BIND_OPTIMIZATION
	if (0 != current_binded_buffer)
	#endif
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	current_binded_buffer = 0;
}