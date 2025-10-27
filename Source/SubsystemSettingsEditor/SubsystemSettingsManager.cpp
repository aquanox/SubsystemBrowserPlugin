// Copyright 2022, Aquanox.

#include "SubsystemSettingsManager.h"

#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserStyle.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#include "ISettingsCategory.h"
#include "ISettingsEditorModule.h"
#include "ISettingsEditorModel.h"
#include "IDetailsView.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectIterator.h"
#include "UI/SubsystemBrowserPanel.h"
#include "UI/SubsystemSettingsWidget.h"
#include "SubsystemSettingsEditorModule.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

const FName FSubsystemSettingsUserMeta::MD_SBHidden(TEXT("SBHidden"));
const FName FSubsystemSettingsUserMeta::MD_SBSection(TEXT("SBSettingsSection"));
const FName FSubsystemSettingsUserMeta::MD_SBSectionDesc(TEXT("SBSettingsSectionDesc"));

const FName FSubsystemSettingsManager::SubsystemSettingsTabName = TEXT("SubsystemSettings");

class SBTrackableDockTab : public SDockTab
{
public:
	void Construct(const FArguments& InArgs, FSubsystemSettingsManager* InManager)
	{
		InManager->TrackedSettingsWidget = SharedThis(this);
		SDockTab::Construct(InArgs);
	}
};

template<typename T>
TSharedPtr<T> RecursiveFindWidget(const TSharedRef<SWidget>& InWidget, const FName& InName)
{
	if (InWidget->GetType() == InName)
		return StaticCastSharedRef<T>(InWidget);

	for(int32 Idx = 0, Num = InWidget->GetChildren()->Num(); Idx < Num; ++Idx)
	{
		auto Inner = RecursiveFindWidget<T>(InWidget->GetChildren()->GetChildAt(Idx), InName);
		if (Inner.IsValid())
			return Inner;
	}
	return nullptr;
}

void FSubsystemSettingsManager::Register()
{
	USubsystemBrowserSettings* SettingsObject = USubsystemBrowserSettings::Get();
	SettingsObject->OnSettingChanged().AddRaw(this, &FSubsystemSettingsManager::HandleSettingsChanged);

	// Create Subsystem Settings viewer and viewer tab spawner
	ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>("Settings");
	SettingsModule.RegisterViewer(TEXT("Subsystem"), *this);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SubsystemSettingsTabName, FOnSpawnTab::CreateRaw(this, &FSubsystemSettingsManager::HandleSpawnSettingsTab))
		.SetDisplayName(LOCTEXT("SubsystemBrowserSettingsTabTitle", "Subsystem Settings"))
		.SetTooltipText(LOCTEXT("SubsystemBrowserSettingsTabTooltip", "Open the Subsystem Settings tab."))
		.SetMenuType(ETabSpawnerMenuType::Hidden) // Hide for now, got menu extenders to access it plus browser menu
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
#else
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
#endif
		.SetIcon(FStyleHelper::GetSlateIcon(FStyleHelper::PanelIconName));

	//
	FModuleManager::Get().OnModulesChanged().AddRaw(this, &FSubsystemSettingsManager::HandleModulesChanges);
}

void FSubsystemSettingsManager::Unregister()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings")))
	{
		SettingsModule->UnregisterViewer(TEXT("Subsystem"));
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SubsystemSettingsTabName);

	FModuleManager::Get().OnModulesChanged().RemoveAll(this);
}

void FSubsystemSettingsManager::ShowSettings(const FName& CategoryName, const FName& SectionName)
{
#if UE_VERSION_OLDER_THAN(4, 26, 0)
	FGlobalTabmanager::Get()->InvokeTab(FSubsystemBrowserModule::SubsystemSettingsTabName);
#else
	FGlobalTabmanager::Get()->TryInvokeTab(SubsystemSettingsTabName);
#endif

	ISettingsEditorModelPtr SettingsEditorModel = SettingsEditorModelPtr.Pin();
	if (SettingsEditorModel.IsValid())
	{
		ISettingsCategoryPtr Category = SettingsEditorModel->GetSettingsContainer()->GetCategory(CategoryName);
		if (Category.IsValid())
		{
			ISettingsSectionPtr Section = Category->GetSection(SectionName);

			SettingsEditorModel->SelectSection(Section);
		}
	}
}

