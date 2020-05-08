// ----------------------------------------------------------------------- //
//
// MODULE  : SAGameServerShell.cpp
//
// PURPOSE : The game's server shell - Implementation
//
// CREATED : 9/17/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SAGameServerShell.h"
#include "GameStartPoint.h"
#include "ServerMissionMgr.h"
#include "iserverdir.h"
#include "PlayerObj.h"
#include "msgids.h"
#include "iserverdir_titan.h"

// Refresh every 5 seconds
const uint32 k_nRepublishDelay = 5000;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAGameServerShell::CSAGameServerShell()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CSAGameServerShell::CSAGameServerShell() : CGameServerShell(),
	m_nLastPublishTime(0)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAGameServerShell::~CSAGameServerShell()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

CSAGameServerShell::~CSAGameServerShell()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAGameServerShell::CreatePlayer()
//
//	PURPOSE:	Create the player object, and associated it with the client.
//
// ----------------------------------------------------------------------- //

CPlayerObj* CSAGameServerShell::CreatePlayer(HCLIENT hClient, ModelId ePlayerModelId )
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    theStruct.m_Rotation.Init();
	VEC_INIT(theStruct.m_Pos);
	theStruct.m_Flags = 0;

	HCLASS hClass = g_pLTServer->GetClass("CPlayerObj");

	GameStartPoint* pStartPoint = FindStartPoint(LTNULL);
	if (pStartPoint)
	{
		g_pLTServer->GetObjectPos(pStartPoint->m_hObject, &(theStruct.m_Pos));
	}

	CPlayerObj* pPlayer = NULL;
	if (hClass)
	{
		theStruct.m_UserData = ePlayerModelId; //pStartPoint->GetPlayerModelId();
        pPlayer = (CPlayerObj*) g_pLTServer->CreateObject(hClass, &theStruct);
	}

	return pPlayer;
}

CPlayerObj* CSAGameServerShell::CreatePlayerExt(HCLIENT hClient, ModelId ePlayerModelId, ModelLoadType eModelType)
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    theStruct.m_Rotation.Init();
	VEC_INIT(theStruct.m_Pos);
	theStruct.m_Flags = 0;

	HCLASS hClass = g_pLTServer->GetClass("CPlayerObj");

	GameStartPoint* pStartPoint = FindStartPoint(LTNULL);
	if (pStartPoint)
	{
		g_pLTServer->GetObjectPos(pStartPoint->m_hObject, &(theStruct.m_Pos));
	}

	CPlayerObj* pPlayer = NULL;
	if (hClass)
	{
		theStruct.m_UserData = ePlayerModelId;
        pPlayer = (CPlayerObj*) g_pLTServer->CreateObject(hClass, &theStruct);

		pPlayer->InitializeModel( ePlayerModelId, eModelType );
	}

	return pPlayer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAGameServerShell::OnServerInitialized()
//
//	PURPOSE:	Initialize the server directory information which is required for SA
//
// ----------------------------------------------------------------------- //

