#pragma once

#include "PacketCheckSum.h"
#include "SignalQueue.h"

#include "PacketBuffer.h"
#include "QueryBuffer.h"

namespace GamehiSolution
{
	namespace Session
	{
		namespace Message
		{
			class CMessage
			{
			public:
				enum	MessageType
				{
					SignalType		= 1,	// Event Type

					AcceptType		= 2,	// Accept �Ǿ����� ����
					ConnectType		= 3,	// Connect �Ǿ����� ����
					BeginConnectType= 4,
					DataType		= 5,	// Recv Data���� ����
					CloseType		= 6,	// Peer�� Close ������ �� ����
					DisconnectType	= 7,	// �ɵ������� Close�Ǿ��� �� ����

					FirstSend		= 8,
					DoCloseType		= 9,	// �ɵ������� Close�� ��Ű��� ���� : Send Queue�� ���
					DoShutdownType	= 10,	// �ɵ������� Close�� ��Ű��� ���� : Send Queue�� ���

					HttpRequest		= 11,
					HttpResponse	= 12,
					HttpResponseEnd	= 13,
					HttpError		= 14,
				};

			protected:
				CHAR*			m_Buffer;
				INT				m_nBufferSize;
				INT				m_nPacketSize;	// ���� ��Ŷ�� ũ��
				CSignal			m_Signal;

				unsigned long		m_lpNetworkKey;	// ��Ŷ�� ������ object�� Ű
//				DWORD			m_dwSendHandle;	// Send ���� NetworkKey�� handle

				MessageType		m_MessageType;	// �޽����� Ÿ��
				CPacketCheckSum::CheckSumType
					m_CheckSumType;	// ��Ŷ äũ�� Ÿ��

				DWORD			m_dwHttpResult;
				CHAR*			m_lpszServerName;
				CHAR*			m_lpszObjectName;
				CHAR*			m_lpszBody;

				Manage::CPacketBuffer*		m_pPacketBuffer;
				Manage::CRequestObject*		m_pRqeustObject;

			public:
				CMessage( VOID );
				CMessage( const CSignal& signal );
				CMessage( MessageType packetType );
				CMessage( unsigned long lpNetworkKey, MessageType packetType );
				CMessage( unsigned long lpNetworkKey, CHAR* pPacketBuffer, INT nPacketSize );

				CMessage( unsigned long lpNetworkKey, MessageType packetType, DWORD dwHttpResult );



				CMessage( MessageType messageType, unsigned long lpNetworkKey, Manage::CPacketBuffer* pPacketBuffer )
				{
					m_MessageType	= messageType;
					m_lpNetworkKey	= lpNetworkKey;
					m_pPacketBuffer	= pPacketBuffer;
				}
				CMessage( MessageType messageType, Manage::CRequestObject* pRqeustObject )
				{
					m_MessageType	= messageType;
					m_pRqeustObject	= pRqeustObject;
				}
				CMessage( MessageType messageType, Manage::CRequestObject* pRqeustObject, Manage::CPacketBuffer* pPacketBuffer )
				{
					m_MessageType	= messageType;
					m_pRqeustObject	= pRqeustObject;
					m_pPacketBuffer	= pPacketBuffer;
				}

				virtual ~CMessage( VOID );

			public:
				// ������ VOID�� �Ͽ��� ��� ���� ���۸� ������Ű�� ���ؼ� Create �Ѵ�.
//				virtual bool	Create( INT nBufferSize );
//				virtual VOID	Destroy();

				Manage::CPacketBuffer*
								GetPacketBuffer()
				{
					return m_pPacketBuffer;
				}
				Manage::CRequestObject*
								GetRequestObject()
				{
					return m_pRqeustObject;
				}
				Manage::CQueryBuffer*
								GetQueryBuffer()
				{
					return m_pRqeustObject->GetQueryBuffer();
				}

				virtual bool	Create( INT nBufferSize ){return TRUE;}
				virtual VOID	Destroy(){}

				// ��Ŷ Ÿ���� ī���ϸ�, ������ Ÿ���� ��� ���� ���۸� ī���Ѵ�.
				// ī�Ǹ� �޴� ��Ŷ�� ���۰� NULL�̸�, ������ Ÿ���� ��� ���� ���۸� ī���� �� ����.
				VOID	operator=( CMessage message );

				// ��Ŷ�� ������ Peer�� Key�Ǵ� �� Key�� �����ϴ� Packet,
				// ��Ŷ ������
				// ��Ŷ�� ����
				VOID	SetSendPacket( const CMessage& PeerPacket, VOID* pPacket, INT nLen );
				VOID	SetSendPacket( unsigned long lpPeerNetworkKey, VOID* pPacket, INT nLen );

				VOID	SetHttpData( unsigned long lpNetworkKey, const CHAR* lpszServerName, const CHAR* lpszObjectName, const CHAR* lpszBody );

//				VOID	SetSendHandle( DWORD dwSendHandle )
//				{
//					m_dwSendHandle = dwSendHandle;
//				}
//				DWORD	GetSendHandle()
//				{
//					return m_dwSendHandle;
//				}

				// ��Ŷ Ÿ���� ��ȯ�Ѵ�.
				MessageType	GetType()
				{
					return m_MessageType;
				}

				// ��Ŷ Ÿ���� �����Ѵ�.
				VOID		SetType( MessageType messageType )
				{
					m_MessageType = messageType;
				}

				CHAR*		GetBuffer()
				{
					return m_pPacketBuffer->GetData();
				}

				VOID		SetHttpData( INT nSize, unsigned long lpNetworkKey )
				{
					m_MessageType		= MessageType::HttpResponse;
					m_nPacketSize		= nSize;
					m_lpNetworkKey		= lpNetworkKey;
				}

				INT			GetHttpResult()
				{
					return m_dwHttpResult;
				}

				const CHAR* GetServerName()
				{
					return m_lpszServerName;
				}
				const CHAR* GetObjectName()
				{
					return m_lpszObjectName;
				}
				const CHAR* GetBody()
				{
					return m_lpszBody;
				}

				// ��Ŷ�� äũ�� Ÿ���� �����Ѵ�.
				VOID		SetCheckSumType( CPacketCheckSum::CheckSumType checkSumType )
				{
					m_CheckSumType = checkSumType;
				}
				// ��Ŷ�� äũ�� Ÿ���� ��ȯ�Ѵ�.
				CPacketCheckSum::CheckSumType
					GetCheckSumType()
				{
					return m_CheckSumType;
				}


				unsigned long	GetNetworkKey()
				{
					return m_lpNetworkKey;
				}

				VOID		SetNetworkKey( unsigned long lpNetworkKey )
				{
					m_lpNetworkKey = lpNetworkKey;
				}

				INT			GetPacketSize()
				{
					return m_pPacketBuffer->GetDataSize();
				}

				CSignal		GetSignal()
				{
					return m_Signal;
				}
			};// class CMessage

		}// namespace Message

	}// namespace Session

}// namespace GamehiSolution