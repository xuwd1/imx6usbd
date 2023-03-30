#include <cstdint>
#include <cstddef>
#include <type_traits>

template <typename T>
inline T BE32(const T& x){
    static_assert(std::is_integral_v<T>,"");
    static_assert(sizeof(T)==sizeof(uint32_t),"");
    return __builtin_bswap32(static_cast<uint32_t>(x));
}

template <typename T>
inline T BE16(const T& x){
    static_assert(std::is_integral_v<T>,"");
    static_assert(sizeof(T)==sizeof(uint16_t),"");
    return __builtin_bswap16(static_cast<uint16_t>(x));
}


template <typename T>
inline T align(const T& x, const size_t alignto){
    return ((x + alignto -1)/alignto)*alignto;
}

template <typename T>
inline T min(const T& x, const T& y){
    return x<y?x:y;
}