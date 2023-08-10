#pragma once
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <mutex>
#include <numbers>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include <d3d11.h>
#include <wrl.h>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include "Defs.h"
#include "Singleton.h"

#define COM_RELEASE(x) \
    if(x) {            \
        x->Release();  \
        x = nullptr;   \
    }
#define NULL_COALESCE(a, b) ((a) != nullptr ? (a) : (b))
#define OPT_COALESCE(a, b) ((a) ? (a) : (b))
#define SQUARE(x) ((x) * (x))
#define implicit explicit(false)
#define FWD(x) static_cast<decltype(x)&&>(x)
#define RETURNS(expr) \
    noexcept(noexcept(expr))->decltype(expr) { return expr; }
#define OVERLOADS_OF(name) [](auto&&... args) RETURNS(name(FWD(args)...))

using Microsoft::WRL::ComPtr;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using f32 = float;
using f64 = double;
using tstring = std::basic_string<TCHAR>;
using mstime = unsigned __int64;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;

using std::tie;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif

#ifdef _DEBUG
#define HOT_RELOAD_SHADERS
#endif

i32 CRTReportHook(i32 reportType, char* message, i32* returnValue);

#include "Assertions.h"
#include "BaseCore.h"
#include "EnumUtils.h"
#include "Log.h"
#include "Win.h"
#include "Utility.h"