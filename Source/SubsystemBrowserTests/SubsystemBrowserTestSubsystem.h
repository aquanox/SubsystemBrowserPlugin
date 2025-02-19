// Copyright 2022, Aquanox.

#pragma once

#include "CoreFwd.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Misc/EngineVersionComparison.h"
#include "SubsystemBrowserTestSubsystem.generated.h"

class USBDemoObject;

UENUM()
enum class ESBDemoEnum : uint8
{
	Alpha,
	Beta,
	Gamma,
	Delta,
	Epsilon
};

USTRUCT()
struct SUBSYSTEMBROWSERTESTS_API FSBDemoStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="SubsystemBrowserTest")
	int32 Foo = 0;
	UPROPERTY(EditAnywhere, Category="SubsystemBrowserTest")
	int32 Bar = 0;
	UPROPERTY(EditAnywhere, Category="SubsystemBrowserTest")
	ESBDemoEnum Baz = ESBDemoEnum::Alpha;
};

UCLASS(EditInlineNew)
class SUBSYSTEMBROWSERTESTS_API USBDemoObject : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="SubsystemBrowserTest")
	int32 Foo;
	UPROPERTY(EditAnywhere, Category="SubsystemBrowserTest")
	int32 Bar;
	UPROPERTY(EditAnywhere, Category="SubsystemBrowserTest")
	ESBDemoEnum Baz;
	UPROPERTY(EditAnywhere, Category="SubsystemBrowserTest")
	FSBDemoStruct Structz;
	UPROPERTY(EditAnywhere, Instanced, Category="SubsystemBrowserTest")
	USBDemoObject* Obz = nullptr;
};

UCLASS()
class USBDemoChunkAssistantManager : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="Manager")
	TArray<FName> ChunkNames;

	UFUNCTION(CallInEditor, Category="Manager")
	void Selfdestruct()
	{
#if !UE_VERSION_OLDER_THAN(5,0,0)
		MarkAsGarbage();
#endif
	}
};

UCLASS()
class USBDemoInteractionAssistantObject : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category="Manager")
	TWeakObjectPtr<AActor> LastUsedObject;
};

UCLASS()
class USBDemoWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional), Category="SubsystemBrowserTest")
	class UButton* SampleButton;
	UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional), Category="SubsystemBrowserTest")
	class UTextBlock* SampleTextBlock;
};

DECLARE_DYNAMIC_DELEGATE(FSBTestDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBTestDynamicMCDelegate);

/**
 *
 */
UCLASS(Abstract, Config=Test, DefaultConfig,
	meta=(SBTooltip="SB Tooltip Text", SBColor="(R=255,G=128,B=0)", SBOwnerName="GetSBOwnerName", SBGetSubobjects="GetImportantSubobjectsToDisplay"))