TSharedRef<SDockTab> FSubsystemSettingsManager::HandleSpawnSettingsTab(const FSpawnTabArgs& Args)
{
	UpdateDiscoveredSubsystems(true);

	ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));

	TSharedRef<SWidget> SettingsEditor = SNullWidget::NullWidget;

	ISettingsContainerPtr SettingsContainer = SettingsModule.GetContainer(TEXT("Subsystem"));
	if (SettingsContainer.IsValid())
	{
		ISettingsEditorModule& SettingsEditorModule = FModuleManager::GetModuleChecked<ISettingsEditorModule>(TEXT("SettingsEditor"));
		ISettingsEditorModelRef SettingsEditorModel = SettingsEditorModule.CreateModel(SettingsContainer.ToSharedRef());

		SettingsEditor = SettingsEditorModule.CreateEditor(SettingsEditorModel);

		SettingsEditorPtr = SettingsEditor;
		SettingsEditorModelPtr = SettingsEditorModel;

		if (USubsystemBrowserSettings::Get()->ShouldUseCustomPropertyFilterInSettings())
		{
			TSharedPtr<IDetailsView> InnerDetailsView = RecursiveFindWidget<IDetailsView>(SettingsEditor, TEXT("SDetailsView"));
			if (InnerDetailsView.IsValid())
			{
				InnerDetailsView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateStatic(&SSubsystemSettingsWidget::IsDetailsPropertyVisible));
				InnerDetailsView->ForceRefresh();
			}
		}
	}

	return SNew(SBTrackableDockTab, this)
		.TabRole(ETabRole::NomadTab)
		[
			SettingsEditor
		];
}

void FSubsystemSettingsManager::HandleCategoriesChanged()
{
	bNeedsRediscover = true;
	UpdateDiscoveredSubsystems();
}

void FSubsystemSettingsManager::HandleModulesChanges(FName Name, EModuleChangeReason ModuleChangeReason)
{
	bNeedsRediscover = true;
	UpdateDiscoveredSubsystems();
}

void FSubsystemSettingsManager::UpdateDiscoveredSubsystems(bool bForce)
{
	ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));

	if ((TrackedSettingsWidget.IsValid() || bForce) && bNeedsRediscover)
	{
		bNeedsRediscover = false;

		UnregisterDiscoveredSubsystems(SettingsModule);
		RegisterDiscoveredSubsystems(SettingsModule);
	}
}

void FSubsystemSettingsManager::RegisterDiscoveredSubsystems(ISettingsModule& SettingsModule)
{
	TArray<UObject*> AllKnownSubsystems;

	const TArray<SubsystemCategoryPtr>& RegisteredCategories = FSubsystemBrowserModule::Get().GetCategories();
	for (const SubsystemCategoryPtr& Ptr : RegisteredCategories)
	{
		if (!Ptr->IsVisibleInSettings() || Ptr->GetSettingsName().IsNone())
			continue;

		TArray<UObject*> ObjectArray;
		Ptr->SelectSettings(ObjectArray);

		for (UObject* Subsystem : ObjectArray)
		{
			UClass* SSClass = Subsystem->GetClass();

			if (!Subsystem->HasAnyFlags(RF_ClassDefaultObject))
				continue;
			if (!SSClass->HasAnyClassFlags(CLASS_Config))
				continue;
			if (SSClass->HasAnyClassFlags(CLASS_Deprecated|CLASS_Abstract))
				continue;
			if (SSClass->FindMetaData(FSubsystemSettingsUserMeta::MD_SBHidden) != nullptr)
				continue;
			if (!AllKnownSubsystems.Contains(Subsystem))
			{
				FSubsystemBrowserUtils::FClassFieldStats ClassFieldStats = FSubsystemBrowserUtils::GetClassFieldStats(SSClass);
				// ignore entries with Class-level config specifier but no actual Config properties
				if (!ClassFieldStats.NumConfig)
					continue;

				// custom mode allows showing props without Edit specifier, if there's none - ignore object
				bool bUseCustom = USubsystemBrowserSettings::Get()->ShouldUseCustomSettingsWidget();
				if (!bUseCustom && !ClassFieldStats.NumConfigWithEdit)
					continue;

				const FName CategoryName = Ptr->GetSettingsName();

				UE_LOG(LogSubsystemSettingsEditor, Verbose, TEXT("Discovered object %s:%s [%s] (editable=%d, total=%d, customui=%d)"),
					*CategoryName.ToString(), *GetNameSafe(Subsystem), *GetNameSafe(SSClass),
					ClassFieldStats.NumConfigWithEdit, ClassFieldStats.NumConfig, (int32)bUseCustom);

				RegisterSubsystemSettings(SettingsModule, CategoryName, Subsystem, bUseCustom);

				AllKnownSubsystems.Add(Subsystem);
			}
		}
	}
}

