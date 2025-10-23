// Copyright 2022, Aquanox.

#include "SubsystemBrowserTestSubsystem.h"

#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/EngineVersionComparison.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Package.h"

void USBDemoInteractionAssistantObject::QuickActionTest()
{
	UE_LOG(LogTemp, Log, TEXT("Invoke QuickActionTest"));
}

USubsystemBrowserTestSubsystem::USubsystemBrowserTestSubsystem()
{
	IAObject = CreateDefaultSubobject<USBDemoInteractionAssistantObject>("InteractionManager");
	CAObject = CreateDefaultSubobject<USBDemoChunkAssistantManager>("ChunkManager");
}

bool USubsystemBrowserTestSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#ifdef WITH_SB_HOST_PROJECT
	return Super::ShouldCreateSubsystem(Outer);
#else
	return GetClass() != USubsystemBrowserTestSubsystem::StaticClass() && Super::ShouldCreateSubsystem(Outer);
#endif
}

void USubsystemBrowserTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USubsystemBrowserTestSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FString USubsystemBrowserTestSubsystem::GetSBOwnerName() const
{
	return TEXT("Hello from ") + GetNameSafe(GetOuter());
}

TArray<UObject*> USubsystemBrowserTestSubsystem::GetImportantSubobjectsToDisplay() const
{
	return TArray<UObject*>({ IAObject, CAObject });
}

void USubsystemBrowserTestSubsystem::EditorFunction()
{
	++EditorFunctionCallCounter;
}

void USubsystemBrowserTestSubsystem::EditorFunctionReset()
{
	EditorFunctionCallCounter = 0;
}

void USubsystemBrowserTestSubsystem::FillArrays()
{
	for (uint32 Idx = 0; Idx < ArrayIntegersFillValue; ++Idx)
	{
		ArrayIntegers.Add(FMath::RandHelper(100000));
	}
}

void USubsystemBrowserTestSubsystem::EmptyArrays()
{
	ArrayIntegers.Empty();
}

void USubsystemBrowserTestSubsystem::FillArrayObjs()
{
	for (uint32 Idx = 0; Idx < ArrayObjectsFillValue; ++Idx)
	{
		auto Object = NewObject<USBDemoObject>(GetTransientPackage(), NAME_None, RF_Transient, nullptr);
		Object->Obz = ChainObjects;
		ChainObjects = Object;

		ArrayObjects.Add(Object);
	}
}

void USubsystemBrowserTestSubsystem::EmptyArrayObjs()
{
	ChainObjects = nullptr;
}

void USubsystemBrowserTestSubsystem::FillArrayWidgets()
{
	for (uint32 Idx = 0; Idx < ArrayWidgetsFillValue; ++Idx)
	{
		auto Widget = CreateWidget<USBDemoWidget>(GetWorld());
		ArrayWidgets.Add(Widget);
	}
}

void USubsystemBrowserTestSubsystem::EmptyArrayWidgets()
{
	ArrayWidgets.Empty();
}

void USubsystemBrowserTestSubsystem::QuickActionTest()
{
	UE_LOG(LogTemp, Log, TEXT("Invoke QuickActionTest"));
}

void USubsystemBrowserTestSubsystem::QuickActionTest2()
{
	UE_LOG(LogTemp, Log, TEXT("Invoke QuickActionTest2"));
}
