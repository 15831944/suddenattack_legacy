#include "stdafx.h"
#include "FinalGameText.h"
#include "GameText.h"

CFinalGameText* CFinalGameText::m_pInstance = NULL;

CFinalGameText::CFinalGameText()
{
	m_pArrFinalGameTextHeader	= NULL;
	m_pWholeContext				= NULL;
	m_dwHeaderCount				= NULL;
}

CFinalGameText::~CFinalGameText()
{
	if ( NULL != m_pArrFinalGameTextHeader )
	{
		delete [] m_pArrFinalGameTextHeader;
		m_pArrFinalGameTextHeader	= NULL;
	}

	if( NULL != m_pWholeContext )
	{
		delete [] m_pWholeContext;
		m_pWholeContext	= NULL;
	}
}

CFinalGameText* CFinalGameText::GetInstance()
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new CFinalGameText();
	}

	return m_pInstance;
}

BOOL CFinalGameText::Destroy()
{
	if ( NULL != m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}

	return TRUE;
}

LPCTSTR CFinalGameText::GetMsg( DWORD dwIdx )
{
	if ( dwIdx > m_dwHeaderCount ) return "";

	return (NULL != (m_pWholeContext + m_pArrFinalGameTextHeader[dwIdx].dwOffset)) ?
		(m_pWholeContext + m_pArrFinalGameTextHeader[dwIdx].dwOffset) : "";
}

BOOL CFinalGameText::SetHeaderWithParsing( LPVOID lpHeaderBuf )
{
	DWORD dwHeaderCount		= 0;
	DWORD dwHeaderBufOffset = 0;
	DWORD dwContextOffset	= 0;

	// �� ó�� ��� ���� �����´�.
	CopyMemory( &dwHeaderCount, lpHeaderBuf, sizeof(DWORD) );
	dwHeaderBufOffset += sizeof(DWORD);

	m_dwHeaderCount = dwHeaderCount - 1;

	m_pArrFinalGameTextHeader = new FinalGameTextHeader[dwHeaderCount + 1];
	ZeroMemory( m_pArrFinalGameTextHeader, sizeof(FinalGameTextHeader) * ( dwHeaderCount + 1 ) );
	
	for ( DWORD dwCnt = 0; dwCnt < dwHeaderCount; dwCnt++ )
	{
		CopyMemory( &m_pArrFinalGameTextHeader[dwCnt].dwIdx, (char*)lpHeaderBuf + dwHeaderBufOffset, sizeof(DWORD) );
		dwHeaderBufOffset += sizeof(DWORD);						// �ε�������
		dwHeaderBufOffset += sizeof(DWORD) * 2;					// ���������� �پ�Ѱ�,
		dwHeaderBufOffset += sizeof(DWORD) * MAX_MESSAGE_LANG;	// ���� �޼����� ������ �پ�Ѱ�,
		dwHeaderBufOffset += sizeof(DWORD) * NLC_NATION;		// �ش籹�� �޼��� �������� ��������,

		m_pArrFinalGameTextHeader[dwCnt+1].dwOffset =
			(DWORD)(*((DWORD*)((char*)lpHeaderBuf + dwHeaderBufOffset))) + dwContextOffset + 1;	// Null Character Included

		dwContextOffset	  = m_pArrFinalGameTextHeader[dwCnt+1].dwOffset;

		dwHeaderBufOffset += (sizeof(DWORD) * (MAX_MESSAGE_LANG - NLC_NATION));	// ������ ����� �پ�Ѱ�,
	}

	m_dwWholeContextSize = dwContextOffset;

	return TRUE;
}

BOOL CFinalGameText::SetContextWithParsing( LPVOID lpHeaderInfoBuf, LPVOID lpContextBuf )
{
	// ���� �ؽ�Ʈ ������� ��������..
	DWORD dwHeaderCount		= 0;
	DWORD dwContextOffset	= 0;
	CopyMemory( &dwHeaderCount, lpHeaderInfoBuf, sizeof(DWORD) );

	// �� ó�� ������ �ǳʶٰ�, �� �ں��� ���������.
	TCHAR*				pGameTextContext	= (TCHAR*)lpContextBuf;
	LPGameTextHeader	pGameTextHeaderInfo = (LPGameTextHeader)((char*)lpHeaderInfoBuf + sizeof(DWORD));

	m_pWholeContext = new char[m_dwWholeContextSize];
	ZeroMemory( m_pWholeContext, m_dwWholeContextSize );
	
	LPCTSTR lpCurMsgPos		= NULL;
	DWORD	dwCurMsgLength	= 0;

	pGameTextContext += ( dwHeaderCount * sizeof(GameTextHeader) ) + 12;

	for ( DWORD dwCnt = 0; dwCnt < dwHeaderCount; dwCnt++ )
	{
		lpCurMsgPos		= pGameTextContext + pGameTextHeaderInfo[dwCnt].dwOffset[NLC_NATION];
		dwCurMsgLength	= lstrlen( lpCurMsgPos );

		CopyMemory( &m_pWholeContext[dwContextOffset], lpCurMsgPos, dwCurMsgLength );
		dwContextOffset += dwCurMsgLength + 1;		// Null Character Included
	}

	return TRUE;
}

