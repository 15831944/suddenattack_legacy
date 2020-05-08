// ----------------------------------------------------------------------- //
//
// MODULE  : MessageBox.cpp
//
// PURPOSE : Handle the display of a simple message box
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MessageBox.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"

#include "ControlConstant.h"

// Added By kwangsik ==>
// Time : 2004-07-05 ���� 1:47:08
#include "..\\sa\\GameHIIME.h"
// Added By kwangsik <==
#include "..\\SA\\ScreenWeapon.h"

extern CGameClientShell* g_pGameClientShell;


CMsgDlg::CMsgDlg()
{
}

CMsgDlg::~CMsgDlg()
{
}

void CMsgDlg::PlaySelectSound( CLTGUICtrl* pCtrl )
{
	if( pCtrl && pCtrl->IsPlaySound() )
		g_pInterfaceMgr->RequestInterfaceSound( (InterfaceSound)pCtrl->GetOverSound() );
}

void CMsgDlg::PlayClickSound( CLTGUICtrl* pCtrl )
{
	if( pCtrl && pCtrl->IsPlaySound() )
		g_pInterfaceMgr->RequestInterfaceSound( (InterfaceSound)pCtrl->GetClickSound() );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageBox::CMessageBox()
{
    m_pCallback			= LTNULL;
    m_bVisible			= LTFALSE;
    m_bGameWasPaused	= LTFALSE;

	m_pData				= LTNULL;
	m_pBtnOk			= LTNULL;
	m_pBtnCancel		= LTNULL;

	m_pMsgFont			= LTNULL;
	m_nWidth			= 0;
	m_nHeight			= 0;
	m_nStrLen			= 0;
	ZeroMemory( m_szStrBuf, sizeof(m_szStrBuf) );


	m_bOnlyMessage		= LTFALSE;

	memset( m_hYes, 0, sizeof(HTEXTURE) * 4 );
	memset( m_hNo , 0, sizeof(HTEXTURE) * 4 );
	memset( m_TxMsgBoxBackFrame, 0, sizeof(HTEXTURE) * 4 );
	
	m_nControlInterval	= 0;
	m_fTextGap			= 0.0f;
	m_fOffsetXOfAllPoly = 0.0f;			// ������ ��ġ	
	m_fOffsetYOfTopPoly	= 0.0f;
	m_fOffsetYOfMidPoly	= 0.0f;
	m_fOffsetYOfBotPoly	= 0.0f;
	
	m_fWidthOfAllPoly	= 0.0f;			// ������ ����
	
	m_fHeightOfAllPoly	= 0.0f;			// ������ ����
	m_fHeightOfTopPoly	= 0.0f;
	m_fHeightOfMidPoly	= 0.0f;
	m_fHeightOfBotPoly	= 0.0f;

	m_bIgnoreAfter		= LTFALSE;
}

CMessageBox::~CMessageBox()
{
	SAFE_DELETE( m_pMsgFont );
	Term();
}

LTBOOL CMessageBox::Init()
{
	// UI �ٽ� �۾�
	//----------------------------------------------------
	m_Dlg.Create( NULL, 100, 100);		// �׳� �����, Show() ���� ũ�� ���� �ٽ� �Ѵ�.
	
	m_nWidth	= 300;
	m_nHeight	= 200;

	m_pMsgFont = new CSAD3DText();
	m_pMsgFont->CreateD3DText(m_nWidth, m_nHeight, true);
	m_pMsgFont->SetPosition(0.0f, 0.0f);
	m_pMsgFont->SetTextColor(RGB(175,148,104));
	m_pMsgFont->SetTransparent(true);

	// ���� ��ư //////////////////////////////////////////////////////////////////////////
	m_rtYes	= g_pLayoutMgr->GetRect(LO_MISC_TAG, "BtnMessageOkTxPos");
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_hYes[BTN_NORMAL],	 LO_MISC_TAG, "BtnOKTxN" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_hYes[BTN_HIGHLIGHT], LO_MISC_TAG, "BtnOKTxH" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_hYes[BTN_DISABLED],	 LO_MISC_TAG, "BtnOKTxD" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_hYes[BTN_CLICKED],	 LO_MISC_TAG, "BtnOKTxC" );

	m_pBtnOk = new CLTGUIButton();

	if ( !m_pBtnOk->Create(	CMD_MESSAGE_OK, 0, 86.0f/128.0f, 27.0f/32.0f,
							m_hYes[BTN_NORMAL], m_hYes[BTN_HIGHLIGHT],
							m_hYes[BTN_DISABLED], m_hYes[BTN_CLICKED], this) )
	{
		SAFE_DELETE( m_pBtnOk );
		return LTFALSE;
	}
	
	m_pBtnOk->SetRect( m_rtYes );
	m_pBtnOk->SetScale( g_pInterfaceResMgr->GetXRatio() );
	m_pBtnOk->SetOverSound( (int)IS_OVER );
	m_pBtnOk->SetClickSound( (int)IS_CLICK_FLAT );
	m_Dlg.AddControl( m_pBtnOk );

	// ĵ�� ��ư //////////////////////////////////////////////////////////////////////////
	m_rtNo	= g_pLayoutMgr->GetRect(LO_MISC_TAG, "BtnMessageCancelTxPos");
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_hNo[BTN_NORMAL],	LO_MISC_TAG, "BtnCancelTxN" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_hNo[BTN_HIGHLIGHT],	LO_MISC_TAG, "BtnCancelTxH" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_hNo[BTN_DISABLED],	LO_MISC_TAG, "BtnCancelTxD" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_hNo[BTN_CLICKED],	LO_MISC_TAG, "BtnCancelTxC" );

	m_pBtnCancel = new CLTGUIButton();
	
	if ( !m_pBtnCancel->Create(	CMD_MESSAGE_CANCEL, 0, 86.0f/128.0f, 27.0f/32.0f,
								m_hNo[BTN_NORMAL], m_hNo[BTN_HIGHLIGHT],
								m_hNo[BTN_DISABLED], m_hNo[BTN_CLICKED], this) )
	{
		SAFE_DELETE( m_pBtnCancel );
		return LTFALSE;
	}

	m_pBtnCancel->SetRect( m_rtNo );
	m_pBtnCancel->SetScale( g_pInterfaceResMgr->GetXRatio() );
	m_pBtnCancel->SetOverSound( (int)IS_OVER );
	m_pBtnCancel->SetClickSound( (int)IS_CLICK_CANCEL );
	m_Dlg.AddControl( m_pBtnCancel );

	// �޽��� �ڽ� ���� ������ �ؽ��� //////////////////////////////////////////////////////////////////////////
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_TxMsgBoxBackFrame[MBF_TOP],		LO_MISC_TAG, "MsgBoxBackFrameT" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_TxMsgBoxBackFrame[MBF_MIDDLE],	LO_MISC_TAG, "MsgBoxBackFrameM" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_TxMsgBoxBackFrame[MBF_BOTTOM],	LO_MISC_TAG, "MsgBoxBackFrameB" );
	GET_TEXTURE_FROM_LAYOUT_MNGR( m_TxMsgBoxBackFrame[MBF_BOTTOM_ONLY_NOTICE],	LO_MISC_TAG, "MsgBoxBackFrameB_ON" );
	
	m_fTextGap			= g_pLayoutMgr->GetFloat( LO_MISC_TAG, "MsgBoxMessageGap", 32.0f );
	m_nControlInterval	= g_pLayoutMgr->GetInt( LO_MISC_TAG, "MsgBoxBtnInterval", 90 );

	g_pDrawPrim->SetRGBA( &m_PolyMsgBoxBackFrame[MBF_TOP],		0xFFFFFFFF );
	g_pDrawPrim->SetRGBA( &m_PolyMsgBoxBackFrame[MBF_MIDDLE],	0xFFFFFFFF );
	g_pDrawPrim->SetRGBA( &m_PolyMsgBoxBackFrame[MBF_BOTTOM],	0xFFFFFFFF );
	g_pDrawPrim->SetRGBA( &m_PolyMsgBoxBackFrame[MBF_BOTTOM_ONLY_NOTICE], 0xFFFFFFFF );

	//[MURSUM]=======================================================
	// �޼��� �ڽ� �ö�ö�, �ؿ� ȭ�� ������ ���� ���� ���� ������
	g_pDrawPrim->SetRGBA( &m_PolyBack, 0x80000000 );
	this->ScreenDimsChanged();
	//================================================================

    return LTTRUE;
}

