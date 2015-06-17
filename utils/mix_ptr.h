#pragma once

#include <functional>
#include <memory>

namespace utils {

template<typename T>
using mix_ptr = std::unique_ptr<T, std::function<void(T*)> >;

template<typename T>
mix_ptr<T> borrow(T* ptr) {
	return mix_ptr<T>(ptr, [](T*){});
}

template<typename T>
mix_ptr<T> takeover(T* ptr) {
	return mix_ptr<T>(ptr, [](T* p){ delete p; });
}

template<typename T>
mix_ptr<T> takeover(T* ptr, const std::function<void(T*)>& deleter) {
	return mix_ptr<T>(ptr, deleter);
}

template<typename T, typename... Args>
mix_ptr<T> create(Args... args) {
	return mix_ptr<T>(new T(args...), [](T* p){ delete p; });
}

}//namespace
