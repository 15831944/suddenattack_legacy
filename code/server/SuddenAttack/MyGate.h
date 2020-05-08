#pragma once

#include "..\jolyo\casualgame\njeangate.h"
using namespace CasualGame;

#include "MyPacket.h"
using namespace MyPacket;

#include "ServerInfo.h"			// added by minjin.		2005. 01. 31.

#include "GradeTable.h"

class CMyUserPocket;
class CMyRoomPocket;

class CMyGate :
	public CNJeanGate
{
public:
	CMyGate(void);
	virtual ~CMyGate(void);

private:
	struct	TagPackets
	{
		CMyPacketMyProperty						MyProperty;
		CMyPacketStartGame						StartGame;
		CMyPacketChangeRoomState				ChangeRoomState;
		CMyPacketStartHost						StartHost;
		CMyPacketInstrutionReply				InstrutionReply;
		CMyPacketFailStartGame					FailStartGame;
		CMyPacketJoinRoomReply					JoinRoomReply;
		CMyPacketRoomUserProperty				RoomUserProperty;
		CMyPacketRequestUserInfo				RequestUserInfo;
		CMyPacketUserInfo						UserInfo;
		CMyPacketRoomOption						RoomOption;
		CMyPacketEndRound						EndRound;
		CMyPacketRoomStateChange				RoomStateChange;

		CMyPacketGameResultUser					GameResultUser[MAX_ROOM_USER];
		CMyPacketGameResultTeam					GameResultTeam;

		CMyPacketExitGameUser					ExitGameUser;
		CMyPacketChangeTeam						ChangeTeam;
			
		CMyPacketRoundScore						RoundScore;
	};

	TagPackets*			m_pPackets;

	CGradeTable			m_GradeTable;

public:
	BOOL				Create( SERVER_INFO* pServerInfo );
	BOOL				Create( INT nMaxPacketSize, INT nMaxClients, INT nMaxResponseBufferSize, INT nMaxRoomUser, INT nMaxChannels, LPCSTR lpszIP, INT nPort );
	virtual BOOL		Create( INT nMaxPacketSize, INT nMaxClients, INT nMaxResponseBufferSize, INT nMaxRoomUser, INT nMaxChannels, INT nPort );
	virtual VOID		Destroy();

private:
	// Pocket�� �����Ѵ�.
	virtual CPocket*	NewPocket( PocketType type );

	// Pocket�� �����Ѵ�.
	virtual VOID		DeletePocket( PocketType type, CPocket* pPocket );

	// ä���� �ʱ�ȭ �Ѵ�.
	virtual VOID		InitChannel( INT nChannelIndex, INT& out_nMaxUsers, INT& out_nMaxRooms, string& out_strChannelName );

	// ���� Ŭ������ �ñ׳� ���ǰ˻�
	virtual BOOL	CanBeCaptine		( CUser* pUser );

	// ������ ���� ���� ������ ��
	virtual VOID	OnNoneCaptine		( CRoom* pRoom );

	// ���� Ŭ���� ��Ŷ�� ���ǰ˻�
	virtual BOOL	IsCorrectCondition	( SHORT &nResult, CUser* pUser, PacketCondition condition, ULONG_PTR rParam = NULL, ULONG_PTR lParam = NULL );

	// ���� Ŭ���� Packet �޽��� ó��
	virtual VOID	OnPacketSecLogin	( CUser* pUser, const char* pSecData, INT nDataSize );
	virtual VOID	OnPacketLogin		( CUser* pUser, LPCTSTR strUserNo, LPCTSTR strID, LPCTSTR strNick );

	// ���� Ŭ������ ����� �޽��� �̺�Ʈ ó��
	virtual VOID	OnCasualSignal		( UINT uiMessage, ULONG_PTR wParam, ULONG_PTR lParam );
	virtual VOID	OnCasualResponse	( CUser* pUser, UINT uiMessage, BOOL bSuccess, _CommandPtr& pCmd );
	virtual VOID	OnCasualAccept		( CUser* pUser );
	virtual VOID	OnCasualConnect		( CUser* pUser );
	virtual VOID	OnCasualClose		( CUser* pUser );
	virtual VOID	OnCasualData		( CUser* pUser, CPacket* pPacket, INT nPacketLength );

	// Ÿ�Ӿƿ��� �߻��Ͽ��� �� : return ���� ���� ���� ������ �����Ѵ�.
	virtual BOOL	OnCasualTimeout		( CUser* pUser );


	///////////////////////////////////////////////////////////////////////////////////////////////
	// ���ǰ˻�

	BOOL			IsCorrectJoinChannel	( SHORT &nResult, CUser* pUser, CChannel* pJoinChannel );
	BOOL			IsCorrectOpenRoom		( SHORT &nResult, CUser* pUser );
	BOOL			IsCorrectJoinRoom		( SHORT &nResult, CUser* pUser, CRoom* pJoinRoom );
	BOOL			IsCorrectAutoJoinRoom	( SHORT &nResult, CUser* pUser );
	BOOL			IsCorrectLeaveRoom		( SHORT &nResult, CUser* pUser );
	BOOL			IsCorrectChangeCaptine	( SHORT &nResult, CUser* pUser, CRoom* pRoom );
	BOOL			IsCorrectCompluseExit	( SHORT &nResult, CUser* pCaptain, CUser* pExitUser, CRoom* pRoom );

	///////////////////////////////////////////
	VOID			SetUserInfo( CUser* pUser, _CommandPtr& pCmd );
	virtual VOID	OnLoginSuccess			( CUser* pUser );
	virtual VOID	OnOpenRoomReply			( CUser* pUser, CRoom* pRoom );
	virtual VOID	OnJoinReply				( CUser* pUser, CRoom* pRoom );
	virtual VOID	OnAutoJoinReply			( CUser* pUser, CRoom* pRoom );

	virtual VOID	OnLeaveRoom				( CUser* pUser, CRoom* pRoom, CCasualPacketCompulsionExited::Type ExitType );
	virtual VOID	OnChangeCaptine			( CUser* pCaptineUser, CRoom* pRoom );

	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// ��Ŷ ó��
	VOID			OnPacketRequestUserInfo( CUser* pUser, CMyPacketRequestUserInfo* pRequestUserInfo );
	VOID			OnPacketGetRoomInfo( CUser* pUser, CMyPacketGetRoomInfo* pGetRoomInfo );
	VOID			OnPacketChangeTeam( CUser* pUser, CMyPacketChangeTeam* pChangeTeam );
	VOID			OnPacketChangeOptMap( CUser* pUser, CMyPacketChangeOptMap* pChangeOptMap );
	VOID			OnPacketChangeOptMaxUsers( CUser* pUser, CMyPacketChangeOptMaxUsers* pChangeOptMaxUsers );
	VOID			OnPacketChangeOptGameLimit( CUser* pUser, CMyPacketChangeOptGameLimit* pChangeOptGameLimit );
	VOID			OnPacketChangeOptGradeLimit( CUser* pUser, CMyPacketChangeOptGradeLimit* pChangeOptGradeLimit );
	VOID			OnPacketChangeOptInstrusion( CUser* pUser, CMyPacketChangeOptInstrusion* pChangeOptInstrusion );
	VOID			OnPacketChangeOptTeamBalance( CUser* pUser, CMyPacketChangeOptTeamBalance* pChangeOptTeamBalance );
	VOID			OnPacketStartGameRequest( CUser* pUser, CMyPacketStartGameRequest* pStartGameRequest );
	VOID			OnPacketStartHostReply( CUser* pUser, CMyPacketStartHostReply* pStartHostReply );
	VOID			OnPacketReady( CUser* pUser, CMyPacketReady* pReady );
	VOID			OnPacketKill( CUser* pUser, CMyPacketKill* pKill );
	VOID			OnPacketEndGameRequest( CUser* pUser, CMyPacketEndGameRequest* pEndGameRequest );
	VOID			OnPacketInstrution( CUser* pUser, CMyPacketInstrution* pInstrution );
	VOID			OnPacketExitGameRequest( CUser* pUser, CMyPacketExitGameRequest* pExitGameRequest );
	VOID			OnPacketGameChat( CUser* pUser, CMyPacketGameChat* pGameChat );
	VOID			OnPacketTeamChat( CUser* pUser, CMyPacketTeamChat* pTeamChat );
	VOID			OnPacketLoadComplete( CUser* pUser, CMyPacketLoadComplete* pLoadComplete );
	VOID			OnPacketStartNextRound( CUser* pUser, CMyPacketStartNextRound* pStartNextRound );
	VOID			OnPacketWhisper( CUser* pUser, CMyPacketWhisper* pWhisper );
	VOID			OnPacketTimeBombState( CUser* pUser, CMyPacketTimeBombState* pTimeBombState );

	VOID			OnResponseLoginQuery( CUser* pUser, BOOL bSuccess, _CommandPtr& pCmd );
	VOID			OnResponseLogoutQuery( CUser* pUser, BOOL bSuccess, _CommandPtr& pCmd );
	VOID			OnResponseUserInfo( CUser* pUser, BOOL bSuccess, _CommandPtr& pCmd );
	VOID			OnResponseGameResultQuery( CUser* pUser, BOOL bSuccess, _CommandPtr& pCmd );

	VOID			RequestLogin( CUser* pUser, LPCTSTR lpszUserNo, LPCTSTR lpszIP, LPCTSTR lpszFlag );
	VOID			RequestLogout( CUser* pUser, LPCTSTR lpszUserNo, LPCTSTR lpszIP, LPCTSTR lpszFlag );
	VOID			RequestUserInfo( CUser* pUser, LPCTSTR lpszUserNo );
	VOID			RequestGameResult( CUser* pUser,
									   LPCTSTR lpszUserNo, __int64 nExp, __int64 nMoney,
									   int nKillCnt,  int nDeathCnt,
									   int nH_KillCnt, int nH_DeathCnt,
									   int nWinCnt, int nLoseCnt, int nDrawCnt );


	    
private:
	VOID			DecideHost( CRoom* pRoom, CMyRoomPocket* pRoomPocket, CUser* pExeptUser );
	BOOL			CheckStart( CUser* pCaptain, CRoom* pRoom, CMyRoomPocket* pRoomPocket );
	void			StopLoading( CRoom* pRoom, int nResult );

	BOOL			CheckEndRound_Kill( CRoom* pRoom, CMyRoomPocket* pRoomPocket );
	BOOL			CheckEndRoundTDM_Kill( CRoom* pRoom, CMyRoomPocket* pRoomPocket );
	BOOL			CheckEndRoundTSV_Kill( CRoom* pRoom, CMyRoomPocket* pRoomPocket );

	BOOL			CheckWinTeam_TimeOut( CRoom* pRoom, CMyRoomPocket* pRoomPocket );
	BOOL			CheckWinTeamTDM_TimeOut( CRoom* pRoom, CMyRoomPocket* pRoomPocket );
	BOOL			CheckWinTeamTSV_TimeOut( CRoom* pRoom, CMyRoomPocket* pRoomPocket );

	BOOL			CheckWinTeam_Timebomb( CRoom* pRoom, CMyRoomPocket* pRoomPocket );
	BOOL			CheckWinTeamTSV_TimeBomb( CRoom* pRoom, CMyRoomPocket* pRoomPocket );

	UINT			GetGameExp( CMyUserPocket* pUserPocket, CMyRoomPocket* pRoomPocket, BOOL bWin );
	UINT			GetGameExpTDM( CMyUserPocket* pUserPocket, CMyRoomPocket* pRoomPocket, BOOL bWin );
	UINT			GetGameExpTSV( CMyUserPocket* pUserPocket, CMyRoomPocket* pRoomPocket, BOOL bWin );

	UINT			GetGameMoney( CMyUserPocket* pUserPocket, CMyRoomPocket* pRoomPocket, BOOL bWin );
	UINT			GetGameMoneyTDM( CMyUserPocket* pUserPocket, CMyRoomPocket* pRoomPocket, BOOL bWin );
	UINT			GetGameMoneyTSV( CMyUserPocket* pUserPocket, CMyRoomPocket* pRoomPocket, BOOL bWin );
	

	BOOL			NextRound( CRoom* pRoom, CMyRoomPocket* pRoomPocket );
	BOOL			EndGame( CRoom* pRoom, CMyRoomPocket* pRoomPocket );

	VOID			SetTeamBalanceStartGame( CRoom* pRoom, CMyRoomPocket* pRoomPocket );
	void			SetTeamBalanceInstrution( CRoom* pRoom, CMyRoomPocket* pRoomPocket, CMyUserPocket* pUserPocket );
	void			SetTeamBalanceAfterDeath( CRoom* pRoom, CMyRoomPocket* pRoomPocket, CMyUserPocket* pUserPocket );

	BYTE			GetLimitTime( BYTE nGameType, BYTE nMapIndex, BYTE nRecvValue );



	BOOL			IsEnableIP( LPCSTR lpszIP );
	VOID			SendJoinUserToRoomUser( CUser* pUser, CRoom* pRoom, CMyRoomPocket* pRoomPocket, short nTeam );
	VOID			SendRoomUserToJoinUser( CUser* pUser, CRoom* pRoom, CMyRoomPocket* pRoomPocket );
};
