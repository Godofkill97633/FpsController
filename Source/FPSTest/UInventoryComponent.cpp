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

void UInventoryComponent::NavigateBackpackGrid(int32 Delta, int32 Columns, bool bIs2D, int32 MaxItems)
{
    if (MaxItems <= 0)
    {
        BackpackSelectedIndex = 0;
        return;
    }

    if (bIs2D)
    {
        // 2D Grid navigation (Arrows)
        // Logic handles Clamping to prevent going out of bounds
        BackpackSelectedIndex = FMath::Clamp(BackpackSelectedIndex + Delta, 0, MaxItems - 1);
    }
    else
    {
        // 1D Linear navigation (Scroll Wheel)
        // Logic handles Wrapping for a smoother feel
        BackpackSelectedIndex += Delta;
        if (BackpackSelectedIndex >= MaxItems) BackpackSelectedIndex = 0;
        if (BackpackSelectedIndex < 0) BackpackSelectedIndex = MaxItems - 1;
    }

    // Notify HUD to update the selection highlight
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::HandleQuickAction(UBackpackComponent* BackpackComp)
{
    if (!BackpackComp) return;

    // 1. Identify which hand is NOT holding the backpack
    FInventorySlot* OtherHand = nullptr;

    if (LeftHand.ContainedItem && LeftHand.ContainedItem->ItemType == EItemType::Backpack)
    {
        OtherHand = &RightHand;
    }
    else if (RightHand.ContainedItem && RightHand.ContainedItem->ItemType == EItemType::Backpack)
    {
        OtherHand = &LeftHand;
    }

    // Safety check: Ensure we found a free hand to interact with
    if (!OtherHand) return;

    if (!OtherHand->IsEmpty())
    {
        // QUICK STOW: Move item from free hand into the backpack
        if (BackpackComp->TryAddItem(OtherHand->ContainedItem))
        {
            OtherHand->ContainedItem = nullptr;
        }
    }
    else
    {
        // QUICK UNSTOW: Pull the currently selected item out of the backpack into hand
        if (BackpackComp->StoredItems.IsValidIndex(BackpackSelectedIndex))
        {
            OtherHand->ContainedItem = BackpackComp->StoredItems[BackpackSelectedIndex];

            // Remove from the bag array
            BackpackComp->StoredItems.RemoveAt(BackpackSelectedIndex);

            // Adjust selection index if we removed the last item in the list
            if (BackpackSelectedIndex >= BackpackComp->StoredItems.Num() && BackpackSelectedIndex > 0)
            {
                BackpackSelectedIndex--;
            }
        }
    }

    // Refresh visuals and HUD
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

    // Rule 2: Weapons to Holsters
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

    // Rule 3: Tools/Consumables to Belt
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

    // Determine target hand based on Alt modifier
    FInventorySlot& TargetHand = bAltPressed ? RightHand : LeftHand;

    if (!TargetHand.IsEmpty())
    {
        EItemType Type = TargetHand.ContainedItem->ItemType;

        // ISSUE FIX: If holding a Weapon or Backpack, we can't put it in a belt slot.
        // We now attempt to auto-stow it to a holster/spine to make room for the belt item.
        if (Type == EItemType::Weapon || Type == EItemType::Backpack)
        {
            if (TryStowItem(TargetHand.ContainedItem))
            {
                TargetHand.ContainedItem = nullptr;
                // Success: Slot is now empty, proceeding to pull from belt.
            }
            else
            {
                // Fail: No room in holster/spine, so we cannot clear the hand to pull the belt item.
                return;
            }
        }
    }

    // Standard swap: handles tool-to-tool or empty-to-tool
    UItemData* Temp = TargetHand.ContainedItem;
    TargetHand.ContainedItem = ToolbeltSlots[BeltIndex].ContainedItem;
    ToolbeltSlots[BeltIndex].ContainedItem = Temp;

    // Trigger UI refresh
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