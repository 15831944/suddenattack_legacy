//-------------------------------------------------------------------
// FILE : PopupHelp.cpp
// ����(ScreenMain)�� ���� ��ư�� ������ �� ������ ���� �˾�.

#include "stdafx.h"
#include "PopupHelp.h"
#include "ScreenCommands.h"

CPopupHelp::CPopupHelp()
{
	for(int i = 0; i < _HELPPOPUP_SUBBTN_; ++i)
		m_hHelpFrame[i] = LTNULL;

	m_pBtnOK					= LTNULL;

	m_pTabBtnHelpInterface		= LTNULL;
	m_pTabBtnHelpGamePlay		= LTNULL;

	for(i = 0; i < _HELPPOPUP_SUBBTN_; ++i)
		m_pSubBtn[i] = new CLTGUIButton;

	m_eSubBtnState				= HLPCMD_MOVEMENT; //���� ��ư �⺻ ����(���� �������̽�)
}

CPopupHelp::~CPopupHelp()
{
}

void CPopupHelp::Init( CLTGUICommandHandler * pCommandHandler )
{
	//---------------------------------------------------------------
	//�⺻ ���� ����
	LTIntPt BasePt;
	BasePt.x	= int(USABLE_WIDTH_F*0.5f-200.0f); //ȭ�� �߾�
	BasePt.y	= 50;

	//pScreenTag = "ScreenMain"
	char * pScreenTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName((eScreenID)SCREEN_ID_MAIN);

	//������� ���� �ؽ�ó �б�
	m_hHelpFrame[0] = g_pInterfaceResMgr->GetTexture( "SA_INTERFACE\\menu\\Textures\\help\\TxHelpBase.dtx" );
	m_hHelpFrame[1] = g_pInterfaceResMgr->GetTexture( "SA_INTERFACE\\menu\\Textures\\help\\FmAction.dtx" );
	m_hHelpFrame[2] = g_pInterfaceResMgr->GetTexture( "SA_INTERFACE\\menu\\Textures\\help\\FmChat.dtx" );
	m_hHelpFrame[3] = g_pInterfaceResMgr->GetTexture( "SA_INTERFACE\\menu\\Textures\\help\\FmHotkey.dtx" );
	m_hHelpFrame[4] = g_pInterfaceResMgr->GetTexture( "SA_INTERFACE\\menu\\Textures\\help\\FmBasic.dtx" );
	m_hHelpFrame[5] = g_pInterfaceResMgr->GetTexture( "SA_INTERFACE\\menu\\Textures\\help\\FmMission.dtx" );

	//��� ����� - �ʱⰪ��...
	Create( m_hHelpFrame[0], 512, 512, LTTRUE);
	SetBasePos(BasePt);
	Show(LTTRUE);

	//---------------------------------------------------------------
	//��ư ����
	//basic button
	m_pBtnOK	= new CLTGUIButton();
	AddButton( pScreenTag, m_pBtnOK, CMD_OK, "BtnOKTxN", "BtnOKTxH", "BtnOKTxD", "BtnOKTxC", "BtnRectHLPOk" );

	//tab button
	m_pTabBtnHelpInterface = new CLTGUIButton;
	AddCheckButton( pScreenTag , m_pTabBtnHelpInterface, HLPCMD_TAB_INTERFACE,"TabBtnITFRect", "TabBtnITFD", "TabBtnITFH");
	m_pTabBtnHelpGamePlay = new CLTGUIButton;
	AddCheckButton( pScreenTag , m_pTabBtnHelpGamePlay, HLPCMD_TAB_GAMEPLAY,"TabBtnGPRect", "TabBtnGPD", "TabBtnGPH");

	//sub button
	AddButton( pScreenTag, m_pSubBtn[0], HLPCMD_MOVEMENT, "BtnMoveTxN", "BtnMoveTxN", "BtnMoveTxN", "BtnMoveTxH", "BtnRectMove" );
	AddButton( pScreenTag, m_pSubBtn[1], HLPCMD_ACTION, "BtnActTxN", "BtnActTxN", "BtnActTxN", "BtnActTxH", "BtnRectAct" );
	AddButton( pScreenTag, m_pSubBtn[2], HLPCMD_CHAT, "BtnChatTxN", "BtnChatTxN", "BtnChatTxN", "BtnChatTxH", "BtnRectChat" );
	AddButton( pScreenTag, m_pSubBtn[3], HLPCMD_HOTKEY, "BtnHotkeyTxN", "BtnHotkeyTxN", "BtnHotkeyTxN", "BtnHotkeyTxH", "BtnRectHotkey" );
	AddButton( pScreenTag, m_pSubBtn[4], HLPCMD_BASIC, "BtnBasicTxN", "BtnBasicTxN", "BtnBasicTxN", "BtnBasicTxH", "BtnRectBasic" );
	AddButton( pScreenTag, m_pSubBtn[5], HLPCMD_MISSION, "BtnMissionTxN", "BtnMissionTxN", "BtnMissionTxN", "BtnMissionTxH", "BtnRectMission" );

	//��ư�� üũ�� ���·� �� �� �ְ� �Ѵ�.
	//���� ���� : (LTTRUE == m_bChecked && LTNULL != m_hChecked)
	//��� : m_hChecked = m_hClick; �Ͽ� Checked�� ������ �־���.
	for(int i = 0; i < _HELPPOPUP_SUBBTN_; ++i)
	{
		m_pSubBtn[i]->SetCheckedTexture();
		m_pSubBtn[i]->SetClickSound( (int) IS_CLICK_FLAT );
	}

	// button sound
	m_pTabBtnHelpInterface->SetClickSound( (int)IS_CLICK_FLAT );
	m_pTabBtnHelpGamePlay->SetClickSound( (int)IS_CLICK_FLAT );

	//---------------------------------------------------------------
	//�θ� ����
	//�θ� �����ؾ� �˾��� �� �� �θ� ������ ��.
	m_pCommandHandler	= pCommandHandler;

	SetParentScreen( (CScreenBase*)pCommandHandler );
}

