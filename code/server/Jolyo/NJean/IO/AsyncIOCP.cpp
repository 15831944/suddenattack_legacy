#include "StdAfx.h"
#include ".\asyncIocp.h"

using namespace IO::IOCP;

/**************************************************************************************
	������ / �Ҹ���
***************************************************************************************/

CAsyncIocp::CAsyncIocp( VOID )
:	CIocp			( )
,	m_nNumOfThreads ( 0		)
,	c_lpThreadDie	( 1		)
,	m_pIocpHandler	( NULL	)
{
}

CAsyncIocp::~CAsyncIocp( VOID )
{
	CIocp::~CIocp();
}


/**************************************************************************************
	Public Operations
***************************************************************************************/

// IOCP�� �����Ѵ�.
//		nNumberOfThreads : �������� ���� : 0�̸� CPU���� * 2�� �����ȴ�.
//		dwWaitMilliSeconds : wait �ð�
BOOL CAsyncIocp::Create( IIocpHandler* pIocpHandler, int nNumberOfThreads/* = 0*/, DWORD dwWaitMilliSeconds/* = INFINITE*/ )
{
	m_pIocpHandler = pIocpHandler;

	if( !CIocp::Create( dwWaitMilliSeconds ) )
		return FALSE;

	//
	// Thread ���� ����
	//
	m_nNumOfThreads = nNumberOfThreads;

	if( m_nNumOfThreads <= 0 )
	{
		SYSTEM_INFO si;
		::GetSystemInfo(&si);

		// ����Ʈ ������ ���� 2 * ���μ����� + 2 �� ������ ������
		m_nNumOfThreads = si.dwNumberOfProcessors * 2 + 2;
	}


	//
	// Thread Pool ����
	//
	for( int i=0; i < m_nNumOfThreads; i++ )
	{
		DWORD dwThreadId = 0;
		::CloseHandle( ::CreateThread( NULL, 0, IocpWorkerThreadStartingPoint, this, 0, &dwThreadId ) );
	}

	return TRUE;
}

// IOCP�� �����Ѵ�.
void CAsyncIocp::Destroy()
{
	for( int i = 0; i < m_nNumOfThreads; i++ )
		::PostQueuedCompletionStatus( m_hIOCP, 0, c_lpThreadDie, NULL );

	CIocp::Destroy();
}


/**************************************************************************************
	Private Operations
***************************************************************************************/

// IOCP�� �����ϴ� ������
DWORD __stdcall CAsyncIocp::IocpWorkerThreadStartingPoint(PVOID pvParam)
{
	CAsyncIocp* pIocp = (CAsyncIocp*) pvParam;

	ULONG_PTR		lpCompletionKey	= NULL;
	LPOVERLAPPED	lpOverlapped	= NULL;
	DWORD			dwBytesTransferred = 0;

	while( TRUE )
	{
		//
		// IO Completion Packet ���´�.
		//
		BOOL bRet = pIocp->GetQueuedCompletionStatus(
			&dwBytesTransferred,
			&lpCompletionKey,
			&lpOverlapped );

		if( bRet )
		{
			if( pIocp->IsExitCompletionKey( lpCompletionKey ) )
				break;
		}

		ASSERT( lpOverlapped );

		//
		// IOCP �������̽��� ���� IO�ϷḦ �뺸�Ѵ�.
		//
		pIocp->m_pIocpHandler->OnGetQueuedCompletionStatus(
			bRet,
			dwBytesTransferred,
			lpCompletionKey,
			lpOverlapped );
	}

	return 0;
}