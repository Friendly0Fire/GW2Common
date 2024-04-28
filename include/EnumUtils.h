#pragma once
#include <type_traits>

template<typename T>
concept Enum = std::is_enum_v<T>;

template<typename T>
concept EnumWithNone = Enum<T> && requires() { static_cast<std::underlying_type_t<T>>(T::None) == 0; };

template<typename T>
concept IteratableEnum = Enum<T> && requires() { T::First <= T::Last; };

template<typename T>
concept SizedEnum = Enum<T> && requires() { T::Count; };

template<SizedEnum T>
constexpr auto EnumSize() {
    return std::to_underlying(T::Count);
}

template<IteratableEnum T>
constexpr auto EnumSize() {
    return std::to_underlying(T::Last) - std::to_underlying(T::First) + 1;
}

template<EnumWithNone T>
constexpr bool NotNone(T e) {
    return e != T::None;
}

template<EnumWithNone T>
constexpr bool IsNone(T e) {
    return e == T::None;
}

template<IteratableEnum T>
void IterateEnum(std::function<void(T)> cb) {
    for(auto i = T::First; i <= T::Last; i = static_cast<T>(std::underlying_type_t<T>(i) + 1))
        cb(i);
}

template<typename T>
concept EnumIsFlag = Enum<T> && requires(T e) { T::IsFlag; };

template<typename T>
auto ToUnderlying(T e) {
    return static_cast<std::underlying_type_t<T>>(e);
}

template<EnumIsFlag T>
constexpr T operator&(T a, T b) {
    return static_cast<T>(ToUnderlying(a) & ToUnderlying(b));
}

template<EnumIsFlag T>
constexpr T operator|(T a, T b) {
    return static_cast<T>(ToUnderlying(a) | ToUnderlying(b));
}

template<EnumIsFlag T>
constexpr T operator~(T e) {
    return static_cast<T>(~ToUnderlying(e));
}

template<EnumIsFlag T>
constexpr T& operator|=(T& a, T b) {
    a = a | b;
    return a;
}

template<EnumIsFlag T>
constexpr T& operator&=(T& a, T b) {
    a = a & b;
    return a;
}
