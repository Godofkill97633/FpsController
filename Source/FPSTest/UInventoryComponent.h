// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UItemData.h"
#include "UInventoryComponent.generated.h"

// Define a slot to hold an item
USTRUCT(BlueprintType)
struct FPSTEST_API FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    UItemData* ContainedItem = nullptr;

    bool IsEmpty() const { return ContainedItem == nullptr; }
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPSTEST_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // --- SLOTS ---
    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Hands")
    FInventorySlot RightHand;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Hands")
    FInventorySlot LeftHand;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Holsters")
    FInventorySlot PrimaryHolster; // Key 1

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Holsters")
    FInventorySlot SidearmHolster; // Key 2

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Belt")
    TArray<FInventorySlot> ToolbeltSlots; // Keys 3-0

    // --- FUNCTIONS ---

    /** Logic for picking up an item from the ground (F Key) */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void TryPickupItem(UItemData* NewItem);

    /** Logic for swapping hand with a toolbelt slot (Keys 3-0) */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SwapHandWithBelt(int32 BeltIndex);

    /** Logic for holstering weapon (Keys 1-2) */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleHolster(bool bIsPrimary);

protected:
    // Helper to find empty belt space
    int32 GetFirstEmptyBeltSlot() const;
};