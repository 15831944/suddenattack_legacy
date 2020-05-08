//-------------------------------------------------------------------
// FILE : HUDHelp.h
// ���� : ���� �� F1�� ���� ���� HUD�� ������ ��.
// ���� : �ٸ� Ȱ��/��Ȱ�� HUD�� ��� OPTION�� [Ű����]�� ����Ǿ� �ִ�.
//        �׷��� ������ ���� �ϰ������� F1�� ����Ѵ�.
//        ���� ��� VK_TAB�� ���� ��Ÿ���� HUDScore�� [Ű����]�� ����
//        VK_TAB�� ������ COMMAND_ID_MISSION�� OnCommand(On/Off)�� �޾� ó���Ѵ�.
//        �׷��� HUDHelp�� ���� OnKey(Down/Up)�� �̿��� ���� ó���Ѵ�.
//        (CInterfaceMgr::OnKeyDown, OnKeyUp ����)

#ifndef __YAIIN_HUD_HELP_H_
#define __YAIIN_HUD_HELP_H_

#include "HUDItem.h"
#include "ltguimgr.h"
#include "../../Shared/LTPoly.h"

class CHUDHelp : public CHUDItem
{
public:
	CHUDHelp();
	virtual ~CHUDHelp();

	LTBOOL				Init(void);
	void				Show(LTBOOL bView);
	void				Render(void);
	void				ScreenDimChanged(void);
	void				Update(void);
	void				PrePlayingState(void);

	LTBOOL				IsVisible(void);

private:
	HTEXTURE			m_hHelp[2];
	LTPoly_GT4			m_PolyHelp[2];
	LTBOOL				m_bView;
	
	float				m_fHUDHelpPos;
};

#endif