LTRESULT CSAGameServerShell::OnServerInitialized()
{
	LTRESULT nResult = CGameServerShell::OnServerInitialized();

	// Don't do anything special if we're playing single-player
	if (!IsMultiplayerGame( ))
	{
		SetServerDir(0);
		return nResult;
	}

// Added By Bestline ==>
// Time : 2004-06-29 ���� 12:02:19 
// ���� ���丮�� �ȸ���� �� �� ����?
	/*
	IServerDirectory *pServerDir = Factory_Create_IServerDirectory_Titan( false, *g_pLTServer, NULL );
	if( !pServerDir )
	{	
		ASSERT( !"ServerDir is NULL!" );
		return LT_ERROR;
	}
	SetServerDir(pServerDir);

	// Set the game's name
	pServerDir->SetGameName(g_pVersionMgr->GetNetGameName());
	// Set the version
	pServerDir->SetVersion(g_pVersionMgr->GetNetVersion());
	pServerDir->SetRegion(g_pVersionMgr->GetNetRegion());
	// Set up the network messaging header
	CAutoMessage cMsg;
	cMsg.Writeuint8(0xD); // SMSG_MESSAGE
	cMsg.Writeuint8(MID_MULTIPLAYER_SERVERDIR);
	pServerDir->SetNetHeader(*cMsg.Read());

	StartupInfo_Titan startupInfo;
	startupInfo.m_sGameSpyName = "nolf2";
	// Obfuscate the secret key a little.
	startupInfo.m_sGameSpySecretKey = "g";
	startupInfo.m_sGameSpySecretKey += "3";
	startupInfo.m_sGameSpySecretKey += "F";
	startupInfo.m_sGameSpySecretKey += "o";
	startupInfo.m_sGameSpySecretKey += "6";
	startupInfo.m_sGameSpySecretKey += "x";
	cMsg.Writeuint32(( uint32 )&startupInfo );
	pServerDir->SetStartupInfo( *cMsg.Read( ));
	*/
// Added By Bestline <==

	return nResult;
}

