#include "GraphicsCortex.h"

int main(){
	Frame frame(800, 600, "GraphicsCortex", 0, 0, true, true, false, 3, false);
	Scene scene(frame);
	scene.camera->max_distance = 1000;
	scene.camera->fov = 70;

	std::shared_ptr<UnorderedMaterial> material = std::make_shared<UnorderedMaterial>();
	std::shared_ptr<Program> program = default_program::flatcolor_program_s();
	std::shared_ptr<Graphic> graphic = std::make_shared<Graphic>(default_geometry::cube());

	std::shared_ptr<Program> custom_program = std::make_shared<Program>(Shader("../GraphicsCortex/Custom Shaders/Texture.vert", "../GraphicsCortex/Custom Shaders/Texture.frag"));

	graphic->load_program(custom_program);
	graphic->load_material(material);
	scene.add(graphic);

	Image image1("../GraphicsCortex/Images/orange.png", 4, true);
	Image image2("../GraphicsCortex/Images/cobble.png", 4, true);

	//Texture my_texture(0);
	//my_texture.load_image(image);
	//my_texture.texture_slot = 0;
	//my_texture.bind();

	Texture2D my_texture(image1.get_width(), image1.get_height(), TextureBase2::ColorTextureFormat::RGBA8, 2, -1);
	my_texture.load_data(image1, TextureBase2::ColorFormat::RGBA, TextureBase2::Type::UNSIGNED_BYTE, 0);

	image2.resize(my_texture.query_width(1), my_texture.query_height(1));
	//my_texture.load_data(image2, TextureBase2::ColorFormat::RGBA, TextureBase2::Type::UNSIGNED_BYTE, 1);
	//my_texture.get_image(TextureBase2::ColorFormat::RGBA, TextureBase2::Type::UNSIGNED_BYTE, 0).save_to_disc("test.png");

	while (frame.is_running()) {
		double deltatime = frame.handle_window();
		frame.clear_window();
		frame.display_performance(180);

		scene.render();

		//my_texture.bind(0);
		//custom_program->update_uniform("texture_slot", 0);
		custom_program->bind();
		GLCall(int location = glGetUniformLocation(custom_program->id, "texture_slot"));
		//GLCall(glUniformHandleui64ARB(location, my_texture.texture_handle));
		glProgramUniformHandleui64ARB(custom_program->id, 0, my_texture.texture_handle);
		graphic->update_matrix();
		graphic->update_default_uniforms(*graphic->renderer);
		graphic->draw();

		scene.camera->handle_movements(frame.window, deltatime);
	}

}