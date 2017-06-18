
#include "CaptureBase.h"
#include <assert.h>

// キャプチャに必要なバッファの大きさを計算
unsigned int CaptureBase::CalcNecessaryBufferLength(const RECT *region) const {
	assert(region != nullptr);
	int capturedWidth = region->right - region->left;
	int capturedHeight = region->bottom - region->top;
	int necessaryBufLen = capturedWidth * capturedHeight * 3;
	return necessaryBufLen;
}

