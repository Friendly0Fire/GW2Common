#pragma once
#include <algorithm>
#include <cinttypes>
#include <cwctype>
#include <filesystem>
#include <string>

#include "Common.h"

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr);
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str);

void SplitFilename(const tstring& str, tstring* folder, tstring* file);

mstime TimeInMilliseconds();

// Reverse iteration wrappers for use in range-based for-loops
// ReSharper disable CppInconsistentNaming

// ReSharper disable once CppImplicitDefaultConstructorNotAvailable
template<typename T>
struct reversion_wrapper
{
    T& iterable;
};

template<typename T>
auto begin(reversion_wrapper<T> w) {
    return std::rbegin(w.iterable);
}

template<typename T>
auto end(reversion_wrapper<T> w) {
    return std::rend(w.iterable);
}

template<typename T>
reversion_wrapper<T> reverse(T&& iterable) {
    return { iterable };
}

std::span<byte> LoadResource(HMODULE dll, UINT resId);

// ReSharper restore CppInconsistentNaming

inline f32 Lerp(f32 a, f32 b, f32 s) {
    if(s < 0)
        return a;
    else if(s > 1)
        return b;
    else
        return (1 - s) * a + s * b;
}

template<typename T> requires std::is_arithmetic_v<T>
T Clamp(T x, T min, T max) {
    if (x < min)
        return min;
    if (x > max)
        return max;

    return x;
}

inline f32 SmoothStep(f32 x) { return 3 * x * x - 2 * x * x * x; }

inline f32 frand() { return f32(rand()) / RAND_MAX; }

template<typename T, typename ET, size_t N>
concept VectorLike = (requires(T t) {
                         { t.x } -> std::convertible_to<ET>;
                     }) && (N < 2 || requires(T t) {
                         { t.y } -> std::convertible_to<ET>;
                     }) && (N < 3 || requires(T t) {
                         { t.z } -> std::convertible_to<ET>;
                     }) && (N < 4 || requires(T t) {
                         { t.w } -> std::convertible_to<ET>;
                     });

static_assert(VectorLike<vec2, f32, 2>);
static_assert(VectorLike<vec4, i32, 2>);
static_assert(VectorLike<vec3, f32, 3>);
static_assert(VectorLike<ivec3, i32, 3>);
static_assert(VectorLike<vec4, f32, 4>);
static_assert(VectorLike<ivec4, i32, 4>);

template<typename T>
vec4 ConvertToVector4(const T& val) {
    const f32 IntOffset = 0.f;
    // Fundamental types
    if constexpr(std::is_floating_point_v<T>) {
        return { f32(val), f32(val), f32(val), f32(val) };
    }
    else if constexpr(std::is_integral_v<T> || std::is_enum_v<T>) {
        return { f32(val) + IntOffset, f32(val) + IntOffset, f32(val) + IntOffset, f32(val) + IntOffset };
        // Float vectors
    }
    else if constexpr(std::is_same_v<T, vec2>) {
        return { val.x, val.y, val.x, val.y };
    }
    else if constexpr(std::is_same_v<T, vec3>) {
        return { val.x, val.y, val.z, val.x };
    }
    else if constexpr(std::is_same_v<T, vec4>) {
        return val;
        // Int vectors
    }
    else if constexpr(std::is_same_v<T, ivec2>) {
        return { f32(val.x) + IntOffset, f32(val.y) + IntOffset, f32(val.x) + IntOffset, f32(val.y) + IntOffset };
    }
    else if constexpr(std::is_same_v<T, ivec3>) {
        return { f32(val.x) + IntOffset, f32(val.y) + IntOffset, f32(val.z) + IntOffset, f32(val.x) + IntOffset };
    }
    else if constexpr(std::is_same_v<T, ivec4>) {
        return { f32(val.x) + IntOffset, f32(val.y) + IntOffset, f32(val.z) + IntOffset, f32(val.w) + IntOffset };
    }
    else {
        return { };
    }
}

u32 RoundUpToMultipleOf(u32 numToRound, u32 multiple);

template<typename Char, typename It>
It SplitString(const Char* str, const Char* delim, It out) {
    std::basic_string<Char> s(str);
    if(s.empty())
        return out;

    size_t start = 0;
    size_t end = 0;
    while((end = s.find(delim, start)) != std::string::npos) {
        *out = s.substr(start, end - start);
        ++out;
        start = end + 1;
    }

    *out = s.substr(start);
    ++out;
    return out;
}

inline std::string ToLower(std::string in) {
    std::transform(in.begin(), in.end(), in.begin(), [](const char c) { return std::tolower(uint8_t(c)); });
    return in;
}
inline std::wstring ToLower(std::wstring in) {
    std::transform(in.begin(), in.end(), in.begin(), [](const wchar_t c) { return std::towlower(uint16_t(c)); });
    return in;
}

inline std::string ToUpper(std::string in) {
    std::transform(in.begin(), in.end(), in.begin(), [](const char c) { return std::toupper(uint8_t(c)); });
    return in;
}
inline std::wstring ToUpper(std::wstring in) {
    std::transform(in.begin(), in.end(), in.begin(), [](const wchar_t c) { return std::towupper(uint16_t(c)); });
    return in;
}

template<typename C>
std::basic_string<C> ReplaceChar(std::basic_string<C> in, C a, C b) {
    std::transform(in.begin(), in.end(), in.begin(), [a, b](const C c) { return c == a ? b : c; });
    return in;
}

