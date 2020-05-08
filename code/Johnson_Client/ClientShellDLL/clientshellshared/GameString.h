#ifndef __NF_GAME_STRING_INLINE_FUNCTIONS_H__
#define __NF_GAME_STRING_INLINE_FUNCTIONS_H__

#pragma once

// String Wrapping Inline Functions.
// by Novice. 2005. 03. 12.


// ���ڿ� ��
inline int __strcmp( char* szStr1, const char* szStr2 )
{
	return ( CompareString( LOCALE_USER_DEFAULT, NULL,
		szStr1, lstrlen(szStr1), szStr2, lstrlen(szStr2) ) - CSTR_EQUAL );
}

// ���ڿ� �� - ���ѵ� ����
inline int __strncmp( char* szStr1, const char* szStr2, int nLength )
{
	return ( CompareString( LOCALE_USER_DEFAULT, NULL,
		szStr1, nLength, szStr2, nLength ) - CSTR_EQUAL );
}

// ���ڿ� �� - (��ҹ���, ��Ÿ���� ���󰡳�, ���� �ݰ�) ����
inline int __stricmp( char* szStr1, const char* szStr2 )
{
	return ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH,
		szStr1, lstrlen(szStr1), szStr2, lstrlen(szStr2) ) - CSTR_EQUAL );
}

// ���ڿ� �� - (��ҹ���, ��Ÿ���� ���󰡳�, ���� �ݰ�) ���� - ���ѵ� ����
inline int __strnicmp( char* szStr1, const char* szStr2, int nLength )
{
	return ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH,
		szStr1, nLength, szStr2, nLength ) - CSTR_EQUAL );
}

// ���ڿ� ����
inline LPSTR __strcpy( char* szStr1, const char* szStr2, BOOL bIgnoreTerminate )
{
	return (bIgnoreTerminate) ? strcpy( szStr1, szStr2 ) : lstrcpy( szStr1, szStr2 );
}

// ���ڿ� ���� - ���ѵ� ���� �� ������ ����
inline LPSTR __strncpy( char* szStr1, const char* szStr2, int nLength, BOOL bIgnoreTerminate = FALSE )
{
	return (bIgnoreTerminate) ? strncpy( szStr1, szStr2, nLength ) : lstrcpyn( szStr1, szStr2, nLength + 1 );
}

// ���ڿ� ����
inline int __strlen( const char* szStr )
{
	return lstrlen(szStr);
}

// ����ִ� ���ڿ����� �˻�
inline BOOL __IsEmptyString( const char* szStr )
{
	return ( NULL == lstrlen(szStr) );
}

inline LPSTR __strtok( char* szStr, const char* szToken, OUT char* szOutput, OUT bool& bFinished )
{
	for ( int i = 0; i < __strlen(szStr); i++ )
	{
		if ( 0 == __strncmp( (szStr + i), szToken, __strlen(szToken) ) )
		{
			__strncpy( szOutput, szStr, i );
			return (szStr + i + __strlen(szToken));
		}
	}
	
	if ( i == __strlen(szStr) )
	{
		bFinished = true;
		return szStr;
	}
	
	return NULL;
}

#endif