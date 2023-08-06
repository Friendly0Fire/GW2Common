#pragma once
#include <neargye/semver.hpp>

const char* GetAddonName();
const wchar_t* GetAddonNameW();

const char* GetAddonVersionString();
const semver::version& GetAddonVersion();