void CPopupHelp::Render()
{
	if( IsVisible() == LTFALSE ) return;

	//���� ��� ���ϱ�
	RenderBackground(m_eSubBtnState);

	CLTGUIWindow::Render();
}

void CPopupHelp::RenderBackground(HELPPOPUPCOMMAND eState)
{
	m_Frame.SetFrame(m_hHelpFrame[eState - HLPCMD_MOVEMENT], LTFALSE);
}

void CPopupHelp::OnFocus(LTBOOL bFocus)
{
	if( bFocus )
	{
		HelpFrameSetting(HLPCMD_MOVEMENT);
		m_eSubBtnState = HLPCMD_MOVEMENT;
	}
	else
	{
		//--
	}
}

LTBOOL CPopupHelp::OnEnter()
{
	OnCommand( CMD_OK, LTNULL, LTNULL );
	return LTTRUE;
}

LTBOOL CPopupHelp::OnEscape()
{
	OnCommand( CMD_CANCEL, LTNULL, LTNULL );
	return LTTRUE;
}

uint32 CPopupHelp::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case HLPCMD_TAB_INTERFACE:
		if(HLPCMD_MOVEMENT == m_eSubBtnState || HLPCMD_ACTION == m_eSubBtnState ||
			HLPCMD_CHAT == m_eSubBtnState || HLPCMD_HOTKEY == m_eSubBtnState) break;
		HelpFrameSetting(HLPCMD_MOVEMENT);
		m_eSubBtnState = HLPCMD_MOVEMENT;
		break;
	case HLPCMD_TAB_GAMEPLAY:
		if(HLPCMD_BASIC == m_eSubBtnState || HLPCMD_MISSION == m_eSubBtnState) break;
		HelpFrameSetting(HLPCMD_BASIC);
		m_eSubBtnState = HLPCMD_BASIC;
		break;

	case HLPCMD_MOVEMENT:
		HelpFrameSetting(HLPCMD_MOVEMENT);
		m_eSubBtnState = HLPCMD_MOVEMENT;
		break;
	case HLPCMD_ACTION:
		HelpFrameSetting(HLPCMD_ACTION);
		m_eSubBtnState = HLPCMD_ACTION;
		break;
	case HLPCMD_CHAT:
		HelpFrameSetting(HLPCMD_CHAT);
		m_eSubBtnState = HLPCMD_CHAT;
		break;
	case HLPCMD_HOTKEY:
		HelpFrameSetting(HLPCMD_HOTKEY);
		m_eSubBtnState = HLPCMD_HOTKEY;
		break;
	case HLPCMD_BASIC:
		HelpFrameSetting(HLPCMD_BASIC);
		m_eSubBtnState = HLPCMD_BASIC;
		break;
	case HLPCMD_MISSION:
		HelpFrameSetting(HLPCMD_MISSION);
		m_eSubBtnState = HLPCMD_MISSION;
		break;

	case CMD_OK: //���͸� �����ų�, ��ư�� ������ �ǰ���
	case CMD_CANCEL:
		CtrlHide();
		break;
	}
	return 1;
}

