// Copyright 2022, Aquanox.

#include "SubsystemDetailsCustomizations.h"

#include "DetailLayoutBuilder.h"
#include "SubsystemBrowserModule.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/EngineVersionComparison.h"

DEFINE_LOG_CATEGORY_STATIC(LogSBHacks, Log, All);

static UClass* TryFindClassByName(const FString& ClassName)
{
	UClass* ResultClass = nullptr;
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	if (FPackageName::IsShortPackageName(ClassName))
	{
		ResultClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
	}
	else
	{
		ResultClass = FindObject<UClass>(nullptr, *ClassName);
	}
#else
	ResultClass =  UClass::TryFindTypeSlow<UClass>(ClassName, EFindFirstObjectOptions::EnsureIfAmbiguous);
	if (!ResultClass)
	{
		ResultClass = LoadObject<UClass>(nullptr, *ClassName);
	}
#endif
	return ResultClass;
}

void SBHacks::CustomizeDetailsView(TSharedRef<IDetailsView> DetailsView)
{
	using OfDetail = FOnGetDetailCustomizationInstance;
	using OfType = FOnGetPropertyTypeCustomizationInstance;

	//DetailsView->RegisterInstancedCustomPropertyLayout(TryFindClassByName("/Script/CoreUObject.Object"), OfDetail::CreateStatic(&FSB_TrackerDetailsCustomization::MakeInstance));
	//DetailsView->RegisterInstancedCustomPropertyLayout(TryFindClassByName("/Script/UMG.UserWidget"), OfDetail::CreateStatic(&FSB_TrackerDetailsCustomization::MakeInstance));
	//DetailsView->RegisterInstancedCustomPropertyTypeLayout(TEXT("Sample"), OfType::CreateStatic(&FSB_TrackerPropertyTypeCustomization::MakeInstance));
}

void FSB_TrackerDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	UE_LOG(LogSBHacks, Log, TEXT("TrackerDetailsCustomization::CustomizeDetails"));
}

void FSB_TrackerPropertyTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UE_LOG(LogSBHacks, Log, TEXT("TrackerPropertyTypeCustomization::CustomizeHeader"));
}

void FSB_TrackerPropertyTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UE_LOG(LogSBHacks, Log, TEXT("TrackerPropertyTypeCustomization::CustomizeChildren"));
}
