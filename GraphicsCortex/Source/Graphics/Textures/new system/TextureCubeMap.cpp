#include "TextureCubeMap.h"
#include "Debuger.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <memory>
#include <functional>

namespace {
	void read_image(const std::string& filename, Image** output_image, unsigned int texture_width, unsigned int texture_height, int desired_channels) {
		std::ifstream file;
		file.open(filename);
		if (file) {
			*output_image = new Image(filename, desired_channels);
		}
		else { // file doesn't exist
			std::cout << "[ERROR] Image path not found : " << filename << std::endl;
			*output_image = new Image("../GraphicsCortex/Images/missing_texture.png", desired_channels);
		}

		if ((*output_image)->get_width() == 0 || (*output_image)->get_height() == 0 || (*output_image)->get_channels() == 0) {
			std::cout << "[ERROR] Image couldn't be properly imported : " << filename << std::endl;
			if (*output_image != nullptr) delete* output_image;
			*output_image = new Image("../GraphicsCortex/Images/missing_texture.png", desired_channels);
		}
		if (texture_width != 0 && texture_height != 0)
			(*output_image)->resize(texture_width, texture_height);
	}
}

TextureCubeMap::TextureCubeMap(int width, int height, ColorTextureFormat internal_format, int mipmap_levels, float mipmap_bias) :
	width(width), height(height), color_texture_format(internal_format), is_color_texture(true), mipmap_levels(mipmap_levels), mipmap_bias(mipmap_bias)
{
	_generate_texture();
}

TextureCubeMap::TextureCubeMap(int width, int height, DepthStencilTextureFormat internal_format, int mipmap_levels, float mipmap_bias) : 
	width(width), height(height), depth_stencil_texture_format(internal_format), is_color_texture(false), mipmap_levels(mipmap_levels), mipmap_bias(mipmap_bias)
{
	_generate_texture();
}

TextureCubeMap::~TextureCubeMap()
{
	release();
}

void TextureCubeMap::release()
{
	if (_texture_generated)
		GLCall(glDeleteTextures(1, &id));
	_texture_generated = false;
	_texture_allocated = false;
}

void TextureCubeMap::bind(int texture_slot)
{
	GLCall(glBindTextureUnit(texture_slot, id));

	if (!_texture_allocated) {
		std::cout << "[OpenGL Warning] TextureCubeMap tried to bind(int) but no user data was loaded yet" << std::endl;
		_allocate_texture();
	}
}

void TextureCubeMap::bind()
{
	if (!_texture_generated) {
		std::cout << "[OpenGL Error] released TextureCubeMap tried to bind()" << std::endl;
		ASSERT(false);
	}
	std::cout << "[OpenGL Warning] Bindless TextureCubeMap tried to bind()" << std::endl;
	GLCall(glBindTexture(target, id));
}

void TextureCubeMap::unbind()
{
	std::cout << "[OpenGL Warning] Bindless TextureCubeMap tried to unbind" << std::endl;

	GLCall(glBindTexture(target, 0));
}

void TextureCubeMap::load_data(Face face, const void* image, ColorFormat format, Type type, int mipmap_target)
{
	if (!_texture_generated) {
		std::cout << "[OpenGL Error] released TextureCubeMap tried to load_data()" << std::endl;
		ASSERT(false);
	}
	_allocate_texture();

	GLCall(glTextureSubImage3D(id, mipmap_target, 0, 0, get_gl_face_index(face), this->width >> mipmap_target, this->height >> mipmap_target, 1, TextureBase2::ColorFormat_to_OpenGL(format), TextureBase2::Type_to_OpenGL(type), image););

	_user_data_loaded = true;
}

void TextureCubeMap::load_data(Face face, const void* image, ColorFormat format, Type type, int x, int y, int custom_width, int custom_height, int mipmap_target)
{
	if (!_texture_generated) {
		std::cout << "[OpenGL Error] released TextureCubeMap tried to load_data()" << std::endl;
		ASSERT(false);
	}
	_allocate_texture();
	
	GLCall(glTextureSubImage3D(id, mipmap_target, x, y, get_gl_face_index(face), custom_width, custom_height, 1, TextureBase2::ColorFormat_to_OpenGL(format), TextureBase2::Type_to_OpenGL(type), image));

	_user_data_loaded = true;
}

