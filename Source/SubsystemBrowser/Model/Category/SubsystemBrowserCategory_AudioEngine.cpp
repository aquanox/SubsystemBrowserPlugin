// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_AudioEngine.h"

#if UE_VERSION_NEWER_THAN(5, 1, 0)

#include "AudioDevice.h"
#include "AudioDeviceHandle.h"
#include "Engine/World.h"
#include "Subsystems/AudioEngineSubsystem.h"
#include "UObject/UObjectHash.h"

FSubsystemCategory_AudioEngine::FSubsystemCategory_AudioEngine()
{
	Name = TEXT("AudioEngineSubsystemCategory");
	SettingsName = TEXT("AudioEngine");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_AudioEngine", "Audio Engine Subsystems");
	SortOrder = 150;
}

void FSubsystemCategory_AudioEngine::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext))
	{
		FAudioDeviceHandle AudioDeviceHandle = InContext->GetAudioDevice();
		if (AudioDeviceHandle.IsValid())
		{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
			OutData.Append(AudioDeviceHandle->GetSubsystemArray<UAudioEngineSubsystem>());
#else
			OutData.Append(AudioDeviceHandle->GetSubsystemArrayCopy<UAudioEngineSubsystem>());
#endif
		}
	}
}

void FSubsystemCategory_AudioEngine::SelectSettings(TArray<UObject*>& OutData) const
{
	::GetObjectsOfClass(UAudioEngineSubsystem::StaticClass(), OutData, true, EObjectFlags::RF_NoFlags);
}

#endif
