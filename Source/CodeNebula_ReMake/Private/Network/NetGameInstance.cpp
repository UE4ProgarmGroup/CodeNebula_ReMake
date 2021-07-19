// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/NetGameInstance.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


UNetGameInstance::UNetGameInstance() { }

void UNetGameInstance::Init() {
	if (IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get()) {
		SessionInterface = SubSystem->GetSessionInterface();
		if (SessionInterface.IsValid()) {
			// Bind Delegates Here
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnDestroySessionComplete);
		}
	}
}

void UNetGameInstance::OnCreateSessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete, Succeeded: %d"), Succeeded);

	if (Succeeded) {
		UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/Room", true, "listen");
	}
}

void UNetGameInstance::OnFindSessionsComplete(bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionComplete, Succeeded: %d"), Succeeded);

	RoomSearching.Broadcast(false);
	if (Succeeded) {
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;

		int32 ArrayIndex = -1;
		for (FOnlineSessionSearchResult Result : SearchResults) {
			ArrayIndex++;
			if (!Result.IsValid())
				continue;

			FRoomInfo Info;
			FString RoomName = "Empty Room Name";
			FString HostName = "Empty Host Name";

			Result.Session.SessionSettings.Get(FName("SERVER_ROOMNAME_KEY"), RoomName);
			Result.Session.SessionSettings.Get(FName("SERVER_HOSTNAME_KEY"), HostName);

			Info.RoomName = RoomName;
			Info.HostName = HostName;
			Info.Ping = Result.PingInMs;
			Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - Result.Session.NumOpenPublicConnections;
			Info.RoomId = ArrayIndex;

			NewRoom.Broadcast(Info);
		}

		UE_LOG(LogTemp, Warning, TEXT("SearchResults, Server Count: %d"), SearchResults.Num());
	}
}

void UNetGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete, SessionName: %s"), *SessionName.ToString());

	if (APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0)) {
		FString JoinAddress = "";
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
		if (JoinAddress != "")
			PController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
	}
}

void UNetGameInstance::OnDestroySessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete, SessionName: %s, Succeed: %d"), *SessionName.ToString(), Succeeded);

	if (Succeeded) {
		UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/MainMenu", true);
	}
}

void UNetGameInstance::CreateRoom(FString RoomName, FString HostName, int32 MaxPlayers, bool UseLAN)
{
	UE_LOG(LogTemp, Warning, TEXT("CreateRoom"));

	IOnlineSubsystem* const SubSystem = IOnlineSubsystem::Get();

	if (SubSystem) {
		IOnlineSessionPtr Sessions = SubSystem->GetSessionInterface();

		if (Sessions.IsValid()) {
			ULocalPlayer* const Player = GetFirstGamePlayer();

			FOnlineSessionSettings SessionSettings;

			SessionSettings.bIsLANMatch = UseLAN;
			SessionSettings.bUsesPresence = true;
			SessionSettings.NumPublicConnections = MaxPlayers;
			SessionSettings.NumPrivateConnections = 0;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bAllowJoinViaPresence = true;
			SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;

			SessionSettings.Set(FName("SERVER_ROOMNAME_KEY"), RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(FName("SERVER_HOSTNAME_KEY"), HostName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

			Sessions->CreateSession(*Player->GetPreferredUniqueNetId(), GameSessionName, SessionSettings);
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}
}

void UNetGameInstance::FindRooms(int32 MaxResults, bool UseLAN)
{
	UE_LOG(LogTemp, Warning, TEXT("FindRooms"));

	IOnlineSubsystem* const SubSystem = IOnlineSubsystem::Get();

	if (SubSystem) {
		ULocalPlayer* const Player = GetFirstGamePlayer();

		IOnlineSessionPtr Sessions = SubSystem->GetSessionInterface();

		if (Sessions.IsValid()) {
			RoomSearching.Broadcast(true);

			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = UseLAN;
			SessionSearch->MaxSearchResults = MaxResults;
			SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

			Sessions->FindSessions(*Player->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}
}

void UNetGameInstance::JoinRoom(int32 RoomId)
{
	UE_LOG(LogTemp, Warning, TEXT("JoinRoom"));

	IOnlineSubsystem* const SubSystem = IOnlineSubsystem::Get();

	if (SubSystem) {
		ULocalPlayer* const Player = GetFirstGamePlayer();

		IOnlineSessionPtr Sessions = SubSystem->GetSessionInterface();

		if (Sessions.IsValid()) {
			FOnlineSessionSearchResult Result = SessionSearch->SearchResults[RoomId];

			if (Result.IsValid()) {
				UE_LOG(LogTemp, Warning, TEXT("Joining Room, Room Id: %d"), RoomId);

				SessionInterface->JoinSession(*Player->GetPreferredUniqueNetId(), GameSessionName, Result);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Failed To Join Room, Room Id: %d"), RoomId);
			}
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}
}

void UNetGameInstance::CloseRoom()
{
	UE_LOG(LogTemp, Warning, TEXT("CloseRoom"));

	IOnlineSubsystem* const SubSystem = IOnlineSubsystem::Get();

	if (SubSystem) {
		IOnlineSessionPtr Sessions = SubSystem->GetSessionInterface();

		if (Sessions.IsValid()) {
			Sessions->DestroySession(GameSessionName);
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}
}

void UNetGameInstance::LeaveRoom()
{
	UE_LOG(LogTemp, Warning, TEXT("LeaveRoom"));

	IOnlineSubsystem* const SubSystem = IOnlineSubsystem::Get();

	if (SubSystem) {
		IOnlineSessionPtr Sessions = SubSystem->GetSessionInterface();

		if (Sessions.IsValid()) {
			Sessions->DestroySession(GameSessionName);
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}
}

