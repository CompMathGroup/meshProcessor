#ifndef __MESH3D__COMMON_H__
#define __MESH3D__COMMON_H__

#include <string>

namespace mesh3d {

/** Signed index used to enumerate mesh elements (vertices, faces, etc). 
*
* This type is signed, so negative values are valid. -1 is often used to indicate absent element */
typedef signed long long index;

const index BAD_INDEX = -1ll;

/** Internal function to be wrapped in ASSERT macro */
void assert(bool condition, const std::string message, const std::string file, const int line);

}

#if !defined(NDEBUG)
# define MESH3D_ASSERT(x) mesh3d::assert((x), #x, __FILE__, __LINE__)
#else
# define MESH3D_ASSERT(x) (void)(x);
#endif

#endif