void CMessageBox::Term()
{
	m_Dlg.Destroy();
}

void CMessageBox::SetMessageBackTexture(HTEXTURE hTexture)
{
	CLTGUIFrame* BackFrame = m_Dlg.GetMainFrameTx();
	BackFrame->SetFrame( hTexture );
}

void CMessageBox::ApplyCarriageReturnToLongSentence()
{
	if( !m_szStrBuf )
		return;
	
	HDC hDC		= m_pMsgFont->GetDC();
	m_nStrLen	= lstrlen( m_szStrBuf );

	char*	pszTemp		= new char[m_nStrLen + 8];
	char*	pszTempLine	= new char[256];
	int		nTempLen	= strlen( m_szStrBuf );

	ZeroMemory( pszTemp, sizeof(char) * (m_nStrLen + 8) );
	ZeroMemory( pszTempLine, sizeof(char) * 256 );

	lstrcpyn( pszTemp, m_szStrBuf, nTempLen + 1 );
	ZeroMemory( m_szStrBuf, sizeof(m_szStrBuf) );	

	SIZE tTextSize;
	::GetTextExtentPoint32( hDC, m_szStrBuf, m_nStrLen, &tTextSize );

	char* pEnd = pszTemp;
	char* pNow = pszTemp;
	
	if ( tTextSize.cx > m_nWidth )
	{
		while ( *pEnd != '\0' )							// �ι��� ������ ������
		{
			pEnd = ::CharNext( pEnd );
			
			if ( *pEnd == '\r' && *(pEnd+1) == '\n' )	// \r\n �� �˻��ؼ� ������
			{
				lstrcpyn( pszTempLine, pNow, pEnd - pNow + 1 );
				lstrcat( m_szStrBuf, pszTempLine );
				pNow = pEnd;
				continue;								// �׳� �����ϰ�
			}

			// \r\n �� ���µ�

			GetTextExtentPoint32( hDC, pNow, pEnd - pNow, &tTextSize );

			if ( tTextSize.cx >= m_nWidth )				// ���̺��� �����������
			{
				pEnd = ::CharPrev( pNow, pEnd );
				pEnd = ::CharPrev( pNow, pEnd );
				
				lstrcpyn( pszTempLine, pNow, pEnd - pNow + 1 );
				lstrcat( m_szStrBuf, pszTempLine );
				lstrcat( m_szStrBuf, "\r\n\r\n" );			// \r\n �����ִ´�
				pNow = pEnd;			
			}
		}
	}

	lstrcat( m_szStrBuf, pNow );

	SAFE_DELETE_ARRAY( pszTemp		);
	SAFE_DELETE_ARRAY( pszTempLine	);
}

