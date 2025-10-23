// Copyright 2022, Aquanox.

#pragma once

#include "CoreFwd.h"
#include "Misc/EngineVersionComparison.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateBrush.h"
#include "Textures/SlateIcon.h"

#if UE_VERSION_OLDER_THAN(5,1,0)
#include "EditorStyleSet.h"
#else
#include "Styling/AppStyle.h"
#endif

class SUBSYSTEMBROWSER_API FSubsystemBrowserStyle final : public FSlateStyleSet
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

/**
 * Internal class for style system compatibility between older and newer engines.
 */
struct SUBSYSTEMBROWSER_API FStyleHelper
{
	static const FName PanelIconName;
	static const FName FolderOpenName;
	static const FName FolderClosedName;

#if UE_VERSION_OLDER_THAN(5,1,0)
	using StyleSource = FEditorStyle;
#else
	using StyleSource = FAppStyle;
#endif

	static const ISlateStyle& Get()
	{
		return StyleSource::Get();
	}
	
	static FName GetStyleSetName()
	{
#if UE_VERSION_OLDER_THAN(5,1,0)
		return FEditorStyle::GetStyleSetName();
#else
		return FAppStyle::GetAppStyleSetName();
#endif
	}

	static const FSlateBrush* GetBrush(const FName& InName)
	{
		return StyleSource::GetBrush(InName);
	}

	static FSlateFontInfo GetFontStyle(const FName& InName)
	{
		return StyleSource::GetFontStyle(InName);
	}

	static FSlateIcon GetSlateIcon(const FName& InIcon)
	{
		return FSlateIcon( GetStyleSetName(), InIcon);
	}
	
	static FSlateColor GetSlateColor(const FName& Name)
	{
		return StyleSource::GetSlateColor(Name);
	}

	template<typename T>
	static const T& GetWidgetStyle(const FName& InName)
	{
		return StyleSource::GetWidgetStyle<T>(InName);
	}

	template<typename T>
	static const T* GetWidgetStylePtr(const FName& InName)
	{
		return &StyleSource::GetWidgetStyle<T>(InName);
	}
};
