#include "PhysicsCortex/core.h"
namespace PhysicsCortex {
	/**
* A particle is the simplest object that can be simulated in the
* physics system.
* they don't have rotation they are just an objects.
*/

	class Particle {

	protected:
		Vector3 Position;
		Vector3 Velocity;
		Vector3 Acceleration;

		//simple innacurate drag force.
		real damping;


		/*
		makes integration simpler and in real time sims
		it's more useful to have infinite mass objects(immovable) than zero mass objects
		*/
		real inverseMass;

		/**
		* Integrates the particle forward in time by the given amount.
		* This function uses a Newton-Euler integration method, which is a
		* linear approximation to the correct integral. For this reason it
		* may be inaccurate in some cases.
		*/
		void integrate(real duration);

		void addForce(const Vector3& force);

		float calcKineticEnergy(real mass, const Vector3 velocity)
		{
			return 0.5f* mass * (velocity * velocity);
 		}
	};

}
