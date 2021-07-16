// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FServerInfo {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FString ServerName;
	UPROPERTY(BlueprintReadOnly)
		FString HostName;
	UPROPERTY(BlueprintReadOnly)
		int32 Ping;
	UPROPERTY(BlueprintReadOnly)
		int32 CurrentPlayers;
	UPROPERTY(BlueprintReadOnly)
		int32 MaxPlayers;
	UPROPERTY(BlueprintReadOnly)
		int32 ServerArrayIndex;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerDel, FServerInfo, ServerListDel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerSearchingDel, bool, SearchingForServer);


UCLASS()
class CODENEBULA_REMAKE_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UNetGameInstance();

protected:
	FName MySessionName;

	UPROPERTY(BlueprintAssignable)
		FServerDel ServerListDel;

	UPROPERTY(BlueprintAssignable)
		FServerSearchingDel SearchingForServer;

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool Succeeded);
	virtual void OnFindSessionsComplete(bool Succeeded);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnDestroySessionComplete(FName SessionName, bool Succeeded);

	UFUNCTION(BlueprintCallable)
		void CreateServer(FString ServerName, FString HostName, int32 MaxPlayers, bool UseLAN);

	UFUNCTION(BlueprintCallable)
		void FindServers(int32 MaxResults, bool UseLAN);

	UFUNCTION(BlueprintCallable)
		void JoinServer(int32 ArrayIndex);

	UFUNCTION(BlueprintCallable)
		void DestroyServer();
};
