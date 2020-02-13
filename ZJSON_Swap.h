#include<iostream>

namespace internal {

    //! Custom swap() to avoid dependency on C++ <algorithm> header
    /*! \tparam T Type of the arguments to swap, should be instantiated with primitive C++ types only.
        \note This has the same semantics as std::swap().
    */
    template <typename T>
    inline void Swap(T& a, T& b) noexcept {
        T tmp = a;
        a = b;
        b = tmp;
    }

} // namespace internal



