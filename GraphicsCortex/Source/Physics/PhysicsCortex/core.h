#include "PhysicsCortex/precision.h"
namespace PhysicsCortex {

	class Vector3 {
	public:
		real x;
		real y;
		real z;

		/** The default constructor creates a zero vector. */
		Vector3() : x(0), y(0), z(0) {}

		/**
			* The explicit constructor creates a vector with the given
			* components.
		*/
		Vector3(const real x, const real y, const real z)
			: x(x), y(y), z(z) {}

		void invert()
		{
			x = -x;
			y = -y;
			x = -z;
		}

		real magnitude()
		{
			//return real_sqrt(x * x + y * y + z * z);
		}
		
		//This is a faster process, because it avoids the call to sqrt which can be slow on some machines
		real square_magnitude()
		{
			return x * x + y * y + z * z;
		}

		void normalize()
		{
			real l = magnitude();
			if (l > 0)
			{
				(*this) *= ((real)1) / l;
			}
		}

		//*= operator definition in order to use it.
		void operator*=(const real value)
		{
			x *= value;
			y *= value;
			z *= value;
		}

		//vector scaling
		Vector3 operator*(const real value) const
		{
			return Vector3(x * value, y * value, z * value);
		}

		//vector adition
		Vector3 operator+=(const Vector3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
		}

		Vector3 operator+(const Vector3& v) const
		{
			return Vector3(x + v.x, y + v.y, z + v.z);
		}

		Vector3 operator-=(const Vector3& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
		}

		Vector3 operator-(const Vector3& v) const
		{
			return Vector3(x - v.x, y - v.y, z - v.z);
		}

		void addScaledVector(const Vector3& vector, real scale)
		{
			x += vector.x * scale;
			y += vector.y * scale;
			z += vector.z * scale;
		}
		
		//component wise product.
		Vector3 componentProduct(const Vector3& vector) const
		{
			return Vector3(x * vector.x, y * vector.y, z * vector.z);
		}

		/**
			* Performs a component-wise product with the given vector and
			* sets this vector to its result.
		*/
		void componentProductUpdate(const Vector3& vector)
		{
			x *= vector.x;
			y *= vector.y;
			z *= vector.z;
		}

		// since scalar product returns a scaler rather than a vector i didn't define *= operator for it.
		//also known as dot product.
		real scalarProduct(const Vector3& vector) const
		{
			return x * vector.x + y * vector.y + z * vector.z;
		}

		real operator *(const Vector3& vector) const
		{
			return x * vector.x + y * vector.y + z * vector.z;
		}


		/**
			* Calculates and returns the vector product of this vector
			* with the given vector.
			* also known as cross product.
		*/
		Vector3 vectorProduct(const Vector3& vector) const
		{
			return Vector3(y * vector.z - z * vector.y,
				z * vector.x - x * vector.z,
				x * vector.y - y * vector.x);
		}

		/**
			* Updates this vector to be the vector product of its current
			* value and the given vector.
		*/
		void operator %=(const Vector3& vector)
		{
			*this = vectorProduct(vector);
		}
		/**
			* Calculates and returns the vector product of this vector
			* with the given vector.
		*/
		Vector3 operator%(const Vector3& vector) const
		{
			return Vector3(y * vector.z - z * vector.y,
				z * vector.x - x * vector.z,
				x * vector.y - y * vector.x);
		}
		
		void makeOrthonormalBasis(Vector3* a, Vector3* b, Vector3* c)
		{
			a->normalize();
			(*c) = (*a) % (*b);
			//if (c.squareMagnitude() == 0.0) return; // Or generate an error.
			c->normalize();
			(*b) = (*c) % (*a);
		}

	private:
		real pad;
	};
}
