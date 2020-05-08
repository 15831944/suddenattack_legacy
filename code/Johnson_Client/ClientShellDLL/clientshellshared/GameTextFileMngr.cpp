// GameTextFileMngr.cpp: implementation of the CGameTextFileMngr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameTextFileMngr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGameTextFileMngr* CGameTextFileMngr::m_pInstance = NULL;	// �̱���� �ν��Ͻ� �ʱ�ȭ

CGameTextFileMngr::CGameTextFileMngr()						// ������
{
	m_hFile			= INVALID_HANDLE_VALUE;
	m_bSave			= FALSE;
	m_pBuffer		= NULL;
	m_dwBufSize		= NULL;
	m_dwFileSize	= NULL;
	m_dwReadSize	= NULL;
	m_dwWriteSize	= NULL;
	m_enStep		= FMPS_NULL;
	m_dwVersionInfo = NULL;
}

CGameTextFileMngr::~CGameTextFileMngr()						// �Ҹ���
{
}

CGameTextFileMngr* CGameTextFileMngr::GetInstance()			// �̱���� �ν��Ͻ� �޾ƿ���
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new CGameTextFileMngr();
	}
	
	return m_pInstance;
}

BOOL CGameTextFileMngr::Create( LPCTSTR lpFileName, BOOL bSave /*= FALSE*/ )
{
	// ���� ������ ����
	ASSERT( FMPS_NULL == m_enStep );
	ASSERT( NULL != lpFileName );

	// ���� �ڵ� �����, ������, ���� �����Ѵ�.
	lstrcpy( m_strFileName, lpFileName );
	m_bSave	= bSave;

	if ( m_bSave )
	{
		// �̳��� �����
		m_hFile = CreateFile( lpFileName, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	}
	else
	{
		// �̳��� �б��
		m_hFile = CreateFile( lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	}

	ASSERT( INVALID_HANDLE_VALUE != m_hFile );

	m_dwVersionInfo = 10;

	m_enStep = FMPS_CREATED;

	return TRUE;
}

DWORD CGameTextFileMngr::Load( BOOL bBinary )
{
	// ���ϳ��� �о����
	ASSERT( FALSE == m_bSave );
	ASSERT( FMPS_CREATED == m_enStep );

	if ( NULL != m_pBuffer ) return FALSE;

	// ���� ũ�⸸ŭ ���� ���,
	m_dwFileSize	= GetFileSize( m_hFile, NULL );
	m_dwBufSize		= m_dwFileSize + 1;
	m_pBuffer		= new TCHAR[m_dwBufSize];

	ZeroMemory( m_pBuffer, m_dwBufSize );

	m_enStep = FMPS_LOADED;

	return Read( bBinary );
}

DWORD CGameTextFileMngr::Read( BOOL bBinary )
{
	// ���� ��Ʈ�� �о����
	ASSERT( FALSE == m_bSave );
	ASSERT( FMPS_LOADED == m_enStep );
	
	// ��� �о������.
	BOOL	bSuccess	= FALSE;
	DWORD	dwReadSize	= NULL;
	TCHAR	szBuffer[NF_FILE_BUFFER_SIZE]	= {0,};
	
	DWORD	dwFileSize = 0;

	if ( TRUE == bBinary )
	{
		ReadSignature( &m_dwVersionInfo, &dwFileSize );
	}

	do
	{
		bSuccess = ReadFile( m_hFile, (LPVOID)szBuffer, NF_FILE_BUFFER_SIZE, &dwReadSize, NULL );
		
		CopyMemory( &m_pBuffer[m_dwReadSize], szBuffer, dwReadSize );
		m_dwReadSize += dwReadSize;
		
		Sleep( 2 );	// ���� ��¦��¦ �ָ鼭~
		
	} while ( ( NF_FILE_BUFFER_SIZE <= dwReadSize ) && ( TRUE == bSuccess ) );
	
	m_enStep = FMPS_READ;

	return m_dwReadSize;
}

BOOL CGameTextFileMngr::ReadSignature( LPDWORD dwVer, LPDWORD dwFileSize )
{
	TCHAR szSignature[NF_FILE_SIGNATURE_SIZE] = {0,};
	DWORD dwRead;
	
//	wsprintf( szSignature, NF_FILE_SIGNATURE, dwVer );
	
	ReadFile( m_hFile, szSignature, NF_FILE_SIGNATURE_SIZE, &dwRead, NULL );

	
	return TRUE;
}


void CGameTextFileMngr::GetBuffer( OUT LPNF_Buff buf )
{
	// ���� �Ŵ��� ���� �ִ� ���� ��������
	ASSERT( FALSE == m_bSave );
	ASSERT( FMPS_READ == m_enStep );


/*
	if ( m_dwFileSize != m_dwReadSize )
	{
		ASSERT( FALSE );
		buf->pBuf		= NULL;
		buf->dwBufSize	= NULL;
		return;
	}
*/
	
	m_enStep = FMPS_GOTBUF;

	buf->pBuf		= m_pBuffer;
	buf->dwBufSize	= m_dwBufSize;
}

BOOL CGameTextFileMngr::SetBuffer( LPNF_Buff buf, BOOL bSave /* = FALSE */, BOOL bBinary /* = TRUE */ )
{
	ASSERT( TRUE == m_bSave );
	ASSERT( FMPS_CREATED == m_enStep );
	ASSERT( NULL != buf->pBuf );
	ASSERT( NULL != buf->dwBufSize );
	
	if ( NULL != m_pBuffer ) return FALSE;

	m_pBuffer	= new TCHAR[buf->dwBufSize + 1];
	m_dwBufSize = buf->dwBufSize + 1;
	ZeroMemory( m_pBuffer, m_dwBufSize );
	CopyMemory( m_pBuffer, buf->pBuf, m_dwBufSize );

	m_enStep = FMPS_SETBUF;

	if ( TRUE == bSave )
	{
		return Save( bBinary );
	}
	else
	{
		return TRUE;
	}
}

BOOL CGameTextFileMngr::Write( BOOL bBinary )
{
	ASSERT( TRUE == m_bSave );
	
	BOOL	bSuccess			= FALSE;
	DWORD	dwWrittenSize		= 0;
	DWORD	dwTotWrittenSize	= 0;
	LPVOID	lpFilePos			= NULL;

	if ( TRUE == bBinary )
	{
		WriteSignature( m_dwVersionInfo, m_dwBufSize );		// �ñ״��� ����.
	}

	do
	{
		lpFilePos = &m_pBuffer[dwTotWrittenSize];

		bSuccess = WriteFile(	m_hFile,
								lpFilePos,
								( NF_FILE_BUFFER_SIZE < m_dwBufSize ) ? NF_FILE_BUFFER_SIZE : m_dwBufSize,
								&dwWrittenSize,
								NULL );

		dwTotWrittenSize += dwWrittenSize;
		m_dwBufSize		 -= dwWrittenSize;

		Sleep( 2 );	// ���� ��¦��¦ �ָ鼭~

	} while( ( NF_FILE_BUFFER_SIZE <= dwWrittenSize ) && ( TRUE == bSuccess ) );
	
	m_enStep = FMPS_WRITTEN;

	return bSuccess;
}

BOOL CGameTextFileMngr::WriteSignature( DWORD dwVer, DWORD dwFileSize )
{
	TCHAR szSignature[NF_FILE_SIGNATURE_SIZE] = {0,};
	DWORD dwWritten;

	wsprintf( szSignature, NF_FILE_SIGNATURE, dwVer );
	
	WriteFile( m_hFile, szSignature, NF_FILE_SIGNATURE_SIZE, &dwWritten, NULL );

	return TRUE;
}

BOOL CGameTextFileMngr::Save( BOOL bBinary )
{
	ASSERT( FMPS_SETBUF == m_enStep );

	Write( bBinary );

	ASSERT( FMPS_WRITTEN == m_enStep );

	m_enStep = FMPS_SAVED;

	return TRUE;
}

BOOL CGameTextFileMngr::Destroy()
{
	if ( INVALID_HANDLE_VALUE != m_hFile )
	{
		CloseHandle( m_hFile );
		m_hFile = NULL;
	}
	
	if ( NULL != m_pBuffer )
	{
		delete [] m_pBuffer;
		m_pBuffer = NULL;
	}
	
	if ( NULL != m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = NULL;
		
		return TRUE;
	}
	
	return FALSE;
}