class SUBSYSTEMBROWSERTESTS_API USubsystemBrowserTestSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	USubsystemBrowserTestSubsystem();
	
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION()
	FString GetSBOwnerName() const;

	UFUNCTION()
	TArray<UObject*> GetImportantSubobjectsToDisplay() const;

	UPROPERTY()
	USBDemoInteractionAssistantObject* IAObject = nullptr;
	UPROPERTY()
	USBDemoChunkAssistantManager* CAObject = nullptr;

	UPROPERTY(BlueprintReadWrite, Category="SubsystemBrowserTest")
	int32 HiddenBlueprintOnlyProperty = 0;
	UPROPERTY(Config)
	int32 HiddenConfigProperty = 0;
	UPROPERTY()
	int32 HiddenProperty = 0;

	UPROPERTY(EditAnywhere, Category=Edit)
	int32 EditAnywhereProperty = 0;
	UPROPERTY(EditDefaultsOnly, Category=Edit)
	int32 EditDefaultsOnlyProperty = 0;
	UPROPERTY(EditInstanceOnly, Category=Edit)
	int32 EditInstanceOnlyProperty = 0;

	UPROPERTY(VisibleAnywhere, Category=Visible)
	int32 VisibleAnywhereProperty = 0;
	UPROPERTY(VisibleDefaultsOnly, Category=Visible)
	int32 VisibleDefaultsOnlyProperty = 0;
	UPROPERTY(VisibleInstanceOnly, Category=Visible)
	int32 VisibleInstanceOnlyProperty = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=General)
	TSubclassOf<AActor> GeneralClassProperty;
	UPROPERTY(EditAnywhere, Category=General)
	TSoftObjectPtr<UWorld> GeneralAssetProperty;
	UPROPERTY(EditAnywhere, Instanced, Category=General)
	USBDemoObject* GeneralInstancedProperty = nullptr;
	UPROPERTY(EditAnywhere, Category=General)
	int32 GeneralIntProperty = 0;
	UPROPERTY(EditAnywhere, Category=General)
	FSBDemoStruct GeneralStructProperty;
	UPROPERTY(EditAnywhere, Category=General)
	ESBDemoEnum GeneralEnumProperty = ESBDemoEnum::Alpha;
	UPROPERTY(EditAnywhere, Category=General)
	FGuid GeneralGuidProperty;
	UPROPERTY(EditAnywhere, Category=General, meta=(ForceInlineRow))
	TMap<FGuid, USBDemoObject*> GeneralMapProperty;

	UPROPERTY(Config, EditAnywhere, Category=Config)
	int32 ConfigIntProperty = 0;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	FString ConfigStringProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TSoftClassPtr<AActor> ConfigClassProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TSoftObjectPtr<UDataAsset> ConfigAssetProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TArray<FSBDemoStruct> ConfigArrayOfStructsProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TArray<ESBDemoEnum> ConfigArrayOfEnumsProperty;

	UFUNCTION(CallInEditor, Category=Tools)
	void EditorFunction();
	UFUNCTION(CallInEditor, Category=Tools)
	void EditorFunctionReset();
	UPROPERTY(VisibleAnywhere, Category=Tools)
	int32 EditorFunctionCallCounter = 0;

	UPROPERTY(VisibleAnywhere, Category=Delegates)
	FSBTestDynamicDelegate SingleDelegate;
	UPROPERTY(VisibleAnywhere, Category=Delegates)
	FSBTestDynamicMCDelegate MulticastDelegate;
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category=Delegates)
	FSBTestDynamicMCDelegate MulticastAssignableDelegate;

	UPROPERTY(EditAnywhere, Category=ArrayInt)
	uint32 ArrayIntegersFillValue = 100;
	UPROPERTY(VisibleAnywhere, Category=ArrayInt)
	TArray<int32> ArrayIntegers;

	UFUNCTION(CallInEditor, Category=ArrayInt)
	void FillArrays();
	UFUNCTION(CallInEditor, Category=ArrayInt)
	void EmptyArrays();

	UPROPERTY(EditAnywhere, Category=ArrayObj)
	uint32 ArrayObjectsFillValue = 1;
	UPROPERTY(VisibleAnywhere, Category=ArrayObj)
	USBDemoObject* ChainObjects = nullptr;
	UPROPERTY(VisibleAnywhere, Category=ArrayObj)
	TArray<USBDemoObject*> ArrayObjects;

	UFUNCTION(CallInEditor, Category=ArrayObj)
	void FillArrayObjs();
	UFUNCTION(CallInEditor, Category=ArrayObj)
	void EmptyArrayObjs();

	UPROPERTY(EditAnywhere, Category=ArrayWidget)
	uint32 ArrayWidgetsFillValue = 1;
	UPROPERTY(VisibleAnywhere, Category=ArrayWidget)
	TArray<USBDemoWidget*> ArrayWidgets;

	UFUNCTION(CallInEditor, Category=ArrayWidget)
	void FillArrayWidgets();
	UFUNCTION(CallInEditor, Category=ArrayWidget)
	void EmptyArrayWidgets();
};


