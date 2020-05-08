#pragma once

#include "PacketCheckSum.h"
#include "SignalQueue.h"

#include "PacketBuffer.h"
#include "QueryBuffer.h"

#include "../../DataStructure/ListEx.h"


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
				DataType		= 4,	// Recv Data���� ����
				CloseType		= 5,	// Peer�� Close ������ �� ����
				DisconnectType	= 6,	// �ɵ������� Close�Ǿ��� �� ����

				FirstSend		= 7,
				DoCloseType		= 8,	// �ɵ������� Close�� ��Ű��� ���� : Send Queue�� ���
				DoShutdownType	= 9,	// �ɵ������� Close�� ��Ű��� ���� : Send Queue�� ���

				HttpRequest		= 10,
				HttpResponse	= 11,
				HttpResponseEnd	= 12,
				HttpError		= 13,


				DbReqeust		= 14,
				DbResponse		= 15,
			};

		protected:
			CHAR*			m_Buffer;
			INT				m_nBufferSize;
			INT				m_nPacketSize;	// ���� ��Ŷ�� ũ��
	
			CSignal			m_Signal;		//SignalType��Ϣ

			ULONG_PTR		m_lpNetworkKey;	// TCPΪCOverlappedListenSocket��HTTPΪCRequestObject
	
			Manage::CRequestObject*		m_pRqeustObject;
			
			Manage::CDbObject*			m_pDbObject;		//DbResponse
			Manage::CDbConnObject*		m_pDbConnObject;

			MessageType		m_MessageType;	// �޽����� Ÿ��
			CPacketCheckSum::CheckSumType
							m_CheckSumType;	// ��Ŷ äũ�� Ÿ��

			DWORD			m_dwHttpResult;
			CHAR*			m_lpszServerName;
			CHAR*			m_lpszObjectName;
			CHAR*			m_lpszBody;

//				DWORD			m_dwSending;

			Manage::CPacketBuffer*		m_pPacketBuffer;
			Common::DataStructure::CListEx<Manage::CPacketBuffer*>*	
										m_pPacketBufferList;


		public:
			CMessage( VOID );
			CMessage( const CSignal& signal );
			CMessage( MessageType packetType );
			CMessage( ULONG_PTR lpNetworkKey, MessageType packetType );
			CMessage( ULONG_PTR lpNetworkKey, CHAR* pPacketBuffer, INT nPacketSize );

			CMessage( ULONG_PTR lpNetworkKey, MessageType packetType, DWORD dwHttpResult );



			CMessage( MessageType messageType, ULONG_PTR lpNetworkKey, Common::DataStructure::CListEx<Manage::CPacketBuffer*>* pPacketList )
			{
				m_MessageType		= messageType;
				m_lpNetworkKey		= lpNetworkKey;
				m_pPacketBufferList	= pPacketList;
			}
			CMessage( MessageType messageType, ULONG_PTR lpNetworkKey, Manage::CPacketBuffer* pPacketBuffer )
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


			CMessage( MessageType messageType, Manage::CDbObject* pDbObject )
			{
				m_MessageType	= messageType;
				m_pDbObject		= pDbObject;
			}

			CMessage( Manage::CDbConnObject* pDbConnObject )
			{
				m_MessageType	= CMessage::MessageType::DbReqeust;
				m_pDbConnObject = pDbConnObject;
			}
			
			virtual ~CMessage( VOID );

		public:
			// ������ VOID�� �Ͽ��� ��� ���� ���۸� ������Ű�� ���ؼ� Create �Ѵ�.
//				virtual bool	Create( INT nBufferSize );
//				virtual VOID	Destroy();
			Common::DataStructure::CListEx<Manage::CPacketBuffer*>*
							GetPacketBufferList()
			{
				return m_pPacketBufferList;
			}

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

			Manage::CDbObject*
							GetDbObject()
			{
				return m_pDbObject;
			}
			Manage::CDbConnObject*
							GetDbConnObject()
			{
				return m_pDbConnObject;
			}

			/*
			VOID	SetSendingIndex( DWORD dwIndex )
			{
				m_dwSending = dwIndex;
			}
			DWORD	GetSendingIndex()
			{
				return m_dwSending;
			}
			*/

			virtual bool	Create( INT nBufferSize ){return TRUE;}
			virtual VOID	Destroy(){}

			// ��Ŷ Ÿ���� ī���ϸ�, ������ Ÿ���� ��� ���� ���۸� ī���Ѵ�.
			// ī�Ǹ� �޴� ��Ŷ�� ���۰� NULL�̸�, ������ Ÿ���� ��� ���� ���۸� ī���� �� ����.
			VOID	operator=( CMessage message );

			// ��Ŷ�� ������ Peer�� Key�Ǵ� �� Key�� �����ϴ� Packet,
			// ��Ŷ ������
			// ��Ŷ�� ����
			VOID	SetSendPacket( const CMessage& PeerPacket, VOID* pPacket, INT nLen );
			VOID	SetSendPacket( ULONG_PTR lpPeerNetworkKey, VOID* pPacket, INT nLen );

			VOID	SetHttpData( ULONG_PTR lpNetworkKey, const CHAR* lpszServerName, const CHAR* lpszObjectName, const CHAR* lpszBody );

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

			VOID		SetHttpData( INT nSize, ULONG_PTR lpNetworkKey )
			{
				m_MessageType		= MessageType::HttpResponse;
				m_nPacketSize		= nSize;
				m_lpNetworkKey		= lpNetworkKey;
			}

			INT			GetHttpResult()
			{
				return m_dwHttpResult; //ʵ��Ϊ��Ϣ����ID���
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


			ULONG_PTR	GetNetworkKey()
			{
				return m_lpNetworkKey;
			}

			VOID		SetNetworkKey( ULONG_PTR lpNetworkKey )
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
