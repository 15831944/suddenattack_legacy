#include "StdAfx.h"
#include ".\iocp.h"

using namespace IO::IOCP;

/**************************************************************************************
	������ / �Ҹ���
***************************************************************************************/

CIocp::CIocp()
:	m_hIOCP				(NULL)
,	m_dwWaitMilliSeconds(INFINITE)
{
}

CIocp::~CIocp(void)
{
}


/**************************************************************************************
	Public Operations
***************************************************************************************/

// IOCP�� �����Ѵ�.
//		dwWaitMilliSeconds : wait �ð�
BOOL CIocp::Create( DWORD dwWaitMilliSeconds/* = INFINITE*/ )
{
	//
	// GetQueuedCompletionStatus������ ��� �ð� ����
	//
	m_dwWaitMilliSeconds = dwWaitMilliSeconds;

	//
	// IOCP Handle ����
	//
	m_hIOCP = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );

	return( m_hIOCP != NULL );
}

// IOCP�� �����Ѵ�.
void CIocp::Destroy()
{
	::CloseHandle( m_hIOCP );
	m_hIOCP = NULL;
}

// IOCP�� �ڵ��� ����Ѵ�.
BOOL CIocp::AssignIoCompletionPort( HANDLE FileHandle, ULONG_PTR CompletionKey )
{
	return( m_hIOCP == ::CreateIoCompletionPort( FileHandle, m_hIOCP, CompletionKey, 0 ) );
}

// IOCP ť�� Post�Ѵ�.
BOOL CIocp::PostQueuedCompletionStatus( ULONG_PTR lpCompletionKey, LPOVERLAPPED lpOverlapped )
{
	return ::PostQueuedCompletionStatus( m_hIOCP, 0, lpCompletionKey, lpOverlapped );
}

// IOCP ť���� Get�Ѵ�.
BOOL CIocp::GetQueuedCompletionStatus( LPDWORD lpNumberOfBytesTransferred, PULONG_PTR lpCompletionKey, LPOVERLAPPED* lpOverlapped )
{
	return ::GetQueuedCompletionStatus( m_hIOCP, lpNumberOfBytesTransferred, lpCompletionKey, lpOverlapped, m_dwWaitMilliSeconds );
}