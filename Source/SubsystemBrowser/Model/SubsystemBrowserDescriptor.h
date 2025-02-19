// Copyright 2022, Aquanox.

#pragma once

#include "SubsystemBrowserUtils.h"
#include "Model/SubsystemBrowserCategory.h"
#include "Misc/TextFilter.h"
#include "Misc/Optional.h"
#include "Templates/SharedPointer.h"

class FSubsystemModel;
struct ISubsystemTreeItem;
struct FSubsystemTreeSubsystemItem;
struct FSubsystemTreeObjectItem;
struct FSubsystemTreeCategoryItem;

using FSubsystemTreeItemID = FName;
using SubsystemTreeItemPtr = TSharedPtr<ISubsystemTreeItem>;
using SubsystemTreeItemConstPtr = TSharedPtr<const ISubsystemTreeItem>;


/**
 * Node base class for Subsystem TreeView
 */
struct SUBSYSTEMBROWSER_API ISubsystemTreeItem : public TSharedFromThis<ISubsystemTreeItem>
{
	enum class EItemType
	{
		Category,
		Object,
		Subsystem
	};

	ISubsystemTreeItem() = default;
	virtual ~ISubsystemTreeItem()  = default;

	virtual EItemType GetType() const  = 0;
	virtual FSubsystemTreeItemID GetID() const = 0;
	virtual int32 GetSortOrder() const { return 0; }

	TSharedPtr<FSubsystemModel> GetModel() const { return Model; }
	SubsystemTreeItemPtr GetParent() const { return Parent; }

	virtual bool CanHaveChildren() const { return false; }
	virtual const TArray<SubsystemTreeItemPtr>& GetChildren() const { return Children; }
	virtual int32 GetNumChildren() const { return Children.Num(); }
	virtual void RemoveAllChildren() { Children.Empty(); }
	virtual bool IsSelected() const { return false; }

	virtual UObject* GetObjectForDetails() const { return nullptr; }
	virtual bool IsStale() const { return false; }
	virtual bool IsGameModule() const { return false; }
	virtual bool IsPluginModule() const { return false; }
	virtual bool HasViewableElements() const { return false; }

	virtual FText GetDisplayName() const = 0;

	virtual const FSubsystemTreeObjectItem* GetAsObjectDescriptor() const { return nullptr; }
	virtual const FSubsystemTreeSubsystemItem* GetAsSubsystemDescriptor() const { return nullptr; }
	virtual const FSubsystemTreeCategoryItem* GetAsCategoryDescriptor() const { return nullptr; }

	virtual const FSlateBrush* GetIcon() const { return nullptr; } 
	virtual void GenerateTooltip(class FSubsystemTableItemTooltipBuilder& TooltipBuilder) const {}
	virtual void GenerateContextMenu(class UToolMenu* MenuBuilder) const { }
public:
	bool bExpanded = true;
	bool bVisible = true;
	bool bNeedsRefresh = true;
	bool bChildrenRequireSort = false;

	TSharedPtr<FSubsystemModel> Model;
	mutable SubsystemTreeItemPtr Parent;
	mutable TArray<SubsystemTreeItemPtr> Children;
};

/**
 * Node representing a Subsystem Category in TreeView
 */
struct SUBSYSTEMBROWSER_API FSubsystemTreeCategoryItem : public ISubsystemTreeItem
{
	FSubsystemTreeCategoryItem() = default;
	FSubsystemTreeCategoryItem(TSharedRef<FSubsystemModel> InModel, TSharedRef<FSubsystemCategory> InCategory);

	virtual EItemType GetType() const override { return EItemType::Category; }
	virtual FSubsystemTreeItemID GetID() const override { return Data->Name; }
	virtual int32 GetSortOrder() const override { return Data->SortOrder; }
	virtual FText GetDisplayName() const override { return Data->Label; }
	virtual bool CanHaveChildren() const override { return true; }
	virtual void GenerateTooltip(class FSubsystemTableItemTooltipBuilder& TooltipBuilder) const override;
	const FSubsystemCategory& GetData() const { check(Data.IsValid()); return *Data; }

