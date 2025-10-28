// Copyright 2022, Aquanox.

#pragma once

#include "IDetailsView.h"
#include "IDetailCustomization.h"

class FSBDetailsCustomization : public IDetailCustomization
{
	using ThisClass = FSBDetailsCustomization;
	using Super = IDetailCustomization;
public:
	/**
	 * Are local details view customizations enabled
	 */
	SUBSYSTEMBROWSER_API static bool IsEnabled();
	/**
	 * Build customization configured for settings panel use
	 */
	SUBSYSTEMBROWSER_API static TSharedRef<IDetailCustomization> MakeForSettings();
	/**
	* Build customization configured for main panel use
	*/
	SUBSYSTEMBROWSER_API static TSharedRef<IDetailCustomization> MakeForPanel();

public:
	FSBDetailsCustomization() = default;

	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	void RequestForceRefresh() const;

private:

	TWeakPtr<IDetailLayoutBuilder> BuilderRef;
};