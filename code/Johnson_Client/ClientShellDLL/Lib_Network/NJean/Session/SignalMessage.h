/*
#pragma once

#include "PacketCheckSum.h"
#include "EventQueue.h"

namespace GamehiSolution
{
	namespace Session
	{
		namespace Message
		{
			class CMessage
			{
			public:
				enum	PacketType
				{
					EventType		= 1,	// Event Type

					AcceptType		= 2,	// Accept �Ǿ����� ����
					ConnectType		= 3,	// Connect �Ǿ����� ����
					DataType		= 4,	// Recv Data���� ����
					CloseType		= 5,	// Peer�� Close ������ �� ����
					DisconnectType	= 6,	// �ɵ������� Close�Ǿ��� �� ����

					FirstSend		= 7,
					DoCloseType		= 8,	// �ɵ������� Close�� ��Ű��� ���� : Send Queue�� ���

					HttpRequest		= 9,
					HttpResponse	= 10,
					HttpResponseEnd	= 11,
					HttpError		= 12,
				};

			protected:
				char*			m_Buffer;
				int				m_nBufferSize;
				int				m_nPacketSize;	// ���� ��Ŷ�� ũ��
				CEvent			m_Event;

				unsigned long	m_dwNetworkKey;	// ��Ŷ�� ������ object�� Ű

				PacketType		m_PacketType;	// ��Ŷ�� Ÿ��
				CPacketCheckSum::CheckSumType
					m_CheckSumType;	// ��Ŷ äũ�� Ÿ��

				DWORD			m_dwHttpResult;
				char*			m_lpszServerName;
				char*			m_lpszObjectName;
				char*			m_lpszBody;

			public:
				CPacket( void );
				CPacket( const CEvent& event );
				CPacket( PacketType packetType );
				CPacket( unsigned long dwNetworkKey, PacketType packetType );
				CPacket( unsigned long dwNetworkKey, char* pPacketBuffer, int nPacketSize );

				CPacket( unsigned long dwNetworkKey, PacketType packetType, DWORD dwHttpResult );
				~CPacket( void );

			public:
				// ������ void�� �Ͽ��� ��� ���� ���۸� ������Ű�� ���ؼ� Create �Ѵ�.
				virtual bool	Create( int nBufferSize );
				virtual void	Destroy();

				// ��Ŷ Ÿ���� ī���ϸ�, ������ Ÿ���� ��� ���� ���۸� ī���Ѵ�.
				// ī�Ǹ� �޴� ��Ŷ�� ���۰� NULL�̸�, ������ Ÿ���� ��� ���� ���۸� ī���� �� ����.
				void	operator=( CPacket packet );

				// ��Ŷ�� ������ Peer�� Key�Ǵ� �� Key�� �����ϴ� Packet,
				// ��Ŷ ������
				// ��Ŷ�� ����
				void	SetSendPacket( const CPacket& PeerPacket, void* pPacket, int nLen );
				void	SetSendPacket( unsigned long dwPeerNetworkKey, void* pPacket, int nLen );

				void	SetHttpData( unsigned long dwNetworkKey, const char* lpszServerName, const char* lpszObjectName, const char* lpszBody );

				// ��Ŷ Ÿ���� ��ȯ�Ѵ�.
				PacketType	GetType()
				{
					return m_PacketType;
				}

				// ��Ŷ Ÿ���� �����Ѵ�.
				void		SetType( PacketType packetType )
				{
					m_PacketType = packetType;
				}

				char*		GetBuffer()
				{
					return m_Buffer;
				}

				void		SetHttpData( int nSize, unsigned long dwNetworkKey )
				{
					m_PacketType	= PacketType::HttpResponse;
					m_nPacketSize	= nSize;
					m_dwNetworkKey	= dwNetworkKey;
				}

				int			GetHttpResult()
				{
					return m_dwHttpResult;
				}

				const char* GetServerName()
				{
					return m_lpszServerName;
				}
				const char* GetObjectName()
				{
					return m_lpszObjectName;
				}
				const char* GetBody()
				{
					return m_lpszBody;
				}

				// ��Ŷ�� äũ�� Ÿ���� �����Ѵ�.
				void		SetCheckSumType( CPacketCheckSum::CheckSumType checkSumType )
				{
					m_CheckSumType = checkSumType;
				}
				// ��Ŷ�� äũ�� Ÿ���� ��ȯ�Ѵ�.
				CPacketCheckSum::CheckSumType
					GetCheckSumType()
				{
					return m_CheckSumType;
				}


				unsigned long GetNetworkKey()
				{
					return m_dwNetworkKey;
				}

				void		SetNetworkKey( unsigned long dwNetworkKey )
				{
					m_dwNetworkKey = dwNetworkKey;
				}

				int			GetPacketSize()
				{
					return m_nPacketSize;
				}

				CEvent		GetEvent()
				{
					return m_Event;
				}
			};// class CMessage

		}// namespace Message

	}// namespace Session

}// namespace GamehiSolution
*/