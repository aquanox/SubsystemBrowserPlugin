// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_Editor.h"

#include "Editor.h"
#include "EditorSubsystem.h"

FSubsystemCategory_Editor::FSubsystemCategory_Editor()
{
	Name = TEXT("EditorSubsystemCategory");
	SettingsName = TEXT("Editor");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_Editor", "Editor Subsystems");
	SortOrder = 200;
}

UClass* FSubsystemCategory_Editor::GetSubsystemClass() const
{
	return UEditorSubsystem::StaticClass();
}

void FSubsystemCategory_Editor::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (GEditor)
	{
		return OutData.Append(GEditor->GetEditorSubsystemArray<UEditorSubsystem>());
	}
}
