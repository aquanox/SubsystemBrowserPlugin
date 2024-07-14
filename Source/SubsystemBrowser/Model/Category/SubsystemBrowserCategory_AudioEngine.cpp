// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_AudioEngine.h"

#if UE_VERSION_NEWER_THAN(5, 1, 0)

#include "AudioDevice.h"
#include "AudioDeviceHandle.h"
#include "Engine/World.h"
#include "Subsystems/AudioEngineSubsystem.h"

FSubsystemCategory_AudioEngine::FSubsystemCategory_AudioEngine()
{
	Name = TEXT("AudioEngineSubsystemCategory");
	SettingsName = TEXT("AudioEngine");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_AudioEngine", "Audio Engine Subsystems");
	SortOrder = 150;
}

UClass* FSubsystemCategory_AudioEngine::GetSubsystemClass() const
{
	return UAudioEngineSubsystem::StaticClass();
}

void FSubsystemCategory_AudioEngine::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext))
	{
		FAudioDeviceHandle AudioDeviceHandle = InContext->GetAudioDevice();
		if (AudioDeviceHandle.IsValid())
		{
			OutData.Append(AudioDeviceHandle->GetSubsystemArray<UAudioEngineSubsystem>());
		}
	}
}

#endif
