// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.

//static ULONG s_ulAllocCount	= 0;

void *
Talloc( unsigned int uiItemSize );

void
Tfree( void * pPointer );

void *
operator new( unsigned int uiItemSize );

void
operator delete( void * pPointer );

void *
operator new[]( unsigned int uiItemSize );
void
operator delete[]( void * pPointer );


#include <WinSock2.h>

#include <iostream>
#include <tchar.h>

#include <windows.h>
//#include <assert.h>

#import "c:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "EndOfFile")


//#ifndef NDEBUG
//	#define ASSERT(expression) assert(expression)
//#else
	#define ASSERT(expression) static_cast<void>( 0 )
//#endif