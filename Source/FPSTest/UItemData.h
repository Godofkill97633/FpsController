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

// Forward declarations to improve compile times and reduce header bloat
class UStaticMesh;
class UMaterialInterface;
class UTexture2D;
class AActor;

/**
 * UItemData serves as the single source of truth for an item's properties.
 */
UCLASS(BlueprintType)
class FPSTEST_API UItemData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // --- IDENTITY ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EItemType ItemType;

    // --- VISUALS ---
    
    /** The 3D model used in the world and in the hand */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    UStaticMesh* PickupMesh; 

    /** The specific material/color for this item. 
     * This ensures Red items stay Red and Blue items stay Blue in the hand.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    UMaterialInterface* ItemMaterial;

    /** The 2D icon for the HUD */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    UTexture2D* ItemIcon;

    /** The actor class to spawn when this item is held (usually BP_HeldItemBase) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSubclassOf<AActor> HeldActorClass; 

    // --- STATS ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Weight;
};