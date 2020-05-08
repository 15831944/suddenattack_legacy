// ----------------------------------------------------------------------- //
//
// MODULE  : Bombable.cpp
//
// PURPOSE : The Bombable implementation
//
// CREATED : 10/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "MsgIds.h"
	#include "PlayerObj.h"
	#include "Spawner.h"
	#include "GadgetTargetTypes.h"
	#include "Bombable.h"
	#include "ParsedMsg.h"
	#include "FXButeMgr.h"
	#include "ServerSpecialFX.h"
	#include "ServerSoundMgr.h"
	#include "ServerMissionMgr.h"
//[SPIKE]
	#include "GameServerShell.h"
	#include "WeaponFireInfo.h"
	#include "Weapons.h"
	#include "Weapon.h"

//
// Defines...
//

	#define TIME_BOMB_AMMO_NAME		"Time Bomb"

LINKFROM_MODULE( Bombable );

BEGIN_CLASS( Bombable )
	
	ADD_STRINGPROP_FLAG( GadgetTargetName, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
	ADD_LONGINTPROP_FLAG( CodeID, 0000, PF_HIDDEN ) 
	ADD_REALPROP_FLAG( MinTime, 0.0f, PF_HIDDEN )
	ADD_REALPROP_FLAG( MaxTime, 0.0f, PF_HIDDEN )
	ADD_REALPROP_FLAG( DefuseTime, 15.0f, 0 )
	ADD_REALPROP_FLAG( DetonateTime, -1.0f, 0 )
	ADD_STRINGPROP_FLAG( Command, "", PF_HIDDEN )
	ADD_STRINGPROP_FLAG( PlantedCommand, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG( DetonatedCommand, "", PF_NOTIFYCHANGE )
	ADD_STRINGPROP_FLAG( DefusedCommand, "", PF_NOTIFYCHANGE )
	ADD_BOOLPROP_FLAG( StartPlanted, LTFALSE, 0 )
	ADD_BOOLPROP_FLAG( CanDefuse, LTFALSE, 0 )
	ADD_BOOLPROP_FLAG( DefuseOnce, LTTRUE, 0 )

END_CLASS_DEFAULT_FLAGS_PLUGIN( Bombable, GadgetTarget, NULL, NULL, 0, CBombablePlugin )


CMDMGR_BEGIN_REGISTER_CLASS( Bombable )

	CMDMGR_ADD_MSG( DETONATE, 1, NULL, "DETONATE" )
	CMDMGR_ADD_MSG( PLANTBOMB, 1, NULL, "PLANTBOMB" )
	CMDMGR_ADD_MSG( VISIBLE, 2, NULL, "VISIBLE <1 or 0>" )
	CMDMGR_ADD_MSG( HIDDEN, 2, NULL, "HIDDEN <1 or 0>" )
	
CMDMGR_END_REGISTER_CLASS( Bombable, GadgetTarget )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CBombablePlugin::PreHook_EditStringList
//
//  PURPOSE:	Populate the Type list
//
// ----------------------------------------------------------------------- //

LTRESULT CBombablePlugin::PreHook_EditStringList(
													const char* szRezPath,
													const char* szPropName,
													char** aszStrings,
													uint32* pcStrings,
													const uint32 cMaxStrings,
													const uint32 cMaxStringLength )
{

	if( LT_OK == CPropPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}
	else if( !_stricmp("GadgetTargetName", szPropName) )
	{
		// Fill the list with our Gadget Target names...

		m_GadgetTargetMgrPlugin.SetFilterOutTypes( GT_TYPE_TO_FLAG( eKeyPad ) |
												   GT_TYPE_TO_FLAG( eCardReader ) |
												   GT_TYPE_TO_FLAG( eCodedText ) |
												   GT_TYPE_TO_FLAG( eComboLock ) |
												   GT_TYPE_TO_FLAG( ePadLock ) |
												   GT_TYPE_TO_FLAG( eDoorKnob ) |
												   GT_TYPE_TO_FLAG( eTelephone ) |
												   GT_TYPE_TO_FLAG( ePhotographable ) |
												   GT_TYPE_TO_FLAG( eInvisibleInk ));

		if( m_GadgetTargetMgrPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings,
			pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::TimeBomb
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Bombable::Bombable( )
:	GadgetTarget		( ),
	m_fDefuseTime		( 0.0f ),
	m_fDetonateTime		( -1.0f ),
	m_hstrDetonateCmd	( LTNULL ),
	m_hstrDefusedCmd	( LTNULL ),
	m_hstrDefusingSnd	( LTNULL ),
	m_hstrDefusedSnd	( LTNULL ),
	m_bStartPlanted		( false ),
	m_bCanDefuse		( false ),
	m_bDefuseOnce		( true )
{
	m_hTimeBombModel.SetReceiver( *this );
	m_hBombPickup.SetReceiver( *this );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::~TimeBomb
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Bombable::~Bombable( )
{
	FREE_HSTRING( m_hstrDetonateCmd );
	FREE_HSTRING( m_hstrDefusedCmd );
	FREE_HSTRING( m_hstrDefusingSnd );
	FREE_HSTRING( m_hstrDefusedSnd );

	if( m_hTimeBombModel )
	{
		// Remove the model...

		HATTACHMENT hAttachment;
        if (g_pLTServer->FindAttachment( m_hObject, m_hTimeBombModel, &hAttachment ) == LT_OK )
		{
            g_pLTServer->RemoveAttachment( hAttachment );
		}

        g_pLTServer->RemoveObject( m_hTimeBombModel );
        m_hTimeBombModel = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::EngineMessageFn
//
//  PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Bombable::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
{
	switch( messageID )
	{
		case MID_UPDATE :
		{
			Update();				
		}
		break;

		case MID_ALLOBJECTSCREATED :
		{
			if( m_bStartPlanted )
				SetupDisabledState( LTFALSE );
		}
		break;

		default : break;
	}

	return GadgetTarget::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::ReadProp
//
//  PURPOSE:	Handle reading in the property values
//
// ----------------------------------------------------------------------- //

LTBOOL Bombable::ReadProp( ObjectCreateStruct *pStruct )
{
	if( !pStruct ) return LTFALSE;

	// Send to base class first...

	GadgetTarget::ReadProp( pStruct );

	GenericProp gProp;

	if( g_pLTServer->GetPropGeneric( "DefuseTime", &gProp ) == LT_OK )
	{
		m_fDefuseTime = gProp.m_Float;
	}

	if( g_pLTServer->GetPropGeneric( "DetonateTime", &gProp ) == LT_OK )
	{
		m_fDetonateTime = gProp.m_Float;
	}

	// Our planted command is actually our regular disabled command...

	if( g_pLTServer->GetPropGeneric( "PlantedCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
			m_hstrDisabledCmd = g_pLTServer->CreateString( gProp.m_String );
	}

	if( g_pLTServer->GetPropGeneric( "DetonatedCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
			m_hstrDetonateCmd = g_pLTServer->CreateString( gProp.m_String );
	}

	if( g_pLTServer->GetPropGeneric( "DefusedCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
			m_hstrDefusedCmd = g_pLTServer->CreateString( gProp.m_String );
	}

	if( g_pLTServer->GetPropGeneric( "StartPlanted", &gProp ) == LT_OK )
	{
		m_bStartPlanted = gProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "CanDefuse", &gProp ) == LT_OK )
	{
		m_bCanDefuse = gProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "DefuseOnce", &gProp ) == LT_OK )
	{
		m_bDefuseOnce = gProp.m_Bool;
	}

	if( m_hstrDisablingSnd )
	{
		m_hstrDefusingSnd = m_hstrDisablingSnd;
		m_hstrDisablingSnd = LTNULL;
	}

	if( m_hstrDisabledSnd )
	{
		m_hstrDefusedSnd = m_hstrDisabledSnd;
		m_hstrDisabledSnd = LTNULL;
	}


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::Update
//
//  PURPOSE:	Update the timebomb...
//
// ----------------------------------------------------------------------- //

void Bombable::Update()
{
//[SPIKE] ������ ������ TimeBomb Timer�ʱ�ȭ ======================
	if(g_pServerMissionMgr->GetTimeBomb_InitTimer()) // �������� ��ġ �ߴٸ� ������ �ȵ�!!!
	{
		SetupDefusedState( false );  //  TimeBomb ���� 
		g_pServerMissionMgr->SetTimeBomb_InitTimer(LTFALSE);
	}
//==================================================================
	if( m_Timer.On() && m_Timer.Stopped() )
		SetupDetonatedState( );

	SetNextUpdate( UPDATE_NEXT_FRAME );	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::SetupDetonatedState
//
//  PURPOSE:	Blow the sucker up...
//
// ----------------------------------------------------------------------- //

void Bombable::SetupDetonatedState( )
{
	//[SPIKE] TimeBomb�� ��ü�� ��Ȳ�̴ϱ� ��� ����...
	if( !m_SoundItemList.empty() )
	{
		VECTOR_SOUND_LIST_ITR itr = m_SoundItemList.begin();
		while (itr!=m_SoundItemList.end()) 
		{
			SoundInfo_Ptr pSoundInfo = (SoundInfo_Ptr)(*itr);
			if( pSoundInfo->m_hSISound )
			{
				g_pLTServer->SoundMgr()->KillSound( pSoundInfo->m_hSISound );
			}
			itr++;
		}
		m_SoundItemList.clear();
	}
	//////////////////////////////////////////////////////////////////////////////////
	

	// Send the detonate command...
	if( m_hstrDetonateCmd )
	{
        const char* szCmd = g_pLTServer->GetStringData( m_hstrDetonateCmd );
		if( szCmd )
		{
			g_pCmdMgr->Process( szCmd, m_hObject, m_hObject );
		}
	}

	// Remove the Time Bomb model...

	HATTACHMENT hAttachment;
	if (g_pLTServer->FindAttachment( m_hObject, m_hTimeBombModel, &hAttachment ) == LT_OK )
	{
		g_pLTServer->RemoveAttachment( hAttachment );
	}

	g_pLTServer->RemoveObject( m_hTimeBombModel );
	m_hTimeBombModel = LTNULL;

	m_Timer.Stop();
	SetNextUpdate( UPDATE_NEVER );

	// Remove ourselves..

//[SPIKE]=====================================================================
//	g_pLTServer->RemoveObject( m_hObject );
//---------------------------------------------------------------------------
	// Reset the gadget target so it can be disabled again...
	m_bDisabled = LTFALSE;
	m_bDisableRequested = LTFALSE;
	m_fDisableTime = m_fTotalTime;	
	//show target 'cause there is no pick up and no attachment	
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, m_dwGadgetUsrFlgs, m_dwGadgetUsrFlgs );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );

	if( m_bDefuseOnce )
	{
		g_pLTServer->RemoveObject( m_hObject );
	}
//=================================================================================================================
	//[SPIKE] Test:������ ȿ������  <<< �����ڵ� >>>
/*	CWeapon* pWeapon = g_pWeapons->GetWeapon(21);
	LTVector vTestPos;
	g_pLTServer->GetObjectPos( m_hObject, &vTestPos );
	WeaponFireInfo weaponFireInfo;
	weaponFireInfo.hFiredFrom	= m_hObject;
	weaponFireInfo.vFlashPos	= vTestPos;
	weaponFireInfo.vFirePos		= vTestPos;
	pWeapon->Fire(weaponFireInfo, LTTRUE);
*/	///////////////////////////////////////////////////////////////

	//��ġ�� �� �¸�
	g_pLTServer->CPrint( "TimeBomb Detonated" );
	
	uint8	nTimeBombID = 255;
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon("TimeBomb");
	if( pWeapon )
	{
		nTimeBombID = pWeapon->nId;
	}

	if( GetTeamID() != INVALID_TEAM ) g_pServerMissionMgr->ForceEndGame( GetTeamID(), 0xFFFFFFFF, m_hObject );
	
	LTVector vImpactPos(0.0f, 0.0f, 0.0f);
	CAutoMessage cMsg;
	//cMsg.Writeuint8(SFX_WEAPON_ID);
	//cMsg.WriteObject(LTNULL);
	//////////////////////////////////////////cMsg.WriteObject(LTNULL);
	cMsg.Writeuint8(255);
    cMsg.Writeuint8(nTimeBombID);
    //[MURSUM] REM 
	//cMsg.Writeuint8(20);
    //cMsg.Writeuint8(1);
    //////////////////////////////////////////cMsg.Writeuint16(0);  // ����
	//cMsg.Writeuint8(0);
    //cMsg.WriteCompLTVector(vTestPos);
	//g_pLTServer->GetObjectPos( m_hObject, &vTestPos );
    //cMsg.WriteCompLTVector(vTestPos);
	//[SPIKE]=================
	g_pLTServer->GetObjectPos( m_hObject, &vImpactPos );
    //////////////////////////cMsg.WriteCompPos(vImpactPos);		// ����
	cMsg.WriteCompPos(vImpactPos);
	//========================
	g_pLTServer->SendWeaponSFXMessage(cMsg.Read(), vImpactPos, 0);


	// TimeBomb ����ġ�� �̵� 
	g_pLTServer->SetObjectPos(m_hObject, &GetInitPosTimeBomb());

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::SetupDisabledState
//
//  PURPOSE:	The bomb is planted so show it and start the countdown...
//
// ----------------------------------------------------------------------- //

void Bombable::SetupDisabledState( LTBOOL bDestroyed )
{
	//[SPIKE] ���� ����� ��ź�� ��ġ���� �ʴ´�.
	if(g_pServerMissionMgr->IsEndRound() || g_pServerMissionMgr->GetTimeBomb_InitTimer())
	{
		SetupDefusedState( false );
		g_pServerMissionMgr->SetTimeBomb_InitTimer(LTFALSE);
		return;
	}
	//////////////////////////////////////////
	if( m_bDisabled )
		return;
	
	GadgetTarget::SetupDisabledState( bDestroyed );

	// Create the time bomb model, and attach it to the bomable area...

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
	theStruct.m_Rotation = rRot;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( TIME_BOMB_AMMO_NAME );
	if( !pAmmo || !pAmmo->pProjectileFX )
		return;

	SAFE_STRCPY(theStruct.m_Filename, pAmmo->pProjectileFX->szModel );
	SAFE_STRCPY(theStruct.m_SkinName, pAmmo->pProjectileFX->szSkin );

	theStruct.m_Scale = pAmmo->pProjectileFX->vModelScale;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
	theStruct.m_ObjectType  = OT_MODEL;

    HCLASS hClass = g_pLTServer->GetClass("GameBase");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hTimeBombModel = pModel->m_hObject;
	//hide target while attachment is visible
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );

	//[SPIKE] ��ġ�� TimeBomb Glow ȿ�� ���� 
//	g_pCommonLT->SetObjectFlags( m_hTimeBombModel, OFT_User, USRFLG_GLOW, USRFLG_GLOW );

	// Attach the model to the the camera...

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hTimeBombModel, "TimeBomb",
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->SetObjectPos( m_hTimeBombModel, &vPos );

		// The time bomb model is rotated 180 degrees...
	//[SPIKE] ������ ���̴� �� ���� 
	//	rRot.Rotate( rRot.Up(), MATH_PI );
		
		g_pLTServer->SetObjectRotation( m_hTimeBombModel, &rRot );
	}
	// Set the Models's animation...

    //HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hTimeBombModel, "Idle");
	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hTimeBombModel, "world");
	if( hAni != INVALID_MODEL_ANIM )
	{
        g_pLTServer->SetModelLooping( m_hTimeBombModel, LTTRUE );
        g_pLTServer->SetModelAnimation( m_hTimeBombModel, hAni );
	}
	
	
	// Set us up so we can now be activated to defuse the TimeBomb...

	if( m_bCanDefuse )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, m_dwGadgetUsrFlgs | USRFLG_CAN_ACTIVATE );

		UpdateSFXMsg(m_pGTInfo->m_eTargetType, false);
	}

	/////////////////////////////////////////////////////////////
	// Start the countdown...
	if( m_fDetonateTime > 0.0f )
	{
		m_Timer.Start( m_fDetonateTime );
		SetNextUpdate( UPDATE_NEXT_FRAME );
		
		// Don't let this object deactivate within Prop::Update()

		m_bCanDeactivate = false;
	}
	/////////////////////////////////////////////////////////////

	// Play the projectiles ClientFX on the time bomb model...

	if( pAmmo->pProjectileFX->szFXName[0] )
		SetObjectClientFXMsg( m_hTimeBombModel, pAmmo->pProjectileFX->szFXName, FXFLAG_LOOP );
	
	g_pServerMissionMgr->SetTimeBomb( LTTRUE, m_fDetonateTime, &vPos ); // TimeBomb�� ��ġ�� ���� 
	
	/////////////////////////////////////////////////////////////
	//[SPIKE] ��ź ��ġ ���� Send~	=========================================
	uint32 nPlayerID = 0xFFFFFFFF;
	CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( m_hLastSender );
	if( pPlayer ) nPlayerID = g_pLTServer->GetClientID( pPlayer->GetClient() );

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_MISSION_INFO);
	cMsg.Writeuint8(MI_BOMB_PLANTED);
	cMsg.Writeuint8(nPlayerID);
	cMsg.Writeuint8( (uint8)m_fDetonateTime );
	cMsg.WriteCompLTVector( vPos );
	g_pLTServer->SendToClient( cMsg.Read(), g_pGameServerShell->GetHostPlayerHandle(), MESSAGE_GUARANTEED );
	//=======================================================================
	/////////////////////////////////////////////////////////////
}	

void Bombable::ForcePlantedTimeBomb( float fTime )
{
	//[SPIKE] 
	if(g_pServerMissionMgr->IsEndRound() || g_pServerMissionMgr->GetTimeBomb_InitTimer())
	{
		SetupDefusedState( false );
		g_pServerMissionMgr->SetTimeBomb_InitTimer(LTFALSE);
		return;
	}
	//////////////////////////////////////////
	
	if( m_bDisabled ) return;
	
	GadgetTarget::SetupDisabledState( false );

	// Create the time bomb model, and attach it to the bomable area...

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
	theStruct.m_Rotation = rRot;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( TIME_BOMB_AMMO_NAME );
	if( !pAmmo || !pAmmo->pProjectileFX )
		return;

	SAFE_STRCPY(theStruct.m_Filename, pAmmo->pProjectileFX->szModel );
	SAFE_STRCPY(theStruct.m_SkinName, pAmmo->pProjectileFX->szSkin );

	theStruct.m_Scale = pAmmo->pProjectileFX->vModelScale;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
	theStruct.m_ObjectType  = OT_MODEL;

    HCLASS hClass = g_pLTServer->GetClass("GameBase");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hTimeBombModel = pModel->m_hObject;
	//hide target while attachment is visible
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );



	// Attach the model to the the camera...
    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hTimeBombModel, "TimeBomb",
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->SetObjectPos( m_hTimeBombModel, &vPos );
		g_pLTServer->SetObjectRotation( m_hTimeBombModel, &rRot );
	}

	// Set the Models's animation...
    HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hTimeBombModel, "world");
	if( hAni != INVALID_MODEL_ANIM )
	{
        g_pLTServer->SetModelLooping( m_hTimeBombModel, LTTRUE );
        g_pLTServer->SetModelAnimation( m_hTimeBombModel, hAni );
	}
	
	
	// Set us up so we can now be activated to defuse the TimeBomb...
	if( m_bCanDefuse )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, m_dwGadgetUsrFlgs | USRFLG_CAN_ACTIVATE );

		UpdateSFXMsg(m_pGTInfo->m_eTargetType, false);
	}

	// Start the countdown...
	if( fTime > 0.0f )
	{
		m_Timer.Start( fTime );
		SetNextUpdate( UPDATE_NEXT_FRAME );
		
		// Don't let this object deactivate within Prop::Update()
		m_bCanDeactivate = false;
	}
	
	// Play the projectiles ClientFX on the time bomb model...
	if( pAmmo->pProjectileFX->szFXName[0] )
		SetObjectClientFXMsg( m_hTimeBombModel, pAmmo->pProjectileFX->szFXName, FXFLAG_LOOP );
	
	g_pServerMissionMgr->SetTimeBomb( LTTRUE, fTime, &vPos ); // TimeBomb�� ��ġ�� ���� 
}	


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::HandleActivateMsg
//
//  PURPOSE:	Start defusing the bomb...
//
// ----------------------------------------------------------------------- //

