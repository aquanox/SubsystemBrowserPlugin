// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserDescriptor.h"

#include "SubsystemBrowserFlags.h"
#include "SubsystemBrowserUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Model/SubsystemBrowserModel.h"

FSubsystemTreeCategoryItem::FSubsystemTreeCategoryItem(TSharedRef<FSubsystemCategory> InCategory)
	: Data(InCategory)
{

}

TArray<UObject*> FSubsystemTreeCategoryItem::Select(UWorld* InContext) const
{
	TArray<UObject*> OutResult;
	Data->Selector.Execute(InContext, OutResult);
	return OutResult;
}

FSubsystemTreeSubsystemItem::FSubsystemTreeSubsystemItem(UObject* Instance)
{
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

	if (InClass->HasAnyClassFlags(CLASS_Config))
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

FString FSubsystemTreeSubsystemItem::GetShortPackageString() const
{
	return ShortPackage;
}

FString FSubsystemTreeSubsystemItem::GetPackageString() const
{
	return Package;
}

FString FSubsystemTreeSubsystemItem::GetConfigNameString() const
{
	return ConfigName.IsNone() ? FString() : ConfigName.ToString();
}

FString FSubsystemTreeSubsystemItem::GetOwnerNameString() const
{
	return OwnerName;
}

FString FSubsystemTreeSubsystemItem::GetPluginNameString() const
{
	return PluginName;
}
