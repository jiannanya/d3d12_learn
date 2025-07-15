
#include "texture_helper.hxx"
#include <cstdint>
#include <spdlog/spdlog.h>

struct WICConvert
{
	GUID source;
	GUID target;
	DXGI_FORMAT format;
};

static WICConvert g_WICConvert[] =
{
	
	{ GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM}, // 

	{ GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM}, // 
	{ GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM}, // 
	{ GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM}, // 
	{ GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM}, // 

	{ GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM}, // 
	{ GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM}, // 

	{ GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT}, // 
	{ GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat,DXGI_FORMAT_R32_FLOAT }, // 

	{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM}, // 

	{ GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102,DXGI_FORMAT_R10G10B10A2_UNORM }, // 

	{ GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA ,DXGI_FORMAT_R8G8B8A8_UNORM}, // 
	{ GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA,DXGI_FORMAT_R8G8B8A8_UNORM }, // 
	{ GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA ,DXGI_FORMAT_R8G8B8A8_UNORM}, // 
	{ GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA ,DXGI_FORMAT_R8G8B8A8_UNORM}, // 
	{GUID_WICPixelFormat32bppBGRA,  GUID_WICPixelFormat32bppBGRA,DXGI_FORMAT_B8G8R8A8_UNORM},
	{ GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 
	{ GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 
	{ GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 
	{ GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 
	{ GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 

	{ GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT}, // 
	{ GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT}, // 
	{ GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT}, // 
	{ GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT}, // 
	{ GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT}, // 
	{ GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT}, // 
	{ GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT}, // 

	{ GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT}, 
	{ GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT}, 
	{ GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT}, // 
	{ GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT}, // 
	{ GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT}, // 

	{ GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM}, // 
	{ GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 
	{ GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 
	{ GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 

	{ GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM}, // 
	{ GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM}, // 
	{ GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT }, // 
	

};


DXGI_FORMAT CWICIamge::GetImageInfo(const wchar_t *pathImage, UINT& w,UINT& h,UINT& bpp, UINT& nPicRowPitch, BYTE**pbPicData)
{
	ComPtr<IWICImagingFactory>			pIWICFactory;
	ComPtr<IWICBitmapDecoder>			pIWICDecoder;
	ComPtr<IWICBitmapFrameDecode>		pIWICFrame;
	ComPtr<IWICBitmapSource> pIBMP;

	WICPixelFormatGUID wpf = {};
	GUID tgFormat = {};

	WicLoadImage(pathImage, pIWICFactory, pIWICDecoder);
	GetImagePixelFormat(pIWICDecoder, pIWICFrame, wpf);
	
	DXGI_FORMAT stTextureFormat = DXGI_FORMAT_UNKNOWN;

	GetTargetPixelFormat(&wpf, &tgFormat, stTextureFormat);
	
	if (DXGI_FORMAT_UNKNOWN == stTextureFormat)
	{
		throw;
	}

	GetBitmapSource(wpf, tgFormat, pIWICFactory, pIWICFrame, pIBMP);
	
	GetBitmapInfo(tgFormat,pIWICFactory, pIBMP,w,h,bpp);

    spdlog::info("Image info: w = {}, h = {}, bpp = {}, format = {}", w, h, bpp, (int)stTextureFormat);

	nPicRowPitch = (uint64_t(w) * uint64_t(bpp) + 8u - 1) / 8u;
	*pbPicData = new BYTE[nPicRowPitch * h];
	memset(*pbPicData, 0, nPicRowPitch * h);

	pIBMP->CopyPixels(nullptr
		, nPicRowPitch
		, static_cast<UINT>(nPicRowPitch * h)  
		, reinterpret_cast<BYTE*>(*pbPicData));


	return stTextureFormat;

}
void CWICIamge::WicLoadImage(const wchar_t * pathImage, ComPtr<IWICImagingFactory>& pIWICFactory, ComPtr<IWICBitmapDecoder>& pIWICDecoder)
{
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory));
    
	pIWICFactory->CreateDecoderFromFilename(
		pathImage,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&pIWICDecoder
	);

    spdlog::info("WIC Imaging load end.");
}
void  CWICIamge::GetImagePixelFormat(ComPtr<IWICBitmapDecoder> pIWICDecoder, ComPtr<IWICBitmapFrameDecode>&pIWICFrame, WICPixelFormatGUID& wpf)
{
	pIWICDecoder->GetFrame(0, &pIWICFrame);
	pIWICFrame->GetPixelFormat(&wpf);
}

void CWICIamge::GetBitmapSource(WICPixelFormatGUID wpf, GUID tgFormat, ComPtr<IWICImagingFactory>pIWICFactory, ComPtr<IWICBitmapFrameDecode>pIWICFrame,ComPtr<IWICBitmapSource>& pIBMP)
{
	if (!InlineIsEqualGUID(wpf, tgFormat))
	{
		ComPtr<IWICFormatConverter> pIConverter;
		pIWICFactory->CreateFormatConverter(&pIConverter);

		pIConverter->Initialize(
			pIWICFrame.Get(),
			tgFormat,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeCustom
		);
		pIConverter.As(&pIBMP);

	}
	else
	{
		pIWICFrame.As(&pIBMP);
	}

}
void CWICIamge::GetBitmapInfo(GUID tgFormat,ComPtr<IWICImagingFactory> pIWICFactory, ComPtr<IWICBitmapSource> pIBMP,UINT& w,UINT& h, UINT& bpp)
{

	pIBMP->GetSize(&w, &h);

	ComPtr<IWICComponentInfo> pIWICmntinfo;
	pIWICFactory->CreateComponentInfo(tgFormat, pIWICmntinfo.GetAddressOf());

	WICComponentType type;
	pIWICmntinfo->GetComponentType(&type);

	if (type != WICPixelFormat)
	{
		throw;
	}

	ComPtr<IWICPixelFormatInfo> pIWICPixelinfo;
	pIWICmntinfo.As(&pIWICPixelinfo);
	pIWICPixelinfo->GetBitsPerPixel(&bpp);

}
bool  CWICIamge::GetTargetPixelFormat(const GUID* pSourceFormat, GUID* pTargetFormat, DXGI_FORMAT& dxForma)
{
	*pTargetFormat = *pSourceFormat;
	for (size_t i = 0; i < _countof(g_WICConvert); ++i)
	{
		if (InlineIsEqualGUID(g_WICConvert[i].source, *pSourceFormat))
		{
			*pTargetFormat = g_WICConvert[i].target;
			dxForma = g_WICConvert[i].format;
			return true;
		}
	}
	return false;
}

