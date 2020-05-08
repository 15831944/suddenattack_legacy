/*****************************************************************************************************************
	CPacketAssembler

	�ۼ��� : ������
	�ۼ��� : 2004�� 3�� 4��

	��  �� :
		���� ���۸� ����Ͽ�, ��Ʈ��ũ �޽������� ��Ŷ������ �������Ͽ� ��ȯ���ش�.
		���� ������ ũ��� �ִ� ��Ŷ ũ���� 2�谡 �Ǿ�� �Ѵ�.
		Length Field�� ũ��� 1, 2, 4Byte �������� ���� �� �ִ�.
	��  �� :
		1. Create�� ���ؼ� ���� ���۸� �����Ѵ�.
		2. Init~~ �� ���ؼ� ��Ŷ�� ������ �����Ѵ�.
		3. ó�� ����� ������ Reset�� ���ش�.
		4. �޽����� �����ϸ� Assemble�� ���ؼ� ���� ���۷� �޽������� �״´�.
		5. 1�� Assemble�ϸ� GetCompletePacket�� ���� ��Ŷ�� ��ȯ�� ������ �ݺ��Ͽ�, ������ ��Ŷ�� �޴´�.
		6. 4, 5���� �ݺ��ϸ鼭 �����Ѵ�.
		7. Destroy�� ���ؼ� ���� ���۸� �����Ѵ�.

	* Length Base Packet Assembler�� �����Ǿ� �ִ�.
	* ���Ŀ� Charector Base Packet Assembler�� �����ؾ� �Ѵ�.
*****************************************************************************************************************/

#pragma once

#include <Windows.H>
#include "PacketCheckSum.h"
#include "SendPacket.h"
#include "..\..\DataStructure\List.h"
#include "..\..\DataStructure\ListEx.h"
#include "PacketBuffer.h"

namespace Session
{
	namespace Message
	{
		class CPacketAssembler
		{
		public:
			CPacketAssembler(void);
			~CPacketAssembler(void);

		public:
			// ��Ŷ ����
			enum	PacketUnitType
			{
				Length		= 1,	// ���̷� ����
				Charector	= 2		// ���� ���ڷ� ����
			};

			// Length Type
			enum	LengthType
			{
				Total			= 1,	// ��Ŷ ��ü�� ����
				After			= 2,	// Length �ʵ� ���������� ����
				IncludeAfter	= 3,	// ��Ŷ ��ü�� ���̿��� Length �ʵ��� ���� ������ ����
			};

			// Length Size Type
			enum	LengthSizeType
			{
				OneByte			= 1,
				TwoByte			= 2,
				FourByte		= 4,
			};

			// Attributes
		private:
			// ���� �ʵ��� ����
			struct TagLengthField
			{
				int		nPos;				// Length Field�� ��ġ
				int		nSize;				// Length Field�� ����
				BOOL	bLittleEndian;		// Length Field�� Little Endian���� ����
			};

			struct	TagAssembleData
			{
				BOOL		bFirst;			// Assemble�� ù��° ����Ÿ���� ����
				char*		pDataBuffer;	// Assemble���� ����Ÿ ������ ������
				int			nLength;		// Assemble�� ����Ÿ�� �� ����
				long int	nPacketLength;	// Assemble�� ��Ŷ�� ����
				char		cFirstChar;		// Assemble�� ��Ŷ ��ȯ�ÿ� ����Ÿ�� �ڿ� �پ� ���� ���
				// ��Ŷ�� �������� NULL�� ġȯ�Ǵ� �κ�, ���� ����Ÿ�� ù��° char
			};

			char*			m_aBuffer;			// ���� �迭�� �Ҵ�޴� ���� ����
			int				m_nBufferSize;		// ���� ������ ũ��

			char*			m_aSendBuffer;		// Send Buffer
			int				m_nSendBufferSize;

			BOOL			m_bCloseWait;		// ���� ��� ���� : Send List�� ��� �����Ҷ� ����Ѵ�.

			DWORD			m_dwHandle;
			static DWORD	g_dwHandle;			//closeʱ++����������Ψһ��


			BOOL			m_bFree;			// Free ��ų�� ����
			BOOL			m_bOver;


			TagAssembleData	m_AssembleData;		// Assemble ����Ÿ ����

			PacketUnitType	m_PacketUnitType;	// ��Ŷ�� ������ ����

			CPacketCheckSum::CheckSumType		// ��Ŷ üũ�� Ÿ��
							m_PacketCheckSumType;

			TagLengthField	m_LengthFieldInfo;	// ���� �ʵ��� ����
			char			m_cTerminatedChar;	// ������ ����

			Manage::CSendPacket*	m_pSendPacketHead;
			Manage::CSendPacket*	m_pSendPacketRear;

			Common::DataStructure::CListEx< Manage::CPacketBuffer* >
								m_lstPacketBuffer;
			Common::DataStructure::CList< Manage::CPacketBuffer* >
								m_lstFirstPacketBuffer;	// �켱������ ���� ��Ŷ ���� ����Ʈ

