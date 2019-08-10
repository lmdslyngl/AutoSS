#pragma once

#include <wx/wx.h>
#include <memory>
#include <functional>
#include "Config.h"

/*
 * 範囲選択ウィンドウプロトタイプ
*/
class RegionSelectWindow;

/*
 * 設定ダイアログ
*/
class ConfigFrame : public wxDialog {
public:

	ConfigFrame(wxFrame *pParent, const std::shared_ptr<Config> &pInitConf);

	// 設定ダイアログが閉じられた状態を取得
	// true: OK
	// false: Cancel
	bool GetCloseState() const {
		return CloseState;
	}

	std::shared_ptr<Config> GetConfig() const;

private:
	void OnSavePathRef(wxCommandEvent &ev);

	void OnPlayNotifChanged(wxCommandEvent &ex);
	void OnStartSoundRef(wxCommandEvent &ex);
	void OnStartSoundPlay(wxCommandEvent &ex);
	void OnStopSoundRef(wxCommandEvent &ex);
	void OnStopSoundPlay(wxCommandEvent &ex);

	void OnOK(wxCommandEvent &ev);
	void OnCancel(wxCommandEvent &ev);

	void OnRegisterHotkey(wxCommandEvent &ev);
	void OnKeyDown(wxKeyEvent &ev);

	void OnRegionComboChanged(wxCommandEvent &ev);
	void OnRegionSelect(wxCommandEvent &ev);
	void OnRegionSelectFinished();

	void OnClose(wxCloseEvent &ev);

	// 撮影範囲コンボボックスによって選択範囲の有効/無効を更新する
	void UpdateRegionSelectionEnabling();

	// 通知音チェックボックスによって通知村選択の有効/無効を更新する
	void UpdateNotifSoundEnabling();

	wxPanel *CreateRegionSelectPanel(
		const std::shared_ptr<Config> &pInitConf);

	// 設定の検証
	bool ValidateConfig();

private:
	wxTextCtrl *pSavePathText;
	wxButton *pSavePathRefButton;
	wxTextCtrl *pWaitTimeText;
	wxComboBox *pCaptureCombo;
	wxComboBox *pRegionCombo;
	wxCheckBox *pIncludeBorderCheck;
	wxPanel *pRegionSelectPanel;
	wxTextCtrl *pRegionXText;
	wxTextCtrl *pRegionYText;
	wxTextCtrl *pRegionWidthText;
	wxTextCtrl *pRegionHeightText;
	wxButton *pRegionSelectBtn;
	wxTextCtrl *pMaxCapCountText;
	wxButton *pHotkeyRegButton;
	wxTextCtrl *pHotkeyText;
	bool RegisteringHotkey;
	int HotkeyCode, HotkeyCodeRaw, HotkeyMod;
	wxCheckBox *pPlayNotifSoundCheck;
	wxTextCtrl *pStartNotifSoundText;
	wxButton *pStartNotifSoundRefButton;
	wxButton *pStartNotifSoundPlayButton;
	wxTextCtrl *pStopNotifSoundText;
	wxButton *pStopNotifSoundRefButton;
	wxButton *pStopNotifSoundPlayButton;
	wxButton *pOKButton, *pCancelButton;
	bool CloseState;
	RegionSelectWindow *pRgnSelWnd;
};

