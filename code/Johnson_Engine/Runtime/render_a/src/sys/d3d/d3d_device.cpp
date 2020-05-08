// D3D_Device.h
//	Contains the active D3D Device. CD3D_Shell & CD3D_Device are the main containers of D3D shtuff...

#include "precompile.h"

#include "d3d_device.h"
#include "d3d_convar.h"
#include "d3d_shell.h"
#include "common_stuff.h"
#include "d3d_utils.h"
#include "renderstruct.h"
#include "3d_ops.h"
#include "d3d_texture.h"
#include "d3dmeshrendobj_rigid.h"
#include "d3dmeshrendobj_skel.h"
#include "d3dmeshrendobj_vertanim.h"
#include "debuggeometry.h"
#include "d3d_renderstatemgr.h"
#include "ltpixelshadermgr.h"
#include "ltvertexshadermgr.h"
#include "d3d_renderworld.h"
#include "screenglowmgr.h"
#include "d3d_draw.h"

#include "GameUtil.h"

extern uint32 g_CV_ValidWndMode;

// The global D3D Shell...
CD3D_Device g_Device;

// Create the Sucka...
bool CD3D_Device::CreateDevice(D3DAdapterInfo* pAdapter,D3DDeviceInfo* pDevice,D3DModeInfo* pMode)
{
	HRESULT hResult;

	FreeDevice();								// Make sure it's all released and groovie...

	m_bRestoreState = false;
	m_bReleaseState = false;
	
	m_pAdapter	= pAdapter;						// Initialization...
	m_pDevice	= pDevice;
	m_pMode		= pMode;

	MursumDX_Create( g_hWnd );
	
	if( false == g_bRunWindowed )
	{
		int nBPP = 0;
		switch (pMode->Format) 
		{
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8R8G8B8:
		case D3DFMT_R8G8B8:
			nBPP = 32;
			break;

		case D3DFMT_R5G6B5:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A4R4G4B4:
			nBPP = 16;
			break;

		case D3DFMT_P8 :
			nBPP = 8;
			break;
		}
		
		if( g_CV_ValidWndMode )
		{
			if( LTFALSE == MursumDX_SetDisplayMode( pMode->Width, pMode->Height, nBPP ) ) 
			{
				g_pStruct->ConsolePrint("[DX_INIT ERROR] MursumDX_SetDisplayMode(%d*%d*%d)", pMode->Width, pMode->Height, nBPP );
				return false;
			}
		}
	}

#ifdef _FINAL
	SetWindowPos(g_hWnd, HWND_TOP, 0, 0, pMode->Width, pMode->Height, SWP_NOMOVE);
#endif

	// Create the sucka...
 	uint32 BehaviorFlags = D3DCREATE_MULTITHREADED;

	if (pDevice->d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		BehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
	else
		BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	D3DPRESENT_PARAMETERS PresentationParam;
	SetPresentationParams(PresentationParam,pMode);
	IDirect3DDevice8 *pD3DDevice;
	hResult = PDIRECT3D->CreateDevice(pAdapter->iAdapterNum,pDevice->DeviceType,g_hWnd,BehaviorFlags,&PresentationParam,&pD3DDevice);
	if ((hResult != D3D_OK) || !pD3DDevice)
	{
		// Give it more more try - Presentation params might have been changed by D3D to something acceptable...
		OUTPUT_D3D_ERROR(1,hResult);			// Report the error...
		OutputDebugString("Warning: Create failed. Attempting to fall back...\n");
		TryFallingBack_OnFailedDevCreate(&PresentationParam);
		hResult = PDIRECT3D->CreateDevice(pAdapter->iAdapterNum,pDevice->DeviceType,g_hWnd,BehaviorFlags,&PresentationParam,&pD3DDevice);
		if ((hResult != D3D_OK) || !pD3DDevice)
		{
			g_pStruct->ConsolePrint("[DX_INIT ERROR] PDIRECT3D->CreateDevice hr(0x%08x)", hResult );
			OUTPUT_D3D_ERROR(0,hResult);
			FreeDevice();
			return false;
		}
	}

	ShowCursor(false);

	// Create our wrapper, which we're going to present to the world as an actual D3D device
	LT_MEM_TRACK_ALLOC(m_pD3DDevice = new CDirect3DDevice8Wrapper,LT_MEM_TYPE_RENDERER);
	m_pD3DDevice->SetDevice(pD3DDevice);

	if (FAILED( hResult = m_pD3DDevice->GetDeviceCaps(&m_DeviceCaps)))
	{
		g_pStruct->ConsolePrint("[DX_INIT ERROR] m_pD3DDevice->GetDeviceCaps hr(0x%08x)", hResult );
		FreeDevice();
		return false;
	}

	SetDefaultRenderStates();					// Set the default render states...
	PreCalcSomeDeviceCaps();					// Do some precalcing of device caps (figure out if we can do some puff)...

	// Display a warning message
	if (m_pDevice->DeviceType == D3DDEVTYPE_REF)
		AddDebugMessage(1,"Warning: Couldn\'t find any HAL devices, Using reference rasterizer");

	//make sure to clear out the back buffer so it isn't filled with left over garbage
	LTRGBColor ClearColor;
	ClearColor.dwordVal = 0;

	d3d_Clear(NULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER, ClearColor);

	return true;
}

void CD3D_Device::TryFallingBack_OnFailedDevCreate(D3DPRESENT_PARAMETERS* pPresentationParam)
{
	// Try forcing the ZBitDepth to be the same as the screen bitdepth...
	uint32 iBitCount, iAlphaMask, iRedMask, iGreenMask, iBlueMask;
	d3d_GetColorMasks(pPresentationParam->BackBufferFormat,iBitCount,iAlphaMask,iRedMask,iGreenMask,iBlueMask);
	pPresentationParam->AutoDepthStencilFormat = GetDefaultDepthStencilFormat(iBitCount,g_CV_StencilBitDepth);
}

bool CD3D_Device::SetMode(D3DModeInfo* pMode)
{
	D3DPRESENT_PARAMETERS PresentationParam;

	if( false == g_bRunWindowed )
	{
		int nBPP = 16;
		switch (pMode->Format) 
		{
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8R8G8B8:
			nBPP = 32;
			break;

		case D3DFMT_R5G6B5:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A4R4G4B4:
			nBPP = 16;
			break;

		case D3DFMT_P8 :
			nBPP = 8;
			break;
		}

		if( g_CV_ValidWndMode )
		{
			if( LTFALSE == MursumDX_SetDisplayMode( pMode->Width, pMode->Height, nBPP ) )
			{
				g_pStruct->ConsolePrint( "[ERROR] MursumDX_SetDisplayMode %d * %d * %d", pMode->Width, pMode->Height, nBPP );
				return false;
			}
		}
	}
	else
	{
		MursumDX_RestoreDisplayMode();
	}
	
	SetPresentationParams(PresentationParam,pMode);
	
	HRESULT hResetResult = m_pD3DDevice->Reset(&PresentationParam);
	
	if (!SUCCEEDED(hResetResult))
	{
		ASSERT(!"ERROR m_pD3DDevice->Reset");
		// Wait for a couple seconds to see if it'll come back...
		g_pStruct->ConsolePrint( "[ERROR] m_pD3DDevice->Reset_1 %d", GetTickCount() );

		uint32 nMaxWait = 20;
		do
		{
			Sleep(100);
			hResetResult = m_pD3DDevice->TestCooperativeLevel();
		} while ((hResetResult == D3DERR_DEVICELOST) && (nMaxWait--));

		g_pStruct->ConsolePrint( "[ERROR] m_pD3DDevice->Reset_2 %d", GetTickCount() );

		// Now try and reset again.
		if (m_pD3DDevice->Reset(&PresentationParam) != D3D_OK)
		{
			g_pStruct->ConsolePrint( "[ERROR] m_pD3DDevice->Reset_4 %d", GetTickCount() );
			// Fall back and try to reset
			TryFallingBack_OnFailedDevCreate(&PresentationParam);

			g_pStruct->ConsolePrint( "[ERROR] m_pD3DDevice->Reset_5 %d", GetTickCount() );

			if (m_pD3DDevice->Reset(&PresentationParam) != D3D_OK)
			{
				g_pStruct->ConsolePrint( "[ERROR] m_pD3DDevice->Reset_6 %d", GetTickCount() );
				return false;
			}
		}

		g_pStruct->ConsolePrint( "[ERROR] m_pD3DDevice->Reset_7 %d", GetTickCount() );
	}
	
	ShowCursor(false);

	// Reset the device caps, since some of them are specific to the screen mode
	PreCalcSomeDeviceCaps();

	m_rcViewport.left			= 0;	// Force a viewport reset...
	m_rcViewport.right			= 0;
	m_rcViewport.top			= 0;
	m_rcViewport.bottom			= 0;

	//[MURSUM]==================================================
	//������ �����ؾ���~ ���߳ѵ��!!!
	m_pMode	= pMode;
	//==========================================================

	SetWindowPos(g_hWnd, HWND_TOP, 0, 0, pMode->Width, pMode->Height, SWP_NOMOVE);
		
	return true;
}

// Releases the device (and resets all that should be reset on device release)...
void CD3D_Device::FreeDevice()
{
	if (m_pD3DDevice)
	{
		uint32 iRefCnt = m_pD3DDevice->Release();
		// Remember, this is actually the wrapper we're deleting here.  The above
		// call to Release is for the actual D3D device.
		delete m_pD3DDevice;
	}

	MursumDX_Release();
	ResetDeviceVars();
}

// Release all the device objects (the Render Objects)...
bool CD3D_Device::ReleaseDevObjects(bool bFullRelease)
{
	m_bReleaseState = true;

	//free the screen glow's resources
	CScreenGlowMgr::GetSingleton().FreeDeviceObjects();

	// free the vertex shaders
	LTVertexShaderMgr::GetSingleton().FreeDeviceObjects();

	// Free the pixel shaders.
	LTPixelShaderMgr::GetSingleton().FreeDeviceObjects();

	// Notify all RenderObjects that we're going to free the device (to give up all there device stuff)
	CRenderObject* pRenderObject = m_pRenderObjectList_Head;
	while (pRenderObject)
	{
		pRenderObject->FreeDeviceObjects();
		pRenderObject = pRenderObject->GetNext();
	}

	if (m_pRenderWorld)
		m_pRenderWorld->Release();

	// Don't actually delete them unless we really want to
	if (bFullRelease)
	{
		delete m_pRenderWorld;
		m_pRenderWorld = 0;
	}

	m_bReleaseState = false;

	return true;
}

// If device we lost, restores the objects (calls ReCreateObject on all the Render Objects)...
bool CD3D_Device::RestoreDevObjects()
{
	m_bRestoreState = true;

	LTVertexShaderMgr::GetSingleton().RecreateVertexShaders();
	LTPixelShaderMgr::GetSingleton().RecreatePixelShaders();

	// Notify all RenderObjects that they need to re-create themselves...
	CRenderObject* pRenderObject = m_pRenderObjectList_Head;
	while (pRenderObject)
	{
		pRenderObject->ReCreateObject();
		pRenderObject = pRenderObject->GetNext();
	}

	m_bRestoreState = false;

	return true;
}

// Do all the resetting that we need to do (device release stuff)...
void CD3D_Device::ResetDeviceVars()
{
	m_pD3DDevice				= NULL;
	m_pAdapter					= NULL;
	m_pDevice					= NULL;
	m_pMode						= NULL;
	m_bWindowed					= false;
	m_bIn3D						= false;
}

// Resets back to initial conditions (doesn't try to free anything)...
void CD3D_Device::Reset()
{
	ResetDeviceVars();
	m_pRenderObjectList_Head	= NULL;

	m_rcViewport.left			= 0;
	m_rcViewport.right			= 0;
	m_rcViewport.top			= 0;
	m_rcViewport.bottom			= 0;

	m_pRenderStyleList_Head		= NULL;

	m_pRenderWorld				= NULL;
}

// Frees all the member vars and resets afterwards...
void CD3D_Device::FreeAll()
{
	// Destoy all the Render Objects in the list...
	while (m_pRenderObjectList_Head)
	{
		CRenderObject* pTmp = m_pRenderObjectList_Head->GetNext();
		DestroyRenderObject(m_pRenderObjectList_Head);
		m_pRenderObjectList_Head = pTmp;
	}

	// Destoy all the Render Styles in the list...
	while (m_pRenderStyleList_Head)
	{
		CD3DRenderStyle* pTmp = m_pRenderStyleList_Head->GetNext();
		DestroyRenderStyle(m_pRenderStyleList_Head);
		m_pRenderStyleList_Head = pTmp;
	}

	delete m_pRenderWorld;
	m_pRenderWorld = NULL;

	FreeDevice();

	Reset();
}

void CD3D_Device::SetPresentationParams(D3DPRESENT_PARAMETERS& PresentationParam,D3DModeInfo* pMode)
{
	if( g_CV_ValidWndMode )
	{
		D3DDISPLAYMODE*	pDesktopFormat						= g_D3DShell.GetDesktopFormat();
		
		PresentationParam.BackBufferWidth					= pMode->Width;
		PresentationParam.BackBufferHeight					= pMode->Height;
		PresentationParam.BackBufferFormat					= pDesktopFormat->Format;//pMode->Format;
		PresentationParam.BackBufferCount					= 1;						// Number of back buffers (1 "double buffer", 2 "triple buffer", etc)
		PresentationParam.MultiSampleType					= GetDefaultMultiSampleType(g_CV_AntiAliasFSOverSample);
		PresentationParam.SwapEffect						= D3DSWAPEFFECT_COPY;
		PresentationParam.hDeviceWindow						= g_hWnd;
		PresentationParam.Windowed							= true;
		PresentationParam.EnableAutoDepthStencil			= true;
		PresentationParam.AutoDepthStencilFormat			= GetDefaultDepthStencilFormat(g_CV_ZBitDepth,g_CV_StencilBitDepth);
		PresentationParam.Flags								= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;	// puffs, but need it for the console (should get rid of this eventually)...
		PresentationParam.FullScreen_RefreshRateInHz		= 0;
		PresentationParam.FullScreen_PresentationInterval	= 0;
	}
	else
	{
		PresentationParam.BackBufferWidth					= pMode->Width;
		PresentationParam.BackBufferHeight					= pMode->Height;
		PresentationParam.BackBufferFormat					= pMode->Format;
		PresentationParam.BackBufferCount					= 1;						// Number of back buffers (1 "double buffer", 2 "triple buffer", etc)
		PresentationParam.MultiSampleType					= GetDefaultMultiSampleType(g_CV_AntiAliasFSOverSample);
		PresentationParam.SwapEffect						= g_bRunWindowed ? D3DSWAPEFFECT_COPY : D3DSWAPEFFECT_DISCARD;
		PresentationParam.hDeviceWindow						= g_hWnd;
		PresentationParam.Windowed							= g_bRunWindowed;
		PresentationParam.EnableAutoDepthStencil			= true;
		PresentationParam.AutoDepthStencilFormat			= GetDefaultDepthStencilFormat(g_CV_ZBitDepth,g_CV_StencilBitDepth);
		PresentationParam.Flags								= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;	// puffs, but need it for the console (should get rid of this eventually)...
		PresentationParam.FullScreen_RefreshRateInHz		= D3DPRESENT_RATE_DEFAULT; //D3DPRESENT_RATE_UNLIMITED
		PresentationParam.FullScreen_PresentationInterval	= (g_bRunWindowed || g_CV_VSyncOnFlip.m_Val) ? NULL : D3DPRESENT_INTERVAL_IMMEDIATE;
	}
}

// Go for the best one that works and fallback till we find one...
//	Note: Need to watch for loops here. Would be very bad.
#define D3DDEVICE_CHECKVALID_ZSFORMAT(MyFormat) (PDIRECT3D->CheckDeviceFormat(m_pAdapter->iAdapterNum,m_pDevice->DeviceType,m_pMode->Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,MyFormat) == D3D_OK ? true : false)
D3DFORMAT CD3D_Device::GetDefaultDepthStencilFormat(uint32 iZBitDepth,uint32 iStencilBitDepth)
{
	D3DFORMAT DepthStencilFormat = D3DFMT_UNKNOWN;
	switch (iZBitDepth)
	{
	case 32 :
		switch (iStencilBitDepth)
		{
		case 8  : DepthStencilFormat = D3DFMT_D24S8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(32,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D24X4S4;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,8); break;
		case 1  : DepthStencilFormat = D3DFMT_D24S8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(32,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D32;		if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,8); break;
		default : assert(0); return D3DFMT_UNKNOWN; break;
		}
		break;
	case 24 :
		switch (iStencilBitDepth)
		{
		case 8  : DepthStencilFormat = D3DFMT_D24S8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D24X4S4;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,0); break;
		case 1  : DepthStencilFormat = D3DFMT_D24S8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(24,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D24X8;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,8); break;
		default : assert(0); return D3DFMT_UNKNOWN; break;
		}
		break;
	case 16 :
		switch (iStencilBitDepth)
		{
		case 8  : DepthStencilFormat = D3DFMT_D15S1;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,4); break;
		case 4  : DepthStencilFormat = D3DFMT_D15S1;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,0); break;
		case 1  : DepthStencilFormat = D3DFMT_D15S1;	if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return GetDefaultDepthStencilFormat(16,4); break;
		case 0  : DepthStencilFormat = D3DFMT_D16;		if (!D3DDEVICE_CHECKVALID_ZSFORMAT(DepthStencilFormat)) return D3DFMT_UNKNOWN; break;
		default : assert(0); return D3DFMT_UNKNOWN; break;
		}
		break;
	default : assert(0); return D3DFMT_UNKNOWN;
	}

	return DepthStencilFormat;
}

