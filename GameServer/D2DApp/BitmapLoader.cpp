#include "stdafx.h"
#include "BitmapLoader.h"

BitmapLoader::BitmapLoader()
{
}

BitmapLoader::~BitmapLoader()
{
}

bool BitmapLoader::DecodeBitmap(ID2D1HwndRenderTarget* rt, const std::wstring& filename)
{
	ComPtr<IWICImagingFactory> pFactory;
	if(FAILED(CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(pFactory.GetAddressOf())))) return false;

	ComPtr<IWICBitmapDecoder> pDecoder;
	HRESULT hr = pFactory->CreateDecoderFromFilename(
		filename.c_str(), NULL,	GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,	pDecoder.GetAddressOf());
	if (FAILED(hr)) return false;

	ComPtr<IWICBitmapFrameDecode> pFrame;
	if (FAILED(pDecoder->GetFrame(0, pFrame.GetAddressOf())))	return false;

	ComPtr<IWICFormatConverter> pConverter;
	if (FAILED(pFactory->CreateFormatConverter(pConverter.GetAddressOf()))) return false;
	if (FAILED(pConverter->Initialize(pFrame.Get(), GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom))) return false;
	
	if(FAILED(rt->CreateBitmapFromWicBitmap(
		pConverter.Get(), NULL, mBitmap.GetAddressOf()))) return false;

	return true;
}
