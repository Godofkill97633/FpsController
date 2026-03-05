// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UItemData.h"
#include "UInventoryComponent.generated.h"

// Forward declaration of our new component
class UBackpackComponent;

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

    // --- HAND SLOTS ---
    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Hands")
    FInventorySlot RightHand;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Hands")
    FInventorySlot LeftHand;

    // --- STORAGE SLOTS ---
    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Holsters")
    FInventorySlot PrimaryHolster;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Holsters")
    FInventorySlot SidearmHolster;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Belt")
    TArray<FInventorySlot> ToolbeltSlots;

    /** This holds the 'Backpack Item' itself */
    UPROPERTY(BlueprintReadWrite, Category = "Inventory|Backpack")
    FInventorySlot BackpackSlot;

    // --- EVENTS ---
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    // --- FUNCTIONS ---

    /** * HUD SYNC FIX: 
     * Manually triggers the OnInventoryUpdated delegate.
     * Call this in Blueprints whenever you manually empty a slot (like on Drop).
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ForceUIUpdate();

    /** Swaps whatever is in the Left hand with the Right hand (X Key) */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SwapHands();

    /** Stows the item in the free hand into the held backpack (R Key) */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void QuickStowToBackpack(UBackpackComponent* ActiveBackpack);

    /** * Contextual Pickup: Weapons prefer Right, Tools prefer Left.
     * Enforces storage rules: Weapons to Holsters, Tools to Belt.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void TryPickupItem(UItemData* NewItem, bool bAltPressed);

    /** Smart Stow: Automatically finds the correct home (Holster vs Belt vs Spine) for an item.
     * Now BlueprintCallable so it can be used in Blueprint scripts!
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryStowItem(UItemData* ItemToStow);

    /** Only allows Weapons to be holstered. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleHolster(bool bIsPrimary, bool bAltPressed);

    /** Swaps hand item with belt (restricted to non-weapon items). */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SwapHandWithBelt(int32 BeltIndex, bool bAltPressed);

protected:
    int32 GetFirstEmptyBeltSlot() const;
    bool HasEmptyWeaponHolster() const;
};