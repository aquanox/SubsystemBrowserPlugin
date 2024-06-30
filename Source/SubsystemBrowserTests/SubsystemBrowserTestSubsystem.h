// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SubsystemBrowserTestSubsystem.generated.h"

UENUM(meta=(Experimental))
enum class ESBDemoEnum : uint8
{
	Alpha,
	Beta,
	Gamma,
	Delta,
	Epsilon
};

USTRUCT(meta=(Experimental))
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

UCLASS(Hidden, DefaultToInstanced, EditInlineNew)
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
};

DECLARE_DYNAMIC_DELEGATE(FSBTestDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBTestDynamicMCDelegate);

/**
 *
 */
UCLASS(Abstract, Config=Test, DefaultConfig,
	meta=(Experimental, SBTooltip="SB Tooltip Text", SBColor="(R=255,G=128,B=0)", SBOwnerName="GetSBOwnerName"))
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

	UPROPERTY(BlueprintReadWrite, Category="SubsystemBrowserTest")
	int32 HiddenBlueprintOnlyProperty;
	UPROPERTY(Config)
	int32 HiddenConfigProperty;
	UPROPERTY()
	int32 HiddenProperty;

	UPROPERTY(EditAnywhere, Category=Edit)
	int32 EditAnywhereProperty;
	UPROPERTY(EditDefaultsOnly, Category=Edit)
	int32 EditDefaultsOnlyProperty;
	UPROPERTY(EditInstanceOnly, Category=Edit)
	int32 EditInstanceOnlyProperty;

	UPROPERTY(VisibleAnywhere, Category=Visible)
	int32 VisibleAnywhereProperty;
	UPROPERTY(VisibleDefaultsOnly, Category=Visible)
	int32 VisibleDefaultsOnlyProperty;
	UPROPERTY(VisibleInstanceOnly, Category=Visible)
	int32 VisibleInstanceOnlyProperty;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=General)
	TSubclassOf<class AActor> GeneralClassProperty;
	UPROPERTY(EditAnywhere, Category=General)
	TSoftObjectPtr<class ULevel> GeneralAssetProperty;
	UPROPERTY(EditAnywhere, Instanced, Category=General)
	USBDemoObject* GeneralInstancedProperty;
	UPROPERTY(EditAnywhere, Category=General)
	int32 GeneralIntProperty;
	UPROPERTY(EditAnywhere, Category=General)
	FSBDemoStruct GeneralStructProperty;
	UPROPERTY(EditAnywhere, Category=General)
	ESBDemoEnum GeneralEnumProperty;
	UPROPERTY(EditAnywhere, Category=General)
	FGuid GeneralGuidProperty;
	UPROPERTY(EditAnywhere, Category=General, meta=(ForceInlineRow))
	TMap<FGuid, USBDemoObject*> GeneralMapProperty;

	UPROPERTY(Config, EditAnywhere, Category=Config)
	int32 ConfigIntProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	FString ConfigStringProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TSoftClassPtr<class AActor> ConfigClassProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TSoftObjectPtr<class UDataAsset> ConfigAssetProperty;
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
};