	virtual const FSubsystemTreeCategoryItem* GetAsCategoryDescriptor() const override { return this; }
public:
	TSharedPtr<FSubsystemCategory> Data;
};

/**
 * Node representing generic viewable object
 */
struct SUBSYSTEMBROWSER_API FSubsystemTreeObjectItem : public ISubsystemTreeItem
{
	FSubsystemTreeObjectItem() = default;
	FSubsystemTreeObjectItem(TSharedRef<FSubsystemModel> InModel, TSharedPtr<ISubsystemTreeItem> InParent, UObject* Instance);

	virtual EItemType GetType() const override { return EItemType::Object; }
	virtual FSubsystemTreeItemID GetID() const override;
	virtual FText GetDisplayName() const override;
	virtual UObject* GetObjectForDetails() const override { return Object.Get(); }
	virtual bool IsSelected() const override;
	virtual bool IsStale() const override;

	virtual const FSubsystemTreeObjectItem* GetAsObjectDescriptor() const override { return this; }

	//virtual const FSlateBrush* GetIcon() const override;
	virtual void GenerateTooltip(class FSubsystemTableItemTooltipBuilder& TooltipBuilder) const override;
public:
	// 
	TWeakObjectPtr<UObject> Object;
	// 
	TWeakObjectPtr<UClass>	ObjectClass;
};

/**
 * Node representing a Subsystem in TreeView
 */
struct SUBSYSTEMBROWSER_API FSubsystemTreeSubsystemItem : public FSubsystemTreeObjectItem
{
	FSubsystemTreeSubsystemItem() = default;
	FSubsystemTreeSubsystemItem(TSharedRef<FSubsystemModel> InModel, TSharedPtr<ISubsystemTreeItem> InParent, UObject* Instance);

	virtual EItemType GetType() const override { return EItemType::Subsystem; }
	virtual FSubsystemTreeItemID GetID() const override { return ClassName; }
	
	virtual FText GetDisplayName() const override;

	bool IsConfigExportable() const { return bConfigExportable; }
	bool IsDefaultConfig() const { return bIsDefaultConfig; }
	virtual bool IsGameModule() const override { return bIsGameModuleClass; }
	virtual bool IsPluginModule() const override { return bIsPluginClass; }
	virtual bool HasViewableElements() const override;

	virtual const FSlateBrush* GetIcon() const override;
	virtual void GenerateTooltip(class FSubsystemTableItemTooltipBuilder& TooltipBuilder) const override;
	virtual void GenerateContextMenu(class UToolMenu* MenuBuilder) const override;

	virtual bool CanHaveChildren() const override;
	
	virtual const FSubsystemTreeSubsystemItem* GetAsSubsystemDescriptor() const override { return this; }
	
public:
	// Friendly display name (Class Name)
	FText							DisplayName;
	// Subsystem class name (ClassName)
	FName							ClassName;
	// Subsystem package name (/Script/ModuleName)
	FString							Package;
	// Full package name (/Script/ModuleName.ClassName)
	FString							ScriptName;
	// Short module name (ModuleName)
	FString							ModuleName;
	// Config category name (Class specifier value of UCLASS)
	FName							ConfigName;

	// Owning object name for LocalPlayerSS and similar
	FString							OwnerName;

	// List of source locations associated with subsystem class
	TArray<FString>					SourceFilePaths;

	// Detected plugin name that this subsystem is part of
	FString							PluginName;
	// Friendly name of plugin
	FString							PluginDisplayName;

	// Optional user color override
	TOptional<FLinearColor>			UserColor;
	// Optional user extra tooltip text
	TOptional<FString>				UserTooltip;

	using FClassPropertyCounts = FSubsystemBrowserUtils::FClassFieldStats;
	FClassPropertyCounts			PropertyStats;

	bool							bConfigExportable = false;
	bool							bIsDefaultConfig = false;
	bool							bIsGameModuleClass = false;
	bool							bIsPluginClass = false;
	
	bool							bHasSubobjectPicker = false;
	//TArray<TWeakObjectPtr<UObject>> SubobjectsToDisplay;
};
