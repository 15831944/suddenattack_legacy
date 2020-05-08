#include "StdAfx.h"
#include ".\Message.h"

using namespace GamehiSolution::Session::Message;

CMessage::CMessage(VOID)
:	m_Buffer		(NULL)
,	m_nBufferSize	(0)
,	m_nPacketSize	(0)
{
}

CMessage::CMessage( const CSignal& signal )
:	m_MessageType	( MessageType::SignalType )
{
	m_Signal = signal;
}

CMessage::CMessage( MessageType MessageType )
:	m_MessageType	( MessageType )
{
}

CMessage::CMessage( unsigned long lpNetworkKey, MessageType MessageType )
:	m_MessageType	( MessageType	)
,	m_Buffer		( NULL			)
,	m_nBufferSize	( 0				)
,	m_nPacketSize	( 0				)
,	m_lpNetworkKey( lpNetworkKey)
{
}

CMessage::CMessage( unsigned long lpNetworkKey, MessageType MessageType, DWORD dwHttpResult )
:	m_MessageType	( MessageType	)
,	m_Buffer		( NULL			)
,	m_nBufferSize	( 0				)
,	m_nPacketSize	( 0				)
,	m_lpNetworkKey( lpNetworkKey)
,	m_dwHttpResult	( dwHttpResult	)
{
}

CMessage::CMessage( unsigned long lpNetworkKey, CHAR* pPacketBuffer, INT nPacketSize )
:	m_MessageType	( MessageType::DataType	)
,	m_Buffer		( pPacketBuffer			)
,	m_nBufferSize	( nPacketSize			)
,	m_nPacketSize	( nPacketSize			)
,	m_lpNetworkKey( lpNetworkKey		)
{
}

CMessage::~CMessage(VOID)
{
}

VOID CMessage::operator=( CMessage message )
{
	m_MessageType	= message.m_MessageType;
	m_lpNetworkKey	= message.m_lpNetworkKey;
//	m_dwSendHandle	= message.m_dwSendHandle;

	switch( m_MessageType )
	{
	case MessageType::DataType :
	case MessageType::HttpRequest :
	case MessageType::HttpResponse :
//		if( m_Buffer == NULL )
//			return;
//		::memcpy( m_Buffer, message.m_Buffer, message.m_nPacketSize );
		m_pPacketBuffer		= message.m_pPacketBuffer;
		m_nPacketSize		= message.m_nPacketSize;
		message.m_pPacketBuffer	= NULL;

		m_lpszServerName	= &m_Buffer[0];
		m_lpszObjectName	= &m_Buffer[50];
		m_lpszBody			= &m_Buffer[100];
		break;
		
	case MessageType::HttpResponseEnd :
	case MessageType::HttpError :
		m_dwHttpResult	= message.m_dwHttpResult;
		break;

	case MessageType::SignalType :
		m_Signal = message.m_Signal;
		break;

	default :
		break;
	}
}

// ��Ŷ�� üũ�� Ÿ��
// ��Ŷ�� ������ Peer�� Key�� �����ϴ� Packet,
// ��Ŷ ������
// ��Ŷ�� ����
VOID CMessage::SetSendPacket( const CMessage& PeerPacket, VOID* pPacket, INT nLen )
{
	m_lpNetworkKey	= PeerPacket.m_lpNetworkKey;
	::memcpy( m_Buffer, pPacket, nLen );

	m_nPacketSize = nLen;
	return;

	// ��Ŷ�� Check Sum�� �����Ѵ�.
	CPacketCheckSum::SetCheckSum( m_CheckSumType, (CHAR*)m_Buffer, nLen );

	m_nPacketSize = nLen + CPacketCheckSum::GetCheckSumSize( m_CheckSumType );
}

// ��Ŷ�� üũ�� Ÿ��
// ��Ŷ�� ������ Peer�� Key
// ��Ŷ ������
// ��Ŷ�� ����
VOID CMessage::SetSendPacket( unsigned long lpPeerNetworkKey, VOID* pPacket, INT nLen )
{
	m_lpNetworkKey = lpPeerNetworkKey;
	::memcpy( m_Buffer, pPacket, nLen );

	m_nPacketSize = nLen;
}


VOID CMessage::SetHttpData( unsigned long lpNetworkKey, const CHAR* lpszServerName, const CHAR* lpszObjectName, const CHAR* lpszBody )
{
	m_lpNetworkKey		= lpNetworkKey;
	m_lpszServerName	= &m_Buffer[0];
	m_lpszObjectName	= &m_Buffer[50];
	m_lpszBody			= &m_Buffer[100];

	INT nServerName	= 0;
	INT nObjectName	= 0;
	if( lpszServerName != NULL )
	{
		nServerName	= (INT)::strlen( lpszServerName	);
		nObjectName	= (INT)::strlen( lpszObjectName	);

		::memcpy( m_lpszServerName,	lpszServerName,	nServerName	);
		::memcpy( m_lpszObjectName,	lpszObjectName,	nObjectName	);
	}
	INT nBody = (INT)::strlen( lpszBody );

	::memcpy( m_lpszBody, lpszBody, nBody );

	m_lpszServerName	[nServerName]	= NULL;
	m_lpszObjectName	[nObjectName]	= NULL;
	m_lpszBody			[nBody]			= NULL;

	m_nPacketSize	= 100 + nBody + 1;
}

/*
BOOL CMessage::Create( INT nBufferSize )
{
	m_Buffer = new CHAR[nBufferSize];
	if( m_Buffer == NULL )
		return FALSE;

	m_nBufferSize = nBufferSize;
	return TRUE;
}

VOID CMessage::Destroy()
{
	if( m_Buffer )
	{
		delete[] m_Buffer;
		m_Buffer = NULL;
		m_nBufferSize = 0;
		m_nPacketSize = 0;
	}
}
*/