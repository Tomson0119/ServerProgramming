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
	ThrowIfFailed(CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(pFactory.GetAddressOf())));

	ComPtr<IWICBitmapDecoder> pDecoder;
	ThrowIfFailed(pFactory->CreateDecoderFromFilename(
		filename.c_str(), NULL, GENERIC_READ,
		WICDecodeMetadataCacheOnDemand, pDecoder.GetAddressOf()));

	ComPtr<IWICBitmapFrameDecode> pFrame;
	ThrowIfFailed(pDecoder->GetFrame(0, pFrame.GetAddressOf()));

	ComPtr<IWICFormatConverter> pConverter;
	ThrowIfFailed(pFactory->CreateFormatConverter(pConverter.GetAddressOf()));
	ThrowIfFailed(pConverter->Initialize(pFrame.Get(), GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom));
	
	ThrowIfFailed(rt->CreateBitmapFromWicBitmap(
		pConverter.Get(), NULL, mBitmap.GetAddressOf()));

	return true;
}