void CMessageBox::Show(int nStringID, MBCreate* pCreate, uint8 nFontSize, LTBOOL bDefaultReturn, LTBOOL bIgnoreAfter )
{
	if (m_bVisible || !pCreate)		return;

	Show( LoadTempString(nStringID), pCreate, nFontSize, bDefaultReturn, bIgnoreAfter );
}

void CMessageBox::Show(const char *pString, MBCreate* pCreate, uint8 nFontSize, LTBOOL bDefaultReturn, LTBOOL bIgnoreAfter )
{
	if ( NULL == pString || m_bVisible || !pCreate )	return;

	m_eType				= pCreate->eType;
	m_bGameWasPaused	= g_pGameClientShell->IsGamePaused();

	if ( !m_bGameWasPaused )
	{
        g_pGameClientShell->PauseGame( LTTRUE, LTTRUE );
	}

	//////////////////////////////////////////////////////////////////////////
	// �� �����ε� ĳ���� ������ ������ �׳� ©�� �־������.

	lstrcpy( m_szStrBuf, pString );			// ����� ��Ʈ���� ���ۿ� ���� ��
	ApplyCarriageReturnToLongSentence();	// ���۸� �̿��ؼ� ĳ���� ������ �����Ѵ�.
	//////////////////////////////////////////////////////////////////////////
	

	m_pMsgFont->DrawD3DText(m_szStrBuf, strlen(m_szStrBuf), DT_LEFT, D3D_TEXT_SHADOW, false );

	//[MURSUM]=========================================================
	//�������ϰ�� ���� �� ���� ���̸� ������~!
	int msgLength	= 0;
	int	nTextLen	= 0;
	int	nLineIndex	= 0;

	while ( nTextLen >= 0 )
	{
		nTextLen	= m_pMsgFont->GetLineLength( nLineIndex++ );

		if ( msgLength < nTextLen )
		{
			msgLength = nTextLen;
		}
	}
	//=================================================================


	//////////////////////////////////////////////////////////////////////////
	// �޼��� �ڽ� �� ������ ����~

	m_fWidthOfAllPoly	= g_pLayoutMgr->GetFloat( LO_MISC_TAG, "MsgBoxVisibleTextureWidth", 326.0f );		// ��ü ����
	m_fOffsetXOfAllPoly	= ( g_pInterfaceResMgr->GetScreenWidth() - m_fWidthOfAllPoly ) * 0.5f;
	
	m_fHeightOfTopPoly	= g_pLayoutMgr->GetFloat( LO_MISC_TAG, "MsgBoxVisibleTextureTopHeight", 36.0f );	// ����
	m_fHeightOfBotPoly	= g_pLayoutMgr->GetFloat( LO_MISC_TAG, "MsgBoxVisibleTextureBotHeight", 64.0f );	// �Ʒ���

	g_pDrawPrim->SetUVWH( &m_PolyMsgBoxBackFrame[MBF_TOP],			// ���� UV
							0.0f,
							0.0f,
							m_fWidthOfAllPoly / 512.0f,
							m_fHeightOfTopPoly / 64.0f );

	g_pDrawPrim->SetUVWH( &m_PolyMsgBoxBackFrame[MBF_MIDDLE],		// ����� UV
							0.0f,
							0.0f,
							m_fWidthOfAllPoly / 512.0f,
							1.0f );

	g_pDrawPrim->SetUVWH( &m_PolyMsgBoxBackFrame[MBF_BOTTOM],		// �Ʒ��� UV
							0.0f,
							0.0f,
							m_fWidthOfAllPoly / 512.0f,
							m_fHeightOfBotPoly / 64.0f );

	g_pDrawPrim->SetUVWH( &m_PolyMsgBoxBackFrame[MBF_BOTTOM_ONLY_NOTICE],		// �̰Ŵ� ��ư���� �Ʒ���
							0.0f,
							0.0f,
							m_fWidthOfAllPoly / 512.0f,
							m_fHeightOfBotPoly / 64.0f );


	m_fHeightOfMidPoly	= (float)m_pMsgFont->GetTextHeight() + m_fTextGap;
	m_fHeightOfAllPoly	= m_fHeightOfTopPoly + m_fHeightOfMidPoly + m_fHeightOfBotPoly;

	m_fOffsetYOfTopPoly	= ( g_pInterfaceResMgr->GetScreenHeight() - m_fHeightOfAllPoly ) * 0.5f;
	m_fOffsetYOfMidPoly	= m_fOffsetYOfTopPoly + m_fHeightOfTopPoly;
	m_fOffsetYOfBotPoly	= m_fOffsetYOfMidPoly + m_fHeightOfMidPoly;

	g_pDrawPrim->SetXYWH( &m_PolyMsgBoxBackFrame[MBF_TOP],			// ���� ũ��, ��ġ ����
							m_fOffsetXOfAllPoly,
							m_fOffsetYOfTopPoly,
							m_fWidthOfAllPoly,
							m_fHeightOfTopPoly );
	g_pDrawPrim->SetXYWH( &m_PolyMsgBoxBackFrame[MBF_MIDDLE],		// ����� ũ��, ��ġ ����
							m_fOffsetXOfAllPoly,
							m_fOffsetYOfMidPoly,
							m_fWidthOfAllPoly,
							m_fHeightOfMidPoly );
	g_pDrawPrim->SetXYWH( &m_PolyMsgBoxBackFrame[MBF_BOTTOM],		// �Ʒ��� ũ��, ��ġ ����
							m_fOffsetXOfAllPoly,
							m_fOffsetYOfBotPoly,
							m_fWidthOfAllPoly,
							m_fHeightOfBotPoly );
	g_pDrawPrim->SetXYWH( &m_PolyMsgBoxBackFrame[MBF_BOTTOM_ONLY_NOTICE],		// ��ư���� �Ʒ��� ũ��, ��ġ ����
							m_fOffsetXOfAllPoly,
							m_fOffsetYOfBotPoly,
							m_fWidthOfAllPoly,
							m_fHeightOfBotPoly );
	
	m_Dlg.SetSize( (uint16)m_fWidthOfAllPoly, (uint16)m_fHeightOfAllPoly );
	//////////////////////////////////////////////////////////////////////////
	
	

	//////////////////////////////////////////////////////////////////////////
	// �޼��� ��ġ ����~
	//
	// ���� ��ġ = ��ü������ ������ġ(�޼����ڽ� ���ʳ�) + ( ( �޼����ڽ� ���� - �޼��� ���� ) / 2 )
	// ���� ��ġ = �߾������� ������ġ(����� ���ʳ�) + ( ( ����� ���� - ���� ��ü ���� ) / 2 )

	LTIntPt ptFontOffset;
	ptFontOffset.x = (int)( m_fOffsetXOfAllPoly + ( ( m_fWidthOfAllPoly - msgLength ) * 0.5f ) );
	ptFontOffset.y = (int)( m_fOffsetYOfMidPoly + ( m_fHeightOfMidPoly - m_pMsgFont->GetTextHeight() ) * 0.5f );

	//ONLY_DEBUG(g_pLTClient->CPrint( "@@ MessageBox Position  %d : %d ", ptFontOffset.x, ptFontOffset.y ));

	m_pMsgFont->SetPosition( ptFontOffset.x, ptFontOffset.y );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ��Ʈ�� ��ġ ����~
	LTIntPt posCenter, OkPos, CancelPos;
	posCenter.x = (int)( ( m_Dlg.GetWidth()  - m_pBtnOk->GetBaseWidth()  ) * 0.5f );
	posCenter.y = (int)( ( m_Dlg.GetHeight() - m_pBtnOk->GetBaseHeight() ) * 0.5f );

	int nControlYPos = (int)( m_fHeightOfTopPoly + m_fHeightOfMidPoly + 
							  ( m_fHeightOfBotPoly - m_pBtnOk->GetBaseHeight() ) * 0.5f - 5 );

	m_bOnlyMessage = LTFALSE;

	m_pBtnOk->Show(LTFALSE);
	m_pBtnCancel->Show(LTFALSE);

	m_pBtnOk->Select(LTFALSE);
	m_pBtnCancel->Select(LTFALSE);
	
	switch( m_eType )
	{
		case LTMB_MESSAGE :
			{
				m_bOnlyMessage = LTTRUE;
			}break;
		case LTMB_OK :
			{
				OkPos.x = posCenter.x;
				OkPos.y = nControlYPos;
				m_Dlg.SetControlOffset(m_pBtnOk, OkPos);
				m_pBtnOk->Show(LTTRUE);
			}break;
		case LTMB_YESNO :
			{
				OkPos.x = (int)(posCenter.x - ( m_nControlInterval * 0.5f ));
				OkPos.y = (int)nControlYPos;
				m_Dlg.SetControlOffset(m_pBtnOk, OkPos);
				m_pBtnOk->Show(LTTRUE);
				
				CancelPos.x = (int)(posCenter.x + ( m_nControlInterval * 0.5f ));
				CancelPos.y = (int)OkPos.y;
				m_Dlg.SetControlOffset(m_pBtnCancel, CancelPos);
				m_pBtnCancel->Show(LTTRUE);
			}break;
	}
	//////////////////////////////////////////////////////////////////////////
	

	m_pCallback = pCreate->pFn;
	m_pData = pCreate->pData;

	m_Dlg.SetBasePos( (int)m_fOffsetXOfAllPoly, (int)m_fOffsetYOfTopPoly );
	m_Dlg.Show(LTTRUE);
	
	m_bVisible = LTTRUE;

    g_pInterfaceMgr->UpdateCursorState();
}


