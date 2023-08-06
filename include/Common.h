#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include <d3d11.h>
#include <wrl.h>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include "Assertions.h"
#include "BaseCore.h"
#include "Defs.h"
#include "EnumUtils.h"
#include "Log.h"
#include "Singleton.h"
#include "Win.h"

#define COM_RELEASE(x) if(x) { x->Release(); x = nullptr; }
#define NULL_COALESCE(a, b) ((a) != nullptr ? (a) : (b))
#define OPT_COALESCE(a, b) ((a) ? (a) : (b))
#define SQUARE(x) ((x) * (x))
#define implicit explicit(false)

using Microsoft::WRL::ComPtr;

using uchar = unsigned char;
using uint = unsigned int;
using ushort = unsigned short;
using tstring = std::basic_string<TCHAR>;
using mstime = unsigned __int64;

using std::tie;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifdef _DEBUG
#define HOT_RELOAD_SHADERS
#endif

struct fVector4
{
    float x;
    float y;
    float z;
    float w;
};

struct fVector3
{
    float x;
    float y;
    float z;
};

struct fVector2
{
    float x;
    float y;
};

struct iVector4
{
    int x;
    int y;
    int z;
    int w;
};

struct iVector3
{
    int x;
    int y;
    int z;
};

struct iVector2
{
    int x;
    int y;
};

struct fMatrix44
{
    float mat[3][3];
};

int CRTReportHook(int reportType, char* message, int* returnValue);
