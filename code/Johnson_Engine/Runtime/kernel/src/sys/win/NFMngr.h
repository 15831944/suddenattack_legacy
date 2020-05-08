//-----------------------------------------------------------------------------
//
//	Network Firewall Manager DLL
//
//	!!!!! �� ���α׷� ������ ����� ���� ������ ��ȭ���� ������ ���Ѵ�. !!!!!
//
//											Written by minjin. 2005. 02. 07.
//
//	�Ϳ�.. �� ���� �ٷ� �����̴�.. -_-;;
//	DirectX 9.0 ������ �̿��ؼ�, DLL �� ����.
//
//	���� ������ �����Ͽ��� ��!!
//	
//	- �Լ� ����
//
//	  _fnSTART		<-	NF_Create �� �Լ� ������.
//						��ȭ�� ����� �����Ѵ�.
//						���μ������, ��Ͽ� ǥ�õ� �̸��� ���� �����ָ�,
//						wchar �� ���� �Է¹޴´�.
//						��ȭ���� ���� ��Ͽ� ������ �׸��� �߰��Ѵ�.
//
//	  _fnISALLOWED	<-	NF_IsAllowed �� �Լ� ������.
//						��ȭ���� ���������� ��ϵǾ� �ִ��� Ȯ���Ͽ�,
//						�����̸� TRUE, �ƴϸ� FALSE -_-;;
//
//	  _fnEND		<-	NF_End �� �Լ� ������.
//						��ȭ���� ���� ��Ͽ��� �׸� ����.
//						��ȭ�� ��� Release.
//
//-----------------------------------------------------------------------------

#ifndef __NFMNGR_DLL_H__
#define __NFMNGR_DLL_H__

#pragma once


#define NFMNGR_API __declspec(dllexport)


#define _FIREWALL_MANAGER_DLL_				TEXT("NFMngr.dll")			// DLL �̸�
#define _FIREWALL_EXCEPTION_LIST_NAME_		L"Sudden Attack"			// ��ȭ�� ��Ͽ� �߰��� �̸�



// �Լ� ������ Ÿ�� ���Ǻκ�
// * �˾ƺ��� ���� ������ ���ĵ� �ٸ� �̸����� typedef �Ѵ�
typedef BOOL ( *_fnSTART		)( IN const wchar_t* szProcPath, IN const wchar_t* szProcName );
typedef BOOL ( *_fnEND			)( IN const wchar_t* szProcPath );
typedef BOOL ( *_fnISALLOWED	)( IN const wchar_t* szProcPath );


_fnSTART		__NF_Start		= NULL;
_fnISALLOWED	__NF_IsAllowed	= NULL;
_fnEND			__NF_End		= NULL;


// DLL ���� Export �� �Լ�
extern "C" NFMNGR_API BOOL _NF_Start( IN const wchar_t* szProcPath, IN const wchar_t* szProcName );
extern "C" NFMNGR_API BOOL _NF_IsAllowed( IN const wchar_t* szProcPath );
extern "C" NFMNGR_API BOOL _NF_End( IN const wchar_t* szProcPath );

HMODULE g_hNF_Dll		= NULL;
BOOL	g_bNFActivated	= FALSE;


inline BOOL NF_Initialize()
{
	if ( g_hNF_Dll )			return TRUE;
	
	// DLL �ε� �� �Լ� �ʱ�ȭ �κ�
	g_hNF_Dll	= LoadLibrary( _FIREWALL_MANAGER_DLL_ );
	
	if ( !g_hNF_Dll )			return FALSE;
	
	
	__NF_Start		= (_fnSTART)	GetProcAddress( g_hNF_Dll, "_NF_Start" );
	__NF_End		= (_fnEND)		GetProcAddress( g_hNF_Dll, "_NF_End" );
	__NF_IsAllowed	= (_fnISALLOWED)GetProcAddress( g_hNF_Dll, "_NF_IsAllowed" );
	
	if ( NULL == __NF_Start )		return FALSE;
	if ( NULL == __NF_End )			return FALSE;
	if ( NULL == __NF_IsAllowed )	return FALSE;
	
	g_bNFActivated = TRUE;

	return TRUE;
}

inline BOOL NF_Release()
{
	if ( g_hNF_Dll )
	{
		return FreeLibrary( g_hNF_Dll );
	}
	
	return FALSE;
}

inline void NF_Start( IN const wchar_t* szProcPath, IN const wchar_t* szProcName )
{
	if ( FALSE == g_bNFActivated || FALSE == g_hNF_Dll )
		return;

	__NF_Start( szProcPath, szProcName );
}

inline void NF_End( IN const wchar_t* szProcPath )
{
	if ( FALSE == g_bNFActivated || FALSE == g_hNF_Dll )
		return;

	__NF_End( szProcPath );
}

inline void NF_IsAllowed( IN const wchar_t* szProcPath )
{
	if ( FALSE == g_bNFActivated || FALSE == g_hNF_Dll )
		return;

	__NF_IsAllowed( szProcPath );
}


#endif //__NFMNGR_DLL_H__