void CMessageBox::Close(LTBOOL bReturn)
{
	if (!m_bVisible)			return;

	m_bIgnoreAfter = LTFALSE;	// NOVICE

	m_bVisible = LTFALSE;
	m_Dlg.Show(LTFALSE);

	if (!m_bGameWasPaused)
        g_pGameClientShell->PauseGame(LTFALSE);

	if (m_pCallback)
	{
		m_pCallback(bReturn, m_pData);
	}

	g_pInterfaceMgr->UpdateCursorState();
}

void CMessageBox::Draw()
{
	if (m_bVisible)
	{
		g_pDrawPrim->DrawPrim( &m_PolyBack );

		g_pDrawPrim->SetTexture( m_TxMsgBoxBackFrame[MBF_TOP] );
		g_pDrawPrim->DrawPrim( &m_PolyMsgBoxBackFrame[MBF_TOP] );

		g_pDrawPrim->SetTexture( m_TxMsgBoxBackFrame[MBF_MIDDLE] );
		g_pDrawPrim->DrawPrim( &m_PolyMsgBoxBackFrame[MBF_MIDDLE] );

		//////////////////////////////////////////////////////////////////////////
		if ( m_bOnlyMessage )		// ��ư ���� �׳� �˸��޼����϶���
		{
			g_pDrawPrim->SetTexture( m_TxMsgBoxBackFrame[MBF_BOTTOM_ONLY_NOTICE] );
			g_pDrawPrim->DrawPrim( &m_PolyMsgBoxBackFrame[MBF_BOTTOM_ONLY_NOTICE] );
		}
		else
		{
			g_pDrawPrim->SetTexture( m_TxMsgBoxBackFrame[MBF_BOTTOM] );
			g_pDrawPrim->DrawPrim( &m_PolyMsgBoxBackFrame[MBF_BOTTOM] );
		}
		//////////////////////////////////////////////////////////////////////////

		m_Dlg.Render();

		m_pMsgFont->Render();
	}
}