template<typename C>
std::basic_string<C> ReplaceChars(std::basic_string<C> in, std::initializer_list<std::pair<C, C>> replacements) {
    std::transform(in.begin(), in.end(), in.begin(), [&replacements](const C c) {
        for(const auto& [a, b] : replacements)
            if(c == a)
                return b;

        return c;
    });
    return in;
}

template<typename Vec>
f32 Luma(const Vec& v) {
    return v.x * 0.2126 + v.y * 0.7152 + v.z * 0.0722;
}

constexpr u32 operator"" _len(const char*, size_t len) { return u32(len); }

std::filesystem::path GetGameFolder();
std::optional<std::filesystem::path> GetDocumentsFolder();
std::optional<std::filesystem::path> GetAddonFolder();

template<typename T>
T SafeToUpper(T c) {
    if constexpr(std::is_same_v<T, char>)
        return std::toupper(uint8_t(c));
    else
        return std::towupper(uint16_t(c));
}

template<typename T>
struct ci_char_traits : std::char_traits<T>
{
    static bool eq(T c1, T c2) { return SafeToUpper(c1) == SafeToUpper(c2); }
    static bool ne(T c1, T c2) { return SafeToUpper(c1) != SafeToUpper(c2); }
    static bool lt(T c1, T c2) { return SafeToUpper(c1) < SafeToUpper(c2); }
    static i32 compare(const T* s1, const T* s2, size_t n) {
        while(n-- != 0) {
            if(SafeToUpper(*s1) < SafeToUpper(*s2))
                return -1;
            if(SafeToUpper(*s1) > SafeToUpper(*s2))
                return 1;
            ++s1;
            ++s2;
        }
        return 0;
    }
    static const T* find(const T* s, i32 n, char a) {
        while(n-- > 0 && SafeToUpper(*s) != toupper(a)) {
            ++s;
        }
        return s;
    }
};

template<typename T>
concept string_like = requires(T&& t) {
    requires std::is_pointer_v<decltype(t.data())>;
    { t.size() } -> std::convertible_to<size_t>;
    typename T::value_type;
};

template<string_like T>
auto ToCaseInsensitive(const T& s) {
    using V = typename T::value_type;
    return std::basic_string_view<V, ci_char_traits<V>>(s.data(), s.size());
}

template<typename Str>
Str Trim(const Str& scIn) {
    using Char = typename Str::value_type;
    constexpr auto trimmable = []() constexpr {
        if constexpr(std::is_same_v<Char, char>)
            return " \t\n\r";
        else if constexpr(std::is_same_v<Char, wchar_t>)
            return L" \t\n\r";
    }();

    auto start = scIn.find_first_not_of(trimmable);
    auto end = scIn.find_last_not_of(trimmable);

    return scIn.substr(start, end - start + 1);
}

template<typename T>
struct PtrComparator
{
    inline bool operator()(const T* a, const T* b) const { return (*a) < (*b); }
};

std::span<const wchar_t*> GetCommandLineArgs();

const wchar_t* GetCommandLineArg(const wchar_t* name);

template<std::integral T, std::integral T2>
auto RoundUp(T numToRound, T2 multiple) -> std::common_type_t<T, T2> {
    GW2_ASSERT(multiple > 0);
    return ((numToRound + multiple - 1) / multiple) * multiple;
}

template<typename... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};

template<typename F, typename T>
concept CanCall = requires(F f, T t) {
    f(t);
};

template<typename... Ts>
struct PartialOverloaded : Ts...
{
    using Ts::operator()...;

    template<typename T>
    void operator()(T&&) requires (!CanCall<Ts, T> && ...) {}
};

RTL_OSVERSIONINFOW GetOSVersion();

std::string GetCpuInfo();

void LogCurrentModules();

template<typename T, typename V>
consteval size_t get_index() {
    size_t r = 0;
    auto test = [&](bool b) {
        if(!b)
            ++r;
        return b;
    };
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (test(std::is_same_v<T, std::variant_alternative_t<Is, V>>) || ...);
    }(std::make_index_sequence<std::variant_size_v<V>>());

    return r;
}
static_assert(get_index<float, std::variant<float, double, int>>() == 0);
static_assert(get_index<double, std::variant<float, double, int>>() == 1);
static_assert(get_index<int, std::variant<float, double, int>>() == 2);

namespace glm
{

template<typename T>
    requires (T::length() == 2)
void to_json(nlohmann::json& j, const T& v) {
    j = nlohmann::json::array({ v.x, v.y });
}

template<typename T>
    requires (T::length() == 3)
void to_json(nlohmann::json& j, const T& v) {
    j = nlohmann::json::array({ v.x, v.y, v.z });
}

template<typename T>
    requires (T::length() == 4)
void to_json(nlohmann::json& j, const T& v) {
    j = nlohmann::json::array({ v.x, v.y, v.z, v.w });
}

template<typename T>
    requires (T::length() == 2)
void from_json(const nlohmann::json& j, T& v) {
    j.at(0).get_to(v.x);
    j.at(1).get_to(v.y);
}

template<typename T>
    requires (T::length() == 3)
void from_json(const nlohmann::json& j, T& v) {
    j.at(0).get_to(v.x);
    j.at(1).get_to(v.y);
    j.at(2).get_to(v.z);
}

template<typename T>
    requires (T::length() == 4)
void from_json(const nlohmann::json& j, T& v) {
    j.at(0).get_to(v.x);
    j.at(1).get_to(v.y);
    j.at(2).get_to(v.z);
    j.at(2).get_to(v.w);
}

} // namespace glm