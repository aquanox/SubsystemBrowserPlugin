#pragma once

#include "UI/SubsystemTableItem.h"
#include "Widgets/SToolTip.h"

class SSubsystemTableItemTooltip : public SToolTip
{
	using Super = SToolTip;
public:
	SLATE_BEGIN_ARGS(SSubsystemTableItemTooltip)
	    { }
		SLATE_ARGUMENT(TSharedPtr<SSubsystemTableItem>, SubsystemTableItem)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// IToolTip interface
	virtual bool IsEmpty() const override
	{
		return !SubsystemTableItem.IsValid();
	}

	virtual void OnOpening() override;

	virtual void OnClosed() override;

	/** Creates a tooltip widget for this item */
	TSharedRef<SWidget> CreateToolTipWidget(TSharedRef<SSubsystemTableItem> TableItem) const;

private:
	TWeakPtr<SSubsystemTableItem> SubsystemTableItem;
};

class SUBSYSTEMBROWSER_API FSubsystemTableItemTooltipBuilder
{
	friend SSubsystemTableItemTooltip;

	TSharedRef<SSubsystemTableItem> Item;
	TSharedPtr<SVerticalBox>		Primary;
	TSharedPtr<SVerticalBox>		Secondary;
public:
	enum
	{
		DF_NONE				= 0,
		DF_IMPORTANT		= 1 << 0,
		DF_WITH_HIGHLIGHT	= 1 << 1,
	};

	FSubsystemTableItemTooltipBuilder(TSharedRef<SSubsystemTableItem> Item) : Item(Item) {}

	void AddPrimary(const FText& Key, const FText& Value, uint32 DisplayFlags = DF_NONE);
	void AddSecondary(const FText& Key, const FText& Value, uint32 DisplayFlags = DF_NONE);
	bool HasAnyData() const { return Primary.IsValid() || Secondary.IsValid(); }
private:
	void AddBox(TSharedRef<SVerticalBox> Target, const FText& Key, const FText& Value, uint32 DisplayFlags);
};