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
	IOnlineSubsystem* subSystem = Online::GetSubsystem(GetWorld());
	bool result{ false };
	if (subSystem)
	{
		IOnlineSessionPtr session = subSystem->GetSessionInterface();

		if (session.IsValid())
		{
			TSharedPtr<class FOnlineSessionSettings> sessionSettings = MakeShareable(new FOnlineSessionSettings());

			sessionSettings->NumPrivateConnections = 0;
			sessionSettings->NumPublicConnections = 4;

			sessionSettings->bShouldAdvertise = true;
			sessionSettings->bAllowJoinInProgress = true;
			sessionSettings->bAllowInvites = true;
			sessionSettings->bUsesPresence = true;
			sessionSettings->bAllowJoinViaPresence = true;
			sessionSettings->bUseLobbiesIfAvailable = true;

			sessionSettings->Set(SEARCH_KEYWORDS, FString("Custom"), EOnlineDataAdvertisementType::ViaOnlineService);

			session->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AMyPlayerController::OnCreateSessionCompleteDelegate));
			TSharedPtr<const FUniqueNetId> uniqueNetIdPtr = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
			result = session->CreateSession(*uniqueNetIdPtr, SESSION_NAME, *sessionSettings);
			if (result)
			{
				DISPLAY_LOG("CreatedSession: Success");
			}
			else
			{
				DISPLAY_LOG("CreateSession: Failed");
			}
		}
	}

	return result;
}

void AMyPlayerController::FindSession()
{
	IOnlineSubsystem* const subSystem = Online::GetSubsystem(GetWorld());
	if (subSystem)
	{
		IOnlineSessionPtr session = subSystem->GetSessionInterface();

		if (session.IsValid())
		{
			SearchSettings = MakeShareable(new FOnlineSessionSearch());

			SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
			SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
			SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("Custom"), EOnlineComparisonOp::Equals);

			session->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &AMyPlayerController::OnFindSessionsCompleteDelegate));

			TSharedRef<FOnlineSessionSearch> searchSettingsRef = SearchSettings.ToSharedRef();
			TSharedPtr<const FUniqueNetId> uniqueNetIdPtr = GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId();

			bool success = session->FindSessions(*uniqueNetIdPtr, searchSettingsRef);
		}
	}
}

void AMyPlayerController::QuitSession()
{
}

void AMyPlayerController::OnLoginCompleteDelegate(int32 _localUserNum, bool _bWasSuccessful, const FUniqueNetId& _userId, const FString& _error)
{
	IOnlineIdentityPtr identity = Online::GetIdentityInterface();

	if (identity.IsValid())
	{
		ULocalPlayer* localPlayer = Cast<ULocalPlayer>(Player);

		if (localPlayer != NULL)
		{
			FUniqueNetIdRepl uniqueId = PlayerState->GetUniqueId();
			uniqueId.SetUniqueNetId(FUniqueNetIdWrapper(_userId).GetUniqueNetId());

			PlayerState->SetUniqueId(uniqueId);

			int controllerID = localPlayer->GetControllerId();
			ELoginStatus::Type status = identity->GetLoginStatus(controllerID);
			DISPLAY_LOG("Login Status: %s", ELoginStatus::ToString(status));
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("LOGIN ERROR: %s", FCommandLine::Get()));
			DISPLAY_LOG("Login Status: IDENTITY INVALID");
		}
	}
}

void AMyPlayerController::OnCreateSessionCompleteDelegate(FName _inSessionName, bool _wasSuccessful)
{
	if (_wasSuccessful)
	{
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap")), true, "listen");
	}
}

void AMyPlayerController::OnFindSessionsCompleteDelegate(bool _wasSuccessful)
{
	if (_wasSuccessful)
	{
		if (SearchSettings->SearchResults.Num() == 0)
		{
			DISPLAY_LOG("NO SESSION IDs FOUND!!");
		}
		else
		{
			const TCHAR* sessionId = *SearchSettings->SearchResults[0].GetSessionIdStr();

			DISPLAY_LOG("FOUND SESSION ID: %s", sessionId);

			//JoinSession(SearchSettings->SearchResults[0]);
		}
	}
	else
	{
		DISPLAY_LOG("FAILED TO FIND SESSION!!");
	}
}
