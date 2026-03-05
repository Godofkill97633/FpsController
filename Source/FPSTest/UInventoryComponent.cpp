// Fill out your copyright notice in the Description page of Project Settings.

#include "UInventoryComponent.h"
#include "UBackpackComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ToolbeltSlots.SetNum(4);
}

void UInventoryComponent::ForceUIUpdate()
{
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SwapHands()
{
    UItemData* Temp = LeftHand.ContainedItem;
    LeftHand.ContainedItem = RightHand.ContainedItem;
    RightHand.ContainedItem = Temp;

    OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::TryStowItem(UItemData* ItemToStow)
{
    if (!ItemToStow) return false;

    // RULE 1: Backpacks ONLY go to the Spine.
    if (ItemToStow->ItemType == EItemType::Backpack)
    {
        if (BackpackSlot.IsEmpty())
        {
            BackpackSlot.ContainedItem = ItemToStow;
            OnInventoryUpdated.Broadcast();
            return true;
        }
        return false;
    }

    // RULE 2: Weapons ONLY go to Holsters.
    if (ItemToStow->ItemType == EItemType::Weapon)
    {
        if (PrimaryHolster.IsEmpty())
        {
            PrimaryHolster.ContainedItem = ItemToStow;
            OnInventoryUpdated.Broadcast();
            return true;
        }
        if (SidearmHolster.IsEmpty())
        {
            SidearmHolster.ContainedItem = ItemToStow;
            OnInventoryUpdated.Broadcast();
            return true;
        }
        return false;
    }

    // RULE 3: Tools & Consumables go to the Belt.
    if (ItemToStow->ItemType == EItemType::Tool || ItemToStow->ItemType == EItemType::Consumable)
    {
        int32 EmptySlot = GetFirstEmptyBeltSlot();
        if (EmptySlot != INDEX_NONE)
        {
            ToolbeltSlots[EmptySlot].ContainedItem = ItemToStow;
            OnInventoryUpdated.Broadcast();
            return true;
        }
    }

    return false;
}

void UInventoryComponent::TryPickupItem(UItemData* NewItem, bool bAltPressed)
{
    if (!NewItem) return;

    // 1. Determine "Preferred" Hand based on Item Type
    bool bTargetRight = (NewItem->ItemType == EItemType::Weapon || NewItem->ItemType == EItemType::Backpack);

    // Apply Alt modifier to flip the preference manually
    if (bAltPressed) bTargetRight = !bTargetRight;

    // 2. EMPTY HAND FALLBACK
    // If our preferred hand is full, but the other hand is completely empty, 
    // we use the empty hand to avoid unnecessary drops/stows.
    if (bTargetRight && !RightHand.IsEmpty() && LeftHand.IsEmpty())
    {
        bTargetRight = false; 
    }
    else if (!bTargetRight && !LeftHand.IsEmpty() && RightHand.IsEmpty())
    {
        bTargetRight = true;
    }

    FInventorySlot& TargetHand = bTargetRight ? RightHand : LeftHand;

    // 3. Displacement Check
    if (!TargetHand.IsEmpty())
    {
        if (TryStowItem(TargetHand.ContainedItem))
        {
            TargetHand.ContainedItem = nullptr;
        }
        else
        {
            // If stow fails, we null it out. 
            // Note: Blueprint Smart Drop handles the physical world actor.
            TargetHand.ContainedItem = nullptr;
        }
    }

    TargetHand.ContainedItem = NewItem;
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::QuickStowToBackpack(UBackpackComponent* ActiveBackpack)
{
    if (!ActiveBackpack) return;

    FInventorySlot* SlotToStow = nullptr;

    if (RightHand.ContainedItem && RightHand.ContainedItem->ItemType == EItemType::Backpack)
        SlotToStow = &LeftHand;
    else if (LeftHand.ContainedItem && LeftHand.ContainedItem->ItemType == EItemType::Backpack)
        SlotToStow = &RightHand;

    if (SlotToStow && !SlotToStow->IsEmpty())
    {
        if (ActiveBackpack->TryAddItem(SlotToStow->ContainedItem))
        {
            SlotToStow->ContainedItem = nullptr;
            OnInventoryUpdated.Broadcast();
        }
    }
}

void UInventoryComponent::ToggleHolster(bool bIsPrimary, bool bAltPressed)
{
    FInventorySlot& TargetHolster = bIsPrimary ? PrimaryHolster : SidearmHolster;

    // Determine which hand we are interacting with based on Alt
    FInventorySlot& TargetHand = bAltPressed ? LeftHand : RightHand;

    // SCENARIO A: Holster is Empty -> We are putting a weapon AWAY.
    if (TargetHolster.IsEmpty())
    {
        // Try to holster the weapon from the SELECTED hand (Alt = Left, No Alt = Right)
        if (!TargetHand.IsEmpty() && TargetHand.ContainedItem->ItemType == EItemType::Weapon)
        {
            TargetHolster.ContainedItem = TargetHand.ContainedItem;
            TargetHand.ContainedItem = nullptr;
        }
        // FALLBACK: If the selected hand is empty, but the OTHER hand has a weapon, 
        // we can still holster it as a convenience, or stay strict. 
        // Let's stay strict for dual-wielding precision.
    }
    // SCENARIO B: Holster has a weapon -> We are pulling it OUT or SWAPPING.
    else
    {
        if (TargetHand.IsEmpty())
        {
            // Pull weapon from holster into empty selected hand
            TargetHand.ContainedItem = TargetHolster.ContainedItem;
            TargetHolster.ContainedItem = nullptr;
        }
        else if (TargetHand.ContainedItem->ItemType == EItemType::Weapon)
        {
            // SWAP: Both hand and holster have weapons. Switch them.
            UItemData* Temp = TargetHand.ContainedItem;
            TargetHand.ContainedItem = TargetHolster.ContainedItem;
            TargetHolster.ContainedItem = Temp;
        }
        else
        {
            // Hand is full of a non-weapon (Tool/Consumable). Try to stow it to make room.
            if (TryStowItem(TargetHand.ContainedItem))
            {
                TargetHand.ContainedItem = TargetHolster.ContainedItem;
                TargetHolster.ContainedItem = nullptr;
            }
        }
    }

    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SwapHandWithBelt(int32 BeltIndex, bool bAltPressed)
{
    if (!ToolbeltSlots.IsValidIndex(BeltIndex)) return;

    FInventorySlot& TargetHand = bAltPressed ? RightHand : LeftHand;

    if (TargetHand.ContainedItem)
    {
        EItemType Type = TargetHand.ContainedItem->ItemType;
        if (Type == EItemType::Weapon || Type == EItemType::Backpack) return;
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