LTBOOL CMessageBox::HandleChar(unsigned char c)
{
	if (!m_bVisible)	return LTFALSE;

	if ( m_eType == LTMB_EDIT )
	{
		return LTTRUE;
	}

	return LTFALSE;	
}

LTBOOL CMessageBox::HandleKeyDown(int key, int rep)
{
	if( !m_bVisible || LTMB_MESSAGE == m_eType )		return LTFALSE;

    LTBOOL handled = LTFALSE;

	switch (key)
	{
	case VK_LEFT:
		{
			break;
		}
	case VK_UP:
		{
			handled = m_Dlg.OnUp();
			break;
		}
	case VK_RIGHT:
		{
			break;
		}
	case VK_DOWN:
		{
			handled = m_Dlg.OnDown();
			break;
		}
	case VK_RETURN:
		{
			OnCommand_CmdOk( NULL, NULL );
			handled	= LTTRUE;
			break;
		}
	case VK_ESCAPE:
		{	
			OnCommand_CmdCancel( NULL, NULL );
			handled = LTTRUE;
			break;
		}
	default:
		{
			CLTGUICtrl* pCtrl = m_Dlg.GetSelectedControl();
			if (pCtrl)
				handled = pCtrl->HandleKeyDown(key,rep);
			else
				handled = LTFALSE;
			break;
		}
	}

	// Handled the key
	return handled;
}

