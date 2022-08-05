// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserDescriptor.h"

#include "SubsystemBrowserFlags.h"
#include "SubsystemBrowserUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Model/SubsystemBrowserModel.h"

FSubsystemTreeCategoryItem::FSubsystemTreeCategoryItem(TSharedRef<FSubsystemModel> InModel, TSharedRef<FSubsystemCategory> InCategory)
	: Data(InCategory)
{
	Model = InModel;
}

TArray<UObject*> FSubsystemTreeCategoryItem::Select(UWorld* InContext) const
{
	TArray<UObject*> Result;
	Data->Select(InContext,Result);
	return Result;
}

FSubsystemTreeSubsystemItem::FSubsystemTreeSubsystemItem(TSharedRef<FSubsystemModel> InModel, TSharedPtr<ISubsystemTreeItem> InParent, UObject* Instance)
{
	Model = InModel;
	Parent = InParent;

	check(Instance);
	Subsystem = Instance;

	UClass* const InClass = Instance->GetClass();
	Class = InClass;

	// save data needed to display so hotreload or other things won't crash editor or disaster happen
	// maybe let each column allocate some kind of struct to hold it for own usage?
	DisplayName = InClass->GetDisplayNameText();
	ClassName = InClass->GetFName();
	Package = InClass->GetOuterUPackage()->GetName();
	ShortPackage = FPackageName::GetShortName(Package);
	LongPackage = FString::Printf(TEXT("%s.%s"), *Package, *ClassName.ToString());

	if (InClass->HasAnyClassFlags(CLASS_Config) && !InClass->ClassConfigName.IsNone())
	{
		bConfigExportable = true;
		ConfigName = InClass->ClassConfigName;
	}

	bIsGameModuleClass = FSubsystemBrowserUtils::IsGameModuleClass(InClass);

	FSubsystemBrowserUtils::CollectSourceFiles(InClass, SourceFilePaths);

	OwnerName = FSubsystemBrowserUtils::GetSubsystemOwnerName(Instance);

	TSharedPtr<IPlugin> Plugin = FSubsystemBrowserUtils::GetPluginForClass(InClass);
	if (Plugin.IsValid())
	{
		bIsPluginClass = true;
		PluginName = Plugin->GetName();
#if UE_VERSION_OLDER_THAN(4, 26, 0)
		PluginDisplayName = Plugin->GetName();
#else
		PluginDisplayName = Plugin->GetFriendlyName();
#endif
	}

	bHasViewableProperties = FSubsystemBrowserUtils::HasPropertiesToDisplay(InClass);
}

bool FSubsystemTreeSubsystemItem::IsSelected() const
{
	return Model.IsValid() && Model->IsItemSelected(SharedThis(const_cast<FSubsystemTreeSubsystemItem*>(this)));
}

FText FSubsystemTreeSubsystemItem::GetDisplayName() const
{
	return DisplayName;
}
