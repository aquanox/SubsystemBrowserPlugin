// Copyright 2022, Aquanox.

#include "SubsystemSettingsWidget.h"
#include "IDetailsView.h"
#include "ISettingsSection.h"
#include "PropertyEditorModule.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserStyle.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

void SSubsystemSettingsWidget::Construct(const FArguments& InArgs, UObject* InObject)
{
	TargetObject = InObject;

	const FText SectionTitleText = FText::Format(INVTEXT("{0} - {1}"),
		FText::FromName(InArgs._CategoryName.Get()),
		InArgs._SectionDisplayName.Get()
	);

	// initialize settings view
	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.ViewIdentifier = FName(*FString::Printf(TEXT("SBSettingsViewer.%s"), *GetNameSafe(InObject)));
		DetailsViewArgs.bAllowSearch = true;
		DetailsViewArgs.bHideSelectionTip = true;
		DetailsViewArgs.bLockable = false;
		DetailsViewArgs.bSearchInitialKeyFocus = true;
		DetailsViewArgs.bUpdatesFromSelection = false;
		DetailsViewArgs.NotifyHook = this;
		DetailsViewArgs.bShowOptions = true;
		DetailsViewArgs.bShowModifiedPropertiesOption = false;
		DetailsViewArgs.bShowAnimatedPropertiesOption = false;
		DetailsViewArgs.bShowDifferingPropertiesOption = false;
		DetailsViewArgs.bShowKeyablePropertiesOption = false;
		DetailsViewArgs.bShowPropertyMatrixButton = false;
		DetailsViewArgs.bAllowMultipleTopLevelObjects = true;
		DetailsViewArgs.bCustomNameAreaLocation = true;
		DetailsViewArgs.bCustomFilterAreaLocation = true;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		// enforce display of all properties and filter them later via delegate
		DetailsViewArgs.bForceHiddenPropertyVisibility = true;
	}

	{
		TSharedRef<IDetailsView> View = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor")).CreateDetailView(DetailsViewArgs);
		if (USubsystemBrowserSettings::Get()->ShouldUseCustomPropertyFilterInSettings())
		{
			View->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateStatic(&SSubsystemSettingsWidget::IsDetailsPropertyVisible));
		}
		View->SetObject(InObject);

		FSubsystemBrowserModule::OnCustomizeDetailsView.Broadcast(View, TEXT("SubsystemSettingsPanel"));

		SettingsView = View;
	}

	ChildSlot
	.Padding(0, 8, 16, 5)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(2, 2, 2, 2)
					.AutoWidth()
					[
						SNew(SBox).HeightOverride(16).WidthOverride(16) //tbd
					]
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Font(FStyleHelper::GetFontStyle("SettingsEditor.CatgoryAndSectionFont"))
						.Text(SectionTitleText)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(16, 8, 0, 0)
				[
					SNew(STextBlock)
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					.Text(InArgs._SectionTooltipText)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(16, 0, 0, 0)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(SButton)
					.Visibility(this, &SSubsystemSettingsWidget::HandleSetAsDefaultButtonVisibility)
					.IsEnabled(this, &SSubsystemSettingsWidget::HandleSetAsDefaultButtonEnabled)
					.OnClicked(this, &SSubsystemSettingsWidget::HandleSetAsDefaultButtonClicked)
					.Text(LOCTEXT("SaveDefaultsButtonText", "Set as Default"))
					.ToolTipText(LOCTEXT("SaveDefaultsButtonTooltip", "Save the values below as the new default settings"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8, 0, 0, 0)
				[
					SNew(SButton)
					.OnClicked(this, &SSubsystemSettingsWidget::HandleExportButtonClicked)
					.Text(LOCTEXT("ExportButtonText", "Export to Clipboard"))
					.ToolTipText(LOCTEXT("ExportButtonTooltip", "Export these settings to a clipboard"))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 8, 0, 0)
		[
			// file area
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8, 0, 0, 0)
			.VAlign(VAlign_Center)
			[
				SNew(SBox).HeightOverride(16).WidthOverride(16) //tbd
			]
			+ SHorizontalBox::Slot()
			.Padding(8, 0, 0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.FillWidth(1)
			[
				SNew(STextBlock)
				.Text(this, &SSubsystemSettingsWidget::GetSettingsStorageLocationText)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 8, 0, 16)
		[
			// warning area
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(8, 0, 0, 0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.HeightOverride(16)
				.WidthOverride(16)
				[
					SNew(SImage)
					.Image(FStyleHelper::GetBrush("SettingsEditor.WarningIcon"))
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(8, 0, 0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.FillWidth(1)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SubsystemMissingEditableConfigs", "This class has no publicly editable config properties, functionality is limited."))
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			// settings area
			SettingsView.ToSharedRef()
		]
	];
}

SSubsystemSettingsWidget::~SSubsystemSettingsWidget()
{
}

FText SSubsystemSettingsWidget::GetSettingsStorageLocationText() const
{
	FString ShortFile = FPaths::GetCleanFilename(GetTargetObjectConfigFilePath());
	if (ShortFile == TEXT("Engine"))
		ShortFile = TEXT("Engine.ini"); // visual fix
	return FText::Format(INVTEXT("These settings are saved in {0}."),  FText::FromString(ShortFile));
}

bool SSubsystemSettingsWidget::IsDetailsPropertyVisible(const FPropertyAndParent& InProperty)
{
	if (InProperty.Property.HasAnyPropertyFlags(CPF_Config))
	{
		return true;
	}

	for (const FProperty* ParentProperty : InProperty.ParentProperties)
	{
		if (ParentProperty->HasAnyPropertyFlags(CPF_Config))
		{
			return true;
		}
	}

	return false;
}

void SSubsystemSettingsWidget::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged)
{
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
		return;

	if (PropertyChangedEvent.GetNumObjectsBeingEdited() > 0)
	{
		UObject* ObjectBeingEdited = const_cast<UObject*>(PropertyChangedEvent.GetObjectBeingEdited(0));

		{
			// Determine if the Property is an Array or Array Element
			bool bIsArrayOrArrayElement = PropertyThatChanged->GetActiveMemberNode()->GetValue()->IsA(FArrayProperty::StaticClass())
				|| PropertyThatChanged->GetActiveMemberNode()->GetValue()->ArrayDim > 1
				|| PropertyChangedEvent.Property->GetOwner<FArrayProperty>();

			bool bIsSetOrSetElement = PropertyThatChanged->GetActiveMemberNode()->GetValue()->IsA(FSetProperty::StaticClass())
				|| PropertyChangedEvent.Property->GetOwner<FSetProperty>();

			bool bIsMapOrMapElement = PropertyThatChanged->GetActiveMemberNode()->GetValue()->IsA(FMapProperty::StaticClass())
				|| PropertyChangedEvent.Property->GetOwner<FMapProperty>();

			if (ObjectBeingEdited->GetClass()->HasAnyClassFlags(CLASS_DefaultConfig) && !bIsArrayOrArrayElement && !bIsSetOrSetElement && !bIsMapOrMapElement)
			{
				ObjectBeingEdited->UpdateSinglePropertyInConfigFile(PropertyThatChanged->GetActiveMemberNode()->GetValue(), ObjectBeingEdited->GetDefaultConfigFilename());
			}
			else if (TargetObject == ObjectBeingEdited)
			{
				SaveSettings();
			}
			else if (ObjectBeingEdited->GetClass()->HasAnyClassFlags(CLASS_DefaultConfig))
			{
#if UE_VERSION_OLDER_THAN(5,0,0)
				ObjectBeingEdited->UpdateDefaultConfigFile();
#else
				ObjectBeingEdited->TryUpdateDefaultConfigFile();
#endif
			}
		}
	}
}

FString SSubsystemSettingsWidget::GetTargetObjectConfigFilePath() const
{
	if (TargetObject.IsValid())
	{
		if (TargetObject->GetClass()->HasAnyClassFlags(CLASS_DefaultConfig))
		{
			return GetDefaultConfigFilePath();
		}
		if (TargetObject->GetClass()->HasAnyClassFlags(CLASS_Config))
		{
			return TargetObject->GetClass()->GetConfigName();
		}
	}

	return FString();
}

FString SSubsystemSettingsWidget::GetDefaultConfigFilePath() const
{
	FString RelativeConfigFilePath = TargetObject->GetDefaultConfigFilename();
	return FPaths::ConvertRelativePathToFull(RelativeConfigFilePath);
}

EVisibility SSubsystemSettingsWidget::HandleSetAsDefaultButtonVisibility() const
{
	bool bCanSaveDefaults = TargetObject.IsValid()
		&& TargetObject->GetClass()->HasAnyClassFlags(CLASS_Config)
		&& !TargetObject->GetClass()->HasAnyClassFlags(CLASS_DefaultConfig | CLASS_GlobalUserConfig | CLASS_ProjectUserConfig);

	return bCanSaveDefaults ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SSubsystemSettingsWidget::HandleSetAsDefaultButtonClicked()
{
	if (TargetObject.IsValid())
	{
		if(FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SaveAsDefaultUserConfirm", "Are you sure you want to update the default settings?")) != EAppReturnType::Yes)
		{
			return FReply::Handled();
		}

#if UE_VERSION_OLDER_THAN(5, 0, 0)
		TargetObject->UpdateDefaultConfigFile();
		TargetObject->ReloadConfig(nullptr, nullptr, UE4::LCPF_PropagateToInstances);
#else
		TargetObject->TryUpdateDefaultConfigFile();
		TargetObject->ReloadConfig(nullptr, nullptr, UE::LCPF_PropagateToInstances);
#endif

		FSubsystemBrowserUtils::ShowBrowserInfoMessage(LOCTEXT("SaveAsDefaultsSucceededMessage", "The default configuration file for these settings was updated successfully."), SNotificationItem::CS_Success);
	}

	return FReply::Handled();
}

bool SSubsystemSettingsWidget::HandleSetAsDefaultButtonEnabled() const
{
	bool bCanSaveDefaults = TargetObject.IsValid()
		&& TargetObject->GetClass()->HasAnyClassFlags(CLASS_Config)
		&& !TargetObject->GetClass()->HasAnyClassFlags(CLASS_DefaultConfig | CLASS_GlobalUserConfig | CLASS_ProjectUserConfig);

	return bCanSaveDefaults;
}

FReply SSubsystemSettingsWidget::HandleExportButtonClicked() const
{
	FString ClipboardText = FSubsystemBrowserUtils::GenerateConfigExport(TargetObject.Get(), false);
	FSubsystemBrowserUtils::SetClipboardText(ClipboardText);
	FSubsystemBrowserUtils::ShowBrowserInfoMessage(LOCTEXT("SubsystemBrowserClipboardCopy_Success", "Copied to clipboard"), SNotificationItem::CS_Success);

	return FReply::Handled();
}

void SSubsystemSettingsWidget::SaveSettings()
{
	if (TargetObject.IsValid())
	{
		if (TargetObject->GetClass()->HasAnyClassFlags(CLASS_DefaultConfig))
		{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			TargetObject->UpdateDefaultConfigFile();
#else
			TargetObject->TryUpdateDefaultConfigFile();
#endif
		}
		else if (TargetObject->GetClass()->HasAnyClassFlags(CLASS_GlobalUserConfig))
		{
			TargetObject->UpdateGlobalUserConfigFile();
		}
		else if (TargetObject->GetClass()->HasAnyClassFlags(CLASS_ProjectUserConfig))
		{
			TargetObject->UpdateProjectUserConfigFile();
		}
		else
		{
			TargetObject->SaveConfig();
		}
	}
}

#undef LOCTEXT_NAMESPACE
