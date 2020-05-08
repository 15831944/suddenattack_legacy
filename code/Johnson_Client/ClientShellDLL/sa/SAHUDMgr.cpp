// ----------------------------------------------------------------------- //
//
// MODULE  : SAHUDMgr.cpp
//
// PURPOSE : Implementation of CSAHUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SAHUDMgr.h"
#include "SAPlayerStats.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"

extern CGameClientShell* g_pGameClientShell;

CHUDCrosshair*		g_pCrosshair		= LTNULL;
CHUDCompass*		g_pCompass			= LTNULL;
CHUDTimer*			g_pHUDTimer			= LTNULL;
CHUDTimeBombTimer*	g_pHUDTimeBombTimer	= LTNULL;
CHUDRadio*			g_pRadio			= LTNULL;
CHUDGameOver*		g_pHUDGameOver		= LTNULL;
CHUDWpnChooser*		g_pWpnChooser		= LTNULL;
CHUDFlashBangFX*	g_pHUDFlashBangFX	= LTNULL;
CHUDSensiCtrl*		g_pHUDSensiCtrl		= LTNULL;
CHUDPlayerState*	g_pHUDPlayerState	= LTNULL;
CHUDGameState*		g_pHUDGameState		= LTNULL;
CHUDAmmo*			g_pHUDAmmo			= LTNULL;
CHUDMiniMap*		g_pHUDMiniMap		= LTNULL;

//CHUDCameraBloodFX*	g_pCameraBloodFX	= LTNULL;
//CHUDGogglesFX*		g_pHUDGogglesFX		= LTNULL;

inline void CHUDItem::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAHUDMgr::CSAHUDMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSAHUDMgr::CSAHUDMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAHUDMgr::~CSAHUDMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSAHUDMgr::~CSAHUDMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAHUDMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CSAHUDMgr::Init()
{
	//crosshair should be first so that it is rendered first (potential overlap)
	m_itemArray.push_back(&m_Crosshair);		//���ؼ�
//	m_itemArray.push_back(&m_CameraBloodFX);	//��~~
//	m_itemArray.push_back(&m_HUDGogglesFX);		//���ܼ� ī�޶�
	m_itemArray.push_back(&m_HUDFlashBangFX);	//�÷�����
	
	m_itemArray.push_back(&m_Ammo);				//���� ���õ� ����
	m_itemArray.push_back(&m_Health);			//HP N' AP
	m_itemArray.push_back(&m_Timer);			//�ð�
	m_itemArray.push_back(&m_TimeBombTimer);
	m_itemArray.push_back(&m_GameOver);			//Win, Defeat, Draw
	m_itemArray.push_back(&m_Radio);			//V ������ ������ ä�� ����Ű
	m_itemArray.push_back(&m_DamageDir);		//�� ���� ��ġ ǥ��
	m_itemArray.push_back(&m_Compass);			//�̴ϸ�
	m_itemArray.push_back(&m_Respawn);			//������
	m_itemArray.push_back(&m_WpnChooser);		//���� ����Ʈ �� ���� ���õ� ����
	m_itemArray.push_back(&m_ProgressBar);		//���߿� ����� �� �����ϱ� ���ܵ���~	
	m_itemArray.push_back(&m_HUDSensiCtrl);		
	m_itemArray.push_back(&m_HUDPlayerState);
	m_itemArray.push_back(&m_HUDGameState);

	m_itemArray.push_back(&m_HUDMiniMap);
		
	g_pCrosshair		= &m_Crosshair;
	g_pCompass			= &m_Compass;
	g_pHUDTimer			= &m_Timer;
	g_pHUDTimeBombTimer	= &m_TimeBombTimer;
	g_pRadio			= &m_Radio;
	g_pHUDGameOver		= &m_GameOver;
	g_pWpnChooser		= &m_WpnChooser;
	g_pHUDFlashBangFX	= &m_HUDFlashBangFX;
	g_pHUDSensiCtrl		= &m_HUDSensiCtrl;
	g_pHUDPlayerState	= &m_HUDPlayerState;
	g_pHUDGameState		= &m_HUDGameState;
	g_pHUDAmmo			= &m_Ammo;

	g_pHUDMiniMap		= &m_HUDMiniMap;

//	g_pCameraBloodFX	= &m_CameraBloodFX;	
//	g_pHUDGogglesFX		= &m_HUDGogglesFX;

	return CHUDMgr::Init();
}

void CSAHUDMgr::WaitRestart()
{
	CHUDMgr::WaitRestart();
}