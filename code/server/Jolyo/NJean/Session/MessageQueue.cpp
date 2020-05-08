#include "StdAfx.h"
#include ".\MessageQueue.h"

using namespace Session::Message;

#include "..\Application\Packet.h"

CMessageQueue::CMessageQueue(void)
{
}

CMessageQueue::~CMessageQueue(void)
{
}

// ť ���۸� �����Ѵ�.
BOOL CMessageQueue::Create( int nQueueSize, int nPacketSize, int nSignalSize )
{
	m_lstMessage.Create( nQueueSize );

	m_paMessage = new CMessage*[nQueueSize];
	if( m_paMessage == NULL )
		return FALSE;

	m_MessagePool.Create( nQueueSize );
	for( int i = 0; i < nQueueSize; i++ )
	{
		m_paMessage[i] = m_MessagePool.Alloc();
		m_paMessage[i]->Create( nPacketSize );
	}
	for( int i = 0; i < nQueueSize; i++ )
	{
		m_MessagePool.Free( m_paMessage[i] );
	}


	m_nQueueSize	= nQueueSize;
	m_nHead			= 0;
	m_nRear			= 0;
	m_nWaiter		= 0;


	m_SignalQueue.Create( nSignalSize );

	m_hSemaphore = ::CreateSemaphore(
						NULL,		// Sempahore Attributes
						0,			// initialCount
						65000,		// MaximumCount
						NULL		// object name
						);

	m_nPacketSize	= nPacketSize;


	::InitializeCriticalSection( &m_csWrite );
	::InitializeCriticalSection( &m_csRead );
	::InitializeCriticalSection( &m_csReAlloc );
	
	return TRUE;
}

// ť ���۸� �����Ѵ�.
void CMessageQueue::Destroy()
{
	m_lstMessage.Destroy();

	// �޸� Ǯ�� �����ؾ� �Ѵ�. -_-;;
	if( m_paMessage )
	{
		delete[] m_paMessage;
		m_paMessage		= NULL;
		m_nQueueSize	= 0;
	}


	m_SignalQueue.Destroy();

	// IOCP ť�� �����Ѵ�.
	::CloseHandle( m_hSemaphore );

	::DeleteCriticalSection( &m_csWrite );
	::DeleteCriticalSection( &m_csRead );
	::DeleteCriticalSection( &m_csReAlloc );
}

// ť�� ��Ŷ�� �����Ѵ�.
void CMessageQueue::EnqueuePacket( const CMessage& packet )
{
//	DWORD dwFirst = GetTickCount();

	
	CMessage* pMessage;
	pMessage = m_MessagePool.Alloc();
	/*
	if( pMessage->GetBuffer() == NULL )
	{
		ASSERT( pMessage->Create( m_nPacketSize ) );
	}
	*/
	*pMessage = packet;
	

	//
	// n���� �����尡 Enqueue �� ���� ����ȭ ó��
	//
	
	::EnterCriticalSection( &m_csWrite );

//	DWORD dwTime = GetTickCount() - dwFirst;

	m_lstMessage.Add( pMessage );
	if( m_nWaiter > 0 )
	{
//		char a[100];
//		sprintf( a, "MSG Q Empty %d %d\n", (int)this, m_nWaiter );
//		::OutputDebugString( a );

		::ReleaseSemaphore( m_hSemaphore, 1, NULL );
		m_nWaiter--;
	}

	::LeaveCriticalSection( &m_csWrite );

//	dwTime = GetTickCount() - dwFirst;
}

// ť���� ��Ŷ�� ������.
void CMessageQueue::DequeuePacket( CMessage& outPacket )
{
	::EnterCriticalSection( &m_csWrite );

	CMessage* pMessage;
	while( true )
	{
		// ���ȴ����ź���
		CSignal signal;
		if( m_SignalQueue.DequeueSignal( signal ) )
		{
			outPacket = CMessage( signal );
			::LeaveCriticalSection( &m_csWrite );
			return;
		}

		pMessage = m_lstMessage.Dequeue();
		if( pMessage != NULL )
			break;

		m_nWaiter++;
		::LeaveCriticalSection( &m_csWrite );

		DWORD dwWaitResult = ::WaitForSingleObject( m_hSemaphore, INFINITE );

		::EnterCriticalSection( &m_csWrite );
	}

	::LeaveCriticalSection( &m_csWrite );

	outPacket = *pMessage;
	m_MessagePool.Free( pMessage );
}

// �̺�Ʈ�� �߰��Ѵ�.
void CMessageQueue::AddSignalPacket( const CSignal& signal )
{
	::EnterCriticalSection( &m_csWrite );
	m_SignalQueue.EnqueueSignal( signal );

	if( m_nWaiter > 0 )
	{
		::ReleaseSemaphore( m_hSemaphore, 1, NULL );
		m_nWaiter--;
	}
	::LeaveCriticalSection( &m_csWrite );
}