			// SendIndex�� 1�̻��̸�, 0�� Ping ���� �켱 ó�� ��Ŷ�� ����Ѵ�.
			DWORD			m_dwSendIndex;			// Send�� Index : 1�̻��̾�� �ϸ�, �� ������ ���ų� ���� ��Ŷ�� ���۽�Ų��.
			DWORD			m_dwAllocSendIndex;		// �Ҵ��� Send Index : 1�̻��̾�� �ϸ�, �Ҵ�ø��� 1�� ������Ų��.

			CRITICAL_SECTION		m_cs;

		public:
			int						m_nQueueingCount;

			// Operations
		public:
			/*/
			DWORD		GetSendIndex()
			{
				return m_dwSendIndex;
			}
			/*/
			BOOL		IsSending( DWORD dwSendIndex )
			{
				return m_dwSendIndex >= dwSendIndex;
			}
			VOID		AddSendIndex()
			{
				m_dwSendIndex++;
			}
			DWORD		AllocSendIndex()
			{
				DWORD dwIndex;
//					::EnterCriticalSection( &m_cs );

				dwIndex = m_dwAllocSendIndex;
				m_dwAllocSendIndex++;

//					::LeaveCriticalSection( &m_cs );
				return dwIndex;
			}

			// ���� ���۸� �����Ѵ�.
			BOOL		Create( int nBufferSize, INT nSendPacketListSize/* = 500*/ );
			// ���� ���۸� �����Ѵ�.
			void		Destroy();

			// ������ ��Ŷ�� Ÿ���� �����Ѵ�.
			void		Init(
				CPacketCheckSum::CheckSumType	checkSumType,
				PacketUnitType					packetUnitType = PacketUnitType::Length );
			// ���� ��� ��Ŷ���� Length �ʵ带 �����Ѵ�.
			void		InitLengthField(
				int				nLengthPos,
				LengthSizeType	lengthSizeType,
				LengthType		lengthType = LengthType::Total,
				BOOL			bLittleEndian = true );
			// ���� ��� ��Ŷ���� ���� ���ڸ� �����Ѵ�.
			void		InitTerminatedCharector( char cTerminatedChar );

			// ������ �Ѵ�. ���� ���� ������ ũ�⸦ �ʰ��ϰԵǸ� false�� ��ȯ�Ѵ�.
			BOOL		Assemble( char* buffer, int nBufferSize );
			// ������ �ϳ��� ������ ��Ŷ�� ��ȯ�Ѵ�.
			void*		GetCompletePacket( int& nOutPacketSize, int& nOutResult );

			// Assemble �۾��� Reset�Ѵ�.
			void		Reset();

			Common::DataStructure::CListEx< Manage::CPacketBuffer* >*
						GetPacketBufferList()
			{
				return	&m_lstPacketBuffer;
			}
			Common::DataStructure::CList< Manage::CPacketBuffer* >*
						GetFirstPacketBufferList()
			{
				return	&m_lstFirstPacketBuffer;
			}

			void		AddCheckSum( const char* pData, int nDataLength );
			void		SetCheckSum( CHAR* pData, INT nDataLength, INT& out_nDataLength );

			char*					GetSendBuffer();
			int						GetSendSize();
			BOOL					IsEmptySendList();
			Manage::CSendPacket*	RemoveFromSendList( int nSendingLength );
			void					AddToSendList( Manage::CSendPacket* pSendPacket, BOOL bFirst = FALSE );
			Manage::CSendPacket*	FreeSendList();

			BOOL		IsCreated()
			{
				return m_aBuffer != NULL;
			}

			BOOL		IsFree()
			{
				return m_bFree;
			}
			VOID		SetFree()
			{
				m_bFree = TRUE;
			}

			BOOL		IsOverlapped()
			{
				return m_bOver;
			}
			VOID		SetOverlapped( BOOL bOver )
			{
				m_bOver = bOver;
			}

			VOID		ResetHandle()
			{
				m_dwHandle = g_dwHandle++;
			}

			DWORD		GetHandle()
			{
				return m_dwHandle;
			}

			BOOL					IsCloseWait()
			{
				return m_bCloseWait;
			}
			void					ResetCloseWait()
			{
				m_bCloseWait = FALSE;
			}

			void	Lock()
			{
				EnterCriticalSection( &m_cs );
			}
			void	Unlock()
			{
				LeaveCriticalSection( &m_cs );
			}

		private:
			// ���� ����� ���� ��Ŷ�� ��ȯ�Ѵ�.
			void*	GetCompletePacketLengthType( int& nOutPacketSize, int& nOutResult );
			// ���� ����� ���� ��Ŷ�� ��ȯ�Ѵ�.
			void*	GetCompletePacketCharType( int& nOutPacketSize, int& nOutResult );
		};// class CPacketAssembler

	}// namespace Message

}// namespace Session