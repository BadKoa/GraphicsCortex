#include "API/GraphicsCortex.h"

class PhysicsLink {
public:
	PhysicsObject* actor0;
	PhysicsObject* actor1;

	physx::PxJoint* joint;

	unsigned int type;

	enum type {
		FIXED = 0,
		DISTANCE,
		SPHERICAL,
		REVOLUTE,
		PRISMATIC,
		D6,
	};

	PhysicsLink(PhysicsObject& actor0, PhysicsObject& actor1, enum PhysicsLink::type link_type) :
		type(link_type), actor0(&actor0), actor1(&actor1)
	{
		physx::PxVec3 offset(0.5f, 0, 0);
		auto context = PhysxContext::get();
		joint = physx::PxFixedJointCreate(*context.physics, actor0.actor, physx::PxTransform(-offset), actor1.actor, physx::PxTransform(offset));
		joint->setConstraintFlag(physx::PxConstraintFlag::eVISUALIZATION, true);

	}

	void remove() {
		std::vector<PhysicsLink*>::iterator index0 = std::find(actor0->links.begin(), actor0->links.end(), this);
		std::vector<PhysicsLink*>::iterator index1 = std::find(actor1->links.begin(), actor0->links.end(), this);
		actor0->links.erase(index0);
		actor1->links.erase(index1);

		joint->release();
	}
};


int main() {
	PhysicsScene scene;

	PhysicsObject box(create_geometry::box(1.0f, 1.0f, 1.0f), PhysicsObject::DYNAMIC, true);
	scene.add_actor(box);
	box.set_position(4.0f, 10.0f, 4.0f);

	PhysicsObject sphere(create_geometry::sphere(1.0f), PhysicsObject::DYNAMIC, true);
	scene.add_actor(sphere);
	sphere.set_position(4.0f, 10.0f, 4.0f);

	physx::PxVec3 offset(2.0f, 0, 0);
	glm::quat rotatoion_quat_glm(glm::vec3(0, 0, 0));
	physx::PxQuat rotation_quat(rotatoion_quat_glm.x, rotatoion_quat_glm.y, rotatoion_quat_glm.z, rotatoion_quat_glm.w);
	auto context = PhysxContext::get();
	physx::PxSphericalJoint* joint = physx::PxSphericalJointCreate(*context.physics, box.actor, physx::PxTransform(offset, rotation_quat), sphere.actor, physx::PxTransform(-offset, rotation_quat));
	joint->setConstraintFlag(physx::PxConstraintFlag::eVISUALIZATION, true);

	PhysicsObject capsule(create_geometry::capsule(1.0f, 1.0f), PhysicsObject::DYNAMIC, true);
	scene.add_actor(capsule);
	capsule.set_position(0.0f, 10.0f, 0.0f);
	capsule.set_rotation(physx::PxHalfPi, 10.0f, 0.0f);

	std::vector<physx::PxVec3> pyramid_index = { physx::PxVec3(0,1,0), physx::PxVec3(1,0,0), physx::PxVec3(-1,0,0), physx::PxVec3(0,0,1), physx::PxVec3(0,0,-1) };
	PhysicsObject pyramid(create_geometry::convex_hull(pyramid_index), PhysicsObject::DYNAMIC, true);
	scene.add_actor(pyramid);
	pyramid.set_position(0.0f, 10.0f, 20.0f);

	PhysicsObject plane(create_geometry::plane(0.0f, 1.0f, 0.0f, 10.0f), true);
	scene.add_actor(plane);

	std::vector<std::vector<physx::PxHeightFieldSample>> samples;
	for (int i = 0; i < 5; i++){
		std::vector<physx::PxHeightFieldSample> sample_inner;
		for (int j = 0; j < 5; j++) {
			physx::PxHeightFieldSample sample;
			if ( j < 2)
				sample.height = 2;
			else 
				sample.height = 0;
			sample_inner.push_back(sample);
		}
		samples.push_back(sample_inner);
	}
	PhysicsObject heightfield(create_geometry::heightfield(samples), PhysicsObject::STATIC, true);
	scene.add_actor(heightfield);

	while (true) {
		scene.simulation_step_start(1 / 1000.0f);
		scene.simulation_step_finish();
	}

}