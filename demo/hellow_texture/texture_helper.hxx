#pragma once
#ifndef TEXTURE_HELPER_HXX
#define TEXTURE_HELPER_HXX
#include <wincodec.h>
#include <wrl.h>


using Microsoft::WRL::ComPtr;

class CWICIamge
{
public:
	DXGI_FORMAT GetImageInfo(const wchar_t *pathImage, UINT& w, UINT& h, UINT& bpp, UINT& nPicRowPitch, BYTE**pbPicData);

private:
	void WicLoadImage(const wchar_t * pathImage,ComPtr<IWICImagingFactory>& pIWICFactory, ComPtr<IWICBitmapDecoder>& pIWICDecoder);
	void GetImagePixelFormat(ComPtr<IWICBitmapDecoder> pIWICDecoder,ComPtr<IWICBitmapFrameDecode>&pIWICFrame, WICPixelFormatGUID& wpf);
	void GetBitmapSource(WICPixelFormatGUID wpf, GUID tgFormat, ComPtr<IWICImagingFactory>pIWICFactory, ComPtr<IWICBitmapFrameDecode>pIWICFrame, ComPtr<IWICBitmapSource>& pIBMP);
	void GetBitmapInfo(GUID tgFormat,ComPtr<IWICImagingFactory> pIWICFactory, ComPtr<IWICBitmapSource> pIBMP, UINT& w, UINT& h, UINT& bpp);
	
	bool GetTargetPixelFormat(const GUID* pSourceFormat, GUID* pTargetFormat, DXGI_FORMAT& dxForma);
};

#endif // TEXTURE_HELPER_HXX