void Bombable::HandleActivateMsg( HOBJECT hSender )
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_User, dwFlags );

	if( !IsPlayer( hSender ) ) return;

	CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hSender );
	if( !pPlayer ) return;

	if( IsTeamGameType() )
	{
		// Don't let players on our own team defuse the bomb...
		
		if( pPlayer->GetTeamID() == m_nTeamID )
			return;
	}

	if( (dwFlags & USRFLG_CAN_ACTIVATE) && m_bDisabled )
	{
		// Play the disabling sound...

		if( m_hstrDefusingSnd )
		{
			const char *szSound = g_pLTServer->GetStringData( m_hstrDefusingSnd );
			if( szSound )
			{
				LTVector vPos;
				g_pLTServer->GetObjectPos( m_hObject, &vPos );

				uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
				m_hSound = g_pServerSoundMgr->PlaySoundFromPos( vPos, szSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags );
				
				//[SPIKE]
				SoundInfo_Ptr pSoundInfo	= new SoundInfo;
				pSoundInfo->m_hSenderObject	= hSender;
				pSoundInfo->m_hSISound		= m_hSound;
				m_SoundItemList.push_back((VOID*)pSoundInfo);
			}
		}


		// Send a message to the client letting it know how long it should take
		// and whether or not it should show the bar...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_GADGETTARGET );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( 0 /*eINVALID*/ );
		cMsg.Writefloat( m_fDefuseTime );
		cMsg.Writefloat( m_fDefuseTime );
		cMsg.Writeuint32( 0 );
		g_pLTServer->SendToClient( cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::HandleStoppedMsg
//
//  PURPOSE:	Handle stopping the disabling
//
// ----------------------------------------------------------------------- //

void Bombable::HandleStoppedMsg( const CParsedMsg &cMsg, HOBJECT hSender)
{
	if( !m_bDisabled )
	{
		GadgetTarget::HandleStoppedMsg( cMsg );
		return;
	}

	//[SPIKE] Defuse Sound ���� ���� ===========================================================================
	// Stop the disabling sound if one was playing
	/*
	if(m_hSound)
	{
		g_pLTServer->SoundMgr()->KillSound( m_hSound );
		m_hSound = LTNULL;
	}*/
	if( !m_SoundItemList.empty() )
	{
		VECTOR_SOUND_LIST_ITR itr = m_SoundItemList.begin();
		while (itr!=m_SoundItemList.end()) 
		{
			SoundInfo_Ptr pSoundInfo = (SoundInfo_Ptr)(*itr);
			if( pSoundInfo->m_hSISound && pSoundInfo->m_hSenderObject == hSender)
			{
				g_pLTServer->SoundMgr()->KillSound( pSoundInfo->m_hSISound );
				
				m_SoundItemList.erase(itr); // erase()���������� ������ �������� ����! 
			}
			else 
			{
				itr++;
			}
		}
	}
	//  ==========================================================================================================

	// We already planted the bomb...
	// The second stopped message is for stopping the defusing of the bomb...
	
	LTFLOAT	fTime = (LTFLOAT)atof( cMsg.GetArg(1) );
	
	if( fTime > 0.0f )
	{
		// Make sure we can try and defuse it again...

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE );

		// Allow this object to deactivate within Prop::Update()

	//[SPIKE �ּ�] Defuse �ߴ� ��ҽ� ��ź�� �������� ���󶧹���
		m_bCanDeactivate = false;
	}
	else
	{
		SetupDefusedState( );		
	}	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::SetupDefusedState
//
//  PURPOSE:	The bomb was defused... Set us back up to plant the bomb again.
//
// ----------------------------------------------------------------------- //

void Bombable::SetupDefusedState( bool bSend /* = true  */)
{
	//[SPIKE] TimeBomb�� ��ü�� ��Ȳ�̴ϱ� ��� ����...
	if( !m_SoundItemList.empty() )
	{
		VECTOR_SOUND_LIST_ITR itr = m_SoundItemList.begin();
		while (itr!=m_SoundItemList.end()) 
		{
			SoundInfo_Ptr pSoundInfo = (SoundInfo_Ptr)(*itr);
			if( pSoundInfo->m_hSISound )
			{
				g_pLTServer->SoundMgr()->KillSound( pSoundInfo->m_hSISound );
			}
			itr++;
		}
		m_SoundItemList.clear();
	}
	//////////////////////////////////////////////////////////////////////////////////
	
	
	// The Bomb is no longer planted...
	
	m_bDisabled			= LTFALSE;
	m_bDisableRequested = LTFALSE;

	UpdateSFXMsg(m_pGTInfo->m_eTargetType, true);

	// Play the disabled sound...

	if( m_hstrDefusedSnd )
	{
		const char *szSound = g_pLTServer->GetStringData( m_hstrDefusedSnd );
		if( szSound )
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos( m_hObject, &vPos );

			uint32 dwFlags = (PLAYSOUND_GETHANDLE);
			m_hSound = g_pServerSoundMgr->PlaySoundFromPos( vPos, szSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags );

		}
	}

	// Send the defused command...
	
	if( m_hstrDefusedCmd )
	{
        const char* szCmd = g_pLTServer->GetStringData( m_hstrDefusedCmd );
		if( szCmd )
		{
			g_pCmdMgr->Process( szCmd, m_hObject, m_hObject );
		}
	}

	// Stop the countdown...

	m_Timer.Stop();
	SetNextUpdate( UPDATE_NEVER );


	//====================================================================
	//[MURSUM] ��ü�� ��ź�� ���������� �ʴ´�.
	/*/

	// Create the TimeBomb ammo powerup...

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( TIME_BOMB_AMMO_NAME );
	if( !pAmmo || !m_hTimeBombModel || !pAmmo->pProjectileFX )
		return;

	char szSpawn[512];
	sprintf(szSpawn, "WeaponItem MPRespawn 0;WeaponType TimeBomb;AmmoCount 1;MoveToFloor 0" );

	LTVector vScale = pAmmo->pProjectileFX->vModelScale;

	LTRotation	rRot;
	g_pLTServer->GetObjectRotation( m_hTimeBombModel, &rRot );

	LTVector	vPos;
	g_pLTServer->GetObjectPos( m_hTimeBombModel, &vPos );

	BaseClass* pClass = SpawnObject(szSpawn, vPos, rRot);

	if (pClass)
	{
		SetObjectFilenames( pClass->m_hObject, pAmmo->pProjectileFX->szModel, pAmmo->pProjectileFX->szSkin );

		g_pLTServer->ScaleObject(pClass->m_hObject, &vScale);

		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(pClass->m_hObject, &vDims);
		vDims.x *= vScale.x;
		vDims.y *= vScale.y;
		vDims.z *= vScale.z;

		g_pPhysicsLT->SetObjectDims(pClass->m_hObject, &vDims, 0);

		m_hBombPickup = pClass->m_hObject;
	}
	/*/
	//====================================================================


	// Remove the Time Bomb model...

	HATTACHMENT hAttachment;
	if (g_pLTServer->FindAttachment( m_hObject, m_hTimeBombModel, &hAttachment ) == LT_OK )
	{
		g_pLTServer->RemoveAttachment( hAttachment );
	}

	g_pLTServer->RemoveObject( m_hTimeBombModel );
	m_hTimeBombModel = LTNULL;

	//====================================================================
	//[MURSUM] ��ü�� ��ź�� ���������� �ʴ´�.
	/*
	//if (m_hBombPickup)
	if( 0 )
	{	
		// Clients can't try to plant another bomb until the pickup goes away
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, 0, m_dwGadgetUsrFlgs | USRFLG_CAN_ACTIVATE );
		UpdateSFXMsg(0, false);

		//hide target while pickup is visible
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );
	}
	else
	{
		//show target 'cause there is no pick up and no attachment	
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );
	}

	// Allow this object to deactivate within Prop::Update()

	m_bCanDeactivate = true;

	if( m_bDefuseOnce )
	{
		g_pLTServer->RemoveObject( m_hObject );
	}
	*/
	//====================================================================
	
	m_bDisabled = LTFALSE;
	m_bDisableRequested = LTFALSE;
	m_fDisableTime = m_fTotalTime;	
	//show target 'cause there is no pick up and no attachment	
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, m_dwGadgetUsrFlgs, m_dwGadgetUsrFlgs );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );

	if( m_bDefuseOnce )
	{
		g_pLTServer->RemoveObject( m_hObject );
	}
	//��ü�� �� �¸�
	g_pLTServer->CPrint( "TimeBomb Defused" );

	// TimeBomb ����ġ�� �̵� 
	g_pLTServer->SetObjectPos(m_hObject, &GetInitPosTimeBomb());
	if( bSend )
	{
		if( GetTeamID() != INVALID_TEAM )
		{
			uint32 nPlayerID = 0xFFFFFFFF;
			CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( m_hLastSender );
			if( pPlayer ) nPlayerID = g_pLTServer->GetClientID( pPlayer->GetClient() );
			g_pServerMissionMgr->ForceEndGame( !GetTeamID(), nPlayerID );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::OnSave
//
//  PURPOSE:	Save it
//
// ----------------------------------------------------------------------- //

LTBOOL Bombable::OnSave( ILTMessage_Write *pMsg )
{
	if( !pMsg ) return LTFALSE;

	m_Timer.Save( pMsg );

	SAVE_FLOAT( m_fDefuseTime );
	SAVE_FLOAT( m_fDetonateTime );
	SAVE_HSTRING( m_hstrDetonateCmd );
	SAVE_HSTRING( m_hstrDefusedCmd );
	SAVE_HOBJECT( m_hTimeBombModel );
	SAVE_HOBJECT( m_hBombPickup );
	SAVE_bool( m_bStartPlanted );
	SAVE_bool( m_bCanDefuse );
	SAVE_bool( m_bDefuseOnce );

	return GadgetTarget::OnSave( pMsg );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::OnLoad
//
//  PURPOSE:	Load it
//
// ----------------------------------------------------------------------- //

LTBOOL Bombable::OnLoad( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return LTFALSE;

	m_Timer.Load( pMsg );

	LOAD_FLOAT( m_fDefuseTime );
	LOAD_FLOAT( m_fDetonateTime );
	LOAD_HSTRING( m_hstrDetonateCmd );
	LOAD_HSTRING( m_hstrDefusedCmd );
	LOAD_HOBJECT( m_hTimeBombModel );
	LOAD_HOBJECT( m_hBombPickup );
	LOAD_bool( m_bStartPlanted );
	LOAD_bool( m_bCanDefuse );
	LOAD_bool( m_bDefuseOnce );

	return GadgetTarget::OnLoad( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Bombable::OnLinkBroken
//
//	PURPOSE:	Handle attached object getting removed.
//
// ----------------------------------------------------------------------- //

void Bombable::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hTimeBombModel )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}
	if( pRef == &m_hBombPickup )
	{
		// pickup is gone player can now plant a new bomb
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, m_dwGadgetUsrFlgs, m_dwGadgetUsrFlgs  | USRFLG_CAN_ACTIVATE );
		UpdateSFXMsg(m_pGTInfo->m_eTargetType, true);

		//show target 'cause there is no pick up and no attachment
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );
	}


	GadgetTarget::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Bombable::OnTrigger
//
//  PURPOSE:	Handle a trigger message...
//
// ----------------------------------------------------------------------- //

bool Bombable::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_1("1");
	static CParsedMsg::CToken s_cTok_True("TRUE");
	static CParsedMsg::CToken s_cTok_0("0");
	static CParsedMsg::CToken s_cTok_False("FALSE");
	static CParsedMsg::CToken s_cTok_Detonate( "DETONATE" );
	static CParsedMsg::CToken s_cTok_PlantBomb( "PLANTBOMB" );
	static CParsedMsg::CToken s_cTok_Visible( "VISIBLE" );
	static CParsedMsg::CToken s_cTok_Hidden( "HIDDEN" );
	
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if (!m_dwOriginalFlags)
	{
		m_dwOriginalFlags = dwFlags;
	}
		
	if( cMsg.GetArg(0) == s_cTok_Detonate )
	{
		SetupDetonatedState();
		return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_PlantBomb )
	{
		SetupDisabledState( LTFALSE );
		return true;
	}
	else if( (cMsg.GetArg(0) == s_cTok_Visible) ||
			 (cMsg.GetArg(0) == s_cTok_Hidden) )
	{
		// Relay the message to the bomb model if one is planted...

		if( m_hTimeBombModel )
		{
			char szMsg[128] = {0};
			cMsg.ReCreateMsg( szMsg, ARRAY_LEN( szMsg ), 0 );
			SendTriggerMsgToObject( this, m_hTimeBombModel, LTFALSE, szMsg );
		

			// Do not let GameBase process this message if the bomb is planted...
			// Instead toggle solid and rayhit but not vsible.

			uint32 dwFlags;
			g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );
			
			if ((cMsg.GetArg(1) == s_cTok_1) ||
				(cMsg.GetArg(1) == s_cTok_True))
			{
				dwFlags &= ~FLAG_SOLID;
				dwFlags &= ~FLAG_RAYHIT;
			}
			else
			{
				if ((cMsg.GetArg(1) == s_cTok_0) ||
					(cMsg.GetArg(1) == s_cTok_False))
				{
					if( m_dwOriginalFlags & FLAG_SOLID )
					{
						dwFlags |= FLAG_SOLID;
					}

					if (m_dwOriginalFlags & FLAG_RAYHIT)
					{
						dwFlags |= FLAG_RAYHIT;
					}
				}
			}

			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL );
		
			return true;
		}
	}
	
	return GadgetTarget::OnTrigger( hSender, cMsg );
}


void Bombable::UpdateSFXMsg(uint8 nTargetType, bool bSwitch)
{
	//[MURSUM]
	/*
	CAutoMessage cMsg;
	
	cMsg.Writeuint8(SFX_GADGETTARGET_ID);
    cMsg.Writeuint8(nTargetType);
	cMsg.Writebool(bSwitch);
	cMsg.Writebool(!!m_bOn);
	cMsg.Writeuint8( m_nTeamID );
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	

	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_GADGETTARGET_ID);
	cMsg.WriteObject(m_hObject);
    cMsg.Writeuint8(nTargetType);
	cMsg.Writebool(bSwitch);
	cMsg.Writebool(!!m_bOn);
	cMsg.Writeuint8( m_nTeamID );
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	*/
}














