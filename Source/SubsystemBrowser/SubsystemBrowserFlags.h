// Copyright 2022, Aquanox.

#pragma once

#include "Misc/EngineVersionComparison.h"

// just two little helpers to help reading code

#define SINCE_UE_VERSION(MajorVersion, MinorVersion, PatchVersion) \
		!UE_VERSION_OLDER_THAN(MajorVersion, MinorVersion, PatchVersion)

#define BEFORE_UE_VERSION(MajorVersion, MinorVersion, PatchVersion) \
		!UE_VERSION_NEWER_THAN(MajorVersion, MinorVersion, PatchVersion)

/**
 * Should tab be a nomad or level editor tab?
 * 0 = Tab is bound to Level Editor
 * 1 = Tab is free
 */
#define SUBSYSTEM_BROWSER_NOMAD_MODE 0

/**
 * Toggle example code validation
 */
#define ENABLE_SUBSYSTEM_BROWSER_EXAMPLES 0

/**
 * Internal debugging things flag.
 */
#define ENABLE_SUBSYSTEM_BROWSER_DEBUG_THINGS 0
