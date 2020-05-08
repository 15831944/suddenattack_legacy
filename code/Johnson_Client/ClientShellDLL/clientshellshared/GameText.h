// GameText.h: interface for the CGameText class.
/*
	GameText Class						Written by Novice.

	���ѹα�, �߱�, �Ϻ� ���� ������ �޽��� ������ ���� Ŭ����.

	�׻� �Ѱ��� ���� ������ �޼�����,
	���� ������ �������� �ѹ��� Insert / Remove / Update �ȴ�.
	���ҽ��� ������ �ִ��� ���� �Ǽ��� ������ �ϱ� �����̴�.
*/
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMETEXT_H__10FB6C7E_D944_461F_A5EE_F17BA685F6B4__INCLUDED_)
#define AFX_GAMETEXT_H__10FB6C7E_D944_461F_A5EE_F17BA685F6B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tchar.h"


//#define KOR_LOCALIZING			// �ѱ���
//#define JPN_LOCALIZING			// �Ϻ�����
#define CHN_LOCALIZING			// �߱�����

#if		defined KOR_LOCALIZING
#define NLC_NATION			NLC_KOREA
#define LOCALIZING_CHARSET	HANGUL_CHARSET
#define DEFAULT_FONT_NAME	"\xb1\xbc\xb8\xb2"										//KOREAN font
#elif	defined JPN_LOCALIZING
#define NLC_NATION			NLC_JAPAN
#define LOCALIZING_CHARSET	SHIFTJIS_CHARSET
#define DEFAULT_FONT_NAME	"\x82\x6c\x82\x72\x20\x83\x53\x83\x56\x83\x62\x83\x4e"	//JAPANESE font
#elif	defined CHN_LOCALIZING
#define NLC_NATION			NLC_CHINA
#define LOCALIZING_CHARSET	GB2312_CHARSET
#define DEFAULT_FONT_NAME	"\xB0\x65\x30\x7D\x0E\x66\xD4\x9A\x00\x00"				//TRADITIONAL_CHINESE font
#else
#define LOCALIZING_CHARSET	DEFAULT_CHARSET
#endif


//////////////////////////////////////////////////////////////////////////
char*	NF_GetMsg( DWORD dwIdx );							// ���ӿ��� �޼��� �������� ����.

#define NF_GAME_TEXT()				CGameText::GetInstance()

#define MAX_MESSAGE_CNT				1024					// ���� �޼��� ���� �ִ밪
#define MAX_MESSAGE_LANG			3						// ���� ����
#define MAX_MESSAGE_LENGTH			512 * sizeof(TCHAR)		// ���� �޼��� ���� �ִ밪
#define MAX_MESSAGE_DEF_LENGTH		256						// ���� �޼��� ǥ���� DEFINE �� ���� �ִ밪

#define MAX_CONTEXT_BODY_SIZE		MAX_MESSAGE_CNT * sizeof(GameText)
#define MAX_CONTEXT_HEADER_SIZE		MAX_MESSAGE_CNT * sizeof(GameTextHeader)
#define MAX_CONTEXT_SIZE			MAX_CONTEXT_BODY_SIZE + MAX_CONTEXT_HEADER_SIZE

enum EN_NATIONAL_LANGUAGE_CODE	// ���� �ڵ� Enum
{
	NLC_KOREA	= 0,
	NLC_CHINA	= 1,
	NLC_JAPAN	= 2,
};

#define GAME_MESSAGE_FILE_NAME		TEXT("SA_MSGS.gsm")

#define END_OF_GAMETEXT_HEADER		TEXT("\xa8\x97\x30\xd0\xa4\xe0\xe0\x02")
#define END_OF_GAMETEXT_CONTEXT		TEXT("\x97\xa4\xa8\xe0\x02\xe0\x30\xd0")


typedef struct _GameTextHeader
{
	DWORD dwIdx;
	DWORD dwDefineOffset;
	DWORD dwDefineLength;
	DWORD dwOffset[MAX_MESSAGE_LANG];
	DWORD dwLength[MAX_MESSAGE_LANG];
} GameTextHeader, *LPGameTextHeader;

