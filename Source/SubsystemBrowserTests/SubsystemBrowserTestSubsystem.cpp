// Copyright 2022, Aquanox.

#include "SubsystemBrowserTestSubsystem.h"

USubsystemBrowserTestSubsystem::USubsystemBrowserTestSubsystem()
{
}

bool USubsystemBrowserTestSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#ifdef WITH_SB_HOST_PROJECT
	return WITH_SB_HOST_PROJECT && Super::ShouldCreateSubsystem(Outer);
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

void USubsystemBrowserTestSubsystem::EditorFunction()
{
	++EditorFunctionCallCounter;
}

void USubsystemBrowserTestSubsystem::EditorFunctionReset()
{
	EditorFunctionCallCounter = 0;
}
