// 2004�� 3�� 8�� ~ 9��

#pragma once

#include <windows.h>
#include "Message.h"
#include "..\..\DataStructure\MemoryPoolSafty.h"
#include "..\..\DataStructure\List.h"

// ť�� �Ŵ��� ����Ʈ ��̸� n�� �̾���� �����̴�.
// Enqueue�� ���ؼ� ��Ŷ�� ť�� copy�� ������ ��Ŷ�� ������ ��Ŷ ���� ť�� ��´�.
// ��Ŷ ���� ť�� IOCP�� ���ؼ� ������ �Ѵ�.
// Dequeue�� ���ؼ� ť���� ��Ŷ�� ������.

// Enqueue�� n���� �����尡 ���� ����������
// Dequeue�� �ݵ�� 1���� �����尡 ������ �Ѵ�. �� ������ n���� �����尡 ������ �Ǹ� TCP�� ��Ʈ���ֿ� ����Ǳ� �����̴�.


// create�� ���ؼ� ��Ŷ ť �迭�� �����ϰ�, ��� ��Ŷ�� �����Ѵ�.
// 

namespace Session
{
	namespace Message
	{
		class CMessageQueue
		{
		public:
			CMessageQueue(void);
			~CMessageQueue(void);

		private:
			Common::DataStructure::CMemoryPoolSafty<CMessage>
							m_MessagePool;			// �޽��� Ǯ
			CMessage**		m_paMessage;			// �޽��� ť �迭


//				CMessage*		m_aMessage;				// ��Ŷ ť �迭

			INT				m_nPacketSize;

			UINT			m_nQueueSize;			// ť�� ���� �� �ִ� ��Ŷ�� �ִ� ����
			UINT			m_nHead;				// Enqueue�� ��ġ
			UINT			m_nRear;				// Dequeue�� ��ġ

			UINT			m_nCount;				// ť�׵� ����

			CRITICAL_SECTION
							m_csWrite;				// For Thread-Safe : N���� Write���� ����ȭ

			CRITICAL_SECTION
							m_csRead;				// For Thread-Safe : M���� Read���� ����ȭ

			CRITICAL_SECTION
							m_csReAlloc;

			INT				m_nWaiter;

			HANDLE			m_hSemaphore;			// queue�� ����ȭ�� ���� �������� ��ü

			Common::DataStructure::CList<CMessage*>
							m_lstMessage;

			CSignalQueue	m_SignalQueue;

		public:
			// ť ���۸� �����Ѵ�.
			BOOL		Create( int nQueueSize, int nPacketSize, int nEventSize = 0 );

			// ť ���۸� �����Ѵ�.
			void		Destroy();

			// ť�� ��Ŷ�� �����Ѵ�.
			void		EnqueuePacket( const CMessage& packet );

			// ť���� ��Ŷ�� ������.
			void		DequeuePacket( CMessage& outPacket );

			// �̺�Ʈ�� �߰��Ѵ�.
			void		AddSignalPacket( const CSignal& signal );

		public:
			BOOL		IsEmpty()
			{
				return m_lstMessage.GetElementsCount() == 0;
//					return m_nHead == m_nRear;
			}
		};// class CMessageQueue

	}// namespace Message

}// namespace Session
