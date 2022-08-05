// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_Player.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h"

FSubsystemCategory_Player::FSubsystemCategory_Player()
{
	Name = TEXT("PlayerCategory");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_Player", "Player Subsystems");
	SortOrder = 500;
}

void FSubsystemCategory_Player::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext) && InContext->GetGameInstance())
	{
		for (ULocalPlayer* const LocalPlayer : InContext->GetGameInstance()->GetLocalPlayers())
		{
			OutData.Append(LocalPlayer->GetSubsystemArray<ULocalPlayerSubsystem>());
		}
	}
}
