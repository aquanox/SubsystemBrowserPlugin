// Copyright 2022, Aquanox.

#include "SubsystemBrowserStyle.h"

#include "SubsystemBrowserFlags.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

const FName FSubsystemBrowserStyle::StyleName("SubsystemBrowserStyle");
TSharedPtr<FSubsystemBrowserStyle> FSubsystemBrowserStyle::StyleInstance;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
const FName FStyleHelper::PanelIconName(TEXT("LevelEditor.GameSettings.Small"));
const FName FStyleHelper::FolderOpenName(TEXT("SceneOutliner.FolderOpen"));
const FName FStyleHelper::FolderClosedName(TEXT("SceneOutliner.FolderClosed"));
#else
const FName FStyleHelper::PanelIconName(TEXT("Icons.Settings"));
const FName FStyleHelper::FolderOpenName(TEXT("Icons.FolderOpen"));
const FName FStyleHelper::FolderClosedName(TEXT("Icons.FolderClosed"));
#endif

void FSubsystemBrowserStyle::Register()
{
	check(!StyleInstance.IsValid());
	StyleInstance = MakeShared<FSubsystemBrowserStyle>();

	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
}

void FSubsystemBrowserStyle::UnRegister()
{
	check(StyleInstance.IsValid());

	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);

	StyleInstance.Reset();
}

FSubsystemBrowserStyle& FSubsystemBrowserStyle::Get()
{
	check(StyleInstance.IsValid());
	return *StyleInstance;
}

FSubsystemBrowserStyle::FSubsystemBrowserStyle() : FSlateStyleSet(StyleName)
{
	const FVector2D Icon16x16(16.f, 16.f);
	const FVector2D Icon64x64(64.f, 64.f);

	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("SubsystemBrowserPlugin"));
	check(Plugin.IsValid());
	FSlateStyleSet::SetContentRoot(FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources")));
	FSlateStyleSet::SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	// register styles here
	// Set("SubsystemBrowser.TabIcon", new IMAGE_BRUSH("Icons/SubsystemBrowser_16x", Icon16x16));
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
