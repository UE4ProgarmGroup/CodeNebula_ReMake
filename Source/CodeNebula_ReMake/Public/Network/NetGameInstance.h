// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FRoomInfo {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FString RoomName;
	UPROPERTY(BlueprintReadOnly)
		FString HostName;
	UPROPERTY(BlueprintReadOnly)
		int32 Ping;
	UPROPERTY(BlueprintReadOnly)
		int32 CurrentPlayers;
	UPROPERTY(BlueprintReadOnly)
		int32 MaxPlayers;
	UPROPERTY(BlueprintReadOnly)
		int32 RoomId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewRoomDel, FRoomInfo, NewRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRoomSearchingDel, bool, Status);


UCLASS()
class CODENEBULA_REMAKE_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UNetGameInstance();

protected:
	UPROPERTY(BlueprintAssignable)
		FNewRoomDel NewRoom;

	UPROPERTY(BlueprintAssignable)
		FRoomSearchingDel RoomSearching;

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool Succeeded);
	virtual void OnFindSessionsComplete(bool Succeeded);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnDestroySessionComplete(FName SessionName, bool Succeeded);

	UFUNCTION(BlueprintCallable)
		void CreateRoom(FString RoomName, FString HostName, int32 MaxPlayers, bool UseLAN);

	UFUNCTION(BlueprintCallable)
		void FindRooms(int32 MaxResults, bool UseLAN);

	UFUNCTION(BlueprintCallable)
		void JoinRoom(int32 RoomId);

	UFUNCTION(BlueprintCallable)
		void CloseRoom();

	UFUNCTION(BlueprintCallable)
		void LeaveRoom();
};
