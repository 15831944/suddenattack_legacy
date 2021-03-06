// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.cpp
//
// PURPOSE : Character hit box object class implementation
//
// CREATED : 01/05/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterHitBox.h"
#include "Character.h"
#include "Body.h"
#include "Projectile.h"
#include "ServerUtilities.h"
#include "MsgIds.h"
#include "ObjectMsgs.h"
#include "Attachments.h"
#include "GameServerShell.h"

#ifndef USE_AI
#include "AIAssert.h"
#include "UberAssert.h"
#endif

// 광식
//-----------------------------
#include "PlayerObj.h"
//-----------------------------

LINKFROM_MODULE( CharacterHitBox );

#define HB_DEFAULT_NODE_RADIUS	12.5f
#define HB_DIMS_ENLARGE_PERCENT 1.2f

#define	HB_DIMS_MIN_XZ	24.0f
#define HB_DIMS_MIN_Y	15.0f

//[yaiin]
//#ifdef _DEBUG
//#define CHB_SHOW_NODE_RADII				1
//#define CHB_SHOW_PLAYER_NODE_RADII		1
//#else
#define CHB_SHOW_NODE_RADII				0
#define CHB_SHOW_PLAYER_NODE_RADII		0
//#endif

#define CHB_NODE_RADIUS_USE_OVERRIDE	0
#define CHB_HEAD_NODE_RADIUS			14.0f
#define CHB_TORSO_NODE_RADIUSE			HB_DEFAULT_NODE_RADIUS
#define CHB_ARM_NODE_RADIUS				HB_DEFAULT_NODE_RADIUS
#define CHB_LEG_NODE_RADIUS				HB_DEFAULT_NODE_RADIUS
#define CHB_HIT_DEBUG_TRACK				0

#pragma force_active on
BEGIN_CLASS(CCharacterHitBox)
END_CLASS_DEFAULT_FLAGS(CCharacterHitBox, GameBase, NULL, NULL, CF_HIDDEN)
#pragma force_active off

