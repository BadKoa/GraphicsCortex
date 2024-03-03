#include <assert.h>
#include <PhysicsCortex/particle.h>

using namespace PhysicsCortex;

void Particle::integrate(real duration)
{
	// don't integrate infinite mass object
	if (inverseMass <= 0.0f) return;

	assert(duration > 0.0);

	//Linear position update
	Position.addScaledVector(Velocity, duration);
	
	Vector3 resultingAcc = Acceleration;

	//update linear velocity based on result acc
	Velocity.addScaledVector(resultingAcc, duration);
	
	//Impose drag
	Velocity *= real_pow(damping, duration);

	//clearAccumulator();
}