void CPopupHelp::HelpFrameSetting(uint8 iSelect)
{
	//�ϴ� �ʱ�ȭ
	m_pTabBtnHelpInterface->SetCheck(LTFALSE);
	m_pTabBtnHelpGamePlay->SetCheck(LTFALSE);

	for(int i = 0; i < _HELPPOPUP_SUBBTN_; ++i)
	{
		m_pSubBtn[i]->Show(LTFALSE);		//���� �ڸ� ��ư�� ���ļ� �� ���̰�.
		m_pSubBtn[i]->SetCheck(LTFALSE);	//üũ�� ���� �ƴϰ�.
	}

	switch(iSelect) //�츱 �͸� Ȯ��
	{
	case HLPCMD_MOVEMENT:
		{
			m_pTabBtnHelpInterface->SetCheck(LTTRUE);

			m_pSubBtn[0]->Show(LTTRUE);	//�� ���̸� Ŭ���� �ȵȴ�.
			m_pSubBtn[1]->Show(LTTRUE);
			m_pSubBtn[2]->Show(LTTRUE);
			m_pSubBtn[3]->Show(LTTRUE);

			m_pSubBtn[0]->SetCheck();	//���� üũ�� �׸��� ����.
		}
		break;
	case HLPCMD_ACTION:
		{
			m_pTabBtnHelpInterface->SetCheck(LTTRUE);

			m_pSubBtn[0]->Show(LTTRUE);
			m_pSubBtn[1]->Show(LTTRUE);
			m_pSubBtn[2]->Show(LTTRUE);
			m_pSubBtn[3]->Show(LTTRUE);

			m_pSubBtn[1]->SetCheck();
		}
		break;
	case HLPCMD_CHAT:
		{
			m_pTabBtnHelpInterface->SetCheck(LTTRUE);

			m_pSubBtn[0]->Show(LTTRUE);
			m_pSubBtn[1]->Show(LTTRUE);
			m_pSubBtn[2]->Show(LTTRUE);
			m_pSubBtn[3]->Show(LTTRUE);

			m_pSubBtn[2]->SetCheck();
		}
		break;
	case HLPCMD_HOTKEY:
		{
			m_pTabBtnHelpInterface->SetCheck(LTTRUE);

			m_pSubBtn[0]->Show(LTTRUE);
			m_pSubBtn[1]->Show(LTTRUE);
			m_pSubBtn[2]->Show(LTTRUE);
			m_pSubBtn[3]->Show(LTTRUE);

			m_pSubBtn[3]->SetCheck();
		}
		break;

	case HLPCMD_BASIC:
		{
			m_pTabBtnHelpGamePlay->SetCheck(LTTRUE);

			m_pSubBtn[4]->Show(LTTRUE);
			m_pSubBtn[5]->Show(LTTRUE);

			m_pSubBtn[4]->SetCheck();
		}
		break;
	case HLPCMD_MISSION:
		{
			m_pTabBtnHelpGamePlay->SetCheck(LTTRUE);

			m_pSubBtn[4]->Show(LTTRUE);
			m_pSubBtn[5]->Show(LTTRUE);

			m_pSubBtn[5]->SetCheck();
		}
		break;
	}
}