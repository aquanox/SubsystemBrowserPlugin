// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserDescriptor.h"

#include "SubsystemBrowserFlags.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Model/SubsystemBrowserModel.h"
#include "UI/SubsystemTableItemTooltip.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

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

void FSubsystemTreeCategoryItem::GenerateTooltip(FSubsystemTableItemTooltipBuilder& TooltipBuilder) const
{
	TArray<SubsystemTreeItemPtr> Subsystems;
	Model->GetAllSubsystemsInCategory(SharedThis(this), Subsystems);
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_NumSub", "Num Subsystems"), FText::AsNumber(Subsystems.Num()));
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

	if (FSubsystemBrowserModule::OnGetSubsystemOwnerName.IsBound())
	{
		OwnerName = FSubsystemBrowserModule::OnGetSubsystemOwnerName.Execute(Instance);
	}
	else
	{
		OwnerName = FSubsystemBrowserUtils::GetDefaultSubsystemOwnerName(Instance);
	}

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

	FSubsystemBrowserUtils::GetClassPropertyCounts(InClass, NumProperties, NumViewableProperties);
}

bool FSubsystemTreeSubsystemItem::IsSelected() const
{
	return Model.IsValid() && Model->IsItemSelected(SharedThis(this));
}

FText FSubsystemTreeSubsystemItem::GetDisplayName() const
{
	return DisplayName;
}

void FSubsystemTreeSubsystemItem::GenerateTooltip(FSubsystemTableItemTooltipBuilder& TooltipBuilder) const
{
	//TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Path", "Path"), FText::FromString(LongPackage));
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Class", "Class"), FText::FromName(ClassName));
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Module", "Module"), FText::FromString(ShortPackage));
	if (IsPluginModule())
	{
		TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Plugin", "Plugin"), FText::FromString(PluginDisplayName));
	}
	if (IsConfigExportable())
	{
		TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Config", "Config"), FText::FromName(ConfigName));
	}
	if (!OwnerName.IsEmpty())
	{
		TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Owner", "Owned by"), FText::FromString(OwnerName));
	}

	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Props", "Num Properties"), FText::AsNumber(NumProperties));
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_PropsVisible", "Num Visible Properties"), FText::AsNumber(NumViewableProperties));

}

#undef LOCTEXT_NAMESPACE
