// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

// Online Subsystem Module Headers
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

// For Debug Logging
#include "EngineGlobals.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

// For loading Level
#include "Kismet/GameplayStatics.h"

const FName SESSION_NAME = "SessionName";
TSharedPtr<class FOnlineSessionSearch> SearchSettings;

#define DISPLAY_LOG(fmt, ...) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, FString::Printf(TEXT(fmt), __VA_ARGS__));



AMyPlayerController::AMyPlayerController()
{
	IOnlineSubsystem* subSystem = Online::GetSubsystem(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] found Subsystem %s"), *subSystem->GetSubsystemName().ToString());
}

void AMyPlayerController::Login()
{
	IOnlineSubsystem* subSystem = Online::GetSubsystem(GetWorld());

	if (subSystem)
	{
		IOnlineIdentityPtr identity = subSystem->GetIdentityInterface();

		if (identity.IsValid())
		{
			ULocalPlayer* localPlayer = Cast<ULocalPlayer>(Player);

			if (localPlayer)
			{
				int controllerID = localPlayer->GetControllerId();

				if (identity->GetLoginStatus(controllerID) != ELoginStatus::LoggedIn)
				{
					identity->AddOnLoginCompleteDelegate_Handle(controllerID, FOnLoginCompleteDelegate::CreateUObject(this, &AMyPlayerController::OnLoginCompleteDelegate));

					identity->AutoLogin(controllerID);
				}
			}
		}
	}
	PlayerState;
}

bool AMyPlayerController::HostSession()
{
	return true;
}

void AMyPlayerController::FindSession()
{
}

void AMyPlayerController::QuitSession()
{
}

void AMyPlayerController::OnLoginCompleteDelegate(int32 _localUserNum, bool _bWasSuccessful, const FUniqueNetId& _userId, const FString& _error)
{

}