void TextureCubeMap::load_data(Face face, const Image& image, ColorFormat format, Type type, int mipmap_target)
{
	int mipmap_width = this->width >> mipmap_target;
	int mipmap_height = this->height >> mipmap_target;

	if (mipmap_width != image.get_width() || mipmap_height != image.get_height()) {
		std::cout << "[OpenGL Error] TextureCubeMap.load_data() image size mismatch. " << " texture size : (" << width << ", " << height << "), " << " image size : (" << image.get_width() << ", " << image.get_height() << ")" << std::endl;
		ASSERT(false);
	}

	load_data(face, image._image_data, format, type, mipmap_target);
}

void TextureCubeMap::load_data(Face face, const Image& image, ColorFormat format, Type type, int x, int y, int width, int height, int mipmap_target)
{
	int mipmap_width = this->width >> mipmap_target;
	int mipmap_height = this->height >> mipmap_target;

	if (width > image.get_width() || height > image.get_height()) {
		std::cout << "[OpenGL Error] TextureCubeMap.load_data() image size mismatch. " << " load_data() size : (" << width << ", " << height << "), " << " image size : (" << image.get_width() << ", " << image.get_height() << ")" << std::endl;
		ASSERT(false);
	}

	if (mipmap_width < width || mipmap_height < height) {
		std::cout << "[OpenGL Error] TextureCubeMap.load_data() image size mismatch. " << " texture size : (" << width << ", " << height << "), " << " load_data() size : (" << image.get_width() << ", " << image.get_height() << ")" << std::endl;
		ASSERT(false);
	}

	load_data(face, image._image_data, format, type, x, y, width, height, mipmap_target);
}

void TextureCubeMap::load_data_async(Face face, const std::string& image_filepath, ColorFormat format, Type type, int mipmap_target)
{
	load_data_async(face, image_filepath, format, type, 0, 0, width, height, mipmap_target);
}

void TextureCubeMap::load_data_async(Face face, const std::string& image_filepath, ColorFormat format, Type type, int x, int y, int width, int height, int mipmap_target)
{
	if (async_load_happening_per_face[get_gl_face_index(face)]) {
		std::cout << "[Thread Error] TextureCubeMap tried to load_data_async() while another async load wasn't terminated" << std::endl;
		ASSERT(false);
	}
	async_load_happening = true;
	async_load_happening_per_face[get_gl_face_index(face)] = true;

	ASSERT(async_images[get_gl_face_index(face)] == nullptr);
	ASSERT(async_loading_threads[get_gl_face_index(face)] == nullptr);
	async_loading_threads[get_gl_face_index(face)] = new std::thread(&read_image, image_filepath, &async_images[get_gl_face_index(face)], this->width, this->height, ColorFormat_channels(format));

	post_async_load_functions[get_gl_face_index(face)] = [this, face, format, type, x, y, width, height, mipmap_target]() {
		this->load_data(face, *async_images[get_gl_face_index(face)], format, type, x, y, width, height, mipmap_target);
		};
}

void TextureCubeMap::load_data(Face face, const void* image, DepthStencilFormat format, Type type, int mipmap_target)
{
	if (!_texture_generated) {
		std::cout << "[OpenGL Error] released TextureCubeMap tried to load_data()" << std::endl;
		ASSERT(false);
	}
	_allocate_texture();

	GLCall(glTextureSubImage3D(id, mipmap_target, 0, 0, get_gl_face_index(face), this->width >> mipmap_target, this->height >> mipmap_target, 1, TextureBase2::DepthStencilFormat_to_OpenGL(format), TextureBase2::Type_to_OpenGL(type), image));

	_user_data_loaded = true;
}

