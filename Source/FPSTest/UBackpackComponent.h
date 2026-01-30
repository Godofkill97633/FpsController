#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UItemData.h"
#include "UBackpackComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPSTEST_API UBackpackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBackpackComponent();

    /** The largest size of a single item that can fit (e.g. 2) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Backpack|Limits")
    int32 MaxItemSize;

    /** Total weight capacity in KG */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Backpack|Limits")
    float MaxWeightCapacity;

    /** The list of items currently inside the backpack */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Backpack|Storage")
    TArray<UItemData*> StoredItems;

    /** Returns true if the item fits both size and weight constraints */
    UFUNCTION(BlueprintPure, Category = "Backpack")
    bool CanFitItem(UItemData* ItemData) const;

    /** Adds item to backpack if it fits */
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    bool TryAddItem(UItemData* ItemData);

    /** Calculates current total weight of all items */
    UFUNCTION(BlueprintPure, Category = "Backpack")
    float GetCurrentWeight() const;
};