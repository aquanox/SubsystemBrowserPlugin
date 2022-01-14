// Copyright 2022, Aquanox.

#pragma once

#include "Misc/TextFilter.h"

DECLARE_DELEGATE_RetVal(TArray<UObject*>, FEnumSubsystemsDelegate);

class FSubsystemModel;
struct ISubsystemTreeItem;
struct FSubsystemTreeSubsystemItem;
struct FSubsystemTreeCategoryItem;

using FSubsystemTreeItemID = FName;
using SubsystemTreeItemPtr = TSharedPtr<struct ISubsystemTreeItem>;

/*
 * Abstract subsystem tree item node
 */
struct ISubsystemTreeItem : public TSharedFromThis<ISubsystemTreeItem>
{
	virtual ~ISubsystemTreeItem()  = default;

	virtual FSubsystemTreeItemID GetID() const = 0;

	TSharedPtr<FSubsystemModel> GetModel() const { return Model; }
	SubsystemTreeItemPtr GetParent() const { return Parent; }

	virtual bool CanHaveChildren() const { return false; }
	virtual TArray<SubsystemTreeItemPtr> GetChildren() const { return Children; }
	virtual int32 GetNumChildren() const { return Children.Num(); }
	virtual void RemoveAllChildren() { Children.Empty(); }

	virtual UObject* GetObjectForDetails() const { return nullptr; }
	virtual bool IsStale() const { return false; }
	virtual bool IsTickable() const { return false; }
	virtual bool IsConfigExportable() const { return false; }
	virtual bool IsGameModule() const { return false; }

	virtual FString GetDisplayNameString() const = 0;
	virtual FString GetClassNameString() const = 0;
	virtual FString GetShortPackageString() const = 0;
	virtual FString GetPackageString() const = 0;
	virtual FString GetConfigClassNameString() const = 0;
	virtual FString GetOwnerNameString() const = 0;

	virtual FSubsystemTreeSubsystemItem* GetAsSubsystemDescriptor() const { return nullptr; }
	virtual FSubsystemTreeCategoryItem* GetAsCategoryDescriptor() const { return nullptr; }

	virtual void GenerateContextMenu(class UToolMenu* MenuBuilder, class SSubsystemBrowserPanel& Browser) const { }

	bool bExpanded = true;
	bool bVisible = true;
	bool bNeedsRefresh = true;

	TSharedPtr<FSubsystemModel> Model;
	mutable SubsystemTreeItemPtr Parent;
	mutable TArray<SubsystemTreeItemPtr> Children;
};

/**
 * Category node
 */
struct FSubsystemTreeCategoryItem  : public ISubsystemTreeItem
{
	FName					CategoryName;
	UClass*					SubsystemBaseClass;
	FText					Label;
	FEnumSubsystemsDelegate Selector;

	FSubsystemTreeCategoryItem() = default;
	FSubsystemTreeCategoryItem(const FName& CategoryName,  const FText& Label, UClass* SubsystemBaseClass, const FEnumSubsystemsDelegate& Selector);

	virtual FSubsystemTreeItemID GetID() const override { return CategoryName; }

	virtual FString GetDisplayNameString() const override { return Label.ToString(); }
	virtual FString GetClassNameString() const override { return FString(); }
	virtual FString GetShortPackageString() const override  { return FString(); }
	virtual FString GetPackageString() const override { return FString(); }
	virtual FString GetConfigClassNameString() const override  { return FString(); }
	virtual FString GetOwnerNameString() const override  { return FString(); }

	virtual bool CanHaveChildren() const override { return true; }

	virtual FSubsystemTreeCategoryItem* GetAsCategoryDescriptor() const override { return const_cast<FSubsystemTreeCategoryItem*>(this); }
};

/**
 * Subsystem node
 */
struct FSubsystemTreeSubsystemItem  : public ISubsystemTreeItem
{
	TWeakObjectPtr<UObject>			Subsystem;
	TWeakObjectPtr<UClass>			Class;

	FText							DisplayName;
	FName							ClassName;
	FString							Package;
	FString							LongPackage;
	FString							ShortPackage;
	FString							ConfigClass;

	FName							LocalPlayerName;

	FString                         ModuleName;
	FString							ModulePath;
	TArray<FString>					SourceFilePaths;

	bool							bTickable = false;
	bool							bConfigExportable = false;
	bool							bIsGameModuleClass = false;

	FSubsystemTreeSubsystemItem() = default;
	FSubsystemTreeSubsystemItem(UObject* Subsystem);

	virtual FSubsystemTreeItemID GetID() const override { return ClassName; }

	virtual FString GetDisplayNameString() const override;
	virtual FString GetClassNameString() const override;
	virtual FString GetShortPackageString() const override;
	virtual FString GetPackageString() const override;
	virtual FString GetConfigClassNameString() const override { return ConfigClass; }
	virtual FString GetOwnerNameString() const override;

	virtual FSubsystemTreeSubsystemItem* GetAsSubsystemDescriptor() const override {  return const_cast<FSubsystemTreeSubsystemItem*>(this); }
	virtual UObject* GetObjectForDetails() const override { return Subsystem.Get(); }
	virtual bool IsStale() const override { return Subsystem.IsStale() || Class.IsStale(); }
	virtual bool IsTickable() const override { return bTickable; }
	virtual bool IsConfigExportable() const override { return bConfigExportable; }
	virtual bool IsGameModule() const override { return bIsGameModuleClass; }
};