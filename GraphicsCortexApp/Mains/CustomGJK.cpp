#include "GraphicsCortex.h"

int main() {

	Frame frame(1920, 1080, "GraphicsCortex", 0, 0, true, false, true);
	Scene scene(frame);
	scene.camera->screen_width = frame.window_width;
	scene.camera->screen_height = frame.window_height;
	scene.camera->max_distance = 10000;

	std::shared_ptr<UnorderedMaterial> cube_material = std::make_shared<UnorderedMaterial>();
	//std::shared_ptr<Program> program = default_program::flatcolor_program_s();
	std::shared_ptr<Graphic> cube_graphic = std::make_shared<Graphic>(default_geometry::cube());

	std::shared_ptr<Program> custom_program = std::make_shared<Program>(Shader("../GraphicsCortex/Custom Shaders/Texture.vert", "../GraphicsCortex/Custom Shaders/Texture.frag"));

	cube_graphic->load_program(custom_program);
	cube_graphic->load_material(cube_material);
	cube_graphic->set_position(glm::vec3(0.0f, 0.0f, -4.0f));
	scene.add(cube_graphic);

	Image image1("../GraphicsCortex/Images/full_white.png", 4, true);
	Image image2("../GraphicsCortex/Images/cobble.png", 4, true);

	BindlessMaterial bindless_material(custom_program);
	std::shared_ptr<Texture2D> my_texture = std::make_shared<Texture2D>(image1.get_width(), image1.get_height(), TextureBase2::ColorTextureFormat::RGBA8, 1, 0);
	bindless_material.add_texture("color_texture", my_texture);

	my_texture->load_data_async("../GraphicsCortex/Images/full_white.png", TextureBase2::ColorFormat::RGBA, TextureBase2::Type::UNSIGNED_BYTE, 0);
	my_texture->wait_async_load();


	{
		std::shared_ptr<DirectionalLight> sunlight = std::make_shared<DirectionalLight>(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(0.4, 0.4, 0.4));
		scene.add(sunlight);

		std::shared_ptr<AmbiantLight> ambinace = std::make_shared<AmbiantLight>(glm::vec3(0.3f, 0.3f, 0.3f));
		scene.add(ambinace);
	}





	PhysicsObject ground_plane(create_geometry::plane(0, 1, 0, 2.4f));
	{
		ground_plane.make_drivable();
		scene.add_physics(ground_plane);
	}

	std::shared_ptr<Program> cubemap_program = std::make_shared<Program>(default_program::cubemap_program());
	std::shared_ptr<CubeMapTexture> cube_map = std::make_shared<CubeMapTexture>();
	cube_map->set_program(cubemap_program);
	cube_map->camera = scene.camera.get();
	cube_map->face_texture_filepaths[RIGHT] = "../GraphicsCortex/Images/CubeMap/Sky/px.jpg";
	cube_map->face_texture_filepaths[LEFT] = "../GraphicsCortex/Images/CubeMap/Sky/nx.jpg";
	cube_map->face_texture_filepaths[TOP] = "../GraphicsCortex/Images/CubeMap/Sky/py.jpg";
	cube_map->face_texture_filepaths[BOTTOM] = "../GraphicsCortex/Images/CubeMap/Sky/ny.jpg";
	cube_map->face_texture_filepaths[FRONT] = "../GraphicsCortex/Images/CubeMap/Sky/pz.jpg";
	cube_map->face_texture_filepaths[BACK] = "../GraphicsCortex/Images/CubeMap/Sky/nz.jpg";

	cube_map->read_queue(3);
	cube_map->load_queue(true);
	scene.set_skybox(cube_map);

	while (frame.is_running()) {

		double delta_time = frame.handle_window();
		frame.clear_window(0.25f, 0.25f, 0.25f);
		frame.display_performance(180);


		bindless_material.update_uniforms();
		scene.render();


		scene.camera->handle_movements(frame.window, delta_time);
	}


}