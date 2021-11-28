#pragma once

class BitmapLoader
{
public:
	BitmapLoader();
	~BitmapLoader();

	bool DecodeBitmap(ID2D1HwndRenderTarget* rt, const std::wstring& filename);
	ID2D1Bitmap* GetBitmapResource() const { return mBitmap.Get(); }

private:
	ComPtr<IWICImagingFactory> mWICFactory;
	ComPtr<ID2D1Bitmap> mBitmap;
};