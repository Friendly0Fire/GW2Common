#pragma once
#include <algorithm>
#include <cinttypes>
#include <istream>
#include <cwctype>
#include <filesystem>
#include <string>
#include <Common.h>

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr);
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str);

void SplitFilename(const tstring & str, tstring * folder, tstring * file);

mstime TimeInMilliseconds();

// Reverse iteration wrappers for use in range-based for-loops
// ReSharper disable CppInconsistentNaming

// ReSharper disable once CppImplicitDefaultConstructorNotAvailable
template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) { return { iterable }; }

std::span<byte> LoadResource(HMODULE dll, UINT resId);

// ReSharper restore CppInconsistentNaming

inline float Lerp(float a, float b, float s)
{
	if (s < 0)
		return a;
	else if (s > 1)
		return b;
	else
		return (1 - s) * a + s * b;
}

inline float SmoothStep(float x)
{
	return 3 * x * x - 2 * x * x * x;
}

inline float frand()
{
	return float(rand()) / RAND_MAX;
}

template<typename T, typename ET, size_t N>
concept VectorLike = (         requires(T t) { { t.x } -> std::convertible_to<ET>; })
                  && (N < 2 || requires(T t) { { t.y } -> std::convertible_to<ET>; })
                  && (N < 3 || requires(T t) { { t.z } -> std::convertible_to<ET>; })
                  && (N < 4 || requires(T t) { { t.w } -> std::convertible_to<ET>; });

static_assert(VectorLike<glm::vec2, float, 2>);
static_assert(VectorLike<glm::ivec2, int, 2>);
static_assert(VectorLike<glm::vec3, float, 3>);
static_assert(VectorLike<glm::ivec3, int, 3>);
static_assert(VectorLike<glm::vec4, float, 4>);
static_assert(VectorLike<glm::ivec4, int, 4>);
static_assert(VectorLike<ImVec2, float, 2>);
static_assert(VectorLike<ImVec4, float, 4>);

template<typename T>
auto ConvertToVector4(const T& val) {
	const float IntOffset = 0.f;
	// Fundamental types
	if constexpr (std::is_floating_point_v<T>) {
		return fVector4{ float(val), float(val), float(val), float(val) };
	} else if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
		return fVector4{ float(val) + IntOffset, float(val) + IntOffset, float(val) + IntOffset, float(val) + IntOffset };
	// Float vectors
	} else if constexpr (std::is_same_v<T, fVector2>) {
		return fVector4{ val.x, val.y, val.x, val.y };
	} else if constexpr (std::is_same_v<T, fVector3>) {
		return fVector4{ val.x, val.y, val.z, val.x };
	} else if constexpr (std::is_same_v<T, fVector4>) {
		return val;
	// Int vectors
	} else if constexpr (std::is_same_v<T, iVector2>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.x) + IntOffset, float(val.y) + IntOffset };
	} else if constexpr (std::is_same_v<T, iVector3>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.z) + IntOffset, float(val.x) + IntOffset };
	} else if constexpr (std::is_same_v<T, iVector4>) {
		return fVector4{ float(val.x) + IntOffset, float(val.y) + IntOffset, float(val.z) + IntOffset, float(val.w) + IntOffset };
	} else {
		return fVector4{ };
	}
}

uint RoundUpToMultipleOf(uint numToRound, uint multiple);

template<typename Char, typename It>
It SplitString(const Char* str, const Char* delim, It out)
{
	std::basic_string<Char> s(str);
	if(s.empty())
		return out;

	size_t start = 0;
    size_t end = 0;
	while ((end = s.find(delim, start)) != std::string::npos) {
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
	std::transform(in.begin(), in.end(), in.begin(), [&replacements](const C c)
	{
	    for(const auto& [a, b] : replacements)
            if(c == a)
                return b;

        return c;
	});
	return in;
}

template<typename Vec>
float Luma(const Vec& v)
{
    return v.x * 0.2126 + v.y * 0.7152 + v.z * 0.0722;
}

constexpr uint operator "" _len(const char*, size_t len) {
    return uint(len);
}

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
struct ci_char_traits : std::char_traits<T> {
    static bool eq(T c1, T c2) { return SafeToUpper(c1) == SafeToUpper(c2); }
    static bool ne(T c1, T c2) { return SafeToUpper(c1) != SafeToUpper(c2); }
    static bool lt(T c1, T c2) { return SafeToUpper(c1) <  SafeToUpper(c2); }
    static int compare(const T* s1, const T* s2, size_t n) {
        while(n-- != 0) {
            if(SafeToUpper(*s1) < SafeToUpper(*s2)) return -1;
            if(SafeToUpper(*s1) > SafeToUpper(*s2)) return 1;
            ++s1; ++s2;
        }
        return 0;
    }
    static const T* find(const T* s, int n, char a) {
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

template <typename Str>
Str Trim(const Str &scIn) {
    using Char = typename Str::value_type;
    constexpr auto trimmable = []() constexpr {
        if constexpr (std::is_same_v<Char, char>)
            return " \t\n\r";
        else if constexpr (std::is_same_v<Char, wchar_t>)
            return L" \t\n\r";
    }
    ();

    auto start = scIn.find_first_not_of(trimmable);
    auto end = scIn.find_last_not_of(trimmable);

    return scIn.substr(start, end - start + 1);
}

template<typename T>
struct PtrComparator {
	inline bool operator()(const T* a, const T* b) const {
		return (*a) < (*b);
	}
};

std::span<const wchar_t*> GetCommandLineArgs();

const wchar_t* GetCommandLineArg(const wchar_t* name);

template<std::integral T, std::integral T2>
auto RoundUp(T numToRound, T2 multiple) -> std::common_type_t<T, T2>
{
	GW2_ASSERT(multiple > 0);
	return ((numToRound + multiple - 1) / multiple) * multiple;
}

template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };