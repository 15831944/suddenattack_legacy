#include "StdAfx.h"
#include ".\packetassembler.h"

using namespace Session::Message;
using namespace Session::Manage;


DWORD CPacketAssembler::g_dwHandle = 0;
CPacketAssembler::CPacketAssembler(void)
:	m_aBuffer			( NULL	)
,	m_nBufferSize		( 0		)
,	m_aSendBuffer		( NULL	)
,	m_nSendBufferSize	( 0		)
,	m_cTerminatedChar	( NULL	)
{
	m_LengthFieldInfo.nPos			= -1;
	m_LengthFieldInfo.nSize			= 0;
	m_LengthFieldInfo.bLittleEndian	= true;

	m_AssembleData.bFirst			= true;
	m_AssembleData.nLength			= 0;
	m_AssembleData.pDataBuffer		= NULL;
	m_AssembleData.cFirstChar		= NULL;

	m_nQueueingCount				= 0;
}

CPacketAssembler::~CPacketAssembler(void)
{
}

// ���� ���۸� �����Ѵ�.
BOOL CPacketAssembler::Create( int nBufferSize, INT nSendPacketListSize )
{
	m_aBuffer = new char[nBufferSize];
	if( m_aBuffer == NULL )
		return false;
	m_nBufferSize = nBufferSize;


	m_aSendBuffer = new char[nBufferSize];
	if( m_aSendBuffer == NULL )
		return false;
	m_nSendBufferSize = nBufferSize;

	m_pSendPacketHead			= NULL;
	m_pSendPacketRear			= NULL;
//	m_dwHandle					= 0;

	m_AssembleData.bFirst		= true;
	m_AssembleData.nLength		= 0;
	m_AssembleData.pDataBuffer	= NULL;
	m_AssembleData.cFirstChar	= NULL;

	m_bCloseWait				= FALSE;

	m_bFree						= FALSE;
	m_bOver						= FALSE;

	m_dwHandle					= g_dwHandle++;

	m_lstPacketBuffer.Create( nSendPacketListSize );
//	m_lstFirstPacketBuffer.Create( 50 );

	::InitializeCriticalSection( &m_cs );

//	char a1[100];
//	sprintf( a1, "Alloc %d\r\n", (int)this );
//	::OutputDebugString( a1 );

	return true;
}

// ���� ���۸� �����Ѵ�.
void CPacketAssembler::Destroy()
{
	if( m_aBuffer )
	{
		delete[] m_aBuffer;

		m_aBuffer		= NULL;
		m_nBufferSize	= 0;
	}
	if( m_aSendBuffer )
	{
		delete[] m_aSendBuffer;

		m_aSendBuffer		= NULL;
		m_nSendBufferSize	= 0;
	}

	::DeleteCriticalSection( &m_cs );
}

// ������ ��Ŷ�� Ÿ���� �����Ѵ�.
void CPacketAssembler::Init( CPacketCheckSum::CheckSumType checkSumType, PacketUnitType packetUnitType )
{
	m_PacketCheckSumType	= checkSumType;
	m_PacketUnitType		= packetUnitType;

	
	if( m_lstPacketBuffer.GetElementsCount() > 0 )
	{
		int a = 3;
	}
	if( m_lstPacketBuffer.GetElementsCount() < 0 )
	{
		int a = 3;
	}
	
//	ASSERT( m_lstPacketBuffer.GetElementsCount() == 0 );
//	ASSERT( m_lstFirstPacketBuffer.GetElementsCount() == 0 );

	m_nQueueingCount		= 0;

	m_dwSendIndex			= 1;
	m_dwAllocSendIndex		= 1;
}

// ������Ϣ�������ԡ����ô���
//1��connect��Accep��
//2��ITcpNetworkHandler::OnAllocNetworkObject;�Ƚ�һ��CPacketAssembler��socket�ؼ��ֽ��ж�Ӧ,Ȼ��InitPacketType->InitLengthFieldOfPacket
//3: ITcpNetworkHandlerʵ����CSessionManager::InitLengthFieldOfPacket�е��øú���
void CPacketAssembler::InitLengthField( int nLengthPos, LengthSizeType lengthSizeType, LengthType lengthType/* = LengthType::Total*/, BOOL bLittleEndian/* = true*/ )
{
	m_LengthFieldInfo.nPos			= nLengthPos;
	m_LengthFieldInfo.nSize			= (int)lengthSizeType;
	m_LengthFieldInfo.bLittleEndian	= bLittleEndian;
}

