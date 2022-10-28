#include "Object.h"
#include "PhysicsGeometry.h"

Object::Object(Graphic& graphic_representation, PhysicsObject& physics_representation) :
	graphics(graphic_representation), physics(physics_representation) { }

void Object::sync_with_physics() {
	physx::PxQuat physx_quat = physics.get_rotation();
	graphics.rotation_quat = glm::quat(physx_quat.w, physx_quat.x, physx_quat.y, physx_quat.z);
	graphics.position.x = physics.get_position().x;
	graphics.position.y = physics.get_position().y;
	graphics.position.z = physics.get_position().z;
}

void Object::set_position(glm::vec3 position){
	graphics.set_position(position);
	physics.set_position(position);
}
void Object::set_rotation(glm::vec3 rotation){
	graphics.set_rotation(rotation);
	physics.set_rotation(rotation);
}

Vehicle::Vehicle(PhysicsVehicle& physics_representation) : 
	physics_representation(physics_representation) {}

void Vehicle::load_model_chassis(Model& chassis, bool use_for_graphics, bool use_for_physics) {
	if(use_for_graphics)
		this->chassis.load_model(chassis);
	if (use_for_physics) {
		std::vector<physx::PxVec3> data = chassis.get_partial_data<physx::PxVec3>("111");
		this->physics_representation.set_chasis_mesh(create_geometry::convex_hull(data));
	}
}
void Vehicle::load_model_left_wheel(Model& left_wheel, bool use_for_graphics, bool use_for_physics) {
	if(use_for_graphics){
		this->wheels[FRONT_LEFT].load_model(left_wheel);
		this->wheels[REAR_LEFT].load_model(left_wheel);
	}
	if (use_for_physics) { // there is only one wheel mesh yet
		std::vector<physx::PxVec3> data = left_wheel.get_partial_data<physx::PxVec3>("111");
		this->physics_representation.set_wheel_mesh(create_geometry::convex_hull(data));
	}
}
void Vehicle::load_model_right_wheel(Model& right_wheel, bool use_for_graphics, bool use_for_physics) {
	if (use_for_graphics) {
		this->wheels[FRONT_RIGHT].load_model(right_wheel);
		this->wheels[REAR_RIGHT].load_model(right_wheel);
	}
	if (use_for_physics) { // there is only one wheel mesh yet
		std::vector<physx::PxVec3> data = right_wheel.get_partial_data<physx::PxVec3>("111");
		this->physics_representation.set_wheel_mesh(create_geometry::convex_hull(data));
	}
}
void Vehicle::load_model_models(Model& chassis, Model& left_wheel, Model& right_wheel, bool use_for_graphics, bool use_for_physics) {
	load_model_chassis(chassis, use_for_graphics, use_for_physics);
	load_model_left_wheel(left_wheel, use_for_graphics, use_for_physics);
	load_model_right_wheel(right_wheel, use_for_graphics, use_for_physics);
}

void Vehicle::load_model_chassis(Model&& chassis, bool use_for_graphics, bool use_for_physics) {
	load_model_chassis(chassis, use_for_graphics, use_for_physics);
}
void Vehicle::load_model_left_wheel(Model&& left_wheel, bool use_for_graphics, bool use_for_physics) {
	load_model_left_wheel(left_wheel, use_for_graphics, use_for_physics);
}
void Vehicle::load_model_right_wheel(Model&& right_wheel, bool use_for_graphics, bool use_for_physics) {
	load_model_right_wheel(right_wheel, use_for_graphics, use_for_physics);
}
void Vehicle::load_model_models(Model&& chassis, Model&& left_wheel, Model&& right_wheel, bool use_for_graphics, bool use_for_physics) {
	load_model_models(chassis, left_wheel, right_wheel, use_for_graphics, use_for_physics);
}


void Vehicle::load_material_chassis(Material& chassis) {
	this->chassis.load_material(chassis);
}
void Vehicle::load_material_left_wheel(Material& left_wheel) {
	this->wheels[FRONT_LEFT].load_material(left_wheel);
	this->wheels[REAR_LEFT].load_material(left_wheel);
}
void Vehicle::load_material_right_wheel(Material& right_wheel) {
	this->wheels[FRONT_RIGHT].load_material(right_wheel);
	this->wheels[REAR_RIGHT].load_material(right_wheel);
}
void Vehicle::load_material_models(Material& chassis, Material& left_wheel, Material& right_wheel) {
	load_material_chassis(chassis);
	load_material_left_wheel(left_wheel);
	load_material_right_wheel(right_wheel);
}
void Vehicle::load_material_models(Material& all) {
	load_material_chassis(all);
	load_material_left_wheel(all);
	load_material_right_wheel(all);
}

void Vehicle::load_program_chassis(Program& chassis) {
	this->chassis.load_program(chassis);
}
void Vehicle::load_program_left_wheel(Program& left_wheel) {
	this->wheels[FRONT_LEFT].load_program(left_wheel);
	this->wheels[REAR_LEFT].load_program(left_wheel);
}
void Vehicle::load_program_right_wheel(Program& right_wheel) {
	this->wheels[FRONT_RIGHT].load_program(right_wheel);
	this->wheels[REAR_RIGHT].load_program(right_wheel);
}
void Vehicle::load_program_models(Program& chassis, Program& left_wheel, Program& right_wheel) {
	load_program_chassis(chassis);
	load_program_left_wheel(left_wheel);
	load_program_right_wheel(right_wheel);
}
void Vehicle::load_program_models(Program& all) {
	load_program_chassis(all);
	load_program_left_wheel(all);
	load_program_right_wheel(all);
}

void Vehicle::sync_with_physics() {
	physx::PxQuat physx_quat = physics_representation.get_rotation();
	chassis.rotation_quat = glm::quat(physx_quat.w, physx_quat.x, physx_quat.y, physx_quat.z);
	chassis.position.x = physics_representation.get_position().x;
	chassis.position.y = physics_representation.get_position().y;
	chassis.position.z = physics_representation.get_position().z;
}

void Vehicle::set_position(glm::vec3 position) {
	physics_representation.set_position(position);
	sync_with_physics();
}
void Vehicle::set_rotation(glm::quat rotation) {
	physics_representation.set_rotation(rotation);
	sync_with_physics();
}
void Vehicle::set_rotation(physx::PxQuat rotation) {
	physics_representation.set_rotation(rotation);
	sync_with_physics();
}

