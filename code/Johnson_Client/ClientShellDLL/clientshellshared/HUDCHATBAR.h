// ----------------------------------------------------------------------- //
//
// MODULE  : HUDChatbar.h
//
// PURPOSE : �ƸӰ������� �ｺ������
//
// Author  : monk77
//
// GMOS 2003.12.01
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_CHATBAR_H
#define __HUD_CHATBAR_H

//->Source �߰� Start.[�߰��Ǿ��� ����:01.12.2003]
//������ ������� �߰�
#include "ilttexinterface.h"
#include "cuifont.h"
#include "iltfontmanager.h"
#include "cuipolystring.h"

#include "LayoutMgr.h"
//->Source �߰� End.  [�߰�       ���:monk77]  [#|#]

class CHUDCHATBAR  
{
public:
	CUIFont*		pMonoFont; 
	HTEXTURE 		hTex;
	CUIPolyString*	pMonoString[5];

	CHUDCHATBAR();

	virtual ~CHUDCHATBAR();

public:
	void DrawBar();
	void DrawIconHeal();
	void DrawIconArm();

	void UpdateLayout();
	void DrawHeal(char* pBuffer);
	void Render();

	void DrawArm(char* pBuffer);

	void Term();

	void Init(const char *szTexName);

private:
	LTBOOL	m_ChatbarText;
	LTIntPt	m_ChatbarTextSize;
	LTIntPt m_ChatbarTextSizeWidth;
};

#endif
