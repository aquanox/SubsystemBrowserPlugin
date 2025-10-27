// Copyright 2022, Aquanox.

#pragma once

#include "PropertyEditorDelegates.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Misc/NotifyHook.h"

class IDetailsView;

/**
 * Custom settings widget for private types
 */
class SSubsystemSettingsWidget : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SSubsystemSettingsWidget)
		{ }
		SLATE_ATTRIBUTE(FName, CategoryName)
		SLATE_ATTRIBUTE(FName, SectionName)
		SLATE_ATTRIBUTE(FText, SectionDisplayName)
		SLATE_ATTRIBUTE(FText, SectionTooltipText)
	SLATE_END_ARGS()


	void Construct(const FArguments& InArgs, UObject* InObject);
	virtual ~SSubsystemSettingsWidget() override;

	FText GetSettingsStorageLocationText() const;
	FString GetTargetObjectConfigFilePath() const;
	FString GetDefaultConfigFilePath() const;

	/* details view handlers */
	static bool IsDetailsPropertyVisible(const FPropertyAndParent& InProperty);
	static bool CallInEditorRowFilter(FName InRowName, FName InParentName, TWeakObjectPtr<UObject> SourceObj);
	
	virtual void NotifyPostChange( const FPropertyChangedEvent& PropertyChangedEvent, class FEditPropertyChain* PropertyThatChanged ) override;

	/* set as default button handlers */
	EVisibility HandleSetAsDefaultButtonVisibility() const;
	FReply HandleSetAsDefaultButtonClicked();
	bool HandleSetAsDefaultButtonEnabled() const;

	/* */
	FReply HandleExportButtonClicked() const;

	/*  */
	void SaveSettings();

private:
	TWeakObjectPtr<UObject> TargetObject;

	TSharedPtr<IDetailsView> SettingsView;
};
