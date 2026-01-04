// Fill out your copyright notice in the Description page of Project Settings.

#include "UInventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ToolbeltSlots.SetNum(4); // Start with 4 slots
}

void UInventoryComponent::TryPickupItem(UItemData* NewItem)
{
    if (!NewItem) return;

    // RULE: Favor Right Hand first
    if (RightHand.IsEmpty())
    {
        RightHand.ContainedItem = NewItem;
        return;
    }

    // RULE: If Right Hand is full, try Left Hand
    if (LeftHand.IsEmpty())
    {
        LeftHand.ContainedItem = NewItem;
        return;
    }

    // RULE: If both hands full, try to stow Right Hand to belt
    int32 EmptyBeltIndex = GetFirstEmptyBeltSlot();
    if (EmptyBeltIndex != INDEX_NONE)
    {
        ToolbeltSlots[EmptyBeltIndex].ContainedItem = RightHand.ContainedItem;
        RightHand.ContainedItem = NewItem;
        return;
    }

    // RULE: If belt is also full, drop Right Hand item and pick up new
    // Note: In Blueprints, you will trigger an 'OnItemDropped' event to spawn the physical actor
    RightHand.ContainedItem = NewItem;
}

void UInventoryComponent::SwapHandWithBelt(int32 BeltIndex)
{
    if (!ToolbeltSlots.IsValidIndex(BeltIndex)) return;

    // Perform the physical swap of data
    UItemData* ItemInHand = RightHand.ContainedItem;
    RightHand.ContainedItem = ToolbeltSlots[BeltIndex].ContainedItem;
    ToolbeltSlots[BeltIndex].ContainedItem = ItemInHand;

    // Logic: If we pulled out a weapon, it should favor Right Hand
    // (Additional logic for hand priority can be added here)
}

int32 UInventoryComponent::GetFirstEmptyBeltSlot() const
{
    for (int32 i = 0; i < ToolbeltSlots.Num(); i++)
    {
        if (ToolbeltSlots[i].IsEmpty()) return i;
    }
    return INDEX_NONE;
}

void UInventoryComponent::ToggleHolster(bool bIsPrimary)
{
    FInventorySlot& TargetHolster = bIsPrimary ? PrimaryHolster : SidearmHolster;

    UItemData* Temp = RightHand.ContainedItem;
    RightHand.ContainedItem = TargetHolster.ContainedItem;
    TargetHolster.ContainedItem = Temp;
}