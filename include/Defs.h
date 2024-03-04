#pragma once
#include <semver.hpp>

const char* GetAddonName();
const wchar_t* GetAddonNameW();

const char* GetAddonVersionString();
const semver::version& GetAddonVersion();
