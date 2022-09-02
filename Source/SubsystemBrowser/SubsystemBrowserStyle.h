// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateBrush.h"
#include "Textures/SlateIcon.h"

class FSubsystemBrowserStyle final : public FSlateStyleSet
{
public:
	static const FName StyleName;

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

struct FStyleHelper
{
	static const ISlateStyle& Get();
	static const FSlateBrush* GetBrush(const FName& InName);
	static FSlateFontInfo GetFontStyle(const FName& InName);
	static FSlateIcon GetSlateIcon(const FName& InIcon);
	static FSlateColor GetSlateColor(const FName& Name);
};
