//-------------------------------------------------------------------
// FILE : PopupHelp.h
// ���ǰ� ��(ScreenMain, ScreenRoom)�� ���� ��ư�� ������ �� ������ �˾�.
// ��Ű�� ������� �ʰ� KeyDown�� ���� ����Ѵ�.

#ifndef __YAIIN_POPUP_HELP_H_
#define __YAIIN_POPUP_HELP_H_

#include "PopupScreenBase.h"
#include "..\\..\\Libs\\LTGUIMgr\\ltguiwindow.h"

#define _HELPPOPUP_SUBBTN_	6

enum HELPPOPUPCOMMAND
{
	HLPCMD_TAB_INTERFACE = 101,
	HLPCMD_TAB_GAMEPLAY,

	//���� ����~!
	HLPCMD_MOVEMENT,           // Btn, Frm - 0
	HLPCMD_ACTION,             // Btn, Frm - 1
	HLPCMD_CHAT,               // Btn, Frm - 2
	HLPCMD_HOTKEY,             // Btn, Frm - 3
	HLPCMD_BASIC,              // Btn, Frm - 4
	HLPCMD_MISSION,            // Btn, Frm - 5
};


class CPopupHelp : public CPopScreenBase
{
public:
	CPopupHelp();
	virtual ~CPopupHelp();

	void		Init( CLTGUICommandHandler*	pCommandHandler );
	void		Render();
	void		OnFocus(LTBOOL bFocus);

protected:
	LTBOOL		OnEnter();
	LTBOOL		OnEscape();
    uint32		OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void		HelpFrameSetting(uint8 iSelect);
	
	void		RenderBackground(HELPPOPUPCOMMAND eState);

	//background
	HTEXTURE				m_hHelpFrame[_HELPPOPUP_SUBBTN_];
	
	//button
	CLTGUIButton *			m_pBtnOK;

	CLTGUIButton *			m_pTabBtnHelpInterface;	//tab_1
	CLTGUIButton *			m_pTabBtnHelpGamePlay;	//tab_2
	CLTGUIButton *			m_pSubBtn[_HELPPOPUP_SUBBTN_]; //sub_0~5

	CLTGUICommandHandler*	m_pCommandHandler;

	HELPPOPUPCOMMAND		m_eSubBtnState; //�̹� ������ ��ư�� ������ ��ȭ ���� ��.
};


#endif