void FSubsystemSettingsManager::RegisterSubsystemSettings(ISettingsModule& SettingsModule, FName Category, UObject* Subsystem, bool bCustomUI)
{
	UClass* const Class = Subsystem->GetClass();

	FDiscoveredSubsystemInfo Registered;
	Registered.ContainerName = TEXT("Subsystem");
	Registered.CategoryName = Category;
	Registered.SectionName = Class->GetFName();

	TOptional<FString> OptionalSection = FSubsystemBrowserUtils::GetMetadataOptional(Class, FSubsystemSettingsUserMeta::MD_SBSection);
	FText DisplayName = OptionalSection.IsSet() ? FText::FromString(OptionalSection.GetValue()) : Class->GetDisplayNameText();

	TOptional<FString> OptionalSectionDesc = FSubsystemBrowserUtils::GetMetadataOptional(Class, FSubsystemSettingsUserMeta::MD_SBSectionDesc);
	FText DisplayTooltip = OptionalSectionDesc.IsSet() ? FText::FromString(OptionalSectionDesc.GetValue()) : Class->GetToolTipText();

	if (bCustomUI)
	{
		// custom widget for engine subsystems that did not expose any properties for editing
		Registered.EditorWidget =  SNew(SSubsystemSettingsWidget, Subsystem)
				.CategoryName(Registered.CategoryName)
				.SectionName(Registered.SectionName)
				.SectionDisplayName(DisplayName)
				.SectionTooltipText(DisplayTooltip);

		SettingsModule.RegisterSettings(Registered.ContainerName, Registered.CategoryName, Registered.SectionName,
            DisplayName,
            DisplayTooltip,
			Registered.EditorWidget.ToSharedRef()
		);
	}
	else
	{
		SettingsModule.RegisterSettings(Registered.ContainerName, Registered.CategoryName, Registered.SectionName,
            DisplayName,
            DisplayTooltip,
            Subsystem
		);
	}

	DiscoveredSettings.Add(Registered);
}

void FSubsystemSettingsManager::UnregisterDiscoveredSubsystems(ISettingsModule& SettingsModule)
{
	for (const FDiscoveredSubsystemInfo& Settings : DiscoveredSettings)
	{
		SettingsModule.UnregisterSettings(Settings.ContainerName, Settings.CategoryName, Settings.SectionName);
	}

	DiscoveredSettings.Reset();
}

void FSubsystemSettingsManager::HandleSettingsChanged(FName InPropertyName)
{
	if (FProperty* Property = USubsystemBrowserSettings::StaticClass()->FindPropertyByName(InPropertyName))
	{
		if (Property->HasMetaData(FSubsystemBrowserConfigMeta::MD_ConfigAffectsSettings))
		{
			bNeedsRediscover = true;
			UpdateDiscoveredSubsystems();
		}
	}
}


#undef LOCTEXT_NAMESPACE
