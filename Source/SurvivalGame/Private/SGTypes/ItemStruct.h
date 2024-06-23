#pragma once

#include "CoreMinimal.h"
#include "ItemStruct.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FItemStruct : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UTexture2D* Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 StackSize;
};