void TextureCubeMap::load_data(Face face, const void* image, DepthStencilFormat format, Type type, int x, int y, int custom_width, int custom_height, int mipmap_target)
{
	if (!_texture_generated) {
		std::cout << "[OpenGL Error] released TextureCubeMap tried to load_data()" << std::endl;
		ASSERT(false);
	}
	_allocate_texture();

	GLCall(glTextureSubImage3D(id, mipmap_target, x, y, get_gl_face_index(face), custom_width, custom_height, 1, TextureBase2::DepthStencilFormat_to_OpenGL(format), TextureBase2::Type_to_OpenGL(type), image));

	_user_data_loaded = true;
}

void TextureCubeMap::load_data(Face face, const Image& image, DepthStencilFormat format, Type type, int mipmap_target)
{
	int mipmap_width = this->width >> mipmap_target;
	int mipmap_height = this->height >> mipmap_target;

	if (mipmap_width != image.get_width() || mipmap_height != image.get_height()) {
		std::cout << "[OpenGL Error] TextureCubeMap.load_data() image size mismatch. " << " texture size : (" << width << ", " << height << "), " << " image size : (" << image.get_width() << ", " << image.get_height() << ")" << std::endl;
		ASSERT(false);
	}

	load_data(face, image._image_data, format, type, mipmap_target);
}

void TextureCubeMap::load_data(Face face, const Image& image, DepthStencilFormat format, Type type, int x, int y, int width, int height, int mipmap_target)
{
	int mipmap_width = this->width >> mipmap_target;
	int mipmap_height = this->height >> mipmap_target;

	if (width > image.get_width() || height > image.get_height()) {
		std::cout << "[OpenGL Error] TextureCubeMap.load_data() image size mismatch. " << " load_data() size : (" << width << ", " << height << "), " << " image size : (" << image.get_width() << ", " << image.get_height() << ")" << std::endl;
		ASSERT(false);
	}

	if (mipmap_width < width || mipmap_height < height) {
		std::cout << "[OpenGL Error] TextureCubeMap.load_data() image size mismatch. " << " texture size : (" << width << ", " << height << "), " << " load_data() size : (" << image.get_width() << ", " << image.get_height() << ")" << std::endl;
		ASSERT(false);
	}

	load_data(face, image._image_data, format, type, x, y, width, height, mipmap_target);
}

void TextureCubeMap::generate_mipmap()
{
	if (!_texture_generated) return;
	if (!_texture_allocated) return;
	if (!_user_data_loaded)  return;

	if (_mipmap_generated) return;

	GLCall(glGenerateTextureMipmap(id));

	_mipmap_generated = true;
}

void TextureCubeMap::load_data_with_mipmaps(Face face, const void* image, ColorFormat format, Type type)
{
	load_data(face, image, format, type, 0);
	generate_mipmap();
}

void TextureCubeMap::load_data_with_mipmaps(Face face, const Image& image, ColorFormat format, Type type)
{
	if (this->width != image.get_width() || this->height != image.get_height()) {
		std::cout << "[OpenGL Error] Texture2D.load_data_with_mipmaps() image size mismatch. " << " texture size : (" << width << ", " << height << "), " << " image size : (" << image.get_width() << ", " << image.get_height() << ")" << std::endl;
		ASSERT(false);
	}

	load_data(face, image, format, type, 0);
	generate_mipmap();
}

void TextureCubeMap::load_data_with_mipmaps(Face face, const void* image, DepthStencilFormat format, Type type)
{
	load_data(face, image, format, type, 0);
	generate_mipmap();
}

void TextureCubeMap::load_data_with_mipmaps(Face face, const Image& image, DepthStencilFormat format, Type type)
{
	if (this->width != image.get_width() || this->height != image.get_height()) {
		std::cout << "[OpenGL Error] Texture2D.load_data_with_mipmaps() image size mismatch. " << " texture size : (" << width << ", " << height << "), " << " image size : (" << image.get_width() << ", " << image.get_height() << ")" << std::endl;
		ASSERT(false);
	}

	load_data(face, image, format, type, 0);
	generate_mipmap();
}

