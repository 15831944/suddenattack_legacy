#include "stdafx.h"	
#include "HUDHelp.h"

CHUDHelp::CHUDHelp()
{
	m_hHelp[0]			= LTNULL;
	m_hHelp[1]			= LTNULL;
	
	m_bView				= LTFALSE;

	m_UpdateFlags		= kHUDNone;
	m_eLevel			= kHUDRenderNone;

	m_fHUDHelpPos		= 0.0f;

}

CHUDHelp::~CHUDHelp()
{
}

LTBOOL CHUDHelp::Init(void)
{
	char szFile[MAX_PATH] = {0, };
	
	g_pLayoutMgr->GetString("HUDHelp", "BGHUDHelpTx0", szFile, MAX_PATH);
	m_hHelp[0] = g_pInterfaceResMgr->GetTexture(szFile);
	g_pLayoutMgr->GetString("HUDHelp", "BGHUDHelpTx1", szFile, MAX_PATH);
	m_hHelp[1] = g_pInterfaceResMgr->GetTexture(szFile);

	//��ġ�� �⺻ ���� 
	g_pDrawPrim->SetUVWH( &m_PolyHelp[0], 0.0f, 0.0f, 1.0f, 1.0f );
	g_pDrawPrim->SetRGBA( &m_PolyHelp[0], 0xFFFFFFFF );

	g_pDrawPrim->SetUVWH( &m_PolyHelp[1], 0.0f, 0.0f, 1.0f, 1.0f );
	g_pDrawPrim->SetRGBA( &m_PolyHelp[1], 0xFFFFFFFF );

	//��ġ �缳��
	this->ScreenDimChanged();
	
	return LTTRUE;
}

void CHUDHelp::Show(LTBOOL bView)
{
	//���� ����, Ȥ�� �ѹ��� �� �Ҹ� ���¿��� ȭ�� üũ �ѹ� ���ش�.
//	if(LTFALSE == m_bView) ScreenDimChanged();

	m_bView = bView;
}

LTBOOL CHUDHelp::IsVisible(void)
{
	return m_bView;
}

void CHUDHelp::Render(void)
{
	if(LTFALSE == m_bView) return;



	GameType eGT = g_pGameClientShell->GetGameType();
	if(eGameTypeTeamDeathmatch == eGT)
	{
		g_pDrawPrim->SetTexture(m_hHelp[0]);
		g_pDrawPrim->DrawPrim(&m_PolyHelp[0]);
	}
	else if(eGameTypeSurvival == eGT)
	{
		g_pDrawPrim->SetTexture(m_hHelp[1]);
		g_pDrawPrim->DrawPrim(&m_PolyHelp[1]);
	}
}

void CHUDHelp::ScreenDimChanged(void)
{
	m_fHUDHelpPos = (g_pInterfaceResMgr->GetScreenWidth() - 512.0f) * 0.5f;
	g_pDrawPrim->SetXYWH( &m_PolyHelp[0], m_fHUDHelpPos, 0.0f, 512.0f, 256.0f);
	m_fHUDHelpPos = (g_pInterfaceResMgr->GetScreenWidth() - 512.0f) * 0.5f;
	g_pDrawPrim->SetXYWH( &m_PolyHelp[1], m_fHUDHelpPos, 0.0f, 512.0f, 512.0f);
}

void CHUDHelp::Update(void)
{
	//--
}

//���� ���� ������ �ʱ�ȭ (CMissionMgr::FinishStartGame()���� ȣ��)
void CHUDHelp::PrePlayingState(void)
{
	m_bView = LTFALSE;
}