D3DMULTISAMPLE_TYPE CD3D_Device::GetDefaultMultiSampleType(uint32 Samples)
{
	D3DMULTISAMPLE_TYPE MultiSampleType = (D3DMULTISAMPLE_TYPE)Samples;
	bool bWindowed;

	if( g_CV_ValidWndMode )
	{
		bWindowed = true;
	}
	else
	{
		bWindowed = g_bRunWindowed;
	}

	if (PDIRECT3D->CheckDeviceMultiSampleType(m_pAdapter->iAdapterNum,m_pDevice->DeviceType,m_pMode->Format,bWindowed,MultiSampleType) != D3D_OK)
	{
		if ((uint32)MultiSampleType > 0)
			return GetDefaultMultiSampleType((uint32)MultiSampleType-1); else return (D3DMULTISAMPLE_TYPE)0;
	}
	return MultiSampleType;
}

// Figure out if we can render some puff or not (note that this is state dependent),

//	I'm assuming it called with the render states set as they will be during rendering...
void CD3D_Device::PreCalcSomeDeviceCaps()
{
	if (!m_pD3DDevice)
		return;

	// Check for TableFog...
	if ((m_DeviceCaps.RasterCaps & D3DPRASTERCAPS_FOGTABLE) && ((m_DeviceCaps.RasterCaps & D3DPRASTERCAPS_ZFOG) || (m_DeviceCaps.RasterCaps & D3DPRASTERCAPS_WFOG)) && g_CV_TableFog.m_Val)
	{
		m_ExtraDevCaps.m_bUsingTableFog = true;
	}
	else
	{
		m_ExtraDevCaps.m_bUsingTableFog = false;
	}

	// Check for stencil buffer & get the format...
	LPDIRECT3DSURFACE8 pDepthStencilBuffer = NULL;
	D3D_CALL(m_pD3DDevice->GetDepthStencilSurface(&pDepthStencilBuffer));
	if (pDepthStencilBuffer)
	{
		D3DSURFACE_DESC SurfDesc; SurfDesc.Size   = sizeof(SurfDesc);
		D3D_CALL(pDepthStencilBuffer->GetDesc(&SurfDesc));
		m_ExtraDevCaps.m_DepthStencilBufferFormat = SurfDesc.Format; uint32 iZDepth,iStencilDepth;
		d3d_GetDepthStencilBits(m_ExtraDevCaps.m_DepthStencilBufferFormat,iZDepth,iStencilDepth);
		m_ExtraDevCaps.m_bHasStencilBuffer = (iStencilDepth>0 ? true : false);
		int iRefCnt	= pDepthStencilBuffer->Release();
	}
}

