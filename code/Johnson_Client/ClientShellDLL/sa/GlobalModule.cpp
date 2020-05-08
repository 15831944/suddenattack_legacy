#include "stdafx.h"
#include "GlobalModule.h"
#include "SAMissionButeMgr.h"
#include "..\\..\\Shared\\WinUtil.h"
#include "GameClientShell.h"

#include "GameMessage.h"
#include "GameText.h"


#define SCREEN_COMMON_TAG	"ScreenCommon"

CGlobalModule::CGlobalModule()
{
}

CGlobalModule::~CGlobalModule()
{
}

void CGlobalModule::GlobalInit(const char* szMission)
{
	int i;

	char szName[128] = {0, };
	for( i=0 ; i<MAX_GRADE_TEXTURE ; ++i )
	{
		sprintf( szName, "Sa_Interface\\Menu\\Textures\\GradeIcon\\Grade_%d.dtx", i );
		m_hGradeTexture[i] = g_pInterfaceResMgr->GetTexture( szName );
	}

	for( i=0 ; i<MAX_GRADE_TEXT ; ++i )
	{
		sprintf( szName, "GridString%d", i+1 );
		int nIdx = g_pLayoutMgr->GetInt(SCREEN_COMMON_TAG, szName, 142 );
		sprintf( m_szGradeText[i], NF_GetMsg(nIdx) );	//������ ������, �Ʒú����� ����
	}

	for( i=0 ; i<MAX_PING_TEXTURE ; ++i )
	{
		sprintf( szName, "Sa_Interface\\Menu\\Textures\\Common\\Ping_%d.dtx", i );
		m_hPingTexture[i] = g_pInterfaceResMgr->GetTexture( szName );
	}
}

HTEXTURE CGlobalModule::GetGradeTexture( int nGrade )
{
	if( nGrade < 0 || nGrade >= MAX_GRADE_TEXTURE ) nGrade = 0;

	return m_hGradeTexture[nGrade];
}

HTEXTURE CGlobalModule::GetPingTexture( int nPing )
{
	if( nPing < 0 || nPing >= MAX_PING_TEXTURE ) nPing = MAX_PING_TEXTURE-1;
	return m_hPingTexture[nPing];
}

const char*	CGlobalModule::GetGradeString( int nGrade )
{
	int nRealGrade = 0;

	if( nGrade < 5 )	//�纴
	{
		nRealGrade = nGrade;
	}
	else if( nGrade < 8 )	//�ϻ�(3)
	{
		nRealGrade = 5;
	}
	else if( nGrade < 12 )	//�߻�(4)
	{
		nRealGrade = 6;
	}
	else if( nGrade < 17 )	//���(5)
	{
		nRealGrade = 7;
	}
	else if( nGrade < 23 )	//����(6)
	{
		nRealGrade = 8;
	}
	else if( nGrade < 29 )	//����(6)
	{
		nRealGrade = 9;
	}
	else if( nGrade < 35 )	//����(6)
	{
		nRealGrade = 10;
	}
	else if( nGrade < 41 )	//�ҷ�(6)
	{
		nRealGrade = 11;
	}
	else if( nGrade < 47 )	//�߷�(6)
	{
		nRealGrade = 12;
	}
	else if( nGrade < 53 )	//���(6)
	{
		nRealGrade = 13;
	}
	else if( nGrade < 58 )	//��
	{
		//14~18
		nRealGrade = nGrade-40;
	}
	
	if( nRealGrade < 0 || nRealGrade >= 20 ) nRealGrade = 0;

	return m_szGradeText[nRealGrade];
}
