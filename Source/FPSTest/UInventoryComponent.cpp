// Fill out your copyright notice in the Description page of Project Settings.

#include "UInventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ToolbeltSlots.SetNum(4);
}

void UInventoryComponent::TryPickupItem(UItemData* NewItem, bool bAltPressed)
{
    if (!NewItem) return;

    // RULE 1: Determine Hand Priority based on Item Type
    bool bWeapon = (NewItem->ItemType == EItemType::Weapon);

    // Default: Weapons -> Right (true), Non-Weapons -> Left (false)
    bool bTargetRight = bWeapon;

    // RULE 2: Manual Override via Alt
    if (bAltPressed) bTargetRight = !bTargetRight;

    FInventorySlot& PrimarySlot = bTargetRight ? RightHand : LeftHand;
    FInventorySlot& SecondarySlot = bTargetRight ? LeftHand : RightHand;

    // 1. Try Priority Hand
    if (PrimarySlot.IsEmpty())
    {
        PrimarySlot.ContainedItem = NewItem;
    }
    // 2. Try Secondary Hand
    else if (SecondarySlot.IsEmpty())
    {
        SecondarySlot.ContainedItem = NewItem;
    }
    // 3. Both full: Stow the prioritized hand's item and take new one
    else
    {
        TryStowItem(PrimarySlot.ContainedItem);
        PrimarySlot.ContainedItem = NewItem;
    }

    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ToggleHolster(bool bIsPrimary, bool bAltPressed)
{
    FInventorySlot& TargetHolster = bIsPrimary ? PrimaryHolster : SidearmHolster;

    // LOGIC: PUTTING AWAY (Holstering)
    if (TargetHolster.IsEmpty())
    {
        // Default priority: Left hand first (to keep right hand weapon active)
        bool bCheckLeftFirst = !bAltPressed;
        FInventorySlot& PriorityHand = bCheckLeftFirst ? LeftHand : RightHand;
        FInventorySlot& SecondaryHand = bCheckLeftFirst ? RightHand : LeftHand;

        if (!PriorityHand.IsEmpty() && PriorityHand.ContainedItem->ItemType == EItemType::Weapon)
        {
            TargetHolster.ContainedItem = PriorityHand.ContainedItem;
            PriorityHand.ContainedItem = nullptr;
        }
        else if (!SecondaryHand.IsEmpty() && SecondaryHand.ContainedItem->ItemType == EItemType::Weapon)
        {
            TargetHolster.ContainedItem = SecondaryHand.ContainedItem;
            SecondaryHand.ContainedItem = nullptr;
        }
    }
    // LOGIC: TAKING OUT (Unholstering)
    else
    {
        // Weapons unholster to Right hand by default unless Alt is held
        bool bTargetRight = !bAltPressed;
        FInventorySlot& TargetHand = bTargetRight ? RightHand : LeftHand;

        if (!TargetHand.IsEmpty())
        {
            TryStowItem(TargetHand.ContainedItem);
        }

        TargetHand.ContainedItem = TargetHolster.ContainedItem;
        TargetHolster.ContainedItem = nullptr;
    }

    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SwapHandWithBelt(int32 BeltIndex, bool bAltPressed)
{
    if (!ToolbeltSlots.IsValidIndex(BeltIndex)) return;

    // Belt items go to Left Hand unless Alt is held
    bool bUseRightHand = bAltPressed;
    FInventorySlot& TargetHand = bUseRightHand ? RightHand : LeftHand;

    UItemData* Temp = TargetHand.ContainedItem;
    TargetHand.ContainedItem = ToolbeltSlots[BeltIndex].ContainedItem;
    ToolbeltSlots[BeltIndex].ContainedItem = Temp;

    OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::TryStowItem(UItemData* ItemToStow)
{
    if (!ItemToStow) return false;
    int32 SlotIndex = GetFirstEmptyBeltSlot();
    if (SlotIndex != INDEX_NONE)
    {
        ToolbeltSlots[SlotIndex].ContainedItem = ItemToStow;
        return true;
    }
    return false; // Belt is full!
}

int32 UInventoryComponent::GetFirstEmptyBeltSlot() const
{
    for (int32 i = 0; i < ToolbeltSlots.Num(); i++)
    {
        if (ToolbeltSlots[i].IsEmpty()) return i;
    }
    return INDEX_NONE;
}