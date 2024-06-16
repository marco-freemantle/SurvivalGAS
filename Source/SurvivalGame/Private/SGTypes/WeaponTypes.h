#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_1HSword UMETA(DisplayName = "1HSword"),
	EWT_2HSword UMETA(DisplayName = "2HSword"),
	EWT_Shield UMETA(DisplayName = "Shield"),
	EWT_MAX UMETA(DisplayName = "DefaultMax")
};