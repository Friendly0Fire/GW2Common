#pragma once
#include <type_traits>

template<typename T>
concept enum_with_none = requires(T && t) {
	requires std::is_enum_v<T>;
	T::NONE;
};

template<enum_with_none Enum>
constexpr bool notNone(Enum e) {
	return e != Enum::NONE;
}

template<enum_with_none Enum>
constexpr bool isNone(Enum e) {
	return e == Enum::NONE;
}