typedef struct _GameText		// Game Text ����ü
{
	DWORD dwIdx;	// �迭 �ε����� ���� ��������, �ϳ� �־���´�.
					// ���߿� ����� ���ĵ� �޼������� Ȯ���Ҷ� ����.

	TCHAR txtDefine[MAX_MESSAGE_DEF_LENGTH];
	TCHAR txtMessage[MAX_MESSAGE_LANG][MAX_MESSAGE_LENGTH];


	// �� ó�� �����Ҷ��� ���� ������������!
	_GameText() {	ZeroMemory( this, sizeof(_GameText) );	}

} GameText, *LPGameText;
//////////////////////////////////////////////////////////////////////////


class CGameText  
{
public:
	static CGameText* GetInstance();
	BOOL	Destroy();

	BOOL	IsExist( LPCTSTR lpDefinition );
	
	BOOL	Insert( LPGameText pGameText );		// Message �� Insert, Remove ��
	BOOL	Remove( DWORD dwIdx );				// ������ �� ������ ��ü �����ϴ°����� �Ѵ�.
	
	void	Update( LPGameText pGameText );
	void	Update( DWORD dwArrIdx, DWORD dwRealIdx );
	void	Update( DWORD dwIdx, LPCTSTR lpszDefine );
	void	Update( DWORD dwIdx, LPCTSTR lpszMsg, EN_NATIONAL_LANGUAGE_CODE nlCode );

	DWORD	GetTotMsgCnt();
	BOOL	GetRow( DWORD dwIdx, OUT LPGameText pGameText );

	LPCTSTR GetMsg( DWORD dwIdx );
	LPCTSTR	GetMsg( EN_NATIONAL_LANGUAGE_CODE nlCode, DWORD dwIdx );


	/////////////////////////// ���� ���� �Լ� ///////////////////////////////
	// ���Ϸ� ������ �� ���� �Լ�
	LPVOID	GetContext();
	DWORD	GetContextSize() { return m_dwGameTextContextSize; }

	LPVOID	GetContextHeader();
	DWORD	GetContextHeaderSize() { return m_dwGameTextContextHeaderSize; }


	// ���Ϸκ��� ���� �޾ƿ� �� ���� �Լ�
	void	SetContext( LPVOID lpContext, DWORD dwContextSize );


	// ���� ������ ���� ��� ���Ͽ�..
	LPVOID	GetRefHeader();
	DWORD	GetRefHeaderSize() { return m_dwGameTextRefHeaderSize; }
	//////////////////////////////////////////////////////////////////////////
	

protected:
	CGameText();
	virtual ~CGameText();

	static CGameText* m_pInstance;

	void	ProcessingArrayToContext();				// �ؽ�Ʈ �迭�� �ִ� ������ ��Ʈ������ ��ȯ
	void	ProcessingContextToArray();				// ��Ʈ�� ������ �ؽ�Ʈ �迭ȭ
	void	ProcessingArrayToGameHeader();			// �ؽ�Ʈ �迭 ������ ���ʷ�, ��� ���.

	// in ProcessingArraryToContext()
	DWORD	MakeContextHeader();
	DWORD	MakeHeaderFromBody( DWORD dwCnt, DWORD dwContextOffset );
	DWORD	WriteHeaderIntoContext( DWORD dwContextOffset );
	DWORD	WriteBodyIntoContext( DWORD dwContextOffset );

	// in ProcessingContextToArray()
	void	AnalyzeContextHeader();
	DWORD	WriteContextIntoBody( DWORD dwCnt, DWORD dwContextOffset );


	TCHAR*	m_pGameTextRefHeader;				// ����� ���(.h) ����
	DWORD	m_dwGameTextRefHeaderSize;

	TCHAR*	m_pGameTextContext;
	TCHAR*	m_pGameTextContextHeader;
	DWORD	m_dwGameTextContextHeaderSize;
	DWORD	m_dwGameTextContextSize;

	TCHAR*	m_pTmpGameTextContext;				// �������� Temp �� ����.
	DWORD	m_dwTmpGameTextContextSize;
	
	DWORD	m_dwTotMsgCnt;						// ��ü �޼��� ����

	GameText		m_arrGameText[MAX_MESSAGE_CNT];
	GameTextHeader	m_arrGameTextHeader[MAX_MESSAGE_CNT];
	
	// BOOL SearchMessage( LPCTSTR lpszMessage );

};

#endif // !defined(AFX_GAMETEXT_H__10FB6C7E_D944_461F_A5EE_F17BA685F6B4__INCLUDED_)
