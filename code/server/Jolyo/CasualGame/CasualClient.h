#pragma once

#include <map>

#include "..\NJean\Session\SessionGate.h"
#include "..\DataStructure\MemoryPool.h"

#include "..\NJean\Application\Peer.h"

namespace GamehiSolution
{
	namespace CasualGame
	{
		using namespace GamehiSolution::Session;

		interface	IClientModuleHandler
		{
			virtual void	InitListenerPacketType( 
				Message::CPacketCheckSum::CheckSumType&		out_CheckSumType,
				Message::CPacketAssembler::PacketUnitType&	out_PacketUnitType )	= 0;

			virtual void	InitLengthFieldOfPacket(
				INT&										out_nLengthFieldPos,
				Message::CPacketAssembler::LengthSizeType&	out_lengthSizeType,
				Message::CPacketAssembler::LengthType&		out_lengthType,
				BOOL&										out_bLittleEndian )		= 0;
		};

		class CCasualClient : public Session::Gate::CSessionGate
		{
		public:
			CCasualClient();
			virtual ~CCasualClient();

		public:
			enum	PacketType
			{
				AcceptType		= 0,
				ConnectType		= 1,
				CloseType		= 2,
				ReceiveType		= 3,
			};

		private:
			enum	ServicerIndex
			{
				GameServer	= 0,
			};

		private:
			int						m_nAliveManaged;	// ��� ����

			struct TagLastValue
			{
				PacketType			packetType;
				Application::CPeer*	pPeer;
				char*				pData;
				int					nDataLength;
			};

//			IClientModuleHandler*	m_pClientModuleHandler;
			TagLastValue			m_LastValue;

			Application::CPeer		m_peer;

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

			BOOL			Connect( LPCTSTR lpszIP, INT nPort );
			void			Send( char* pData, INT nDataSize );
			void			Close();

			void			Pinging();

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
			virtual BOOL	Create( /*IClientModuleHandler* pClientModuleHandler, */HWND hWindow, UINT uiMessage, INT nMaxPacketSize );
			virtual BOOL	Start();
			virtual void	Destroy();

		// Implementation
		protected:
			virtual void	OnSiganl	( DWORD wParam, DWORD lParam );
			virtual void	OnResponse	( Application::CPeer* pPeer, DWORD dwError, INT nResult, char* pBuffer );
			virtual void	OnAccept	( Application::CPeer* pPeer, INT nServicerIndex );
			virtual void	OnConnect	( Application::CPeer* pPeer, INT nServicerIndex );
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