void TextureCubeMap::load_data_width_mipmaps_async(Face face, const std::string& image_filepath, ColorFormat format, Type type)
{
	if (async_load_happening_per_face[get_gl_face_index(face)]) {
		std::cout << "[Thread Error] TextureCubeMap tried to load_data_width_mipmaps_async() while another async load wasn't terminated" << std::endl;
		ASSERT(false);
	}
	async_load_happening = true;
	async_load_happening_per_face[get_gl_face_index(face)] = true;
	ASSERT(async_images[get_gl_face_index(face)] == nullptr);
	ASSERT(async_loading_threads[get_gl_face_index(face)] == nullptr);
	async_loading_threads[get_gl_face_index(face)] = new std::thread(&read_image, image_filepath, &async_images[get_gl_face_index(face)], this->width, this->height, ColorFormat_channels(format));

	post_async_load_functions[get_gl_face_index(face)] = [this, face, format, type]() {
		this->load_data_with_mipmaps(face, *async_images[get_gl_face_index(face)], format, type);
		};
}

void TextureCubeMap::wait_async_load()
{
	if (async_load_happening == false) return;
	for (int i = 0; i < 6; i++) {
		ASSERT(async_loading_threads[i] != nullptr);
		ASSERT(post_async_load_functions[i] != nullptr);

		async_loading_threads[i]->join();
		post_async_load_functions[i]();
		post_async_load_functions[i] = nullptr;
		delete async_loading_threads[i];
		delete async_images[i];
		async_images[i] = nullptr;
		async_load_happening_per_face[i] = false;
	}
	async_load_happening = false;
}

void TextureCubeMap::_set_texture_parameters()
{
	if (_texture_handle_created) {
		std::cout << "[OpenGL Error] TextureCubeMap tried to _set_texture_parameters() but texture handle was already created" << std::endl;
		ASSERT(false);
	}

	GLCall(glTextureParameteri(id, GL_TEXTURE_BASE_LEVEL, mipmap_begin_level));
	GLCall(glTextureParameterf(id, GL_TEXTURE_LOD_BIAS, mipmap_bias));

	int gl_min_filter = SamplingFilter_to_OpenGL(min_filter);
	int gl_mipmap_min_filter = SamplingFilter_to_OpenGL(mipmap_min_filter);
	int combined_min_filter = gl_min_filter;
	if (gl_min_filter == GL_NEAREST && gl_mipmap_min_filter == GL_NEAREST)	combined_min_filter = GL_NEAREST_MIPMAP_NEAREST;
	if (gl_min_filter == GL_NEAREST && gl_mipmap_min_filter == GL_LINEAR)	combined_min_filter = GL_NEAREST_MIPMAP_LINEAR;
	if (gl_min_filter == GL_LINEAR && gl_mipmap_min_filter == GL_NEAREST)	combined_min_filter = GL_LINEAR_MIPMAP_NEAREST;
	if (gl_min_filter == GL_LINEAR && gl_mipmap_min_filter == GL_LINEAR)	combined_min_filter = GL_LINEAR_MIPMAP_LINEAR;

	GLCall(glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, combined_min_filter));	
	GLCall(glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, SamplingFilter_to_OpenGL(mag_filter)));
	GLCall(glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, mipmap_levels));
	GLCall(glTextureParameteri(id, GL_TEXTURE_WRAP_S, WrapMode_to_OpenGL(wrap_u)));
	GLCall(glTextureParameteri(id, GL_TEXTURE_WRAP_T, WrapMode_to_OpenGL(wrap_v)));
}

void TextureCubeMap::_generate_texture()
{
	if (_texture_generated) return;

	GLCall(glCreateTextures(target, 1, &id));

	_texture_generated = true;
}

