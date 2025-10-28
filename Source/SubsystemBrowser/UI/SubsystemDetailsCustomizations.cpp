// Copyright 2022, Aquanox.

#include "SubsystemDetailsCustomizations.h"

#include "DetailLayoutBuilder.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

bool FSBDetailsCustomization::IsEnabled()
{
	return false;
}

TSharedRef<IDetailCustomization> FSBDetailsCustomization::MakeForSettings()
{
	TSharedRef<FSBDetailsCustomization> Instance = MakeShared<FSBDetailsCustomization>();
	return Instance;
}

TSharedRef<IDetailCustomization> FSBDetailsCustomization::MakeForPanel()
{
	TSharedRef<FSBDetailsCustomization> Instance = MakeShared<FSBDetailsCustomization>();
	return Instance;
}

void FSBDetailsCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
	BuilderRef = DetailBuilder;
	Super::CustomizeDetails(DetailBuilder);
}

void FSBDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// AddCIEWarningCategory(DetailBuilder);
}

void FSBDetailsCustomization::RequestForceRefresh() const
{
	if (BuilderRef.IsValid())
	{
		BuilderRef.Pin()->ForceRefreshDetails();
	}
}


#undef LOCTEXT_NAMESPACE
