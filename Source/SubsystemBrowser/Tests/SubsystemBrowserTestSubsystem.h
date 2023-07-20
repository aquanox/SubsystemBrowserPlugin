// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SubsystemBrowserTestSubsystem.generated.h"

UENUM(meta=(Experimental))
enum class EDemoEnum : uint8
{
	Alpha,
	Beta,
	Gamma,
	Delta,
	Epsilon
};


USTRUCT(meta=(Experimental))
struct FDemoStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 Foo = 0;
	UPROPERTY(EditAnywhere)
	int32 Bar = 0;
	UPROPERTY(EditAnywhere)
	EDemoEnum Baz = EDemoEnum::Alpha;
};

UCLASS(DefaultToInstanced, EditInlineNew)
class UDemoObject : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	int32 Foo;
	UPROPERTY(EditAnywhere)
	int32 Bar;
	UPROPERTY(EditAnywhere)
	EDemoEnum Baz;
};

DECLARE_DYNAMIC_DELEGATE(FSubsystemDynamicTestDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemDynamicMCTestDelegate);

/**
 *
 */
UCLASS(Abstract, Config=Test, DefaultConfig)
class SUBSYSTEMBROWSER_API USubsystemBrowserTestSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	USubsystemBrowserTestSubsystem();
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintReadWrite)
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
	UDemoObject* GeneralInstancedProperty;
	UPROPERTY(EditAnywhere, Category=General)
	int32 GeneralIntProperty;
	UPROPERTY(EditAnywhere, Category=General)
	FDemoStruct GeneralStructProperty;
	UPROPERTY(EditAnywhere, Category=General)
	EDemoEnum GeneralEnumProperty;
	UPROPERTY(EditAnywhere, Category=General)
	FGuid GeneralGuidProperty;
	UPROPERTY(EditAnywhere, Category=General, meta=(ForceInlineRow))
	TMap<FGuid, UDemoObject*> GeneralMapProperty;

	UPROPERTY(Config, EditAnywhere, Category=Config)
	int32 ConfigIntProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	FString ConfigStringProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TSoftClassPtr<class AActor> ConfigClassProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TSoftObjectPtr<class UDataAsset> ConfigAssetProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TArray<FDemoStruct> ConfigArrayOfStructsProperty;
	UPROPERTY(Config, EditAnywhere, Category=Config)
	TArray<EDemoEnum> ConfigArrayOfEnumsProperty;

	UFUNCTION(CallInEditor, Category=Tools)
	void EditorFunction();
	UFUNCTION(CallInEditor, Category=Tools)
	void EditorFunctionReset();
	UPROPERTY(VisibleAnywhere, Category=Tools)
	int32 EditorFunctionCallCounter = 0;

	UPROPERTY(VisibleAnywhere, Category=Delegates)
	FSubsystemDynamicTestDelegate SingleDelegate;
	UPROPERTY(VisibleAnywhere, Category=Delegates)
	FSubsystemDynamicMCTestDelegate MulticastDelegate;
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category=Delegates)
	FSubsystemDynamicMCTestDelegate MulticastAssignableDelegate;
};
