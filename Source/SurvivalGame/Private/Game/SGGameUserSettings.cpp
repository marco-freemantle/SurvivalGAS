// Copyright Marco Freemantle

#include "Game/SGGameUserSettings.h"

USGGameUserSettings::USGGameUserSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MasterVolume = 1.f;
	MusicVolume = 1.f;
	SoundEffectsVolume = 1.f;
	MouseSensitivity = 1.f;
}

void USGGameUserSettings::SetMasterVolume(float NewValue)
{
	MasterVolume = NewValue;
}

float USGGameUserSettings::GetMasterVolume() const
{
	return MasterVolume;
}

float USGGameUserSettings::GetMouseSensitivity() const
{
	return MouseSensitivity;
}

USGGameUserSettings* USGGameUserSettings::GetVBGameUserSettings()
{
	return Cast<USGGameUserSettings>(GetGameUserSettings());
}