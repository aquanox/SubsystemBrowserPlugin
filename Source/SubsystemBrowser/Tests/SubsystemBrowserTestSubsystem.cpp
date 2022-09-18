// Copyright 2022, Aquanox.

#include "Tests/SubsystemBrowserTestSubsystem.h"

USubsystemBrowserTestSubsystem::USubsystemBrowserTestSubsystem()
{
}

bool USubsystemBrowserTestSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return GetClass() != USubsystemBrowserTestSubsystem::StaticClass() && Super::ShouldCreateSubsystem(Outer);
}

void USubsystemBrowserTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USubsystemBrowserTestSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USubsystemBrowserTestSubsystem::EditorFunction()
{
	++EditorFunctionCallCounter;
}

void USubsystemBrowserTestSubsystem::EditorFunctionReset()
{
	EditorFunctionCallCounter = 0;
}
