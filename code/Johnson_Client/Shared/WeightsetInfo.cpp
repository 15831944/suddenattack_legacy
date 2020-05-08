//-----------------------------------------------------------------------------
// ��  �� : WeightsetInfo.cpp
//-----------------------------------------------------------------------------
// ��  �� : ĳ���;ִϸ��̼ǿ� ���Ǵ� weightset ���� Ŭ����
// ��  �� : [2003, 5]
// �ۼ��� : [burumal]
// ��  �� : (c) GMOS 
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "CommonUtilities.h"
#include "GameButeMgr.h"
#include "WeightsetInfo.h"

//-----------------------------------------------------------------------------
// ���� : ��� ����
//-----------------------------------------------------------------------------
#define sWSBMGR_TWITCH					"Twitch"
#define sWSBMGR_BLINK					"Blink"
#define sWSBMGR_WEIGHTSET				"Weightset"
#define sWSBMGR_NAME					"Name"
#define sWSBMGR_UPPER_NAME				"UpperName"
#define sWSBMGR_LOWER_NAME				"LowerName"
#define sWSBMGR_PITCH_CONTROL			"PitchControl"
#define sWSBMGR_WEIGHTSET_CHANGE		"WeightsetChange"
#define sWSBMGR_WEIGHTSET_ID			"WeightsetID"
#define sWSBMGR_UPPER_ANI_LIST			"UpperAniList"
#define sWSBMGR_LOWER_ANI_LIST			"LowerAniList"

#define nMAX_WEIGHTSET_NAME_LEN			32

//-----------------------------------------------------------------------------
// ���� : ���� ����
//-----------------------------------------------------------------------------
typedef std::list<CModelWeightsetInfo*> ModelWeightsetInfoPool;
ModelWeightsetInfoPool g_ModelWeightsetInfoPool;

