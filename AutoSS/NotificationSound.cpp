
#include "NotificationSound.h"

NotificationSound::NotificationSound() {
	pStartSound = std::make_unique<wxSound>("notif_start.wav");
	pStopSound = std::make_unique<wxSound>("notif_stop.wav");
}

void NotificationSound::PlayStartSound() {
	pStartSound->Play();
}

void NotificationSound::PlayStopSound() {
	pStopSound->Play();
}
