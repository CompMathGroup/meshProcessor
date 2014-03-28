#ifndef __MESH3D__VECTOR_H__
#define __MESH3D__VECTOR_H__

#include <cmath>

namespace mesh3d {

/** A struct representing 3D vector */
struct vector {
	double x; //!< vector x component
	double y; //!< vector y component
	double z; //!< vector z component
	/** Construct zero vector */
	vector() : x(0), y(0), z(0) { }
	/** Construct a vector with all components equal to a */
	explicit vector(double a) : x(a), y(a), z(a) { }
	/** Construct vector from another vector r */
	vector(const vector &r) : x(r.x), y(r.y), z(r.z) { }
	/** Construct vector from components */
	vector(double x, double y, double z) : x(x), y(y), z(z) { }
#define BINOP(op) \
	const vector operator op(const vector &r) const { \
		return vector(x op r.x, y op r.y, z op r.z); \
	}
	
	/** Add two vectors */
	BINOP(+)
	/** Subtract two vectors */
	BINOP(-)
	/** Element-wise multiply two vectors */
	BINOP(*)
	/** Element-wise divide two vectors */
	BINOP(/)

#undef BINOP

	/** Multiply vector by number a */
	const vector operator *(const double a) const {
		return vector(a * x, a * y, a * z);
	}
	/** Divide vector by number a */
	const vector operator /(const double a) const {
		return vector(x / a, y / a, z / a);
	}
	/** Negate a vector */
	const vector operator -() const {
		return vector(-x, -y, -z);
	}
	/** Scale vector by number a */
	vector &operator *=(double a) {
		x *= a;
		y *= a;
		z *= a;
		return *this;
	}
	/** Cross-product vector with another vector b */
	const vector operator %(const vector &b) const {
		const vector &a = *this;
		return vector(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x);
	}
	/** Return square of vector's euclidean norm */
	double norm2() const {
		return x * x + y * y + z * z;
	}
	/** Return vector's euclidean norm */
	double norm() const {
		return sqrt(norm2());
	}
	/** Dot-product vector with another vector b */
	double dot(const vector &b) const {
		return x * b.x + y * b.y + z * b.z;
	}

#define COMPOP(op) \
	vector &operator op(const vector &b) { \
		x op b.x; \
		y op b.y; \
		z op b.z; \
		return *this; \
	}

	/** Add another vector to this vector */
	COMPOP(+=)
	/** Subtract another vector from this vector */
	COMPOP(-=)
	/** Element-wise multiply this vector by another vector */
	COMPOP(*=)
	/** Element-wise divide this vector by another vector */
	COMPOP(/=)

#undef COMPOP
};

/** Multiply vector b by number a */
inline const vector operator *(double a, const vector &b) {
	return b * a;
}

/** Return dot-product of two vectors */
inline double dot(const vector &a, const vector &b) {
	return a.dot(b);
}

/** Return triple product of three vectors, i.e. a.dot(b.cross(c)) */
inline double triple(const vector &a, const vector &b, const vector &c) {
	return a.dot(b % c);
}

/** Return squared norm of vector a */
inline double norm2(const vector &a) {
	return a.norm2();
}

/** Return norm of vector a */
inline double norm(const vector &a) {
	return a.norm();
}

}

#include <ostream>

namespace mesh3d {

/** Pretty print vector r to output stream o */
inline std::ostream &operator <<(std::ostream &o, const vector &r) {
	return o << "(" << r.x << ", " << r.y << ", " << r.z << ")";
}

}

#endif 
