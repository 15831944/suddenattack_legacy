// GameTextFileMngr.h: interface for the CGameTextFileMngr class.
/*
 *	���� �ؽ�Ʈ�� ���� ���� �Ŵ�����..		Written by Novice.
 *
 *	Win32API �� ���� ���� �Լ����� Wrapping �� ���Ҵ�.
 *	�ƽôٽ���.. �ϳ� �����ϴ�.. -_-;;
 *
 *	���Ŀ�, Load(), Save() �� ���ΰ� �� ���ֹ��� �����̴�.. -_-;;
 */
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMETEXTFILEMNGR_H__E7C2248D_E60D_4A71_AA97_E7046E667FF8__INCLUDED_)
#define AFX_GAMETEXTFILEMNGR_H__E7C2248D_E60D_4A71_AA97_E7046E667FF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////////

#define NF_FILE_MNGR()				CGameTextFileMngr::GetInstance()

#define NF_FILE_SIGNATURE			TEXT( "GameHi Sudden Attack Message File Version 0.1.0 @@ Written by Novice. \x81\x12\x21\x01\x02\x03%d\x08" )
#define NF_FILE_SIGNATURE_SIZE		128
#define NF_FILE_BUFFER_SIZE			4096

typedef struct _Buffer				// File �� Buffer ���� Ÿ��
{
	LPVOID	pBuf;
	DWORD	dwBufSize;
} NF_Buff, *LPNF_Buff;

//////////////////////////////////////////////////////////////////////////

class CGameTextFileMngr  
{
public:
	static CGameTextFileMngr* GetInstance();

	// !!! WARNNING !!!
	// bBinary �÷��׸� TRUE �� �����ϸ�, ���� �տ� �ñ״��� �ٴ´�.
	// �̰� ���߿� ����������.. -_-;;
	//
	// Create ���� Destroy ���� ������� ����. ���� �Ⱦ��� �������� ������.. ����
	BOOL	Create( LPCTSTR lpFileName, BOOL bSave = FALSE );

	DWORD	Load( BOOL bBinary );				// ���� �о ���۷�!
	void	GetBuffer( OUT LPNF_Buff buf );		// ���� ������ ����ִ� ���� ��ȯ

	BOOL	SetBuffer( LPNF_Buff buf,			// ���Ͽ� �� ���� ����
					   BOOL bSave = FALSE,		// ���⼭ bSave �� TRUE �̸�, �ؿ��ִ� Save() �� �����Ѵ�.
					   BOOL bBinary = TRUE );	// �Ѹ����, SetBuffer( ..., TRUE ); �ϸ�, Save() ���ص� �ȴ�.

	BOOL	Save( BOOL bBinary );				// ���۸� ���Ϸ�!

	BOOL	Destroy();							// ����� ������ �ݵ�� ȣ���ض�.. -_-;;


protected:

	CGameTextFileMngr();
	virtual ~CGameTextFileMngr();
	
	static CGameTextFileMngr* m_pInstance;
	
	DWORD	Read( BOOL bBinary );
	BOOL	ReadSignature( LPDWORD dwVer, LPDWORD dwFileSize );

	BOOL	Write( BOOL bBinary );
	BOOL	WriteSignature( DWORD dwVer, DWORD dwFileSize );

	enum EN_FILE_MNGR_PROPER_STEP		// �����Ȳ ������ enum
	{
		FMPS_NULL		= 0x00,
		FMPS_CREATED	= 0x10,
		FMPS_LOADED		= 0x20,
		FMPS_READ		= 0x30,
		FMPS_GOTBUF		= 0x40,
		FMPS_SETBUF		= 0x50,
		FMPS_WRITTEN	= 0x60,
		FMPS_SAVED		= 0x70,
		FMPS_DESTROYED	= 0x80,
		FMPS_INVALID	= 0xff,
	};
	
	EN_FILE_MNGR_PROPER_STEP m_enStep;	// ���� �����Ȳ ������ �ִ³�

	HANDLE	m_hFile;					// ���� �ڵ�
	TCHAR	m_strFileName[_MAX_PATH];	// ���� �̸�
	BOOL	m_bSave;					// ������ ��������� �������, �б������ �������.. ( TRUE -> �����, FALSE -> �б�� )
	DWORD	m_dwFileSize;				// ���� ��ü ũ��
	
	TCHAR*	m_pBuffer;					// ���ϳ��� �״�� ������� ��
	DWORD	m_dwBufSize;				// ����� ���� ���� ũ��

	DWORD	m_dwReadSize;				// ���ݱ��� �о���� ũ��
	DWORD	m_dwWriteSize;				// ���ݱ��� �� ũ��

	DWORD	m_dwVersionInfo;			// ���� ���� ����
};

#endif // !defined(AFX_GAMETEXTFILEMNGR_H__E7C2248D_E60D_4A71_AA97_E7046E667FF8__INCLUDED_)
