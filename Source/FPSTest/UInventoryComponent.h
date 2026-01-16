// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UItemData.h"
#include "UInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

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
    FInventorySlot PrimaryHolster;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Holsters")
    FInventorySlot SidearmHolster;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Belt")
    TArray<FInventorySlot> ToolbeltSlots;

    // --- EVENTS ---
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    // --- FUNCTIONS ---

    /** * Handles contextual pickup logic.
     * Weapons default to Right Hand. Tools/Consumables default to Left Hand.
     * @param bAltPressed If true, flips the priority (Weapon to Left, Tool to Right).
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void TryPickupItem(UItemData* NewItem, bool bAltPressed);

    /** * Handles holstering with contextual priority.
     * Default: Holsters from Left Hand first.
     * @param bAltPressed If true, flips priority to holster from Right Hand first.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleHolster(bool bIsPrimary, bool bAltPressed);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SwapHandWithBelt(int32 BeltIndex, bool bAltPressed);

protected:
    /** Helper to move an item from hand to the first available belt slot */
    bool TryStowItem(UItemData* ItemToStow);
    int32 GetFirstEmptyBeltSlot() const;
};