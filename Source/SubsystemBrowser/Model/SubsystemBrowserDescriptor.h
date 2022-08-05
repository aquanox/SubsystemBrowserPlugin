// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserCategory.h"
#include "Misc/TextFilter.h"

class FSubsystemModel;
struct ISubsystemTreeItem;
struct FSubsystemTreeSubsystemItem;
struct FSubsystemTreeCategoryItem;

using FSubsystemTreeItemID = FName;
using SubsystemTreeItemPtr = TSharedPtr<ISubsystemTreeItem>;
using SubsystemTreeItemRef = TSharedRef<ISubsystemTreeItem>;

/*
 * Abstract subsystem tree item node
 */
struct SUBSYSTEMBROWSER_API ISubsystemTreeItem : public TSharedFromThis<ISubsystemTreeItem>
{
	virtual ~ISubsystemTreeItem()  = default;

	virtual FSubsystemTreeItemID GetID() const = 0;
	virtual int32 GetSortOrder() const { return 0; }

	TSharedPtr<FSubsystemModel> GetModel() const { return Model; }
	SubsystemTreeItemPtr GetParent() const { return Parent; }

	virtual bool CanHaveChildren() const { return false; }
	virtual TArray<SubsystemTreeItemPtr> GetChildren() const { return Children; }
	virtual int32 GetNumChildren() const { return Children.Num(); }
	virtual void RemoveAllChildren() { Children.Empty(); }
	virtual bool IsSelected() const { return false; }

	virtual UObject* GetObjectForDetails() const { return nullptr; }
	virtual bool IsStale() const { return false; }
	virtual bool IsConfigExportable() const { return false; }
	virtual bool IsGameModule() const { return false; }
	virtual bool IsPluginModule() const { return false; }
	virtual bool HasViewableProperties() const { return false; }

	virtual FText GetDisplayName() const = 0;

	virtual FSubsystemTreeSubsystemItem* GetAsSubsystemDescriptor() const { return nullptr; }
	virtual FSubsystemTreeCategoryItem* GetAsCategoryDescriptor() const { return nullptr; }

	virtual void GenerateContextMenu(class UToolMenu* MenuBuilder, class SSubsystemBrowserPanel& Browser) const { }

	bool bExpanded = true;
	bool bVisible = true;
	bool bNeedsRefresh = true;
	bool bChildrenRequireSort = false;

	TSharedPtr<FSubsystemModel> Model;
	mutable SubsystemTreeItemPtr Parent;
	mutable TArray<SubsystemTreeItemPtr> Children;
};

/**
 * Category node
 */
struct SUBSYSTEMBROWSER_API FSubsystemTreeCategoryItem final : public ISubsystemTreeItem
{
	TSharedPtr<FSubsystemCategory> Data;

	FSubsystemTreeCategoryItem() = default;
	FSubsystemTreeCategoryItem(TSharedRef<FSubsystemModel> InModel, TSharedRef<FSubsystemCategory> InCategory);

	virtual FSubsystemTreeItemID GetID() const override { return Data->Name; }
	virtual int32 GetSortOrder() const override { return Data->SortOrder; }
	virtual FText GetDisplayName() const override { return Data->Label; }
	virtual bool CanHaveChildren() const override { return true; }
	virtual FSubsystemTreeCategoryItem* GetAsCategoryDescriptor() const override { return const_cast<FSubsystemTreeCategoryItem*>(this); }

	TArray<UObject*> Select(UWorld* InContext) const;
};

/**
 * Subsystem node
 */
struct SUBSYSTEMBROWSER_API FSubsystemTreeSubsystemItem final : public ISubsystemTreeItem
{
	TWeakObjectPtr<UObject>			Subsystem;
	TWeakObjectPtr<UClass>			Class;

	FText							DisplayName;
	FName							ClassName;
	FString							Package;
	FString							LongPackage;
	FString							ShortPackage;
	FName							ConfigName;

	FString							OwnerName;

	TArray<FString>					SourceFilePaths;

	FString							PluginName;
	FString							PluginDisplayName;

	bool							bConfigExportable = false;
	bool							bIsGameModuleClass = false;
	bool							bIsPluginClass = false;
	bool							bHasViewableProperties = false;

	FSubsystemTreeSubsystemItem() = default;
	FSubsystemTreeSubsystemItem(TSharedRef<FSubsystemModel> InModel, TSharedPtr<ISubsystemTreeItem> InParent, UObject* Instance);

	virtual FSubsystemTreeItemID GetID() const override { return ClassName; }
	virtual bool IsSelected() const override;

	virtual FText GetDisplayName() const override;

	virtual FSubsystemTreeSubsystemItem* GetAsSubsystemDescriptor() const override {  return const_cast<FSubsystemTreeSubsystemItem*>(this); }
	virtual UObject* GetObjectForDetails() const override { return Subsystem.Get(); }
	virtual bool IsStale() const override { return Subsystem.IsStale() || Class.IsStale(); }
	virtual bool IsConfigExportable() const override { return bConfigExportable; }
	virtual bool IsGameModule() const override { return bIsGameModuleClass; }
	virtual bool IsPluginModule() const override { return bIsPluginClass; }
	virtual bool HasViewableProperties() const override { return bHasViewableProperties; }
};
