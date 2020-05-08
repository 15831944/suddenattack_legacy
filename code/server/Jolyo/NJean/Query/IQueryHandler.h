/*
	InternetWrite�� Read���� ū �뷮 ó�� �κ��� �����ִ�.
	����� ���� Command ������ �ۼ����̶� ������ ������,
	��뷮 ó���� ���ؼ� ������ �صξ�� �Ѵ�.
*/

#pragma once

#include "..\..\DataStructure\MemoryPoolSafty.h"
#include <wininet.h>


namespace Query
{
	interface IQueryHandler
	{
		virtual void OnRecvInet		( ULONG_PTR lpInetKey, ULONG_PTR dwContext )	= 0;
		virtual void OnErrorInet	( ULONG_PTR dwContext, DWORD nError )			= 0;
	};
}// namespace Query

