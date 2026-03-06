#include "BaseItem.h"

// This file can remain empty of logic for now. 
// Its primary purpose is to tell the compiler that ABaseItem is a real class
// that should be registered with the Blueprint system. 

ABaseItem::ABaseItem()
{
    // Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    // Initialize the root component so the item has a physical presence
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}
