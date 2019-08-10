
#include "NotificationSound.h"

NotificationSound::NotificationSound(
	const std::wstring &startSoundPath,
	const std::wstring &stopSoundPath)
{
	pStartSound = std::make_unique<wxSound>(startSoundPath);
	if( !pStartSound->IsOk() ) {
		pStartSound = nullptr;
	}

	pStopSound = std::make_unique<wxSound>(stopSoundPath);
	if( !pStopSound->IsOk() ) {
		pStopSound = nullptr;
	}
}

void NotificationSound::PlayStartSound() {
	if( pStartSound ) pStartSound->Play();
}

void NotificationSound::PlayStopSound() {
	if( pStopSound ) pStopSound->Play();
}