void CSAGameServerShell::OnServerTerm()
{
	CGameServerShell::OnServerTerm();

	if (m_pServerDir)
	{
		if (!m_ServerGameOptions.m_bLANOnly)
		{
			// Clear the server directory's processing queue
			m_pServerDir->BlockOnProcessing(3000);
			// Remove us from the list
			m_pServerDir->SetActivePeer(0);
			m_pServerDir->ProcessRequest(IServerDirectory::eRequest_Remove_Server, 3000);
		}
		SetServerDir(0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAGameServerShell::Update()
//
//	PURPOSE:	Update the server
//
// ----------------------------------------------------------------------- //

void CSAGameServerShell::Update(LTFLOAT timeElapsed)
{
	// Update the main server first
	CGameServerShell::Update(timeElapsed);

	m_VersionMgr.Update();

	if (!GetServerDir())
		return;

	//if we're hosting LANOnly game, don't publish the server
	if( m_ServerGameOptions.m_bLANOnly )
		return;

	// Are we still waiting?
	static std::string status;
	switch (GetServerDir()->GetCurStatus())
	{
		case IServerDirectory::eStatus_Processing : 
			status ="";
			break;
		case IServerDirectory::eStatus_Waiting : 
			if (status.empty())
				status = GetServerDir()->GetLastRequestResultString();
			break;
		case IServerDirectory::eStatus_Error : 
			{
				
				IServerDirectory::ERequest eErrorRequest = GetServerDir()->GetLastErrorRequest();
				status = GetServerDir()->GetLastRequestResultString();
				GetServerDir()->ProcessRequestList();
			}
			break;
	};



	// Publish the server if we've waited long enough since the last directory update
	uint32 nCurTime = (uint32)GetTickCount();
	if ((m_nLastPublishTime == 0) || 
		((nCurTime - m_nLastPublishTime) > k_nRepublishDelay))
	{
		status = "";
		m_nLastPublishTime = nCurTime;
		uint32 nMax = 0;
		g_pLTServer->GetMaxConnections(nMax);

		// If not run by a dedicated server, we need to add one connection
		// for the local host.
		if( !m_ServerGameOptions.m_bDedicated )
			nMax++;

		GetServerDir()->SetActivePeer(0);

		CAutoMessage cMsg;

		// Update the summary info
		cMsg.WriteString(GetHostName());
		GetServerDir()->SetActivePeerInfo(IServerDirectory::ePeerInfo_Name, *cMsg.Read());

		char fname[_MAX_FNAME] = "";
		_splitpath( GetCurLevel(), NULL, NULL, fname, NULL );

		// Update the summary info
		cMsg.WriteString(g_pVersionMgr->GetBuild());
		cMsg.WriteString( fname );
		cMsg.Writeuint8(GetNumPlayers());
		cMsg.Writeuint8(nMax);
		cMsg.Writebool(m_ServerGameOptions.m_bUsePassword);
		cMsg.Writeuint8((uint8)GetGameType());
		cMsg.WriteString( m_ServerGameOptions.m_sModName.c_str() );

		GetServerDir()->SetActivePeerInfo(IServerDirectory::ePeerInfo_Summary, *cMsg.Read());


		// Update the details
		ServerMissionSettings sms = g_pServerMissionMgr->GetServerSettings();
		cMsg.Writebool(sms.m_bUseSkills);
		cMsg.Writebool(sms.m_bFriendlyFire);
		cMsg.Writeuint8(sms.m_nMPDifficulty);
		cMsg.Writefloat(sms.m_fPlayerDiffFactor);

		CPlayerObj* pPlayer = GetFirstNetPlayer();
	    while (pPlayer)
		{
			//has player info
			cMsg.Writebool(true);
			cMsg.WriteString(pPlayer->GetNetUniqueName());
			cMsg.Writeuint16( Min( GetPlayerPing(pPlayer), ( uint32 )65535 ));
			pPlayer = GetNextNetPlayer();
		};

		//end of player info
		cMsg.Writebool(false);

	
		cMsg.Writeuint8(sms.m_nRunSpeed);
		cMsg.Writeuint8(sms.m_nScoreLimit);
		cMsg.Writeuint16(sms.m_nTimeLimit);

		GetServerDir()->SetActivePeerInfo(IServerDirectory::ePeerInfo_Details, *cMsg.Read());

		// Update the port
		char aHostAddr[16];
		uint16 nHostPort;
		g_pLTServer->GetTcpIpAddress(aHostAddr, sizeof(aHostAddr), nHostPort);
		cMsg.Writeuint16(nHostPort);
		GetServerDir()->SetActivePeerInfo(IServerDirectory::ePeerInfo_Port, *cMsg.Read());
		
		// Tell serverdir again about info, but in service specific manner.
		PeerInfo_Service_Titan peerInfo;
		peerInfo.m_sHostName = GetHostName( );
		peerInfo.m_sCurWorld = fname; 
		peerInfo.m_nCurNumPlayers = GetNumPlayers( );
		peerInfo.m_nMaxNumPlayers = nMax;
		peerInfo.m_bUsePassword = m_ServerGameOptions.m_bUsePassword;
		peerInfo.m_sGameType = GameTypeToString( GetGameType( ));
		peerInfo.m_nScoreLimit = sms.m_nScoreLimit;
		peerInfo.m_nTimeLimit = sms.m_nTimeLimit;

		PeerInfo_Service_Titan::Player player;
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj* pPlayerObj = *iter;

			player.m_sName = pPlayerObj->GetNetUniqueName( );
			player.m_nScore = pPlayerObj->GetPlayerScore()->GetScore( );

			float fPing;
			g_pLTServer->GetClientPing( pPlayerObj->GetClient( ), fPing );
			player.m_nPing = ( uint16 )( fPing + 0.5f );

			peerInfo.m_PlayerList.push_back( player );

			iter++;
		}


		cMsg.Writeuint32(( uint32 )&peerInfo );
		GetServerDir()->SetActivePeerInfo(IServerDirectory::ePeerInfo_Service, *cMsg.Read());

		// Tell the world about me...
		GetServerDir()->QueueRequest(IServerDirectory::eRequest_Publish_Server);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSAGameServerShell::IsCapNumberOfBodies()
//
//	PURPOSE:	Cap the number of bodies in a radius.
//
// ----------------------------------------------------------------------- //

bool CSAGameServerShell::IsCapNumberOfBodies( )
{
	switch( GetGameType( ))
	{
		case eGameTypeCooperative:
		case eGameTypeSingle:
			return true;
		default:
			return false;
	}
}