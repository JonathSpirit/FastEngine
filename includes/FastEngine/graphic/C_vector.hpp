#ifndef _FGE_C_VECTOR_HPP_INCLUDED
#define _FGE_C_VECTOR_HPP_INCLUDED

#include <cstdint>
#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>

namespace fge
{

template<class T>
using Vector2 = glm::vec<2, T>;
template<class T>
using Vector3 = glm::vec<3, T>;

using Vector2i = Vector2<int32_t>;
using Vector2u = Vector2<uint32_t>;
using Vector2f = Vector2<float>;

using Vector3i = Vector3<int32_t>;
using Vector3u = Vector3<uint32_t>;
using Vector3f = Vector3<float>;

}//end fge

#endif //_FGE_C_VECTOR_HPP_INCLUDED