static char g_pTagName[64];
static char g_pAttName[128];

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfo::Init()
//-----------------------------------------------------------------------------
// ���� : �ʱ�ȭ �Լ�
//-----------------------------------------------------------------------------
// ���� : 1��
//-----------------------------------------------------------------------------
// 1) 	: 
//-----------------------------------------------------------------------------
LTBOOL CModelWeightsetInfo::Init(const char* pAttributeFile)
{		
	char pBuff[nMAX_WEIGHTSET_NAME_LEN + 1];

	if ( !pAttributeFile )
		return LTFALSE;
	
	if ( !Parse(pAttributeFile) )
	{		
		return LTFALSE;
	}	
	
	// WeightsetNameList...	

	// twitch
	if ( m_buteMgr.Exist(sWSBMGR_TWITCH) )
	{
		m_buteMgr.GetString(sWSBMGR_TWITCH, sWSBMGR_NAME, pBuff, nMAX_WEIGHTSET_NAME_LEN);
		if ( IsSameUpper2Tracker(pBuff) )
		{
			_ASSERT(0);
			return LTFALSE;
		}
		
		m_cTwitch = pBuff;
	}

	// blink
	if ( m_buteMgr.Exist(sWSBMGR_BLINK) )
	{
		m_buteMgr.GetString(sWSBMGR_BLINK, sWSBMGR_NAME, pBuff, nMAX_WEIGHTSET_NAME_LEN);
		if ( IsSameUpper2Tracker(pBuff) )
		{
			_ASSERT(0);
			return LTFALSE;
		}

		m_cBlink = pBuff;
	}
	
	// weightset0, ...
	uint32 nWeightsetNum = 0;
	sprintf(g_pTagName, "%s%d", sWSBMGR_WEIGHTSET, nWeightsetNum);

	while ( m_buteMgr.Exist(g_pTagName) )
	{
		if ( !InitWeightsetNameList(g_pTagName) )
			return LTFALSE;

		nWeightsetNum++;
		sprintf(g_pTagName, "%s%d", sWSBMGR_WEIGHTSET, nWeightsetNum);
	}

	/*
	#ifdef _CLIENTBUILD
		g_pLTClient->CPrint("Client: %s %d", GetAttributeFileName(), m_nWeightsetNum);
	#else
		g_pLTServer->CPrint("Server: %s %d", GetAttributeFileName(), m_nWeightsetNum);
	#endif
	*/

	// WeightsetChangeList ...
	uint32 nWeightsetChangeNum = 0;
	sprintf(g_pTagName, "%s%d", sWSBMGR_WEIGHTSET_CHANGE, nWeightsetChangeNum);

	while ( m_buteMgr.Exist(g_pTagName) )
	{
		if ( !InitWeightsetChangeList(g_pTagName) )
			return LTFALSE;

		nWeightsetChangeNum++;
		sprintf(g_pTagName, "%s%d", sWSBMGR_WEIGHTSET_CHANGE, nWeightsetChangeNum);
	}

	return LTTRUE;
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfo::Term()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
void CModelWeightsetInfo::Term()
{	
	ReleaseWeightsetNameList();
	ReleaseWeightsetChangeList();

	CGameButeMgr::Term();
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfo::InitWeightsetNameList()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
// ���� : 1��
//-----------------------------------------------------------------------------
// 1) 	: 
//-----------------------------------------------------------------------------
LTBOOL CModelWeightsetInfo::InitWeightsetNameList(const char* pTagName)
{
	char pUpper[nMAX_WEIGHTSET_NAME_LEN];
	char pLower[nMAX_WEIGHTSET_NAME_LEN];
	char pPitchControl[nMAX_WEIGHTSET_NAME_LEN];

	m_buteMgr.GetString(pTagName, sWSBMGR_UPPER_NAME, pUpper, nMAX_WEIGHTSET_NAME_LEN);
	m_buteMgr.GetString(pTagName, sWSBMGR_LOWER_NAME, pLower, nMAX_WEIGHTSET_NAME_LEN);	
	m_buteMgr.GetString(pTagName, sWSBMGR_PITCH_CONTROL, pPitchControl, nMAX_WEIGHTSET_NAME_LEN);	

	if ( IsSameUpper2Tracker(pUpper) || IsSameUpper2Tracker(pLower) || IsSameUpper2Tracker(pPitchControl) )
	{
		_ASSERT(0);
		return LTFALSE;
	}

	if ( pUpper[0] == '\0' || pLower[0] == '\0' )
		return LTFALSE;	
	
	WeightsetName_t* pNewName = new WeightsetName_t;

	if ( pNewName )
	{		
		pNewName->m_cUpper = pUpper;
		pNewName->m_cLower = pLower;

		if ( pPitchControl[0] != '\0' )
		{
			pNewName->m_cPitchControl = pPitchControl;
		}
		
		m_WeightsetNameList.push_back(pNewName);
	}

	return LTTRUE;
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfo::InitWeightsetChangeList()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
// ���� : 1��
//-----------------------------------------------------------------------------
// 1) 	: 
//-----------------------------------------------------------------------------
LTBOOL CModelWeightsetInfo::InitWeightsetChangeList(const char* pTagName)
{
	#define nMAX_ANILIST 1024
	
	char* pBuff;
	char pAniList[nMAX_ANILIST];
	uint32 nWeightsetID;
	WeightsetChange_t* pNewChangeList = LTNULL;

	nWeightsetID = m_buteMgr.GetInt(pTagName, sWSBMGR_WEIGHTSET_ID, 0);

	// weightset 0 �� ���ؼ��� change list�� �ۼ��� ������ ����.
	if ( nWeightsetID == 0 )
		return LTFALSE;

	// upper list
	m_buteMgr.GetString(pTagName, sWSBMGR_UPPER_ANI_LIST, pAniList, nMAX_ANILIST);

	pBuff = pAniList;
	while ( *pBuff == ' ' || *pBuff == '\t' )
		pBuff++;

	if ( strlen(pBuff) )
	{
		if ( !pNewChangeList )
			pNewChangeList = new WeightsetChange_t;

		if ( pNewChangeList )
		{
			char* pAniName = strtok(pBuff, ",");
			while ( pAniName )
			{
				std::string strAniName = pAniName;
				pNewChangeList->m_lstUpperAni.push_back(strAniName);
				
				pAniName = strtok(NULL, ",");
			}
		}
	}

	// lower list
	m_buteMgr.GetString(pTagName, sWSBMGR_LOWER_ANI_LIST, pAniList, nMAX_ANILIST);

	pBuff = pAniList;
	while ( *pBuff == ' ' || *pBuff == '\t' )
		pBuff++;

	if ( strlen(pBuff) )
	{
		if ( !pNewChangeList )
			pNewChangeList = new WeightsetChange_t;

		if ( pNewChangeList )
		{
			char* pAniName = strtok(pBuff, ",");
			while ( pAniName )
			{
				std::string strAniName = pAniName;
				pNewChangeList->m_lstLowerAni.push_back(strAniName);

				pAniName = strtok(NULL, ",");
			}
		}
	}

	// add to list
	if ( pNewChangeList )
	{
		if ( !GetWeightsetName(nWeightsetID) || 
			(pNewChangeList->m_lstUpperAni.size() == 0 && pNewChangeList->m_lstLowerAni.size() == 0) )
		{
			delete pNewChangeList;
			return LTFALSE;
		}

		pNewChangeList->m_nWeightsetID = nWeightsetID;
		pNewChangeList->m_sWeightsetName = *GetWeightsetName(nWeightsetID);
		
		m_WeightsetChangeList.push_back(pNewChangeList);

		return LTTRUE;
	}

	return LTFALSE;
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfo::ReleaseWeightsetNameList()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
void CModelWeightsetInfo::ReleaseWeightsetNameList()
{
	WeightsetNameList::iterator it;

	for ( it = m_WeightsetNameList.begin(); it != m_WeightsetNameList.end(); ++it )
	{
		WeightsetName_t* pDel = (*it);

		if ( pDel )
		{
			delete pDel;
			(*it) = LTNULL;
		}
	}

	m_WeightsetNameList.clear();
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfo::ReleaseWeightsetChangeList()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
void CModelWeightsetInfo::ReleaseWeightsetChangeList()
{
	WeightsetChangeList::iterator it;

	for ( it = m_WeightsetChangeList.begin(); it != m_WeightsetChangeList.end(); ++it )
	{
		WeightsetChange_t* pDel = (*it);

		if ( pDel )
		{
			delete pDel;
			(*it) = LTNULL;
		}
	}

	m_WeightsetChangeList.clear();
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfo::FindExist()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
// ���� : 1��
//-----------------------------------------------------------------------------
// 1) 	: 
//-----------------------------------------------------------------------------
WeightsetName_t* CModelWeightsetInfo::GetWeightsetName(uint32 nWeightsetID)
{
	WeightsetNameList::iterator it;

	if ( m_WeightsetNameList.size() <= nWeightsetID )
		return LTNULL;

	it = m_WeightsetNameList.begin();
	for ( uint32 nIdx = 0; nIdx < nWeightsetID; nIdx++ )
		it++;

	return (*it);
}


//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfo::IsSameUpper2Tracker()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
LTBOOL CModelWeightsetInfo::IsSameUpper2Tracker(char* pName) const
{
	if ( _stricmp(pName, sANITRACKER_UPPER2_NAME) == 0 )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfoMgr::Init()
//-----------------------------------------------------------------------------
// ���� : e
//-----------------------------------------------------------------------------
// ���� : 1��
//-----------------------------------------------------------------------------
// 1) 	: 
//-----------------------------------------------------------------------------
void CModelWeightsetInfoMgr::Init(const char* pAttributeFile)
{
	if ( FindExist(pAttributeFile) )
		return;

	CModelWeightsetInfo* pNewWeightsetInfo = new CModelWeightsetInfo;

	if ( pNewWeightsetInfo )
	{
		if (pNewWeightsetInfo->Init(pAttributeFile) != LTTRUE )
		{
			delete pNewWeightsetInfo;
			return;
		}

		g_ModelWeightsetInfoPool.push_back(pNewWeightsetInfo);
	}
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfoMgr::FindExist()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
// ���� : 1��
//-----------------------------------------------------------------------------
// 1) 	: 
//-----------------------------------------------------------------------------
CModelWeightsetInfo* CModelWeightsetInfoMgr::FindExist(const char* pAttributeFile)
{
	ModelWeightsetInfoPool::iterator it;

	for ( it = g_ModelWeightsetInfoPool.begin(); it != g_ModelWeightsetInfoPool.end(); ++it )
	{		
		ASSERT((*it));

		if ( _stricmp((*it)->GetAttributeFileName(), pAttributeFile) == 0 )
			return (*it);
	}

	return LTNULL;
}

//-----------------------------------------------------------------------------
// �Լ� : CModelWeightsetInfoMgr::Term()
//-----------------------------------------------------------------------------
// ���� : 
//-----------------------------------------------------------------------------
void CModelWeightsetInfoMgr::Term()
{
	ModelWeightsetInfoPool::iterator it;

	for ( it = g_ModelWeightsetInfoPool.begin(); it != g_ModelWeightsetInfoPool.end(); ++it )
	{
		CModelWeightsetInfo* pWeightsetInfo = (*it);

		if ( pWeightsetInfo )
		{
			delete pWeightsetInfo;
			(*it) = LTNULL;
		}
	}

	g_ModelWeightsetInfoPool.clear();
}
