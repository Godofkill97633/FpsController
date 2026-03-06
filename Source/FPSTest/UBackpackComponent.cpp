#include "UBackpackComponent.h"

UBackpackComponent::UBackpackComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxItemSize = 2;
    MaxWeightCapacity = 20.0f;
}

float UBackpackComponent::GetCurrentWeight() const
{
    float Total = 0.0f;
    for (UItemData* Item : StoredItems)
    {
        if (Item) Total += Item->Weight;
    }
    return Total;
}

bool UBackpackComponent::CanFitItem(UItemData* ItemData) const
{
    if (!ItemData) return false;

    // Check Size
    if (ItemData->ItemSize > MaxItemSize) return false;

    // Check Weight
    if ((GetCurrentWeight() + ItemData->Weight) > MaxWeightCapacity) return false;

    return true;
}

bool UBackpackComponent::TryAddItem(UItemData* ItemData)
{
    if (CanFitItem(ItemData))
    {
        StoredItems.Add(ItemData);
        return true;
    }
    return false;
}