#ifndef __MESH3D__COMMON_H__
#define __MESH3D__COMMON_H__

#include "config.h"

#include <string>
#include <vector>
#include <cstddef>
#include <limits>

namespace mesh3d {

/** Index used to enumerate mesh elements (vertices, faces, etc). 
*
* BAD_INDEX is used to indicate absent element */
typedef size_t index;

const index BAD_INDEX = std::numeric_limits<size_t>::max();

/** Internal function to be wrapped in ASSERT macro. Is not stripped in release */
void assert(bool condition, const std::string &message, const std::string &file, const int line);

/** A class for holding a vector of pointers
*
* Element access is done by reference, bind is used to replace element. 
* Pointers are deleted if they are replaced with bind or ptr_vector is destroyed */
template <class T>
class ptr_vector {
	std::vector<T *> _data;
	ptr_vector(const ptr_vector &);
	ptr_vector &operator =(const ptr_vector &);
public:
	/** Construct empty ptr_vector */
	ptr_vector() { }
	/** Construct ptr_vector of specified size and optional default pointer */
	explicit ptr_vector(size_t n, T *p = 0) : _data(n, p) { }
	/** Get reference to an element */
	T &operator[](ptrdiff_t i) { return *_data[i]; }
	/** Get const reference to an element */
	const T &operator[](ptrdiff_t i) const { return *_data[i]; }
	/** Replace a pointer with another deleting old one */
	void bind(ptrdiff_t i, T *p) { if (_data[i] == p) return; delete _data[i]; _data[i] = p; }
	/** Release a pointer, without deleting it */
	T *release(ptrdiff_t i) { T *tmp = _data[i]; _data[i] = 0; return tmp; }
	/** Destroy ptr_vector and delete every pointer */
	~ptr_vector() { for (typename std::vector<T *>::iterator it = _data.begin(); it != _data.end(); ++it) delete *it; }
	/** Get ptr_vector size */
	size_t size() const { return _data.size(); }
	/** Resize ptr_vector */
	void resize(size_t newsize, T *p = 0) { _data.resize(newsize, p); }
	/** Add a pointer to ptr_vector */
	void push_back(T *p) { _data.push_back(p); }
	/** Return reference to last element */
	T &back() { return *_data.back(); }
	/** Return const reference to last element */
	const T &back() const { return *_data.back(); }
};

/** A class representing float3 or double3 datatype */
template <class T>
struct vec {
	T x; //!< x component
	T y; //!< y component
	T z; //!< z component
	/** Construct with zeros */
	vec() : x(0), y(0), z(0) { }
	/** Construct from components */
	vec(T x, T y, T z) : x(x), y(y), z(z) { }
};

}

#if !defined(NDEBUG)
# define MESH3D_ASSERT(x) mesh3d::assert((x), #x, __FILE__, __LINE__)
#else
# define MESH3D_ASSERT(x) (void)(x);
#endif

#endif