void TextureCubeMap::_allocate_texture()
{
	if (!_texture_generated) return;
	if (_texture_allocated) return;

	if (width >> mipmap_levels == 0 || height >> mipmap_levels == 0 || mipmap_levels >= sizeof(int) * 8) {
		int old_mipmap_levels = mipmap_levels;
		if (width >> mipmap_levels == 0 || mipmap_levels >= sizeof(int) * 8) mipmap_levels = std::log2(width);
		if (height >> mipmap_levels == 0 || mipmap_levels >= sizeof(int) * 8) mipmap_levels = std::log2(height);
		std::cout << "[OpenGL Warning] TextureCubeMap with size (" << width << ", " << height << ") treid to load " << old_mipmap_levels << " mipmap levels, mipmap levels reducing to " << mipmap_levels << std::endl;;
	}

	unsigned int gl_internal_format = is_color_texture ? ColorTextureFormat_to_OpenGL(color_texture_format) : DepthStencilTextureFormat_to_OpenGL(depth_stencil_texture_format);

	GLCall(glTextureStorage2D(id, mipmap_levels, gl_internal_format, width, height));

	_create_handle();
	_texture_allocated = true;
}

void TextureCubeMap::_create_handle()
{
	if (_texture_handle_created) return;

	_set_texture_parameters();

	GLCall(texture_handle = glGetTextureHandleARB(id));
	GLCall(glMakeTextureHandleResidentARB(texture_handle));

	_texture_handle_created = true;
}

int TextureCubeMap::get_gl_face_index(Face face)
{
	switch (face) {
	case Face::RIGHT:	return 0;
	case Face::LEFT:	return 1;
	case Face::UP:		return 2;
	case Face::DOWN:	return 3;
	case Face::BACK:	return 4;
	case Face::FORWARD:	return 5;
	}
	ASSERT(false);
	return -1;
}

Image TextureCubeMap::get_image(Face face, ColorFormat format, Type type, int mipmap_level)
{
	return get_image(face, format, type, mipmap_level, 0, 0, query_width(mipmap_level), query_height(mipmap_level));
}

Image TextureCubeMap::get_image(Face face, ColorFormat format, Type type, int mipmap_level, int x, int y, int width, int height)
{
	if (!_texture_allocated || !_user_data_loaded) {
		std::cout << "[OpenGL Error] TextureCubeMap tried to get_image() but either not allocated any ram or didn't loaded any user data yet" << std::endl;
		ASSERT(false);
	}

	int r_channel = query_red_size(mipmap_level);
	int g_channel = query_green_size(mipmap_level);
	int b_channel = query_blue_size(mipmap_level);
	int a_channel = query_alpha_size(mipmap_level);

	int is_r = r_channel != 0;
	int is_g = g_channel != 0;
	int is_b = b_channel != 0;
	int is_a = a_channel != 0;

	int mipmap_channels = is_r + is_g + is_b + is_a;
	int mipmap_pixel_size = r_channel + g_channel + b_channel + a_channel;

	int mipmap_width = query_width(mipmap_level);
	int mipmap_height = query_height(mipmap_level);

	int format_channels = ColorFormat_channels(format);
	int image_size = width * height * mipmap_pixel_size;
	unsigned char* image = new unsigned char[width * height * mipmap_pixel_size];

	GLCall(glGetTextureSubImage(id, mipmap_level, x, y, get_gl_face_index(face), width, height, 1, ColorFormat_to_OpenGL(format), Type_to_OpenGL(type), image_size, image));

	return Image(image, width, height, format_channels, true);
}

Image TextureCubeMap::get_image(Face face, DepthStencilFormat format, Type type, int mipmap_level)
{
	return get_image(face, format, type, mipmap_level, 0, 0, query_width(mipmap_level), query_height(mipmap_level));
}

