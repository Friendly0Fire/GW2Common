#pragma once
#include <type_traits>

template<typename T>
concept enum_with_none = requires(T && t) {
	requires std::is_enum_v<T>;
	T::NONE;
};

template<typename T>
concept iteratable_enum = requires(T && t) {
	requires std::is_enum_v<T>;
	T::FIRST <= T::LAST;
};

template<enum_with_none Enum>
constexpr bool notNone(Enum e) {
	return e != Enum::NONE;
}

template<enum_with_none Enum>
constexpr bool isNone(Enum e) {
	return e == Enum::NONE;
}

template<iteratable_enum E>
void iterateEnum(std::function<void(E)> cb) {
	for (auto i = E::FIRST; i <= E::LAST; i = static_cast<E>(std::underlying_type_t<E>(i) + 1))
		cb(i);
}