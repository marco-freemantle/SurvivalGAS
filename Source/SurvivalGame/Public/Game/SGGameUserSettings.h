// Copyright Marco Freemantle

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "SGGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API USGGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetMasterVolume(float NewValue);

	UFUNCTION(BlueprintCallable)
	float GetMasterVolume() const;

	UFUNCTION(BlueprintCallable)
	float GetMouseSensitivity() const;

	UFUNCTION(BlueprintPure)
	static USGGameUserSettings* GetVBGameUserSettings();

	UPROPERTY(Config, BlueprintReadWrite)
	float MasterVolume;

	UPROPERTY(Config, BlueprintReadWrite)
	float MusicVolume;

	UPROPERTY(Config, BlueprintReadWrite)
	float SoundEffectsVolume;

	UPROPERTY(Config, BlueprintReadWrite)
	float MouseSensitivity;
	
};
