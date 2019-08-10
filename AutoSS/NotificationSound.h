#pragma once

#include <memory>
#include <wx/sound.h>

/**
 * 撮影開始時や終了時の通知音再生クラス
*/
class NotificationSound {
public:

	NotificationSound();
	~NotificationSound() = default;

	void PlayStartSound();
	void PlayStopSound();

private:
	std::unique_ptr<wxSound> pStartSound;
	std::unique_ptr<wxSound> pStopSound;
};

