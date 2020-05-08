// stdafx.cpp : ǥ�� ���� ���ϸ� ��� �ִ� �ҽ� �����Դϴ�.
// TiniLibGame.pch�� �̸� �����ϵ� ����� �˴ϴ�.
// stdafx.obj���� �̸� �����ϵ� ���� ������ ���Ե˴ϴ�.

#include "stdafx.h"

// TODO: �ʿ��� �߰� �����
// �� ������ �ƴ� STDAFX.H���� �����մϴ�.

void *
operator new( unsigned int uiItemSize )
{
//	s_ulAllocCount++;

//	SAFE_DEFUNLOCK;

	return ::Talloc( uiItemSize );
}

void
operator delete( void * pPointer )
{
	if ( pPointer == 0 )
	{
		return;
	}

	::Tfree( pPointer );
//	::FREE( pPointer );

//	SAFE_DEFLOCK;

//	s_ulAllocCount--;

//	SAFE_DEFUNLOCK;
}

void *
operator new[]( unsigned int uiItemSize )
{
//	SAFE_DEFLOCK;

//	s_ulAllocCount++;

//	SAFE_DEFUNLOCK;

	return ::Talloc( uiItemSize );
}

void
operator delete[]( void * pPointer )
{
	if ( pPointer == NULL )
	{
		return;
	}

	::Tfree( pPointer );

//	SAFE_DEFLOCK;

//	s_ulAllocCount--;

//	SAFE_DEFUNLOCK;
}


void *
Talloc( unsigned int uiItemSize )
{
	return ::malloc( uiItemSize );
}

void
Tfree( void * pPointer )
{
	::free( pPointer );
}