LTBOOL CMessageBox::OnLButtonDown(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnLButtonDown( x, y);
	}
    return LTFALSE;
}

LTBOOL CMessageBox::OnLButtonUp(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnLButtonUp( x, y);
	}
    return LTFALSE;
}


LTBOOL CMessageBox::OnMouseMove(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnMouseMove(x,y);
	}
    return LTFALSE;
}

uint32 CMessageBox::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_OK:				return OnCommand_CmdOk( dwParam1, dwParam2 );				break;
	case CMD_CANCEL:			return OnCommand_CmdCancel( dwParam1, dwParam2 );			break;
	case CMD_MESSAGE_OK :		return OnCommand_CmdMessageOk( dwParam1, dwParam2 );		break;
	case CMD_MESSAGE_CANCEL :	return OnCommand_CmdMessageCancel( dwParam1, dwParam2 );	break;
	default:        
		break;
	}

	return LTFALSE;
}

uint32 CMessageBox::OnCommand_CmdOk( uint32 dwParam1, uint32 dwParam2 )
{
	Close(LTTRUE);
	return LTTRUE;
}

uint32 CMessageBox::OnCommand_CmdCancel( uint32 dwParam1, uint32 dwParam2 )
{
	Close(LTFALSE);
	return LTTRUE;
}

uint32 CMessageBox::OnCommand_CmdMessageOk( uint32 dwParam1, uint32 dwParam2 )
{
	Close(LTTRUE);
	return LTTRUE;
}

uint32 CMessageBox::OnCommand_CmdMessageCancel( uint32 dwParam1, uint32 dwParam2 )
{
	Close(LTFALSE);
	return LTTRUE;
}

//[MURSUM]=======================================================
void CMessageBox::ScreenDimsChanged()
{
	g_pDrawPrim->SetXYWH( &m_PolyBack, 0.0f, 0.0f,
						  (float)g_pInterfaceResMgr->GetScreenWidth(),
						  (float)g_pInterfaceResMgr->GetScreenHeight() );
}
//================================================================