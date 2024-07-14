// Copyright 2022, Aquanox.

#pragma once

#include "IDetailsView.h"
#include "IDetailCustomization.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyTypeCustomization.h"

namespace SBHacks
{
	void CustomizeDetailsView(TSharedRef<IDetailsView> DetailsView);

}

// An "empty" instanced customization that's intended to override any registered
// global details customization for the class type.
class FSB_EmptyDetailsCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override {}

	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return  MakeShared<FSB_EmptyDetailsCustomization>();
	}
};

// An "empty" instanced type customization that's intended to override any registered
// global property type customization.
class FSB_EmptyPropertyTypeCustomization : public IPropertyTypeCustomization
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {}
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override {}

	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return  MakeShared<FSB_EmptyPropertyTypeCustomization>();
	}
};


class FSB_TrackerDetailsCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return  MakeShared<FSB_TrackerDetailsCustomization>();
	}
};

class FSB_TrackerPropertyTypeCustomization : public IPropertyTypeCustomization
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return  MakeShared<FSB_TrackerPropertyTypeCustomization>();
	}
};




