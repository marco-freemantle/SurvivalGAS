#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Sword UMETA(DisplayName = "Sword"),
	EWT_Shield UMETA(DisplayName = "Shield"),
	EWT_MAX UMETA(DisplayName = "DefaultMax")
};