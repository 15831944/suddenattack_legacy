#pragma once

#include <map>

#include "./NJean/Session/SessionGate.h"
#include "./DataStructure/MemoryPool.h"

#include "./NJean/Application/Peer.h"

namespace GamehiSolution
{
	namespace ClientModule
	{
		using namespace GamehiSolution::Session;
		using namespace GamehiSolution::Application;

		/*
		interface	IClientGateHandler
		{
			virtual void	InitListenerPacketType( 
//				INT											nServicerIndex,
				Message::CPacketCheckSum::CheckSumType&		out_CheckSumType,
				Message::CPacketAssembler::PacketUnitType&	out_PacketUnitType )	= 0;

			virtual void	InitLengthFieldOfPacket(
//				INT											nServicerIndex,
				INT&										out_nLengthFieldPos,
				Message::CPacketAssembler::LengthSizeType&	out_lengthSizeType,
				Message::CPacketAssembler::LengthType&		out_lengthType,
				BOOL&										out_bLittleEndian )		= 0;
		};
		*/

		class CClientGate : public Session::Gate::CSessionGate
		{
		public:
			CClientGate();
			virtual ~CClientGate();

		public:
			enum	PacketType
			{
				AcceptType		= 0,
				ConnectType		= 1,
				BeginConnectType= 2,
				CloseType		= 3,
				ReceiveType		= 4,

			};

		public:
			enum	ServicerType
			{
				P2PServer	= 0,	// super peer���� connect
				Peers		= 1,	// peer�� accept
				Client		= 2,	// main server���� connect
			};

		private:
			int						m_nAliveManaged;	// ��� ����

			struct TagLastValue
			{
				INT					nServicerIndex;
				PacketType			packetType;
				Application::CPeer*	pPeer;
				CHAR*				pData;
				INT					nDataLength;
			};

//			IClientGateHandler*	m_pClientGateHandler;
			TagLastValue			m_LastValue;

			Application::CPeer		m_peer;		// connect With Server
			Common::DataStructure::CMemoryPool<Application::CPeer>
									m_PeerPool;		// ����� ��ü���� �޸� Ǯ
			INT						m_nNumOfClients;

			BOOL					m_bConnected;	// ���ӵǾ����� ����


			/*
			// User Session ������ ���� Map : IDD�� ���ؼ� CUser*�� �����´�.
			typedef std::pair<long, CUser*>					UserIdd_Pair;
			struct TagUserMap
			{
				std::map <long, CUser*>						mapUserIdd;
				std::map <long, CUser*>::const_iterator		IterUserIdd;
			};

			Common::DataStructure::CMemoryPool<CUser>		m_UserPool;		// ����� ��ü���� �޸� Ǯ
			TagUserMap										m_UserMap;


			int						m_nMaxRoomUser;				// �Ϲ� Room�� �� �� �ִ� �ִ� ����� ��
			int						m_nMaxRoom;					// ������ �� �ִ� �ִ� Room�� ��
			CRoom*					m_aRoom;					// Room �迭
			Common::DataStructure::CInnerManager<CRoom>
									m_InnerManagerUnusedRoom;	// ������� �ʴ� �� ����Ʈ
			Common::DataStructure::CInnerManager<CRoom>
									m_InnerManagerUsedRoom;		// ������� �� ����Ʈ

		*/

		public:
			void			GetRecentPacket( PacketType &out_packetType, char* &out_pPacket, int &out_nPacketSize )
			{
				out_packetType		= m_LastValue.packetType;
				out_pPacket			= m_LastValue.pData;
				out_nPacketSize		= m_LastValue.nDataLength;
			}

			void			GetRecentPacket( INT &out_nServicerType, Application::CPeer* &out_pPeer, PacketType &out_packetType, char* &out_pPacket, int &out_nPacketSize )
			{
				out_pPeer			= m_LastValue.pPeer;
				out_nServicerType	= m_LastValue.nServicerIndex;
				out_packetType		= m_LastValue.packetType;
				out_pPacket			= m_LastValue.pData;
				out_nPacketSize		= m_LastValue.nDataLength;

				m_LastValue.pPeer			= NULL;
				m_LastValue.pData			= NULL;
			}

			BOOL			Connect( LPCTSTR lpszIP, INT nPort, ServicerType type = ServicerType::Client );
			void			Send( char* pData, INT nDataSize );
			void			Close();
			void			Send( Application::CPeer* pPeer, char* pData, INT nDataSize );
//			template< class Type >
			void			Send( CPacket * pPacket )
			{
				Send( (char*)pPacket, pPacket->GetLength() );
			}
			void			Close( Application::CPeer* pPeer );

			void			Pinging( DWORD dwTick );

			BOOL			IsConnected()
			{
				return m_bConnected;
			}

			DWORD			GetRttTime()
			{
				return CSessionGate::GetRttTime( &m_peer );
			}

			LPCTSTR			GetLocalAddr()
			{
				return CSessionGate::GetLocalAddr( &m_peer );
			}

			LPCTSTR			GetPeerAddr()
			{
				return CSessionGate::GetPeerAddr( &m_peer );
			}

		// Overroad
		public:
			virtual BOOL	Create( /*IClientGateHandler* pClientGateHandler, */HWND hWindow, UINT uiMessage, INT nMaxPacketSize );
			// Retrun : ������ ������ Port
			virtual INT		Start( INT nPort );
			virtual void	Destroy();

		// Implementation
		protected:
			virtual void	OnSiganl	( DWORD wParam, DWORD lParam );
			virtual void	OnResponse	( Application::CPeer* pPeer, DWORD dwError, INT nResult, char* pBuffer );
			virtual void	OnAccept	( Application::CPeer* pPeer, INT nServicerIndex );
			virtual void	OnConnect	( Application::CPeer* pPeer, INT nServicerIndex );
			virtual void	OnBeginConnect	( Application::CPeer* pPeer, INT nServicerIndex );
			virtual void	OnClose		( Application::CPeer* pPeer, INT nServicerIndex );
			virtual void	OnData		( Application::CPeer* pPeer, INT nServicerIndex, char* pPacket, INT nPacketLength );
			virtual void	OnTimeout	( Application::CPeer* pPeer );

			virtual Application::CPeer*
							AllocPeer( INT nServicerIndex );
			virtual void	FreePeer( Application::CPeer* pPeer, INT nServicerIndex );

			virtual void	InitListenerPacketType( 
				INT											nServicerIndex,
				Message::CPacketCheckSum::CheckSumType&		out_CheckSumType,
				Message::CPacketAssembler::PacketUnitType&	out_PacketUnitType );

			virtual void	InitLengthFieldOfPacket(
				INT											nServicerIndex,
				INT&										out_nLengthFieldPos,
				Message::CPacketAssembler::LengthSizeType&	out_lengthSizeType,
				Message::CPacketAssembler::LengthType&		out_lengthType,
				BOOL&										out_bLittleEndian );

			virtual BOOL	IsAliveManageObject( INT nServicerIndex );

		};
	}
}