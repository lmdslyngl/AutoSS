
#include "NotificationSound.h"

NotificationSound::NotificationSound() {
	pStartSound = std::make_unique<wxSound>("notif_start.wav");
	if( !pStartSound->IsOk() ) {
		pStartSound = nullptr;
	}

	pStopSound = std::make_unique<wxSound>("notif_stop.wav");
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
