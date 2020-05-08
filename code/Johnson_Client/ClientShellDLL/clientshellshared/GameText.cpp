// GameText.cpp: implementation of the CGameText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameText.h"
#include "FinalGameText.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////

char* NF_GetMsg( DWORD dwIdx )
{
	return (char*)NF_FINAL_GAME_TEXT()->GetMsg( dwIdx );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGameText* CGameText::m_pInstance = NULL;		// �̱���� �ν��Ͻ� �ʱ�ȭ

CGameText::CGameText()							// ������
{
	m_dwTotMsgCnt = 0;

	m_dwGameTextContextSize			= 0;
	m_dwGameTextContextHeaderSize	= 0;

	ZeroMemory( m_arrGameText, sizeof(m_arrGameText) );
	ZeroMemory( m_arrGameTextHeader, sizeof(m_arrGameTextHeader) );

	m_pGameTextContext = new TCHAR[ MAX_CONTEXT_SIZE ];
	ZeroMemory( m_pGameTextContext, MAX_CONTEXT_SIZE );

	m_pGameTextContextHeader = new TCHAR[ MAX_CONTEXT_HEADER_SIZE ];
	ZeroMemory( m_pGameTextContextHeader, MAX_CONTEXT_HEADER_SIZE );

	m_pGameTextRefHeader = new TCHAR[ MAX_CONTEXT_SIZE ];
	ZeroMemory( m_pGameTextRefHeader, MAX_CONTEXT_SIZE );
}

CGameText::~CGameText()							// �Ҹ���
{
	if ( NULL != m_pGameTextContext )
	{	
		delete [] m_pGameTextContext;
		m_pGameTextContext = NULL;
	}
	
	if ( NULL != m_pGameTextContextHeader)
	{
		delete [] m_pGameTextContextHeader;
		m_pGameTextContextHeader = NULL;
	}
	
	if ( NULL != m_pGameTextRefHeader)
	{
		delete [] m_pGameTextRefHeader;
		m_pGameTextRefHeader = NULL;
	}
}

CGameText* CGameText::GetInstance()				// �̱��� ��ü �޾ƿ��� ��
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new CGameText();
	}
	
	return m_pInstance;
}

BOOL CGameText::Destroy()						// �̱��� ���ֹ����� ��
{
	if ( NULL != m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = NULL;
		return TRUE;
	}
	
	return FALSE;
}

