// Copyright 2022, Aquanox.

#pragma once

/**
 * Toggle example code validation
 */
#define ENABLE_SUBSYSTEM_BROWSER_EXAMPLES 0

/**
 * Dynamic column toggle works only from 4.26 onwards
 * This allows to toggle on-off them in runtime until rewrite of the dynamic columns
 */
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
#define ENABLE_SUBSYSTEM_BROWSER_DYNAMIC_COLUMNS 1
#else
#define ENABLE_SUBSYSTEM_BROWSER_DYNAMIC_COLUMNS 0
#endif

/**
 * WIP: Custom columns in tree table
 */
#define ENABLE_SUBSYSTEM_BROWSER_CUSTOM_COLUMNS 0