// ���� ��� ��Ŷ���� ���� ���ڸ� �����Ѵ�.
void CPacketAssembler::InitTerminatedCharector( char cTerminatedChar )
{
	m_cTerminatedChar	= cTerminatedChar;
}

// ������ �Ѵ�.
BOOL CPacketAssembler::Assemble( char* buffer, int nBufferSize )
{
	if( nBufferSize > m_nBufferSize / 2 )
	{
		return false;
	}

	//
	// ù��° ����Ÿ�̸� ��Ź�� ���۸� �״�� ����Ѵ�.
	//
	if( m_AssembleData.bFirst )
	{
		m_AssembleData.pDataBuffer	= buffer;
		m_AssembleData.nLength		= nBufferSize;
		m_AssembleData.bFirst		= false;
	}

	//
	// ù��° ����Ÿ�� �ƴϸ� ���� ���� �ڿ� copy�� �Ѵ�.
	//
	else
	{
		if( nBufferSize + m_AssembleData.nLength > m_nBufferSize )
		{
			return false;
		}

		// ���� ���۸� ������̸� �ڿ� copy�� �Ѵ�.
		/*
		if( m_AssembleData.pDataBuffer == m_aBuffer )
		{
			::memcpy( &m_AssembleData.pDataBuffer[ m_AssembleData.nLength ], buffer, nBufferSize );
			m_AssembleData.nLength += nBufferSize;
		}
		else
		{
			// ���� ���� ����Ÿ�� �ڿ� Copy
			::memcpy( &m_AssembleData.pDataBuffer[ m_AssembleData.nLength ], buffer, nBufferSize );
			m_AssembleData.nLength += nBufferSize;
		}
		*/
		::memcpy( &m_AssembleData.pDataBuffer[ m_AssembleData.nLength ], buffer, nBufferSize );
		m_AssembleData.nLength += nBufferSize;
	}

	return true;
}

// ������ �ϳ��� ������ ��Ŷ�� ��ȯ�Ѵ�.
void* CPacketAssembler::GetCompletePacket( int& nOutPacketSize, int& nOutResult )
{
	switch( m_PacketUnitType )
	{
	case PacketUnitType::Length :
		return GetCompletePacketLengthType( nOutPacketSize, nOutResult );

	case PacketUnitType::Charector :
		return GetCompletePacketCharType( nOutPacketSize, nOutResult );
	}

	return NULL;
}

// Assemble �۾��� Reset�Ѵ�.
void CPacketAssembler::Reset()
{
	m_AssembleData.bFirst		= true;
	m_AssembleData.nLength		= 0;
	m_AssembleData.pDataBuffer	= NULL;
	m_AssembleData.cFirstChar	= NULL;

	m_bCloseWait				= TRUE;


	if( m_bOver == TRUE )
		int a = 3;
	m_bOver						= FALSE;
	m_bFree						= FALSE;
}

void CPacketAssembler::AddCheckSum( const char* pData, int nDataLength )
{
	::memcpy( m_aSendBuffer, pData, nDataLength );

	CPacketCheckSum::SetCheckSum( m_PacketCheckSumType, (char*)m_aSendBuffer, nDataLength );
	m_nSendBufferSize = nDataLength + CPacketCheckSum::GetCheckSumSize( m_PacketCheckSumType );
}

void CPacketAssembler::SetCheckSum( CHAR* pData, INT nDataLength, INT& out_nDataLength )
{
	CPacketCheckSum::SetCheckSum( m_PacketCheckSumType, pData, nDataLength );
	out_nDataLength = nDataLength + CPacketCheckSum::GetCheckSumSize( m_PacketCheckSumType );
}