//CCharacterHitBox* g_pCharacterHitBox		= LTNULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CCharacterHitBox()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::CCharacterHitBox() : GameBase(OT_NORMAL)
{
	// Setup the TransitionAggregate
	MakeTransitionable();

	m_pHitBoxUser = NULL;
    m_hModel = LTNULL;
	m_vOffset.Init();
	m_bCanActivate = LTTRUE;
	m_bCanBeSearched = false;
	m_bAnimControlsDims = false;
	m_bAnimControlsOffset = false;
	m_hControllingAnim = INVALID_ANI;

    m_NodeRadiusList.Init(LTTRUE);

	//g_pCharacterHitBox = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::~CCharacterHitBox()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacterHitBox::~CCharacterHitBox()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Init()
//
//	PURPOSE:	Initialize object.  Takes as an argument the hObject of the 
//				object using the hitbox
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::Init(HOBJECT hModel, IHitBoxUser* pUser)
{
	AIASSERT( pUser, m_hObject, "CharacterHitBox Inited with non existant IHitBoxUser" );
	AIASSERT( hModel, m_hObject, "CharacterHitBox Inited with non existant hObject" );
	AIASSERT( m_hObject, m_hObject, "CharacterHitBox does not exist" );
	AIASSERT( pUser, m_hObject, "" );
	AIASSERT( dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(hModel)) == pUser, hModel,  "Passed in hModel and pUser should reference the same class -- HandleToObject(hModel) should return what pUser points to" );

	m_hModel = hModel;
	m_pHitBoxUser = pUser;

	// Set my flags...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_RAYHIT | FLAG_TOUCHABLE, FLAGMASK_ALL);

	// Set our user flags to USRFLG_CHARACTER, so the client will process
	// us like a character (for intersect segments)...
	
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CHARACTER, USRFLG_CHARACTER);

	SetNextUpdate(UPDATE_NEVER);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacterHitBox::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacterHitBox::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    // Pass any messages to our model...

	if (m_hModel)
	{
		LPBASECLASS pBase = g_pLTServer->HandleToObject(m_hModel);
		if (pBase)
		{
			pBase->ObjectMessageFn(hSender, pMsg);
		}
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //

LTVector CCharacterHitBox::GetBoundingBoxColor()
{
    return LTVector(1.0, 1.0, 0.0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetCanBeSearched()
//
//	PURPOSE:	Set us to be searchable or not and notify the client...
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetCanBeSearched(bool b) 
{
	if( m_bCanBeSearched == b )
		return;

	m_bCanBeSearched = b;

	if( m_pHitBoxUser )
	{
		m_pHitBoxUser->UpdateClientHitBox();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleImpact()
//
//	PURPOSE:	Handle a vector impacting on us
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector vDir, LTVector & vFrom)
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );
	AIASSERT( pProj, m_hObject, "Called with NULL projectile" );
    if (!pProj || !m_hModel)
	{
		return LTFALSE;
	}

	// Check to see if this CharacterHitBox is using HitDetection.  If not,
	// then return right away.
	if (!m_pHitBoxUser->UsingHitDetection())
	{
		return LTTRUE;
	}

	//[MURSUM]	StartTimingCounter();

	// 총에 맞았을 때 맞은 부위를 찾기 시작한다. 여기서~~~ 
	// 따라서 자기가 맞은 부분을 찾는거네~~ 총을 쏜 놈은 pProj->GetFiredFrom( ) 이고
	// m_hModel 은 총 맞은 넘이지 뭐??
	ModelNode eModelNode = eModelNodeInvalid;
    LTBOOL bHitSomething = LTTRUE;

	// Only do hit detection with player firing at ai.
	if( IsPlayer(m_hModel) && !IsPlayer( pProj->GetFiredFrom( )))
	{
		return LTFALSE;
	}
	else
	{
		// iInfo.m_hObject가 HitBox의 Handle을 가지고 이곳까지와서 character에 총알이 박히면
		// 그녀석의 Object Handle을 iInfo.m_hObject에 넣어주고 eModelNode에 맞은 부위를 넣어서
		// return 해 준다.
		bHitSomething = HandleVectorImpact(pProj, iInfo, vDir, vFrom, eModelNode);
	}

	//[MURSUM]	EndTimingCounter("CCharacterHitBox::HandleImpact()");

	// Did we hit something?

	if (bHitSomething)
	{
		// This is the object that *really* got hit...

		if (eModelNode != eModelNodeInvalid)
		{
			m_pHitBoxUser->SetModelNodeLastHit(eModelNode);
		}
	}

	return bHitSomething;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::FindHitNode()
//
//	PURPOSE:	Find possible node hit.  If no hit, then eModelNodeInvalid
//				returned.
//
// ----------------------------------------------------------------------- //

ModelNode CCharacterHitBox::FindHitNode( LTVector const& vDir, LTVector const& vFrom )
{
	UBER_ASSERT( g_pLTServer, "Null LTServer pointer" );
	UBER_ASSERT( g_pLTServer->GetModelLT(), "Null ModelLT" );
	UBER_ASSERT( g_pModelButeMgr, "Null Model ButeMgr" );

	// 광식 
	//----------------------------------------------------------------------------------------
	//LTBOOL bIsCrouch = LTFALSE;
	//CPlayerObj* pTargetPlayer = dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject(m_hModel) );
	//if( pTargetPlayer )
	//{
	//	if( ( (pTargetPlayer->GetControlFlags() & BC_CFLG_DUCK) != 0 ) || pTargetPlayer->IsForcedToDuck() )
	//	{
	//		bIsCrouch = LTTRUE;
	//	}
	//}
	//----------------------------------------------------------------------------------------

	ModelNode eModelNode = eModelNodeInvalid;
	ModelSkeleton eModelSkeleton = m_pHitBoxUser->GetModelSkeleton();
	
	// This algorithm may need to change since our dims are probably much
	// bigger than the model's actual dims...

    LTFLOAT fMaxPriority = (LTFLOAT)(-INT_MAX);

	if ( CHB_HIT_DEBUG_TRACK )
	{
        g_pLTServer->CPrint("Checking hit nodes..................");
	}

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;

		// Don't do transforms if we don't need to...
		LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, eCurrentNode);
		if (fNodeRadius <= 0.0f)
		{
			continue;
		}
		
		const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
		if( !szNodeName )
		{
			//UBER_ASSERT( 0, "CCharacterHitBox::HandleVectorImpact:  No node name in model skeleton." ); 
			continue;
		}

		LTransform transform;
		if ( GetNodeTransform(szNodeName, transform ) == LTFALSE )
		{
			continue;
		}

		// 광식
		// 앉았을 때는 모든 Node 들이 y 죽으로 -10 만큼 내려가도록 수정
		//-------------------------------------------------------------------------------------------		
		//if( bIsCrouch )
		//{
		//	transform.m_Pos.y -= 10;
		//}
		//-----------------------------------------------------------------------------------
		// 머리 node를 위로 살짝 들어올리기
		if( HL_HEAD == g_pModelButeMgr->GetSkeletonNodeLocation(eModelSkeleton, eCurrentNode) )
		{
			/*
			LTVector tempVector;
			//LTVector tempVector = transform.m_Rot.Up() + transform.m_Rot.Forward() + transform.m_Rot.Right();
			tempVector.y = 1.0f;
			tempVector.x = 0.2f;
			tempVector.z = 0.2f;
			tempVector.Normalize();
			transform.m_Pos += (tempVector*7.0f);
			*/
			LTVector tempVector;
			tempVector.Init();
			tempVector += transform.m_Rot.Right() * 8.5f;
			tempVector += transform.m_Rot.Forward() * 3.8f;
			transform.m_Pos += tempVector;
		}
		//-----------------------------------------------------------------------------------
			// Distance along ray to point of closest approach to node point

        const LTVector vRelativeNodePos = transform.m_Pos - vFrom;
            const LTFLOAT fRayDist = vDir.Dot(vRelativeNodePos);
            const LTFLOAT fDistSqr = (vDir*fRayDist - vRelativeNodePos).MagSqr();


		// Ignore the node if it wasn't within the radius of the hit spot.
		if( fDistSqr > fNodeRadius*fNodeRadius )
		{
			continue;
		}

		if ( CHB_HIT_DEBUG_TRACK )
        {
            g_pLTServer->CPrint("Hit ''%s'' node", g_pModelButeMgr->GetSkeletonNodeName( eModelSkeleton, eCurrentNode ));
		}

		// Get the hit priority of this node.
        LTFLOAT fPriority = g_pModelButeMgr->GetSkeletonNodeHitPriority( eModelSkeleton, eCurrentNode );

		// Ignore if not a higher priority node.
		if ( fPriority < fMaxPriority )
		{
			continue;
		}

		// Highest priority hit node so far.
		eModelNode = eCurrentNode;
		fMaxPriority = fPriority;
	}

	//-------------------------------------
	// 광식
	//-------------------------------------
	//m_eModelNode = eModelNode;
	//-------------------------------------

	return eModelNode;
}


uint8 CCharacterHitBox::FindHitNodeFromBody( LTVector const& vDir, LTVector const& vFrom )
{
	return (uint8)FindHitNode(vDir, vFrom);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::HandleVectorImpact()
//
//	PURPOSE:	Handle being hit by a vector
//
// Peter Higley method...
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::HandleVectorImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom, ModelNode& eModelNode)
{
	// See if we hit any nodes.
	eModelNode = FindHitNode( vDir, iInfo.m_Point );

	// Did we hit something?

	if (eModelNode != eModelNodeInvalid)
	{
		ModelSkeleton eModelSkeleton = m_pHitBoxUser->GetModelSkeleton();
		if ( CHB_HIT_DEBUG_TRACK )
		{
            g_pLTServer->CPrint("...........using ''%s''", g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eModelNode));
		}

		// Set the hit object as us ... attachments may change this

		iInfo.m_hObject = m_hModel;

		// Let the character/body prop's attachments handle the impact...

		if (m_pHitBoxUser->GetAttachments())
		{
			m_pHitBoxUser->GetAttachments()->HandleProjectileImpact(pProj, iInfo, vDir, vFrom, eModelSkeleton, eModelNode);
		}

        return LTTRUE;
	}

	if ( CHB_HIT_DEBUG_TRACK )
	{
        g_pLTServer->CPrint("....................hit nothing");
	}

	//
	// Shot didn't hit box, propagate through box and return that point in
	// vFrom
	//

	// This projects from intersected box face to the opposing face
	//  This assumes axis-aligned boxes.  To get a more general technique
	//	replace every z with a dot product with the forward vector, every
	//	x with a dot product with the right vector, and every y with a dot
	//	product with the up vector ("every x/y/z" doesn't include vObjDim)
/*
    LTVector vObjDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vObjDims);

    LTVector vProjection(0.0f,0.0f,0.0f);
    LTVector vAbsDir((float)fabs(vDir.x),(float)fabs(vDir.y),(float)fabs(vDir.z));

	if (vAbsDir.x > vAbsDir.y && vAbsDir.x > vAbsDir.z)
	{
		VEC_MULSCALAR(vProjection, vDir, vObjDims.x*2.0f/vAbsDir.x);
	}
	else if (vAbsDir.y > vAbsDir.x  && vAbsDir.y > vAbsDir.z)
	{
		VEC_MULSCALAR(vProjection, vDir, vObjDims.y*2.0f/vAbsDir.y);
	}
	else if (vAbsDir.z > vAbsDir.x  && vAbsDir.z > vAbsDir.y)
	{
		VEC_MULSCALAR(vProjection, vDir, vObjDims.z*2.0f/vAbsDir.z);
	}
*/

	//[SPIKE] 임시주석 
	//vFrom = iInfo.m_Point + vDir*5.0f;

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::DidProjectileImpact()
//
//	PURPOSE:	See if the projectile actually hit us
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterHitBox::DidProjectileImpact(CProjectile* pProjectile)
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );
	AIASSERT( pProjectile, m_hObject, "Called with NULL projectile" );
    if (!m_hModel || !pProjectile)
	{
		return LTFALSE;
	}

	// AI can always hit the player in singleplayer.  You don't
	// need hit detection.
	if (!IsMultiplayerGame() && IsPlayer(m_hModel))
	{
		return LTTRUE;
	}

	// See if we hit a node.
	ModelNode eModelNode = FindHitNode( pProjectile->GetFireDir(), pProjectile->GetFirePos() );

	// Did we hit something?

	if (eModelNode != eModelNodeInvalid)
	{
		if ( CHB_HIT_DEBUG_TRACK )
		{
            g_pLTServer->CPrint("...........using ''%s''", g_pModelButeMgr->GetSkeletonNodeName(m_pHitBoxUser->GetModelSkeleton(), eModelNode));
		}

		// Set our last hit node

		m_pHitBoxUser->SetModelNodeLastHit(eModelNode);

		// Adjust damage
		pProjectile->AdjustDamage( m_pHitBoxUser->ComputeDamageModifier(eModelNode) );

        return LTTRUE;
	}

	if ( CHB_HIT_DEBUG_TRACK )
	{
        g_pLTServer->CPrint("....................hit nothing");
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetNodeRadius()
//
//	PURPOSE:	Get the model node's radius, checking first to see if they
//				are being overridden.  If they are, then use the manually
//				set values.  If they are not, then use the ModelBute 
//				systems value for this node.
//
// ----------------------------------------------------------------------- //
LTFLOAT CCharacterHitBox::GetNodeRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	// See if we're overriding the radius...

	if (CHB_NODE_RADIUS_USE_OVERRIDE)
	{
		switch ( g_pModelButeMgr->GetSkeletonNodeLocation(eModelSkeleton, eModelNode) )
		{
			case HL_HEAD :
				return ( CHB_HEAD_NODE_RADIUS );
			break;

			case HL_TORSO :
				return ( CHB_TORSO_NODE_RADIUSE );
			break;

			case HL_ARM :
				return ( CHB_ARM_NODE_RADIUS );
			break;

			case HL_LEG :
				return ( CHB_LEG_NODE_RADIUS );
			break;

			default :
				return ( HB_DEFAULT_NODE_RADIUS );
			break;
		}
	}
	else
	{
		return ( m_pHitBoxUser->GetNodeRadius( eModelSkeleton, eModelNode ) );
	}
}

