// Copyright 2022, Aquanox.

#pragma once

#include "CoreFwd.h"
#include "SubsystemBrowserFlags.h"
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

	static const FName PanelIconName;
	static const FName FolderOpenName;
	static const FName FolderClosedName;

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
	static const ISlateStyle& Get();
	static const FSlateBrush* GetBrush(const FName& InName);
	static FSlateFontInfo GetFontStyle(const FName& InName);
	static FSlateIcon GetSlateIcon(const FName& InIcon);
	static FSlateColor GetSlateColor(const FName& Name);

	template<typename T>
	static const T& GetWidgetStyle(const FName& InName)
	{
#if UE_VERSION_OLDER_THAN(5,1,0)
		return FEditorStyle::GetWidgetStyle<T>(InName);
#else
		return FAppStyle::GetWidgetStyle<T>(InName);
#endif
	}

	template<typename T>
	static const T* GetWidgetStylePtr(const FName& InName)
	{
		return &GetWidgetStyle<T>(InName);
	}
};
