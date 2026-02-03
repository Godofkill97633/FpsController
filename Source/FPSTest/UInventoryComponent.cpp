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
        return false; // Back is full, cannot stow to belt/holsters.
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
        return false; // Holsters full, cannot stow to belt.
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

    return false; // Nowhere valid to put it!
}

void UInventoryComponent::TryPickupItem(UItemData* NewItem, bool bAltPressed)
{
    if (!NewItem) return;

    bool bTargetRight = false;

    // --- BACKPACK PREFERENCE LOGIC ---
    if (NewItem->ItemType == EItemType::Backpack)
    {
        if (RightHand.IsEmpty()) 
        {
            bTargetRight = true;  // Backpack prefers RIGHT hand
        }
        else if (LeftHand.IsEmpty())
        {
            bTargetRight = false;  // RIGHT full, go to LEFT
        }
        else 
        {
            // Both hands full, force to RIGHT hand
            bTargetRight = true;
        }
    }
    else
    {
        // Standard Logic: Weapons prefer Right, Tools/Consumables prefer Left
        bTargetRight = (NewItem->ItemType == EItemType::Weapon);
    }

    // Alt key (bAltPressed) flips the final decision
    if (bAltPressed) bTargetRight = !bTargetRight;

    FInventorySlot& TargetHand = bTargetRight ? RightHand : LeftHand;

    // If the target hand is full, try to stow the current item first
    if (!TargetHand.IsEmpty())
    {
        // This call uses the strict guardrails we defined in TryStowItem
        if (TryStowItem(TargetHand.ContainedItem))
        {
            TargetHand.ContainedItem = nullptr;
        }
        else
        {
            // If stow fails, the hand is cleared here so the new item can be picked up.
            // Note: The physical drop logic should be handled in BP before this call.
            TargetHand.ContainedItem = nullptr;
        }
    }

    TargetHand.ContainedItem = NewItem;
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::QuickStowToBackpack(UBackpackComponent* ActiveBackpack)
{
    if (!ActiveBackpack) return;

    // Find which hand ISN'T holding the backpack
    FInventorySlot* ItemSlot = nullptr;

    if (RightHand.ContainedItem && RightHand.ContainedItem->ItemType == EItemType::Backpack)
    {
        ItemSlot = &LeftHand;
    }
    else if (LeftHand.ContainedItem && LeftHand.ContainedItem->ItemType == EItemType::Backpack)
    {
        ItemSlot = &RightHand;
    }

    if (ItemSlot && !ItemSlot->IsEmpty())
    {
        if (ActiveBackpack->TryAddItem(ItemSlot->ContainedItem))
        {
            ItemSlot->ContainedItem = nullptr;
            OnInventoryUpdated.Broadcast();
        }
    }
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

void UInventoryComponent::SwapHandWithBelt(int32 BeltIndex, bool bAltPressed)
{
    if (!ToolbeltSlots.IsValidIndex(BeltIndex)) return;

    // Logic: No Alt = Left Hand, Alt = Right Hand
    FInventorySlot& TargetHand = bAltPressed ? RightHand : LeftHand;
    
    // GUARDRAIL: Block weapons and backpacks from entering the toolbelt
    if (TargetHand.ContainedItem)
    {
        EItemType Type = TargetHand.ContainedItem->ItemType;
        if (Type == EItemType::Weapon || Type == EItemType::Backpack)
        {
            return; // Prevent swap if hand contains weapon or backpack
        }
    }

    // Safety check for the item currently in the belt (should never be a weapon/backpack anyway)
    if (ToolbeltSlots[BeltIndex].ContainedItem)
    {
        EItemType BeltType = ToolbeltSlots[BeltIndex].ContainedItem->ItemType;
        if (BeltType == EItemType::Weapon || BeltType == EItemType::Backpack)
        {
            return; // Prevent swap if belt contains weapon or backpack
        }
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