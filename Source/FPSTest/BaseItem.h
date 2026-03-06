#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UItemData.h"
#include "BaseItem.generated.h"

/**
 * ABaseItem is the physical representation of an item in the game world.
 * This is the actor you perform Line Traces against in your FirstPersonCharacter.
 *
 * The BlueprintType and Blueprintable tags ensure that "Cast to BaseItem" 
 * appears in your Blueprint search menu.
 */
UCLASS(BlueprintType, Blueprintable)
class FPSTEST_API ABaseItem : public AActor
{
    GENERATED_BODY()

public:
    // Constructor
    ABaseItem();

    /** * The Data Asset containing this item's identity. 
     * In your Blueprint, you will drag off the 'As Base Item' pin and call 
     * 'Get Item Data' to feed your Select node.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    class UItemData* ItemData;
};
