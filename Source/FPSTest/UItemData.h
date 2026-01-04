// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Weapon      UMETA(DisplayName = "Weapon"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Tool        UMETA(DisplayName = "Tool"),
    Backpack    UMETA(DisplayName = "Backpack")
};

// Forward declarations
class UStaticMesh;
class AActor;

UCLASS(BlueprintType)
class FPSTEST_API UItemData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EItemType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    UStaticMesh* PickupMesh; // Model seen on the ground

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSubclassOf<AActor> HeldActorClass; // The Blueprint spawned in hand

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Weight;
};