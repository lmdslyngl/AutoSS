#pragma once

#include <wx/wx.h>
#include <wx/graphics.h>

#include <functional>
#include <memory>
#include "Config.h"

class ConfigFrame;
class RegionSelectWindow;

/*
 * AutoSSウィンドウ
*/
class AutoSSFrame : public wxFrame {
public:
	
	AutoSSFrame(const std::shared_ptr<Config> &pConf);
	
	void SetOnStartFunc(std::function<void()> func) {
		this->OnStartFunc = func;
	}
	void SetOnStopFunc(std::function<void()> func) {
		this->OnStopFunc = func;
	}
	void SetOnChangeConfFunc(std::function<void(const std::shared_ptr<Config>&)> func) {
		this->OnChangeConfFunc = func;
	}
	
	void EnableCapture() {
		IsEnableCapture = true;
		pStartBtn->Enable();
	}
	
	void DisableCapture() {
		IsEnableCapture = false;
		pStartBtn->Disable();
	}
	
	void Start();
	void Stop();
	
private:
	void OnConf(wxCommandEvent &ev);
	
	void OnHotkey(wxKeyEvent &ev);
	void OnStart(wxCommandEvent &ev);
	void OnStartImpl();
	
private:
	ConfigFrame *pConfigFrame;
	bool TakingSS;
	
	wxAppProgressIndicator *pProgressIndicator;
	wxButton *pStartBtn;
	bool IsEnableCapture;
	
	std::function<void()> OnStartFunc;
	std::function<void()> OnStopFunc;
	std::function<void(const std::shared_ptr<Config>&)> OnChangeConfFunc;
	
	std::shared_ptr<Config> pConf;
	
	enum {
		HOTKEY_ID_START
	};
	
};


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


/*
 * 範囲選択ウィンドウ
*/
class RegionSelectWindow : public wxFrame {
public:
	
	RegionSelectWindow();
	virtual ~RegionSelectWindow();
	
	void SetRegion(int startX, int startY, int endX, int endY) {
		RegionStartX = startX;
		RegionStartY = startY;
		RegionEndX = endX;
		RegionEndY = endY;
	}
	
	void GetRegion(int *startX, int *startY, int *endX, int *endY) {
		*startX = RegionStartX;
		*startY = RegionStartY;
		*endX = RegionEndX;
		*endY = RegionEndY;
	}
	
	// 領域が決定したときのコールバック
	void SetRegionFinishedCallback(std::function<void()> func) {
		this->RegionFinishedCallbackFunc = func;
	}
	
	// デスクトップの画像を更新
	void UpdateScreenShot();
	
	// キャンセルされたかどうか
	bool IsCanceled() const {
		return IsCanceledFlag;
	}
	
private:
	
	void OnPaint(wxPaintEvent &ev);
	
	void OnMousePressed(wxMouseEvent &ev);
	void OnMouseReleased(wxMouseEvent &ev);
	void OnMouseMoved(wxMouseEvent &ev);
	void OnMouseCanceled(wxMouseEvent &ev);
	void OnFocusLoad(wxFocusEvent &ev);
	
	// 決定
	void Decide();
	
	// キャンセル
	void Cancel();
	
	// 選択範囲の説明を描画
	void DrawInstruction(wxDC &dc);
	
	// 選択範囲を描画
	void DrawRegion(wxDC &dc);
	
	// デスクトップの画像を取得
	void CaptureDesktop(wxImage *pOutImage);
	
private:
	std::unique_ptr<wxBitmap> pDesktopBitmap;
	unsigned char *pBitmapBuffer;
	size_t BitmapBufferLen;
	std::unique_ptr<wxBitmap> pBackbufferBitmap;
	
	int RegionStartX, RegionStartY;
	int RegionEndX, RegionEndY;
	bool IsMousePressingFlag;
	
	int DesktopWidth, DesktopHeight;
	
	std::function<void()> RegionFinishedCallbackFunc;
	bool IsCanceledFlag;
	
};