Image TextureCubeMap::get_image(Face face, DepthStencilFormat format, Type type, int mipmap_level, int x, int y, int width, int height)
{
	if (!_texture_allocated || !_user_data_loaded) {
		std::cout << "[OpenGL Error] TextureCubeMap tried to get_image() but either not allocated any ram or didn't loaded any user data yet" << std::endl;
		ASSERT(false);
	}

	int d_channel = query_depth_size(mipmap_level);

	int mipmap_channels = 1;
	int mipmap_pixel_size = d_channel;

	int mipmap_width = query_width(mipmap_level);
	int mipmap_height = query_height(mipmap_level);

	int format_channels = 1;
	int image_size = width * height * mipmap_pixel_size;
	unsigned char* image = new unsigned char[width * height * mipmap_pixel_size];

	int gl_format;
	if (format == DepthStencilFormat::DEPTH) gl_format = GL_DEPTH_COMPONENT;
	else if (format == DepthStencilFormat::STENCIL) gl_format = GL_STENCIL_INDEX;
	else {
		std::cout << "[OpenGL Error] TextureCubeMap tried to get_image() with unsuppoerted format" << std::endl;
		ASSERT(false);
	}

	GLCall(glGetTextureSubImage(id, mipmap_level, x, y, get_gl_face_index(face), width, height, 1, gl_format, Type_to_OpenGL(type), image_size, image));

	return Image(image, width, height, format_channels, true);
}

void TextureCubeMap::clear(Face face, unsigned char clear_data, int mipmap_target)
{
	clear(face, clear_data, 0, 0, width, height, mipmap_target);
}

void TextureCubeMap::clear(Face face, float clear_data, int mipmap_target)
{
	clear(face, clear_data, 0, 0, width, height, mipmap_target);
}

void TextureCubeMap::clear(Face face, glm::vec2 clear_data, int mipmap_target)
{
	clear(face, clear_data, 0, 0, width, height, mipmap_target);
}

void TextureCubeMap::clear(Face face, glm::vec3 clear_data, int mipmap_target)
{
	clear(face, clear_data, 0, 0, width, height, mipmap_target);
}

void TextureCubeMap::clear(Face face, glm::vec4 clear_data, int mipmap_target)
{
	clear(face, clear_data, 0, 0, width, height, mipmap_target);
}

void TextureCubeMap::clear(Face face, unsigned char clear_data, int x, int y, int width, int height, int mipmap_target)
{
	if (!_texture_allocated || !_user_data_loaded) {
		std::cout << "[OpenGL Error] TextureCubeMap tried to clear() but either not allocated any ram or didn't loaded any user data yet" << std::endl;
		ASSERT(false);
	}

	GLCall(glClearTexSubImage(id, mipmap_target, x, y, get_gl_face_index(face), width, height, 1, GL_RED, GL_UNSIGNED_BYTE, &clear_data));
}

void TextureCubeMap::clear(Face face, float clear_data, int x, int y, int width, int height, int mipmap_target)
{
	if (!_texture_allocated || !_user_data_loaded) {
		std::cout << "[OpenGL Error] TextureCubeMap tried to clear() but either not allocated any ram or didn't loaded any user data yet" << std::endl;
		ASSERT(false);
	}

	GLCall(glClearTexSubImage(id, mipmap_target, x, y, get_gl_face_index(face), width, height, 1, GL_RED, GL_FLOAT, &clear_data));
}

void TextureCubeMap::clear(Face face, glm::vec2 clear_data, int x, int y, int width, int height, int mipmap_target)
{
	if (!_texture_allocated || !_user_data_loaded) {
		std::cout << "[OpenGL Error] TextureCubeMap tried to clear() but either not allocated any ram or didn't loaded any user data yet" << std::endl;
		ASSERT(false);
	}

	GLCall(glClearTexSubImage(id, mipmap_target, x, y, get_gl_face_index(face), width, height, 1, GL_RG, GL_FLOAT, &clear_data));

}

void TextureCubeMap::clear(Face face, glm::vec3 clear_data, int x, int y, int width, int height, int mipmap_target)
{
	if (!_texture_allocated || !_user_data_loaded) {
		std::cout << "[OpenGL Error] TextureCubeMap tried to clear() but either not allocated any ram or didn't loaded any user data yet" << std::endl;
		ASSERT(false);
	}

	GLCall(glClearTexSubImage(id, mipmap_target, x, y, get_gl_face_index(face), width, height, 1, GL_RGB, GL_FLOAT, &clear_data));
}

