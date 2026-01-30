// Fill out your copyright notice in the Description page of Project Settings.

#include "UInventoryComponent.h"
#include "UBackpackComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ToolbeltSlots.SetNum(4);
}

void UInventoryComponent::SwapHands()
{
    UItemData* Temp = LeftHand.ContainedItem;
    LeftHand.ContainedItem = RightHand.ContainedItem;
    RightHand.ContainedItem = Temp;

    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::QuickStowToBackpack(UBackpackComponent* ActiveBackpack)
{
    if (!ActiveBackpack) return;

    // 1. Identify which hand has the backpack and which has the item
    FInventorySlot* ItemSlot = nullptr;

    if (RightHand.ContainedItem && RightHand.ContainedItem->ItemType == EItemType::Backpack)
    {
        ItemSlot = &LeftHand;
    }
    else if (LeftHand.ContainedItem && LeftHand.ContainedItem->ItemType == EItemType::Backpack)
    {
        ItemSlot = &RightHand;
    }

    // 2. If we found an item hand, try to move it to the backpack
    if (ItemSlot && !ItemSlot->IsEmpty())
    {
        if (ActiveBackpack->TryAddItem(ItemSlot->ContainedItem))
        {
            ItemSlot->ContainedItem = nullptr;
            OnInventoryUpdated.Broadcast();
        }
    }
}

void UInventoryComponent::TryPickupItem(UItemData* NewItem, bool bAltPressed)
{
    if (!NewItem) return;

    // 1. Determine Default Hand Priority
    // Weapons -> Right (true), Everything else -> Left (false)
    bool bTargetRight = (NewItem->ItemType == EItemType::Weapon);

    // 2. Apply Alt Override
    if (bAltPressed) bTargetRight = !bTargetRight;

    FInventorySlot& TargetHand = bTargetRight ? RightHand : LeftHand;

    // 3. If the hand is full, try to STOW the current item to its valid home
    if (!TargetHand.IsEmpty())
    {
        // Try to find a place for the item currently in hand
        bool bStowed = TryStowItem(TargetHand.ContainedItem);

        // If we couldn't stow it (full inventory), we overwrite it (effectively dropping/replacing it)
        TargetHand.ContainedItem = nullptr;
    }

    // 4. Place new item in hand
    TargetHand.ContainedItem = NewItem;
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ToggleHolster(bool bIsPrimary, bool bAltPressed)
{
    FInventorySlot& TargetHolster = bIsPrimary ? PrimaryHolster : SidearmHolster;

    // CASE: Putting away a weapon
    if (TargetHolster.IsEmpty())
    {
        // Prioritize Left hand for holstering (keep right hand free), unless Alt is held
        bool bCheckLeftFirst = !bAltPressed;
        FInventorySlot& PriorityHand = bCheckLeftFirst ? LeftHand : RightHand;
        FInventorySlot& SecondaryHand = bCheckLeftFirst ? RightHand : LeftHand;

        // ONLY allow items classified as Weapons to enter holsters
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
    // CASE: Drawing a weapon
    else
    {
        // Weapons draw to Right hand by default unless Alt is held
        bool bTargetRight = !bAltPressed;
        FInventorySlot& TargetHand = bTargetRight ? RightHand : LeftHand;

        // If hand is full, stow it before drawing weapon
        if (!TargetHand.IsEmpty())
        {
            TryStowItem(TargetHand.ContainedItem);
        }

        TargetHand.ContainedItem = TargetHolster.ContainedItem;
        TargetHolster.ContainedItem = nullptr;
    }

    OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::TryStowItem(UItemData* ItemToStow)
{
    if (!ItemToStow) return false;

    // RULE: Weapons go to Holsters
    if (ItemToStow->ItemType == EItemType::Weapon)
    {
        if (PrimaryHolster.IsEmpty()) { PrimaryHolster.ContainedItem = ItemToStow; return true; }
        if (SidearmHolster.IsEmpty()) { SidearmHolster.ContainedItem = ItemToStow; return true; }
    }
    // RULE: Tools/Consumables go to Belt
    else
    {
        int32 Slot = GetFirstEmptyBeltSlot();
        if (Slot != INDEX_NONE)
        {
            ToolbeltSlots[Slot].ContainedItem = ItemToStow;
            return true;
        }
    }

    return false; // Nowhere to put it!
}

void UInventoryComponent::SwapHandWithBelt(int32 BeltIndex, bool bAltPressed)
{
    if (!ToolbeltSlots.IsValidIndex(BeltIndex)) return;

    // Items from belt prefer Left Hand unless Alt is held
    FInventorySlot& TargetHand = bAltPressed ? RightHand : LeftHand;

    // Strict Rule: Don't let a weapon accidentally be swapped into the belt via this function
    if (!TargetHand.IsEmpty() && TargetHand.ContainedItem->ItemType == EItemType::Weapon)
    {
        // If right hand has a weapon, we must stow the weapon properly first
        TryStowItem(TargetHand.ContainedItem);
        TargetHand.ContainedItem = nullptr;
    }

    UItemData* Temp = TargetHand.ContainedItem;
    TargetHand.ContainedItem = ToolbeltSlots[BeltIndex].ContainedItem;
    ToolbeltSlots[BeltIndex].ContainedItem = Temp;

    OnInventoryUpdated.Broadcast();
}

int32 UInventoryComponent::GetFirstEmptyBeltSlot() const
{
    for (int32 i = 0; i < ToolbeltSlots.Num(); i++)
    {
        if (ToolbeltSlots[i].IsEmpty()) return i;
    }
    return INDEX_NONE;
}

bool UInventoryComponent::HasEmptyWeaponHolster() const
{
    return PrimaryHolster.IsEmpty() || SidearmHolster.IsEmpty();
}