// ���� ����� ���� ��Ŷ�� ��ȯ�Ѵ�.
inline void* CPacketAssembler::GetCompletePacketLengthType( int& nOutPacketSize, int& nOutResult )
{

	// Length : 2Byte
	// Packet : Length + 4Byte
	// Tale : 2Byte

	nOutPacketSize = 0;
	nOutResult = 0;

//	int		nBaseLengthFieldSize	= 2;
	int		nCheckSumSize			= CPacketCheckSum::GetCheckSumSize( m_PacketCheckSumType );

	//
	// Length Field�� ���� ���ϸ� NULL�� ��ȯ�Ѵ�.
	//
	if( m_AssembleData.nLength < m_LengthFieldInfo.nSize )	//����һ�����Ļ�������ȡ
	{
		// ��Ź�� ���۸� ���� ���۷� Copy
		::memcpy( m_aBuffer, m_AssembleData.pDataBuffer, m_AssembleData.nLength );
		m_AssembleData.pDataBuffer = m_aBuffer;

		return NULL;
	}

	//
	// Length Field�� ���ؼ� packet�� length�� ���Ѵ�.
	//
	char* pLengthField = &m_AssembleData.pDataBuffer[m_LengthFieldInfo.nPos];
	if( m_LengthFieldInfo.bLittleEndian )
	{
		short	nLength;
		::memcpy( &nLength, pLengthField, m_LengthFieldInfo.nSize );
		m_AssembleData.nPacketLength = nLength;// + å�������
	}
	else
	{
	}

	//
	// �߸��� Length �ʵ��� ���, Exception �߻�
	//
	if( m_AssembleData.nPacketLength < m_LengthFieldInfo.nPos ||
		m_AssembleData.nPacketLength > m_nBufferSize / 2 )
	{

//		throw 0;
		nOutResult = 1;
		return NULL;
	}

	//
	// ��Ŷ�� ���� 3���� ���� ���еȴ�.
	//		Case 1. ��������� �޽����� �ϳ��� ��Ŷ�� ���
	//		Case 2. ��������� �޽����� ��Ŷ�� �߰������� �� ���
	//		Case 3. ��������� �޽����� �ϳ� �̻��� ��Ŷ�� ���
	//

	// Case 1
	if( m_AssembleData.nLength ==
		m_AssembleData.nPacketLength + CPacketCheckSum::GetCheckSumSize( m_PacketCheckSumType ) )
	{
		// äũ���� �´����� ���Ѵ�.
		if( !CPacketCheckSum::IsCorrectCheckSum(
				m_PacketCheckSumType,
				m_AssembleData.pDataBuffer,
				m_AssembleData.nPacketLength ) )
		{
			nOutResult = 2;
			return NULL;
		}

		char* pDataBuffer;
		
		// Length Type�� ����.
		// Length Type�� ���� Total�� �����ȴ�.

		pDataBuffer = m_AssembleData.pDataBuffer;

//		pDataBuffer[m_AssembleData.nPacketLength-nCheckSumSize] = NULL;

		m_AssembleData.bFirst		= true;
		m_AssembleData.nLength		= 0;
		m_AssembleData.pDataBuffer	= NULL;
		m_AssembleData.cFirstChar	= NULL;

		nOutPacketSize = m_AssembleData.nPacketLength;// + nCheckSumSize;// + 1;	// +1 : NULL�� ����

		return pDataBuffer;
	}

	// Case 2
	else if( m_AssembleData.nLength <
		m_AssembleData.nPacketLength + CPacketCheckSum::GetCheckSumSize( m_PacketCheckSumType ) )
	{
		// ��Ź�� ���۸� ���� ���۷� Copy
		::memcpy( m_aBuffer, m_AssembleData.pDataBuffer, m_AssembleData.nLength );
		m_AssembleData.pDataBuffer = m_aBuffer;

		return NULL;
	}

	// Case 3
	else
	{
		// äũ���� �´����� ���Ѵ�.
		if( !CPacketCheckSum::IsCorrectCheckSum(
				m_PacketCheckSumType,
				m_AssembleData.pDataBuffer,
				m_AssembleData.nPacketLength ) )
		{
			nOutResult = 3;
			return NULL;
		}

		char* pDataBuffer;
		
		// Length Type�� ����.
		// Length Type�� ���� Total�� �����ȴ�.

		pDataBuffer = m_AssembleData.pDataBuffer;

//		pDataBuffer[m_AssembleData.nPacketLength-nCheckSumSize] = NULL;

		m_AssembleData.nLength		= m_AssembleData.nLength - m_AssembleData.nPacketLength - nCheckSumSize;
		m_AssembleData.pDataBuffer	= &m_AssembleData.pDataBuffer[m_AssembleData.nPacketLength+nCheckSumSize];

		nOutPacketSize = m_AssembleData.nPacketLength;// + nCheckSumSize;


		return pDataBuffer;
	}


	return NULL;
}

