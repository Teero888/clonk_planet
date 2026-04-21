/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Dynamic list for crew member info */

#include <C4Include.h>

C4ObjectInfoList::C4ObjectInfoList()
	{
	Default();
	}

C4ObjectInfoList::~C4ObjectInfoList()
	{

	}

void C4ObjectInfoList::Default()
	{
	First=NULL;
	}

void C4ObjectInfoList::Clear()
	{
	C4ObjectInfo *next;
	while (First)
		{
		next=First->Next;
		delete First;
		First=next;
		}
	}

int C4ObjectInfoList::Load(C4Group &hGroup)
	{
  C4ObjectInfo *ninf;
  int infn=0;
  char entryname[256+1];

  // Search all c4i files
	hGroup.ResetSearch();
  while (hGroup.FindNextEntry(C4CFN_ObjectInfoFiles,entryname))
		if (ninf=new C4ObjectInfo)
			if (ninf->Load(hGroup,entryname))	{ Add(ninf); infn++; }
			else delete ninf;

  // Search all c4o files
	hGroup.ResetSearch();
  while (hGroup.FindNextEntry("*.c4o",entryname))
		if (ninf=new C4ObjectInfo)
			if (ninf->Load(hGroup,entryname))	{ Add(ninf); infn++; }
			else delete ninf;

  return infn;
	}

BOOL C4ObjectInfoList::Add(C4ObjectInfo *pInfo)
	{
	if (!pInfo) return FALSE;
	pInfo->Next=First;
	First=pInfo;
	return TRUE;
	}

void C4ObjectInfoList::MakeValidName(char *sName)
	{
	char tstr[_MAX_PATH];
  int iname,namelen=SLen(sName);
  for (iname=2; NameExists(sName); iname++)
    {
    sprintf(tstr," %d",iname);
    SCopy(tstr,sName+Min(namelen,C4MaxName-SLen(tstr)));
    }
	}

BOOL C4ObjectInfoList::NameExists(const char *szName)
	{
  C4ObjectInfo *cinf;
  for (cinf=First; cinf; cinf=cinf->Next)
    if (SEqualNoCase(szName,cinf->Name))
      return TRUE;
  return FALSE;	
	}

C4ObjectInfo* C4ObjectInfoList::GetIdle(C4ID c_id, C4DefList &rDefs)
	{
	C4Def *pDef;
  C4ObjectInfo *pInfo;
  C4ObjectInfo *pHiExp=NULL;

	// Search list 
	for (pInfo=First; pInfo; pInfo=pInfo->Next)
		// Valid only
		if (pDef = rDefs.ID2Def(pInfo->id)) 
			// Use standard crew or matching id
			if ( (!c_id && !pDef->NativeCrew) || (pInfo->id==c_id) )
				// Participating and not in action
				if (pInfo->Participation) if (!pInfo->InAction)
					// Not dead
					if (!pInfo->HasDied)
						// Highest experience
						if (!pHiExp || (pInfo->Experience>pHiExp->Experience))
							// Set this
							pHiExp=pInfo;
	
	// Found
	if (pHiExp)
    { 
		pHiExp->Recruit();
    return pHiExp; 
    } 

  return NULL;
	}

C4ObjectInfo* C4ObjectInfoList::New(C4ID n_id, C4DefList *pDefs, const char *cpNames)
	{
  C4ObjectInfo *pInfo;
  if (!(pInfo=new C4ObjectInfo)) return NULL;
  // Default type clonk if none specified
	if (n_id==C4ID_None) n_id=C4ID_Clonk;
	// Check type valid
	if (pDefs) if (!pDefs->ID2Def(n_id)) return FALSE;
	// Default by type
  ((C4ObjectInfoCore*)pInfo)->Default(n_id,pDefs,cpNames);
	// Set birthday
	pInfo->Birthday=time(NULL);
	// Make valid names
	MakeValidName(pInfo->Name);
  // Add
	Add(pInfo);
  return pInfo;
	}

void C4ObjectInfoList::Evaluate()
	{
	C4ObjectInfo *cinf;
  for (cinf=First; cinf; cinf=cinf->Next)
		cinf->Evaluate();
	}

BOOL C4ObjectInfoList::Save(C4Group &hGroup)
	{
	// Remove any old format infos
	hGroup.Delete("*.c4o");
	// Save in opposite order (for identical crew order on load)
	C4ObjectInfo *pInfo;
  for (pInfo = GetLast(); pInfo; pInfo = GetPrevious(pInfo))
		if (!pInfo->Save(hGroup))
			return FALSE;
	return TRUE;
	}

C4ObjectInfo* C4ObjectInfoList::GetIdle(const char *szByName)
	{
  C4ObjectInfo *pInfo;
	// Find matching name, participating, alive and not in action
  for (pInfo=First; pInfo; pInfo=pInfo->Next)
		if (SEqualNoCase(pInfo->Name,szByName))
			if (pInfo->Participation) if (!pInfo->InAction)
				if (!pInfo->HasDied)
					{
					pInfo->Recruit();
					return pInfo;
					}
	return NULL;
	}

void C4ObjectInfoList::DetachFromObjects()
	{
  C4ObjectInfo *cinf;
  for (cinf=First; cinf; cinf=cinf->Next)
		Game.Objects.ClearInfo(cinf);
	}

C4ObjectInfo* C4ObjectInfoList::GetLast()
	{
	for (C4ObjectInfo *cInfo = First; cInfo && cInfo->Next; cInfo=cInfo->Next);
	return cInfo;
	}

C4ObjectInfo* C4ObjectInfoList::GetPrevious(C4ObjectInfo *pInfo)
	{
	for (C4ObjectInfo *cInfo = First; cInfo; cInfo=cInfo->Next)
		if (cInfo->Next == pInfo)
			return cInfo;
	return NULL;
	}