// Checks what video card we're running on and disables certain features for some.
void CD3D_Device::CheckSpecialCards()
{
	if (!m_pAdapter || !g_pStruct)
		return;


	// First check if they want to force it.
	char* pForceMode = "ForceMode";

	//here we can check the different vendor ID's as well as the ForceMode console variable
	//to determine if we have any special cards that we need to do custom configurations on
}

void CD3D_Device::SetDefaultRenderStates()
{
	// Basic Render State defaults...
	if (m_ExtraDevCaps.m_bUsingTableFog)
	{
		D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE,  D3DFOG_LINEAR));
		D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE));
	}
	else
	{
		D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR));
		D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE,  D3DFOG_NONE));
	}

	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_EDGEANTIALIAS, FALSE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_ZBIAS, 0));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, g_CV_Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_CLIPPING, TRUE));

	// HW Transform & Light...
	PD3DDEVICE->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, ((g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0) ? 1 : 0);

	// Setup Material (for D3D lighting)...
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_LIGHTING,FALSE));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,D3DMCS_MATERIAL));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,D3DMCS_MATERIAL));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_MATERIAL));
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL));

	// Texture Stage States...
	for(uint32 nCurrStage = 0; nCurrStage < 4; nCurrStage++)
	{
		D3D_CALL(m_pD3DDevice->SetTextureStageState(nCurrStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | nCurrStage));
		D3D_CALL(m_pD3DDevice->SetTextureStageState(nCurrStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
		D3D_CALL(m_pD3DDevice->SetTextureStageState(nCurrStage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP));
		D3D_CALL(m_pD3DDevice->SetTextureStageState(nCurrStage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP));
		if (g_CV_Anisotropic.m_Val)
		{
			D3D_CALL(PD3DDEVICE->SetTextureStageState(nCurrStage, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC));
			D3D_CALL(PD3DDEVICE->SetTextureStageState(nCurrStage, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC));
			D3D_CALL(PD3DDEVICE->SetTextureStageState(nCurrStage, D3DTSS_MAXANISOTROPY, g_CV_Anisotropic.m_Val));
		}
		else
		{
			D3D_CALL(PD3DDEVICE->SetTextureStageState(nCurrStage, D3DTSS_MINFILTER, g_CV_Bilinear.m_Val ? D3DTEXF_LINEAR : D3DTEXF_POINT));
			D3D_CALL(PD3DDEVICE->SetTextureStageState(nCurrStage, D3DTSS_MAGFILTER, g_CV_Bilinear.m_Val ? D3DTEXF_LINEAR : D3DTEXF_POINT));
		}
		D3D_CALL(PD3DDEVICE->SetTextureStageState(nCurrStage, D3DTSS_MIPFILTER, g_CV_Trilinear.m_Val ? D3DTEXF_LINEAR:D3DTEXF_POINT));

		d3d_DisableTexture(nCurrStage);
	}

	//setup a default blend
	D3D_CALL(m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
	D3D_CALL(m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT));

	//turn off alpha ref
	D3D_CALL(m_pD3DDevice->SetRenderState(D3DRS_ALPHAREF, 0));

	// Setup the Material...This should be set on the per piece basis, but for the fall release I'm just setting it the once and moving on...
	D3DMATERIAL8 Material;
	D3DCOLORVALUE MatColor1; MatColor1.r = 1.0f; MatColor1.g = 1.0f; MatColor1.b = 1.0f; MatColor1.a = 1.0f;
	D3DCOLORVALUE MatColor2; MatColor2.r = 0.0f; MatColor2.g = 0.0f; MatColor2.b = 0.0f; MatColor2.a = 0.0f;
	Material.Diffuse  = MatColor1; Material.Ambient  = MatColor1;
	Material.Specular = MatColor2; Material.Emissive = MatColor2; Material.Power = 0.0f;
	g_RenderStateMgr.SetMaterial(Material);

	assert(m_pD3DDevice);
}

void CD3D_Device::SetupViewport(uint32 iLeft, uint32 iRight, uint32 iTop, uint32 iBottom, float fMinZ, float fMaxZ)
{
	if (m_rcViewport.left == (LONG)iLeft && m_rcViewport.right == (LONG)iRight && m_rcViewport.top == (LONG)iTop && m_rcViewport.bottom == (LONG)iBottom) return;

	m_rcViewport.left	= iLeft;
	m_rcViewport.right	= iRight;
	m_rcViewport.top	= iTop;
	m_rcViewport.bottom = iBottom;

	D3DVIEWPORT8 viewportData;
	viewportData.X		= m_rcViewport.left;
	viewportData.Y		= m_rcViewport.top;
	viewportData.Width	= m_rcViewport.right  - m_rcViewport.left;
	viewportData.Height = m_rcViewport.bottom - m_rcViewport.top;
	viewportData.MinZ	= fMinZ;
	viewportData.MaxZ	= fMaxZ;

	HRESULT hResult = D3D_CALL(PD3DDEVICE->SetViewport(&viewportData));
	if (FAILED(hResult))
	{
		assert(!"IDirect3DDevice::SetViewport failed.");
		return;
	}
}

// Creates the requested Render Object and returns to the Engine...
CRenderObject* CD3D_Device::CreateRenderObject(CRenderObject::RENDER_OBJECT_TYPES ObjectType)
{
	CRenderObject* pNewObject = NULL;			// Create the Sucka...
	switch (ObjectType)
	{
	case CRenderObject::eDebugLine :
		LT_MEM_TRACK_ALLOC(pNewObject = new CDIDebugLine(),LT_MEM_TYPE_RENDERER);
		break;
	case CRenderObject::eDebugPolygon :
		LT_MEM_TRACK_ALLOC(pNewObject = new CDIDebugPolygon(),LT_MEM_TYPE_RENDERER);
		break;
	case CRenderObject::eDebugText :
		LT_MEM_TRACK_ALLOC(pNewObject = new CDIDebugText(),LT_MEM_TYPE_RENDERER);
		break;
	case CRenderObject::eRigidMesh :
		LT_MEM_TRACK_ALLOC(pNewObject = new CD3DRigidMesh(),LT_MEM_TYPE_RENDERER);
		break;
	case CRenderObject::eSkelMesh :
		LT_MEM_TRACK_ALLOC(pNewObject = new CD3DSkelMesh(),LT_MEM_TYPE_RENDERER);
		break;
	case CRenderObject::eVAMesh :
		LT_MEM_TRACK_ALLOC(pNewObject = new CD3DVAMesh(),LT_MEM_TYPE_RENDERER);
		break;
	case CRenderObject::eNullMesh :
		LT_MEM_TRACK_ALLOC(pNewObject = new CDIModelDrawable,LT_MEM_TYPE_RENDERER);
		break;
	default :
		return NULL; break;
	}

	if (pNewObject)
	{
		// Add it to the Render Object List...
		pNewObject->SetNext(g_Device.m_pRenderObjectList_Head);
		g_Device.m_pRenderObjectList_Head = pNewObject;
	}
	return pNewObject;
}

bool CD3D_Device::DestroyRenderObject(CRenderObject* pObject)
{
	if (pObject == g_Device.m_pRenderObjectList_Head)
	{
		// Remove the sucka from the Render Object List...
		g_Device.m_pRenderObjectList_Head = pObject->GetNext();
	}
	else
	{
		if (!g_Device.m_pRenderObjectList_Head)
		{
			// It's not in the list!?!
			//assert(0); // Note : This really should assert, but this happens VERY frequently and doesn't seem to be causing any problems
			return false;
		}
		CRenderObject* pPrevObject = g_Device.m_pRenderObjectList_Head;
		while (pPrevObject->GetNext() && pPrevObject->GetNext() != pObject)
		{
			pPrevObject = pPrevObject->GetNext();
		}
		if (pPrevObject->GetNext())
		{
			pPrevObject->SetNext(pPrevObject->GetNext()->GetNext());
		}
		else
		{
			// It's not in the list!?!
			assert(0);
			return false;
		}
	}

	delete pObject;								// Ok, now delete the sucka...

	return true;
}

CD3DRenderStyle* CD3D_Device::CreateRenderStyle()
{
	CD3DRenderStyle* pRenderStyle;
	LT_MEM_TRACK_ALLOC(pRenderStyle = new CD3DRenderStyle,LT_MEM_TYPE_RENDERER);
	if (!pRenderStyle)
		return NULL;
	pRenderStyle->IncRefCount();

	// Add it to the Render Object List...
	pRenderStyle->SetNext(g_Device.m_pRenderStyleList_Head);
	g_Device.m_pRenderStyleList_Head = pRenderStyle;

	return pRenderStyle;
}

void CD3D_Device::DestroyRenderStyle(CRenderStyle* pRenderStyle)
{
	CD3DRenderStyle* pD3DRenderStyle = (CD3DRenderStyle*)pRenderStyle;
	if (pD3DRenderStyle)
	{
		if (pD3DRenderStyle == g_Device.m_pRenderStyleList_Head)
		{
			// Remove the sucka from the Render Object List...
			g_Device.m_pRenderStyleList_Head = pD3DRenderStyle->GetNext();
		}
		else
		{
			if (!g_Device.m_pRenderStyleList_Head)
			{
				// It's not in the list!?!
				//assert(0); // Note : This really should assert, but this happens VERY frequently and doesn't seem to be causing any problems
				return;
			}
			CD3DRenderStyle* pPrevRenderStyle = g_Device.m_pRenderStyleList_Head;
			while (pPrevRenderStyle->GetNext() && pPrevRenderStyle->GetNext() != pD3DRenderStyle)
			{
				pPrevRenderStyle = pPrevRenderStyle->GetNext();
			}
			if (pPrevRenderStyle->GetNext())
			{
				pPrevRenderStyle->SetNext(pPrevRenderStyle->GetNext()->GetNext());
			}
			else
			{
				// It's not in the list!?!
				assert(0);
				return;
			}
		}

		delete pD3DRenderStyle; pD3DRenderStyle = NULL; pRenderStyle = NULL;
	}
}

bool CD3D_Device::Standby()
{
	if (PD3DDEVICE)
	{
		// Make sure nothing's still stuck between D3D's teeth.
		PD3DDEVICE->SetStreamSource(0, 0, 0);
		PD3DDEVICE->SetIndices(0, 0);
	}

	return g_Device.GetDeviceInfo() != 0;
}


// Call before you start rendering a frame...
bool CD3D_Device::Start3D()
{
	CSAccess cLoadRenderCSLock(&g_Device.GetLoadRenderCS());

	if (g_Device.m_bIn3D || !g_Device.m_pD3DDevice)
		return false;

	HRESULT hResult = D3D_CALL(g_Device.m_pD3DDevice->BeginScene());
	g_Device.m_bIn3D = (hResult == D3D_OK);

	return g_Device.m_bIn3D;
}

// Call when you are done rendering a frame...
bool CD3D_Device::End3D()
{
	CSAccess cLoadRenderCSLock(&g_Device.GetLoadRenderCS());

	if (!g_Device.m_bIn3D || !g_Device.m_pD3DDevice)
		return false;

	g_Device.m_bIn3D = false;
	HRESULT hResult = g_Device.m_pD3DDevice->EndScene();
	return (hResult == D3D_OK);
}

bool CD3D_Device::IsIn3D()
{
	CSAccess cLoadRenderCSLock(&g_Device.GetLoadRenderCS());
	return g_Device.m_bIn3D;
}

//Helper macro to convert a conditional to a yes no string
#define YESNO(X) ((X) ? "Yes" : "No")

// ConsolePrint the device caps...
void CD3D_Device::ListDeviceCaps()
{
	if (!m_pD3DDevice)
		return;
	if (!m_pAdapter)
		return;
	if (!m_pMode)
		return;

	PFormat PixFormat;
	if(!d3d_D3DFormatToPFormat(m_pMode->Format,&PixFormat))
		return;

	g_pStruct->ConsolePrint("---------------------------------------------------------------");
	g_pStruct->ConsolePrint("Driver: %s", m_pAdapter->AdapterID.Driver);
	g_pStruct->ConsolePrint("Description: %s", m_pAdapter->AdapterID.Description);
	g_pStruct->ConsolePrint("Version: %x", m_pAdapter->AdapterID.DriverVersion);
	g_pStruct->ConsolePrint("VendorID: 0x%x, DeviceID: 0x%x, SubSysID: 0x%x, Revision: 0x%x",
		m_pAdapter->AdapterID.VendorId, m_pAdapter->AdapterID.DeviceId,
		m_pAdapter->AdapterID.SubSysId, m_pAdapter->AdapterID.Revision);
	g_pStruct->ConsolePrint("Width: %d, Height: %d, BitDepth: %d",
		m_pMode->Width,m_pMode->Height,PixFormat.GetBitsPerPixel());
	g_pStruct->ConsolePrint("---------------------------------------------------------------");

	g_pStruct->ConsolePrint("Portals: %s", YESNO(CanDrawPortals()));
	g_pStruct->ConsolePrint("Light add poly: %s", YESNO(m_ExtraDevCaps.m_bLightAddPolyCapable));
	g_pStruct->ConsolePrint("MipMap supported: %s", YESNO(m_DeviceCaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP));
	g_pStruct->ConsolePrint("Square textures only: %s", YESNO(m_DeviceCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY));
	g_pStruct->ConsolePrint("Approx Texture Memory: %d", m_pD3DDevice->GetAvailableTextureMem());
	uint32 iZDepth,iStencilDepth; d3d_GetDepthStencilBits(m_ExtraDevCaps.m_DepthStencilBufferFormat,iZDepth,iStencilDepth);
	g_pStruct->ConsolePrint("ZBuffer Depth: %d, Device Depth: %d", iZDepth, iStencilDepth);
	g_pStruct->ConsolePrint("Z-test: %s, Table fog: %s, Palette alpha: %s", YESNO(m_DeviceCaps.RasterCaps  & D3DPRASTERCAPS_ZTEST),
		YESNO(m_DeviceCaps.RasterCaps  & D3DPRASTERCAPS_FOGTABLE), YESNO(m_DeviceCaps.TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE));
	g_pStruct->ConsolePrint("Max texture size: (%d x %d)",m_DeviceCaps.MaxTextureWidth, m_DeviceCaps.MaxTextureHeight);
}

bool CD3D_Device::LoadWorldData(ILTStream *pStream)
{
	if (m_pRenderWorld)
		delete m_pRenderWorld;
	LT_MEM_TRACK_ALLOC(m_pRenderWorld = new CD3D_RenderWorld,LT_MEM_TYPE_RENDERER);
	return m_pRenderWorld->Load(pStream);
}
