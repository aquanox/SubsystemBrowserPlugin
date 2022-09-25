// Copyright 2022, Aquanox.

#pragma once

#include "Misc/EngineVersionComparison.h"

// just two little helpers to help reading code

#define SINCE_UE_VERSION(MajorVersion, MinorVersion, PatchVersion) \
		!UE_VERSION_OLDER_THAN(MajorVersion, MinorVersion, PatchVersion)

#define BEFORE_UE_VERSION(MajorVersion, MinorVersion, PatchVersion) \
		!UE_VERSION_NEWER_THAN(MajorVersion, MinorVersion, PatchVersion)

/**
 * Toggle example code validation
 */
#define ENABLE_SUBSYSTEM_BROWSER_EXAMPLES 0