void TextureCubeMap::clear(Face face, glm::vec4 clear_data, int x, int y, int width, int height, int mipmap_target)
{
	if (!_texture_allocated || !_user_data_loaded) {
		std::cout << "[OpenGL Error] TextureCubeMap tried to clear() but either not allocated any ram or didn't loaded any user data yet" << std::endl;
		ASSERT(false);
	}

	GLCall(glClearTexSubImage(id, mipmap_target, x, y, get_gl_face_index(face), width, height, 1, GL_RGBA, GL_FLOAT, &clear_data));

}

TextureBase2::SamplingFilter TextureCubeMap::query_mag_filter()
{
	int gl_filter;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_MAG_FILTER, &gl_filter));
	if (gl_filter == GL_NEAREST) return SamplingFilter::NEAREST;
	if (gl_filter == GL_LINEAR) return SamplingFilter::LINEAR;

	ASSERT(false);
	return SamplingFilter();
}

TextureBase2::SamplingFilter TextureCubeMap::query_min_filter()
{
	int gl_filter;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_MIN_FILTER, &gl_filter));
	if (gl_filter == GL_NEAREST || gl_filter == GL_NEAREST_MIPMAP_LINEAR || gl_filter == GL_NEAREST_MIPMAP_NEAREST) return SamplingFilter::NEAREST;
	if (gl_filter == GL_LINEAR || gl_filter == GL_LINEAR_MIPMAP_LINEAR || gl_filter == GL_LINEAR_MIPMAP_NEAREST)	return SamplingFilter::LINEAR;

	ASSERT(false);
	return SamplingFilter();
}

TextureBase2::SamplingFilter TextureCubeMap::query_mipmap_min_filter()
{
	int gl_filter;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_MIN_FILTER, &gl_filter));
	if (gl_filter == GL_LINEAR_MIPMAP_NEAREST || gl_filter == GL_NEAREST_MIPMAP_NEAREST) return SamplingFilter::NEAREST;
	if (gl_filter == GL_LINEAR_MIPMAP_LINEAR || gl_filter == GL_NEAREST_MIPMAP_LINEAR)	return SamplingFilter::LINEAR;

	return SamplingFilter::NEAREST;
}

TextureBase2::WrapMode TextureCubeMap::query_wrap_u()
{
	int gl_wrap;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_WRAP_S, &gl_wrap));
	if (gl_wrap == GL_CLAMP)  return WrapMode::CLAMP;
	if (gl_wrap == GL_REPEAT) return WrapMode::REPEAT;
	if (gl_wrap == GL_MIRRORED_REPEAT) return WrapMode::MIRRORED_REPEAT;

	ASSERT(false);
	return WrapMode();
}

TextureBase2::WrapMode TextureCubeMap::query_wrap_v()
{
	int gl_wrap;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_WRAP_R, &gl_wrap));
	if (gl_wrap == GL_CLAMP)  return WrapMode::CLAMP;
	if (gl_wrap == GL_REPEAT) return WrapMode::REPEAT;
	if (gl_wrap == GL_MIRRORED_REPEAT) return WrapMode::MIRRORED_REPEAT;

	ASSERT(false);
	return WrapMode();
}

TextureBase2::WrapMode TextureCubeMap::query_wrap_w()
{
	int gl_wrap;
	bind();
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_WRAP_T, &gl_wrap));
	if (gl_wrap == GL_CLAMP)  return WrapMode::CLAMP;
	if (gl_wrap == GL_REPEAT) return WrapMode::REPEAT;
	if (gl_wrap == GL_MIRRORED_REPEAT) return WrapMode::MIRRORED_REPEAT;

	ASSERT(false);
	return WrapMode();
}

int TextureCubeMap::query_base_level()
{
	int base_level;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_BASE_LEVEL, &base_level));
	return base_level;
}

float TextureCubeMap::query_lod_bias()
{
	float lod_bias;
	GLCall(glGetTextureParameterfv(id, GL_TEXTURE_LOD_BIAS, &lod_bias));
	return lod_bias;
}

int TextureCubeMap::query_max_level()
{
	int max_level;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_MAX_LEVEL, &max_level));
	return max_level;
}

