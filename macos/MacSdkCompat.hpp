#pragma once

// Upstream Main.cpp is written against RED4ext API v1 names. The macOS SDK
// exposes the same plugin entry points under the v0 names. This header is
// included only from `#ifdef __APPLE__`, so the Windows view of Main.cpp
// stays the tag text.

#include <RED4ext/RED4ext.hpp>

namespace RED4ext::v1
{
using PluginHandle = ::RED4ext::PluginHandle;
using EMainReason = ::RED4ext::EMainReason;
using Sdk = ::RED4ext::Sdk;
using PluginInfo = ::RED4ext::PluginInfo;
using Logger = ::RED4ext::Logger;
}

#ifndef RED4EXT_V1_SEMVER
#define RED4EXT_V1_SEMVER(major, minor, patch) RED4EXT_SEMVER(major, minor, patch)
#endif

#ifndef RED4EXT_V1_RUNTIME_VERSION_2_31
#define RED4EXT_V1_RUNTIME_VERSION_2_31 RED4EXT_RUNTIME_2_31
#endif

#ifndef RED4EXT_V1_SDK_VERSION_CURRENT
#define RED4EXT_V1_SDK_VERSION_CURRENT RED4EXT_SDK_LATEST
#endif

// The macOS loader has one API version. Supports() in Main.cpp still says
// RED4EXT_API_VERSION_1; on Apple that token is this loader's latest version.
#ifndef RED4EXT_API_VERSION_1
#define RED4EXT_API_VERSION_1 RED4EXT_API_VERSION_LATEST
#endif