// ���� ����� ���� ������ �Ѵ�.
inline void* CPacketAssembler::GetCompletePacketCharType( int& nPacketSize, int& nOutResult )
{
	return NULL;
}


BOOL CPacketAssembler::IsEmptySendList()
{
	if( m_pSendPacketHead == NULL )
		return TRUE;
	else
		return FALSE;
}

CSendPacket* CPacketAssembler::RemoveFromSendList( int nSendingLength )
{
	if( m_pSendPacketHead == NULL )
	{
		return NULL;
	}

	ASSERT( m_pSendPacketRear );

	CSendPacket*	pSendPacket	= m_pSendPacketHead;
	int				nRestSend	= pSendPacket->GetRestSend() - nSendingLength;

	if( nRestSend > 0 )
	{
		pSendPacket->SetRestSend( nRestSend );
		return NULL;
	}
	else
	{
		m_pSendPacketHead = pSendPacket->GetNext();
		if( m_pSendPacketHead == NULL )
			m_pSendPacketRear = NULL;
		m_nQueueingCount--;
		return pSendPacket;
	}
}

void CPacketAssembler::AddToSendList( CSendPacket* pSendPacket, BOOL bFirst )
{
	CPacketCheckSum::SetCheckSum( m_PacketCheckSumType, pSendPacket->GetData(), pSendPacket->GetDataSize() );
	pSendPacket->SetDataSize( pSendPacket->GetDataSize() + CPacketCheckSum::GetCheckSumSize( m_PacketCheckSumType ) );

	ASSERT( pSendPacket );

	// bFirst : Q�� �켱������ Enqueue

	if( !bFirst )
	{
		if( m_pSendPacketHead == NULL )
		{
			m_pSendPacketHead = m_pSendPacketRear = pSendPacket;
		}
		else
		{
			ASSERT( m_pSendPacketRear );
			m_pSendPacketRear->SetNext( pSendPacket );
			m_pSendPacketRear = pSendPacket;
		}

		// For Debug
		ASSERT( pSendPacket != pSendPacket->GetNext() );
		ASSERT( m_pSendPacketHead != m_pSendPacketHead->GetNext() );
		
	}
	else
	{
		if( m_pSendPacketHead )
		{
			pSendPacket->SetNext( m_pSendPacketHead->GetNext() );
			m_pSendPacketHead->SetNext( pSendPacket );
			if( m_pSendPacketHead == m_pSendPacketRear )
				m_pSendPacketRear = pSendPacket;

			ASSERT( m_pSendPacketRear != NULL );
		}
		else
		{
			m_pSendPacketHead = m_pSendPacketRear = pSendPacket;
		}

		// For Debug
		ASSERT( pSendPacket != pSendPacket->GetNext() );
		ASSERT( m_pSendPacketHead != m_pSendPacketHead->GetNext() );
	}
	m_nQueueingCount++;
}

CSendPacket* CPacketAssembler::FreeSendList()
{
	if( m_pSendPacketHead == NULL )
		return NULL;
	CSendPacket* pSendPacket	= m_pSendPacketHead;
	m_pSendPacketHead			= m_pSendPacketHead->GetNext();
	if( m_pSendPacketHead == NULL )
		m_pSendPacketRear = NULL;

	m_nQueueingCount--;
	return pSendPacket;
}

char* CPacketAssembler::GetSendBuffer()
{
	return m_pSendPacketHead->GetData();
}

int CPacketAssembler::GetSendSize()
{
	if( m_pSendPacketHead == NULL )
	{
		return 0;
	}
	return m_pSendPacketHead->GetRestSend();
}