int TextureCubeMap::query_max_lod()
{
	float max_lod;
	GLCall(glGetTextureParameterfv(id, GL_TEXTURE_MAX_LOD, &max_lod));
	return max_lod;
}

int TextureCubeMap::query_min_lod()
{
	float min_lod;
	GLCall(glGetTextureParameterfv(id, GL_TEXTURE_MIN_LOD, &min_lod));
	return min_lod;
}

int TextureCubeMap::query_swizzle_r()
{
	int swizzle_red;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_SWIZZLE_R, &swizzle_red));
	return swizzle_red;	// returns GL enum
}

int TextureCubeMap::query_swizzle_g()
{
	int swizzle_green;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_SWIZZLE_G, &swizzle_green));
	return swizzle_green;	// returns GL enum
}

int TextureCubeMap::query_swizzle_b()
{
	int swizzle_blue;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_SWIZZLE_B, &swizzle_blue));
	return swizzle_blue;	// returns GL enum
}

int TextureCubeMap::query_swizzle_a()
{
	int swizzle_alpha;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_SWIZZLE_A, &swizzle_alpha));
	return swizzle_alpha;	// returns GL enum
}

int TextureCubeMap::query_swizzle_rgba()
{
	int swizzle_rgba;
	GLCall(glGetTextureParameteriv(id, GL_TEXTURE_SWIZZLE_RGBA, &swizzle_rgba));
	return swizzle_rgba;	// returns GL enum
}

int TextureCubeMap::query_width(int mipmap_level)
{
	int texture_width;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_WIDTH, &texture_width));
	return texture_width;
}

int TextureCubeMap::query_height(int mipmap_level)
{
	int texture_height;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_HEIGHT, &texture_height));
	return texture_height;
}

int TextureCubeMap::query_depth(int mipmap_level)
{
	int texture_depth;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_DEPTH, &texture_depth));
	return texture_depth;
}

int TextureCubeMap::query_internal_format(int mipmap_level)
{
	int internal_format;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_INTERNAL_FORMAT, &internal_format));
	return internal_format;	// returns gl enum
}

int TextureCubeMap::query_red_type(int mipmap_level)
{
	int type;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_RED_TYPE, &type));
	return type;	// returns gl enum
}

int TextureCubeMap::query_green_type(int mipmap_level)
{
	int type;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_GREEN_TYPE, &type));
	return type;	// returns gl enum
}

int TextureCubeMap::query_blue_type(int mipmap_level)
{
	int type;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_BLUE_TYPE, &type));
	return type;	// returns gl enum
}

int TextureCubeMap::query_alpha_type(int mipmap_level)
{
	int type;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_ALPHA_TYPE, &type));
	return type;	// returns gl enum
}

int TextureCubeMap::query_depth_type(int mipmap_level)
{
	int type;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_DEPTH_TYPE, &type));
	return type;	// returns gl enum
}

int TextureCubeMap::query_red_size(int mipmap_level)
{
	int size;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_RED_SIZE, &size));
	return size;
}

int TextureCubeMap::query_green_size(int mipmap_level)
{
	int size;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_GREEN_SIZE, &size));
	return size;
}

int TextureCubeMap::query_blue_size(int mipmap_level)
{
	int size;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_BLUE_SIZE, &size));
	return size;
}

int TextureCubeMap::query_alpha_size(int mipmap_level)
{
	int size;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_ALPHA_SIZE, &size));
	return size;
}

int TextureCubeMap::query_depth_size(int mipmap_level)
{
	int size;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_DEPTH_SIZE, &size));
	return size;
}

bool TextureCubeMap::query_is_compressed(int mipmap_level)
{
	int is_compressed;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_COMPRESSED, &is_compressed));
	if (is_compressed == GL_FALSE) return false;
	if (is_compressed == GL_TRUE) return true;

	ASSERT(false);
	return false;
}

int TextureCubeMap::query_compressed_image_size(int mipmap_level)
{
	int size;
	GLCall(glGetTextureLevelParameteriv(id, mipmap_level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &size));
	return size;
}