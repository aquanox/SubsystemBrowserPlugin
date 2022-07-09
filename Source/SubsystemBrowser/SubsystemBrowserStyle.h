// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FSubsystemBrowserStyle final : public FSlateStyleSet
{
public:
	static FName StyleName;

	/** Register style set */
	static void Register();
	/** Unregister style set */
	static void UnRegister();

	/** Access the singleton instance for this style set */
	static FSubsystemBrowserStyle& Get();

	FSubsystemBrowserStyle();
	virtual ~FSubsystemBrowserStyle() = default;

private:

	static TSharedPtr<FSubsystemBrowserStyle> StyleInstance;
};