BOOL CGameText::IsExist( LPCTSTR lpDefinition )
{
	for ( DWORD dwCnt = 0; dwCnt < MAX_MESSAGE_CNT; dwCnt++ )
	{
		if ( 0 != dwCnt && 0 == m_arrGameText[dwCnt].dwIdx )		break;

		if ( NULL == lstrcmp( m_arrGameText[dwCnt].txtDefine, lpDefinition ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CGameText::Insert( LPGameText pGameText )
{
	DWORD dwIdx = m_dwTotMsgCnt;

	pGameText->dwIdx	= dwIdx;

	// Insert �� �߰��� ���� ���� �ִٸ� �߰����� �ʴ´�.
	if ( ( NULL != m_arrGameText[dwIdx].txtMessage[NLC_KOREA][0] ) ||
		 ( NULL != m_arrGameText[dwIdx].txtMessage[NLC_CHINA][0] ) ||
		 ( NULL != m_arrGameText[dwIdx].txtMessage[NLC_JAPAN][0] )  )
	{
		return FALSE;
	}
	
	Update( pGameText );

	m_dwTotMsgCnt++;

	return TRUE;
}

BOOL CGameText::Remove( DWORD dwIdx )
{
	// �����.
	ZeroMemory( &m_arrGameText[dwIdx], sizeof(m_arrGameText[dwIdx]) );

	m_dwTotMsgCnt--;
	
	return TRUE;
}

void CGameText::Update( DWORD dwArrIdx, DWORD dwRealIdx )
{
	m_arrGameText[dwArrIdx].dwIdx = dwRealIdx;
}

void CGameText::Update( DWORD dwIdx, LPCTSTR lpszDefine )
{
	// Define ���ڿ��� �����Ѵ�.
	_tcscpy( m_arrGameText[dwIdx].txtDefine, lpszDefine );
}

void CGameText::Update( DWORD dwIdx, LPCTSTR lpszMsg, EN_NATIONAL_LANGUAGE_CODE nlCode )
{
	// ���� �Ѱ��� �޼����� �����Ѵ�.
	_tcscpy( m_arrGameText[dwIdx].txtMessage[nlCode],	lpszMsg );
}

void CGameText::Update( LPGameText pGameText )
{
	// �޼��� ����!!!
	DWORD dwIdx = pGameText->dwIdx;

	Update( dwIdx, pGameText->dwIdx );
	Update( dwIdx, pGameText->txtDefine );
	Update( dwIdx, pGameText->txtMessage[NLC_KOREA], NLC_KOREA );
	Update( dwIdx, pGameText->txtMessage[NLC_CHINA], NLC_CHINA );
	Update( dwIdx, pGameText->txtMessage[NLC_JAPAN], NLC_JAPAN );
}

DWORD CGameText::GetTotMsgCnt()
{
	// ���� �޽��� ũ�⸦ ��ȯ�Ѵ�.
	return m_dwTotMsgCnt;
}

BOOL CGameText::GetRow( DWORD dwIdx, OUT LPGameText pGameText )
{
	// �����ؽ�Ʈ ���� ( ��ü ������ ) �� �����´�.
	if ( dwIdx > m_dwTotMsgCnt )		return FALSE;
	if ( NULL == pGameText )			return FALSE;

	pGameText->dwIdx = m_arrGameText[dwIdx].dwIdx;

	_tcscpy( pGameText->txtDefine, m_arrGameText[dwIdx].txtDefine );

	for ( int nlCode = 0; nlCode < MAX_MESSAGE_LANG; nlCode++ )
	{
		_tcscpy( pGameText->txtMessage[nlCode], m_arrGameText[dwIdx].txtMessage[nlCode] );
	}

	return TRUE;
}

LPCTSTR	CGameText::GetMsg( EN_NATIONAL_LANGUAGE_CODE nlCode, DWORD dwIdx )
{
	if ( dwIdx > GetTotMsgCnt() || nlCode > ( MAX_MESSAGE_LANG - 1 ) )
	{
		return "";
	}
	
	return m_arrGameText[dwIdx].txtMessage[nlCode];
}

LPVOID CGameText::GetContextHeader()
{
	MakeContextHeader();

	return m_pGameTextContextHeader;
}

LPVOID CGameText::GetContext()
{
	ProcessingArrayToContext();
	
	return m_pGameTextContext;
}

LPVOID CGameText::GetRefHeader()
{
	ProcessingArrayToGameHeader();

	return m_pGameTextRefHeader;
}

void CGameText::ProcessingArrayToContext()
{
	// �迭�� ����ִ� ���� ��Ʈ������ ��ȯ
	DWORD	dwContextOffset	= NULL;

	// ���ؽ�Ʈ ����� ����� ���� �������� ����
	dwContextOffset = MakeContextHeader();

	// Context �� Header ���� �ֱ�.
	dwContextOffset = WriteHeaderIntoContext( dwContextOffset );

	// ���ؽ�Ʈ ���۷�, �迭 ���� ��Ʈ��ȭ �ؼ� �ִ´�.	
	dwContextOffset = WriteBodyIntoContext( dwContextOffset );
}

DWORD CGameText::MakeContextHeader()
{
	// ���ؽ�Ʈ ����� ���ۿ� ����.
	// ���۴� ����� �����ִ� ��..
	DWORD dwCnt				= NULL;
	DWORD dwContextOffset	= NULL;
	DWORD dwTotHeaderInfo	= NULL;


	// ���ؽ�Ʈ �ٵ� �ֿ�.. ���鼭 ��� ������ �����.
	for ( dwCnt = 0; dwCnt < GetTotMsgCnt(); dwCnt++ )
	{
		// ���ؽ�Ʈ ��� ���� �����ϸ鼭, ���ؽ�Ʈ ��ġ ��� �����Ѵ�.
		dwContextOffset = MakeHeaderFromBody( dwCnt, dwContextOffset );
	}
	

	// ���� ���������.. ����.
	dwContextOffset = 0;
	dwTotHeaderInfo = GetTotMsgCnt();
	CopyMemory( &m_pGameTextContextHeader[dwContextOffset],		// ��������� ����
				&dwTotHeaderInfo,
				sizeof(DWORD) );
	dwContextOffset += sizeof(DWORD);

	CopyMemory( &m_pGameTextContextHeader[dwContextOffset],		// ���� ��� ����
				&m_arrGameTextHeader,
				sizeof(GameTextHeader) * GetTotMsgCnt() );
	dwContextOffset += sizeof(GameTextHeader) * GetTotMsgCnt();
	
	return dwContextOffset;
}

DWORD CGameText::MakeHeaderFromBody( DWORD dwCnt, DWORD dwContextOffset )
{
	// �迭 ���� ������ ����� ���� ����, ���(m_arrGameTextHeader) �� �ִ´�.
	m_arrGameTextHeader[dwCnt].dwIdx = m_arrGameText[dwCnt].dwIdx;
	dwContextOffset += sizeof(m_arrGameText[dwCnt].dwIdx);
	
	m_arrGameTextHeader[dwCnt].dwDefineOffset = dwContextOffset;
	m_arrGameTextHeader[dwCnt].dwDefineLength = lstrlen( m_arrGameText[dwCnt].txtDefine );
	dwContextOffset += m_arrGameTextHeader[dwCnt].dwDefineLength + 1;
	
	m_arrGameTextHeader[dwCnt].dwOffset[NLC_KOREA] = dwContextOffset;
	m_arrGameTextHeader[dwCnt].dwLength[NLC_KOREA] = lstrlen( m_arrGameText[dwCnt].txtMessage[NLC_KOREA] );
	dwContextOffset += m_arrGameTextHeader[dwCnt].dwLength[NLC_KOREA] + 1;
	
	m_arrGameTextHeader[dwCnt].dwOffset[NLC_CHINA] = dwContextOffset;
	m_arrGameTextHeader[dwCnt].dwLength[NLC_CHINA] = lstrlen( m_arrGameText[dwCnt].txtMessage[NLC_CHINA] );
	dwContextOffset += m_arrGameTextHeader[dwCnt].dwLength[NLC_CHINA] + 1;
	
	m_arrGameTextHeader[dwCnt].dwOffset[NLC_JAPAN] = dwContextOffset;
	m_arrGameTextHeader[dwCnt].dwLength[NLC_JAPAN] = lstrlen( m_arrGameText[dwCnt].txtMessage[NLC_JAPAN] );
	dwContextOffset += m_arrGameTextHeader[dwCnt].dwLength[NLC_JAPAN] + 1;

	return dwContextOffset;
}

DWORD CGameText::WriteHeaderIntoContext( DWORD dwContextOffset )
{
	TCHAR	szBuf[32]		= {0,};
	DWORD	dwBufSize		= NULL;

	// ���ؽ�Ʈ �ȿ� ��� ����ְ�,
	CopyMemory( m_pGameTextContext, m_pGameTextContextHeader, dwContextOffset );
	
	// ����� �����ٴ� ǥ�ø� �־��ش�.
	lstrcpy( szBuf, END_OF_GAMETEXT_HEADER );
	dwBufSize = lstrlen( szBuf );
	
	CopyMemory( &m_pGameTextContext[dwContextOffset], szBuf, dwBufSize );
	dwContextOffset += dwBufSize;
	
	return dwContextOffset;
}

DWORD CGameText::WriteBodyIntoContext( DWORD dwContextOffset )
{
	TCHAR	szBuf[4096]		= {0,};
	DWORD	dwBufSize		= NULL;

	for ( DWORD dwCnt = 0; dwCnt < GetTotMsgCnt(); dwCnt++ )
	{
		CopyMemory( &m_pGameTextContext[dwContextOffset],
					&m_arrGameText[dwCnt].dwIdx,
					sizeof(m_arrGameText[dwCnt].dwIdx) );
		
		dwContextOffset += sizeof(m_arrGameText[dwCnt].dwIdx);
		
		lstrcpy( szBuf, m_arrGameText[dwCnt].txtDefine );
		dwBufSize = lstrlen( m_arrGameText[dwCnt].txtDefine );					// ����� ������ ����ؼ�
		CopyMemory( &m_pGameTextContext[dwContextOffset], szBuf, dwBufSize );	// ���ؽ�Ʈ�� ����
		dwContextOffset += dwBufSize + 1;	// ������ ��ġ �̵� ( ���� �ι��� �ϳ� ���� )
		
		lstrcpy( szBuf, m_arrGameText[dwCnt].txtMessage[NLC_KOREA] );
		dwBufSize = lstrlen( m_arrGameText[dwCnt].txtMessage[NLC_KOREA] );		// ����� ������ ����ؼ�
		CopyMemory( &m_pGameTextContext[dwContextOffset], szBuf, dwBufSize );	// ���ؽ�Ʈ�� ����
		dwContextOffset += dwBufSize + 1;	// ������ ��ġ �̵� ( ���� �ι��� �ϳ� ���� )
		
		lstrcpy( szBuf, m_arrGameText[dwCnt].txtMessage[NLC_CHINA] );
		dwBufSize = lstrlen( m_arrGameText[dwCnt].txtMessage[NLC_CHINA] );		// ����� ������ ����ؼ�
		CopyMemory( &m_pGameTextContext[dwContextOffset], szBuf, dwBufSize );	// ���ؽ�Ʈ�� ����
		dwContextOffset += dwBufSize + 1;	// ������ ��ġ �̵� ( ���� �ι��� �ϳ� ���� )

		lstrcpy( szBuf, m_arrGameText[dwCnt].txtMessage[NLC_JAPAN] );
		dwBufSize = lstrlen( m_arrGameText[dwCnt].txtMessage[NLC_JAPAN] );		// ����� ������ ����ؼ�
		CopyMemory( &m_pGameTextContext[dwContextOffset], szBuf, dwBufSize );	// ���ؽ�Ʈ�� ����
		dwContextOffset += dwBufSize + 1;	// ������ ��ġ �̵� ( ���� �ι��� �ϳ� ���� )
	}
	
	// ������ �κп� ���ؽ�Ʈ �����ٰ� �˸���.
	CopyMemory( &m_pGameTextContext[dwContextOffset], END_OF_GAMETEXT_CONTEXT, sizeof(TCHAR) * 8 );
	
	// ������ �κп� ���ؽ�Ʈ ���ܹ��ڿ�, 1 ����Ʈ�� ������ �ش�.
	m_dwGameTextContextSize = dwContextOffset + ( sizeof(TCHAR) * 8 ) + 1;

	return dwContextOffset;
}

void CGameText::SetContext( LPVOID lpContext, DWORD dwContextSize )
{
	m_pTmpGameTextContext		= (TCHAR*)lpContext;
	m_dwTmpGameTextContextSize	= dwContextSize;
	
	m_dwTotMsgCnt = 0;
	
	ZeroMemory( m_arrGameTextHeader, sizeof(m_arrGameTextHeader) );
	ZeroMemory( m_arrGameText, sizeof(m_arrGameText) );

	ProcessingContextToArray();
}

void CGameText::ProcessingContextToArray()
{
	DWORD dwContextOffset = NULL;
	
	AnalyzeContextHeader();
}

void CGameText::AnalyzeContextHeader()
{
	// �о�� ���ؽ�Ʈ ����� �м�
	DWORD	dwCnt				= NULL;
	DWORD	dwContextOffset		= NULL;
	DWORD	dwTotHeaderInfo		= NULL;
	DWORD	dwBufSize			= NULL;
	TCHAR	szBuf[32]			= {0,};
	
	
	CopyMemory( &dwTotHeaderInfo,			// �� ó��.. ��� ����ִ��� �����´�.
				m_pTmpGameTextContext,
				sizeof(DWORD) );
	dwContextOffset += sizeof(DWORD);

	
	CopyMemory( m_arrGameTextHeader,		// ������ ������, ���� �ؽ�Ʈ ����� �����.
				&m_pTmpGameTextContext[dwContextOffset],
				sizeof(GameTextHeader) * dwTotHeaderInfo );
	dwContextOffset += sizeof(GameTextHeader) * dwTotHeaderInfo;

	// ��� ���ܺκ� Ȯ��..
	if ( 0 == lstrcmp( &m_pTmpGameTextContext[dwContextOffset], END_OF_GAMETEXT_HEADER ) )
	{
		dwContextOffset += lstrlen( END_OF_GAMETEXT_HEADER );
	}
	else
	{
		// ����~!! ���� ��������~!! -_-;;
#ifndef _FINAL
		MessageBox( NULL, "File Header was Infected !!!\n\nIt will be Fixed !!!",
					"Error", MB_OK | MB_ICONEXCLAMATION );
#endif //_FINAL

		int nRet = -1;

		// ��� �մܿ� ���ִ� ������ŭ�� �츮��, �� �������ʹ� ������.
		while ( 0 != nRet )
		{
			nRet = lstrcmp( END_OF_GAMETEXT_HEADER, &m_pTmpGameTextContext[dwContextOffset] );
			dwContextOffset++;
		};
		
		dwContextOffset += lstrlen( END_OF_GAMETEXT_HEADER );
	}

	// ��� ������ ������, �ٵ� �����..
	for ( dwCnt = 0; dwCnt < dwTotHeaderInfo; dwCnt++ )
	{
		dwContextOffset = WriteContextIntoBody( dwCnt, dwContextOffset );
	}
	
}

DWORD CGameText::WriteContextIntoBody( DWORD dwCnt, DWORD dwContextOffset )
{
	// ���ؽ�Ʈ�� �о� GameText �迭��..
	CopyMemory( &m_arrGameText[dwCnt].dwIdx,
				&m_pTmpGameTextContext[dwContextOffset],
				sizeof(DWORD) );
	dwContextOffset += sizeof(DWORD);
	
	CopyMemory( m_arrGameText[dwCnt].txtDefine,
				&m_pTmpGameTextContext[dwContextOffset],
				m_arrGameTextHeader[dwCnt].dwDefineLength );
	dwContextOffset += m_arrGameTextHeader[dwCnt].dwDefineLength + 1;
	
	CopyMemory( m_arrGameText[dwCnt].txtMessage[NLC_KOREA],
				&m_pTmpGameTextContext[dwContextOffset],
				m_arrGameTextHeader[dwCnt].dwLength[NLC_KOREA] );
	dwContextOffset += m_arrGameTextHeader[dwCnt].dwLength[NLC_KOREA] + 1;
	
	CopyMemory( m_arrGameText[dwCnt].txtMessage[NLC_CHINA],
				&m_pTmpGameTextContext[dwContextOffset],
				m_arrGameTextHeader[dwCnt].dwLength[NLC_CHINA] );
	dwContextOffset += m_arrGameTextHeader[dwCnt].dwLength[NLC_CHINA] + 1;
	
	CopyMemory( m_arrGameText[dwCnt].txtMessage[NLC_JAPAN],
				&m_pTmpGameTextContext[dwContextOffset],
				m_arrGameTextHeader[dwCnt].dwLength[NLC_JAPAN] );
	dwContextOffset += m_arrGameTextHeader[dwCnt].dwLength[NLC_JAPAN] + 1;

	m_dwTotMsgCnt++;
	
	return dwContextOffset;
}

void CGameText::ProcessingArrayToGameHeader()
{
	TCHAR		szBuf[512]			= {0,};
	TCHAR		szAdjustBuf[512]	= {0,};
	TCHAR		szCaptionBuf[512]	= {0,};
	DWORD		dwAdjustLength		= 0;
	DWORD		dwParseCnt			= 0;
	DWORD		dwFilteredCnt		= 0;
	GameText	stGameText;
	
	SYSTEMTIME st;
	GetLocalTime( &st );

	wsprintf( szBuf, "%d / %d / %d\t%.2d:%.2d:%.2d",
			  st.wYear, st.wMonth, st.wDay,
			  st.wHour, st.wMinute, st.wSecond );

	lstrcpy( m_pGameTextRefHeader, 
			"#ifndef __SUDDEN_ATTACK_GAME_TEXT_HEADER__\r\n"
			"#define __SUDDEN_ATTACK_GAME_TEXT_HEADER__\r\n"
			"\r\n"
			"// Sudden Attack Game Text Header File\r\n"
			"// Generated by Game Text Manager\r\n"
			"// Last Generated : " );

	lstrcat( m_pGameTextRefHeader, szBuf );
	lstrcat( m_pGameTextRefHeader, "\r\n\r\n\r\n" );



	for ( DWORD dwCnt = 0; dwCnt < MAX_MESSAGE_CNT; dwCnt++ )
	{
		GetRow( dwCnt, &stGameText );

		if ( NULL == stGameText.txtDefine[0] )	break;

		dwParseCnt		= 0;
		dwFilteredCnt	= 0;
		ZeroMemory( szCaptionBuf, sizeof(szCaptionBuf) );

		do
		{
			switch ( stGameText.txtMessage[NLC_KOREA][dwParseCnt] )
			{
			case '\r':
				{
					szCaptionBuf[dwParseCnt + dwFilteredCnt] = '\\';	dwFilteredCnt++;
					szCaptionBuf[dwParseCnt + dwFilteredCnt] = 'r';		
				}
				break;
			case '\n':
				{
					szCaptionBuf[dwParseCnt + dwFilteredCnt] = '\\';	dwFilteredCnt++;
					szCaptionBuf[dwParseCnt + dwFilteredCnt] = 'n';		
				}
				break;
			default:
				{
					szCaptionBuf[dwParseCnt + dwFilteredCnt] = stGameText.txtMessage[NLC_KOREA][dwParseCnt];
				}
				break;
			}

			dwParseCnt++;

		} while ( NULL != stGameText.txtMessage[NLC_KOREA][dwParseCnt] );

		wsprintf( szAdjustBuf, "#define %s ", stGameText.txtDefine );

		dwAdjustLength	= lstrlen( szAdjustBuf );

		for ( DWORD dwAdjustCnt = 0; dwAdjustCnt < (13 - (DWORD)(dwAdjustLength / 4)); dwAdjustCnt++ )
		{
			lstrcat( szAdjustBuf, "\t" );
		}

		wsprintf( szBuf, "%s %u\t\t// \"%s\"\r\n",
				  szAdjustBuf, stGameText.dwIdx, szCaptionBuf );
		
		lstrcat( m_pGameTextRefHeader, szBuf );
	}


	lstrcat( m_pGameTextRefHeader,
			"\r\n\r\n\r\n"
			"#endif // GAME TEXT MANAGER GENERATED MESSAGE HEADER!!!\r\n" );

	m_dwGameTextRefHeaderSize = lstrlen( m_pGameTextRefHeader );
}