static LTVector GetNodeModelColor(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	LTVector vColor(1, 1, 1);

	switch ( g_pModelButeMgr->GetSkeletonNodeLocation(eModelSkeleton, eModelNode) )
	{
		case HL_HEAD :
			vColor = LTVector(1, 0, 0);
		break;

		case HL_TORSO :
			vColor = LTVector(1, 1, 0);
		break;

		case HL_ARM :
			vColor = LTVector(0, 1, 0);
		break;

		case HL_LEG :
			vColor = LTVector(0, 0, 1);
		break;
	}

	return vColor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::CreateNodeRadiusModels()
//
//	PURPOSE:	Create models representing the model node radii.  Iterates
//				over all of the nodes in a model, and if a node transform 
//				successfully can be retrieved, created the model and adds
//				it to the list.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::CreateNodeRadiusModels()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );

	if (!m_hModel)
	{
		return;
	}

	ModelSkeleton eModelSkeleton = m_pHitBoxUser->GetModelSkeleton();
	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);

	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;
		const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

        LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, eCurrentNode);
		if (fNodeRadius <= 0.0f)
		{
			continue;
		}

  		if (!szNodeName)
		{
			continue;
		}

        LTransform transform;
		if ( GetNodeTransform(szNodeName, transform ) == LTFALSE )
		{
			continue;
		}

		// Create the radius model...

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		theStruct.m_Pos = transform.m_Pos;
		// 광식 : Path가 틀림
		//SAFE_STRCPY(theStruct.m_Filename, "Models\\sphere.ltb");
		SAFE_STRCPY(theStruct.m_Filename, "GLOBAL\\MODELS\\sphere.ltb");
		theStruct.m_Flags = FLAG_VISIBLE;
		theStruct.m_ObjectType = OT_MODEL;

        HCLASS hClass = g_pLTServer->GetClass("BaseClass");
        LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
		UBER_ASSERT( pModel, "Failed to create BaseClass" );
		if (!pModel)
		{
			return;
		}

		// Don't eat ticks please...
		::SetNextUpdate(pModel->m_hObject, UPDATE_NEVER);

        LTVector vScale;
		vScale.Init(fNodeRadius, fNodeRadius, fNodeRadius);
        g_pLTServer->ScaleObject(pModel->m_hObject, &vScale);

		NodeRadiusStruct* pNRS = debug_new(NodeRadiusStruct);

		pNRS->eNode	 = eCurrentNode;
		pNRS->hModel = pModel->m_hObject;

		// Set the model's color...
		LTVector vColor = GetNodeModelColor(eModelSkeleton, eCurrentNode);
		g_pLTServer->SetObjectColor(pModel->m_hObject, vColor.x, vColor.y, vColor.z, 0.9f);

		// Add the model to our list...

		m_NodeRadiusList.AddTail(pNRS);
	}

	// 광식 : 다리 사이에 구멍 매꾸기
	//------------------------------------
	/*
	{
		const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, (ModelNode)15);
		LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, (ModelNode)15);
		LTransform transform;
		if ( GetNodeTransform( szNodeName, transform ) == LTFALSE )
		{
			return;
		}

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		theStruct.m_Pos = transform.m_Pos;
		SAFE_STRCPY(theStruct.m_Filename, "GLOBAL\\MODELS\\sphere.ltb");
		theStruct.m_Flags = FLAG_VISIBLE;
		theStruct.m_ObjectType = OT_MODEL;

        HCLASS hClass = g_pLTServer->GetClass("BaseClass");
        LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
		UBER_ASSERT( pModel, "Failed to create BaseClass" );
		if (!pModel)
		{
			return;
		}

		::SetNextUpdate(pModel->m_hObject, UPDATE_NEVER);

		LTVector vScale;
		vScale.Init(fNodeRadius, fNodeRadius, fNodeRadius);
        g_pLTServer->ScaleObject(pModel->m_hObject, &vScale);

		NodeRadiusStruct* pNRS = debug_new(NodeRadiusStruct);
		
		pNRS->eNode	 = (ModelNode)m_NodeRadiusList.GetLength();
		pNRS->hModel = pModel->m_hObject;

		// Set the model's color...
		LTVector vColor = LTVector(0, 0, 1);
		g_pLTServer->SetObjectColor(pModel->m_hObject, vColor.x, vColor.y, vColor.z, 0.9f);

		m_NodeRadiusList.AddTail(pNRS);
	}
	*/
	//------------------------------------
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::RemoveNodeRadiusModels()
//
//	PURPOSE:	Remove models representing the model node radii
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::RemoveNodeRadiusModels()
{
	if (m_NodeRadiusList.GetLength())
	{
		m_NodeRadiusList.Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::UpdateNodeRadiusModels()
//
//	PURPOSE:	Update models representing the model node radii
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::UpdateNodeRadiusModels()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );

	// Create the models if necessary...

	if (!m_NodeRadiusList.GetLength())
	{
		CreateNodeRadiusModels();
	}

	// 광식 
	//----------------------------------------------------------------------------------------
	LTBOOL bIsCrouch = LTFALSE;
	CPlayerObj* pTargetPlayer = dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject(m_hModel) );
	if( pTargetPlayer )
	{
		if( ( (pTargetPlayer->GetControlFlags() & BC_CFLG_DUCK) != 0 ) || pTargetPlayer->IsForcedToDuck() )
		{
			bIsCrouch = LTTRUE;
		}
	}
	//----------------------------------------------------------------------------------------

	ModelSkeleton eModelSkeleton = m_pHitBoxUser->GetModelSkeleton();

	NodeRadiusStruct** pNRS = m_NodeRadiusList.GetItem(TLIT_FIRST);

	while (pNRS && *pNRS && (*pNRS)->hModel)
	{
		const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, (*pNRS)->eNode);

		LTransform resultTransform;
		if ( GetNodeTransform( szNodeName, resultTransform ) != LTFALSE )
		{
			// 광식
			// 앉았을 때 node 들을 y 축으로 -10 만큼 이동 시키기
			//-----------------------------------------------------------------------------------
			//if( bIsCrouch )
			//{
			//	resultTransform.m_Pos.y -= 10;
			//}
			// 머리 node를 살짝 들어올리기.
			//-----------------------------------------------------------------------------------
			if( HL_HEAD == g_pModelButeMgr->GetSkeletonNodeLocation(eModelSkeleton, (*pNRS)->eNode) )
			{
				LTVector tempVector;
				tempVector.Init();
				tempVector += resultTransform.m_Rot.Right() * 8.5f;
				tempVector += resultTransform.m_Rot.Forward() * 3.8f;
				resultTransform.m_Pos += tempVector;
			}
			//-----------------------------------------------------------------------------------

			g_pLTServer->SetObjectPos((*pNRS)->hModel, &(resultTransform.m_Pos));

			LTFLOAT fNodeRadius = GetNodeRadius(eModelSkeleton, (*pNRS)->eNode);

			LTVector vScale;
			vScale.Init(fNodeRadius, fNodeRadius, fNodeRadius);
			g_pLTServer->ScaleObject((*pNRS)->hModel, &vScale);
		}
	
		pNRS = m_NodeRadiusList.GetItem(TLIT_NEXT);
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacterHitBox::GetNodeTransform()
//              
//	PURPOSE:	Returns True if the function succeeded, false if it failed.
//
//				Returns by parameter the transform for the passed in node.  If
//				the owner is an attachment, then be sure to use the tranform 
//				information from the parent.
//              
//----------------------------------------------------------------------------
LTBOOL CCharacterHitBox::GetNodeTransform( const char* const pszNodeName, LTransform& NodeTransform )
{
	UBER_ASSERT( m_pHitBoxUser, "No registered Hitbox User" );
	UBER_ASSERT( g_pCommonLT, "No g_pCommonLT" );
	UBER_ASSERT( g_pModelLT, "No g_pModelLT" );

	if ( m_pHitBoxUser->GetAttachment() != NULL )
	{
		LTRESULT ltResult;

		// Get the Parent through the use of the Attachment Parent/Child retreval
		HOBJECT hChild;
		HOBJECT hParent;
		ltResult = g_pCommonLT->GetAttachmentObjects( m_pHitBoxUser->GetAttachment(), hParent, hChild);
//		UBER_ASSERT( ltResult == LT_OK, "Unable to find Parent");
		if ( ltResult != LT_OK )
		{
			return LTFALSE;
		}

		// Get the Parents socket where this attachment is attached.
		HMODELSOCKET hSocket;
		ltResult = g_pModelLT->GetSocket(hParent, const_cast<char*>(pszNodeName), hSocket);
//		UBER_ASSERT1( ltResult == LT_OK, "Unable to find Socket %s", pszNodeName );
		if ( ltResult != LT_OK )
		{
			return LTFALSE;
		}

		ltResult = g_pCommonLT->GetAttachedModelNodeTransform( m_pHitBoxUser->GetAttachment(), hSocket, NodeTransform);
//		UBER_ASSERT1( ltResult == LT_OK, "Unable to find Socket %s", pszNodeName );
		if ( ltResult != LT_OK )
		{
			return LTFALSE;
		}
	}
	else
	{
		LTRESULT ltResult;
		HMODELNODE hNode;
		ltResult = g_pModelLT->GetNode(m_hModel, const_cast<char*>(pszNodeName), hNode);
//		UBER_ASSERT1( ltResult == LT_OK, "Unable to find Socket %s", pszNodeName );
		if ( ltResult != LT_OK )
		{
			return LTFALSE;
		}

        LTransform transform;
        ltResult = g_pModelLT->GetNodeTransform(m_hModel, hNode, NodeTransform, LTTRUE);
//		UBER_ASSERT( ltResult == LT_OK, "" );
		if ( ltResult != LT_OK )
		{
			return LTFALSE;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Update()
//
//	PURPOSE:	Update our position
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Update()
{
	AIASSERT( m_pHitBoxUser, m_hObject, "Called with NULL HitBoxUser" );
	AIASSERT( m_hModel, m_hObject, "Called with NULL m_hModel" );
	AIASSERT( m_hObject, m_hObject, "Called with NULL m_hObject" );
	if (!m_hModel || !m_hObject)
	{
		return;
	}

    LTVector vPos, vMyPos;
	g_pLTServer->GetObjectPos(m_hModel, &vPos);
	
	// Make sure the hit box offset is relative to the model...
	
	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hModel, &rRot );

	LTMatrix mMat;
	rRot.ConvertToMatrix( mMat );

	vPos += (mMat * m_vOffset);

	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

	if (vPos.DistSqr(vMyPos) > 0.1)
	{
		g_pLTServer->SetObjectPos(m_hObject, &vPos);
	}
	
	if( (m_bAnimControlsDims || m_bAnimControlsOffset) && (m_hControllingAnim != INVALID_ANI) )
	{
		HMODELANIM hCurAnim = INVALID_ANI;
		if( LT_OK == g_pModelLT->GetCurAnim( m_hModel, MAIN_TRACKER, hCurAnim ))
		{
			if( hCurAnim != m_hControllingAnim )
			{
				// We changed animations from our controlling anim so default our dims and offset...

				// Set offset first since SetDimsToModel() will update the client
				SetOffset( LTVector(0,0,0) );
				SetDimsToModel();

				// The animation is no longer controlling us...

				m_bAnimControlsDims = m_bAnimControlsOffset = false;
				m_hControllingAnim = INVALID_ANI;
			}	
		}
	}
	
	// Make sure the hit box is at least at the minimum dims...

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

	if( vDims.x < HB_DIMS_MIN_XZ ||
		vDims.z < HB_DIMS_MIN_XZ )
	{
		vDims.x = vDims.z = HB_DIMS_MIN_XZ;
		g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

		m_pHitBoxUser->UpdateClientHitBox();
	}

	if( vDims.y < HB_DIMS_MIN_Y	)
	{
		vDims.y = HB_DIMS_MIN_Y;
		g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

		m_pHitBoxUser->UpdateClientHitBox();
	}

	// See if we should show our model node radii...
	
#ifndef _FINAL
	float fShowNodeRadii = (CHB_SHOW_NODE_RADII ? (IsPlayer(m_hModel) ? CHB_SHOW_PLAYER_NODE_RADII : 1.0f) : 0.0f);
	if (fShowNodeRadii)
	{
		UpdateNodeRadiusModels();
	}
	else
	{
		RemoveNodeRadiusModels();
	}
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::SetDimsToModel
//
//	PURPOSE:	Set the dims of the hit box to be an enlarged percentage of the models dims.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::SetDimsToModel()
{
	if( !m_hModel || !m_hObject )
		return;

	LTVector vModelDims;
	GetDefaultModelDims( vModelDims );
	
	EnlargeAndSetDims( vModelDims );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::GetDefaultModelDims
//
//	PURPOSE:	Grab the dims of the model that owns us...
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::GetDefaultModelDims( LTVector &vDims )
{
	vDims.Init();

	if( !m_hModel || !m_hObject )
		return;

	HMODELANIM hAni = g_pModelLT->GetCurAnim( m_hModel, MAIN_TRACKER, hAni );
	if( hAni != INVALID_ANI )
	{
		g_pCommonLT->GetModelAnimUserDims( m_hModel, &vDims, hAni );
	}
	else
	{
		g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::EnlargeAndSetDims
//
//	PURPOSE:	Set the dims of the hit box to be an enlarged percentage of the given dims.
//				Use this to be consistent when setting the hitbox dims.
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::EnlargeAndSetDims( LTVector &vDims )
{
	if( !m_hObject )
		return;

	// Enlarge the hit box dims by a percentage of the given dims...
	 
	vDims *= HB_DIMS_ENLARGE_PERCENT;

	g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );
	
	// Let the HitBoxUser notify the clients of the change...

	if( m_pHitBoxUser )
	{
		m_pHitBoxUser->UpdateClientHitBox();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Save(ILTMessage_Write *pMsg)
{
	AIASSERT( pMsg, m_hObject,  "Save with no pMsg" );
	if (!pMsg)
	{
		return;
	}

    SAVE_HOBJECT(m_hModel);
	SAVE_VECTOR(m_vOffset);
	SAVE_BOOL(m_bCanActivate);
	SAVE_bool(m_bCanBeSearched);
	SAVE_bool( m_bAnimControlsDims );
	SAVE_bool( m_bAnimControlsOffset );
	SAVE_DWORD( m_hControllingAnim );
	AIASSERT( dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(m_hModel)) == m_pHitBoxUser, NULL, "" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterHitBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCharacterHitBox::Load(ILTMessage_Read *pMsg)
{
	AIASSERT( pMsg, m_hObject, "Load with no pMsg" );
	if (!pMsg)
	{
		return;
	}

	LOAD_HOBJECT(m_hModel);
	LOAD_VECTOR(m_vOffset);
    LOAD_BOOL(m_bCanActivate);
	LOAD_bool(m_bCanBeSearched);
	LOAD_bool( m_bAnimControlsDims );
	LOAD_bool( m_bAnimControlsOffset );
	LOAD_DWORD_CAST( m_hControllingAnim, HMODELANIM );

	m_pHitBoxUser = dynamic_cast<IHitBoxUser*>(g_pLTServer->HandleToObject(m_hModel));
	AIASSERT( m_pHitBoxUser, m_hModel, "Unable to get the HitBoxUser interface from m_hModel." );
}


