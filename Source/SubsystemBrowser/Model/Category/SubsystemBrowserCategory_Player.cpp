﻿// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_Player.h"

#include "Misc/EngineVersionComparison.h"
#include "Engine/GameInstance.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

FSubsystemCategory_Player::FSubsystemCategory_Player()
{
	Name = TEXT("PlayerCategory");
	SettingsName = TEXT("LocalPlayer");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_Player", "Player Subsystems");
	SortOrder = 500;
}

UClass* FSubsystemCategory_Player::GetSubsystemClass() const
{
	return ULocalPlayerSubsystem::StaticClass();
}

void FSubsystemCategory_Player::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext) && InContext->GetGameInstance())
	{
		for (ULocalPlayer* const LocalPlayer : InContext->GetGameInstance()->GetLocalPlayers())
		{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
			OutData.Append(LocalPlayer->GetSubsystemArray<ULocalPlayerSubsystem>());
#else
			OutData.Append(LocalPlayer->GetSubsystemArrayCopy<ULocalPlayerSubsystem>());
#endif
		}
	}
}
