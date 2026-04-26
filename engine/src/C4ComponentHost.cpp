/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Holds a single text file component from a group */

#include <C4Include.h>

C4ComponentHost *pCmpHost=NULL;

#if defined(C4ENGINE) && defined(_WIN32)

BOOL CALLBACK ComponentDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
	{

	if (!pCmpHost) return FALSE;

	switch (Msg)
		{
		//------------------------------------------------------------------
		case WM_CLOSE:
			pCmpHost->Close();
			break;
		//-------------------------------------------------------------------------------------------------
		case WM_DESTROY:  
			StoreWindowPosition(hDlg,"Component","Software\\RedWolf Design\\Clonk 4\\Console",FALSE);
			break;
		//-------------------------------------------------------------------------------------------------
		case WM_INITDIALOG:
			pCmpHost->InitDialog(hDlg);
			RestoreWindowPosition(hDlg,"Component","Software\\RedWolf Design\\Clonk 4\\Console");
			return TRUE;
		//-------------------------------------------------------------------------------------------------
		case WM_COMMAND:  
			// Evaluate command
			switch (LOWORD(wParam))
				{
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDCANCEL:
					pCmpHost->Close();
					return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDOK:
					// IDC_EDITDATA to Data
					char buffer[65000];
					GetDlgItemText(hDlg,IDC_EDITDATA,buffer,65000);
					pCmpHost->Modified=TRUE;
					if (pCmpHost->Data) delete [] pCmpHost->Data; pCmpHost->Data=NULL;
					pCmpHost->Size = SLen(buffer);
					if (pCmpHost->Size)
						{
						pCmpHost->Data = new char [pCmpHost->Size+1];
						SCopy(buffer,pCmpHost->Data);
						}
					pCmpHost->Close();
					return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				}
			return FALSE;
		//-----------------------------------------------------------------------------------------------------------------------------------
		}
	return FALSE;
	}

#endif 

C4ComponentHost::C4ComponentHost()
	{
	Default();
	}

C4ComponentHost::~C4ComponentHost()
	{
	Clear();
	}

void C4ComponentHost::Default()
	{
	Data=NULL;
	Size=0;
	Modified=FALSE;
	Name[0]=0;
	Filename[0]=0;
	hDialog=NULL;
	}

void C4ComponentHost::Clear()
	{
	if (Data) delete [] Data; Data=NULL;
#ifdef _WIN32
	if (hDialog) DestroyWindow(hDialog);
#endif
	hDialog=NULL;
	}

BOOL C4ComponentHost::Load(const char *szName,
													 C4Group &hGroup, const char *szFilename,
													 const char *szLanguage)
	{
	Clear();

	// Store name & filename
	SCopy(szName,Name);
	SCopy(szFilename,Filename);

	// Load component (segmented filename)
	char str1[_MAX_FNAME+1],str2[_MAX_FNAME+1];
	for (int cseg=0; SCopySegment(Filename,cseg,str1,'|'); cseg++)
		{
		sprintf(str2,str1,szLanguage);
		if (hGroup.LoadEntry(str2,(BYTE**)&Data,&Size,1))
			{
			hGroup.FindEntry(str2,Filename);
			return TRUE;
			}
		}
	SReplaceChar(Filename,'|',0);

	return FALSE;
	}

BOOL C4ComponentHost::Save(C4Group &hGroup)
	{
	if (!Modified) return TRUE;
	if (!Data) return hGroup.Delete(Filename);
	return hGroup.Add(Filename,Data,Size);
	}

void C4ComponentHost::Open()
	{
	
	pCmpHost=this;

#if defined(C4ENGINE) && defined(_WIN32)

	DialogBox(Application.hInstance,
						MAKEINTRESOURCE(IDD_COMPONENT),
						Application.hWindow,
						(DLGPROC) ComponentDlgProc);

#endif

	pCmpHost=NULL;

	}

BOOL C4ComponentHost::GetLanguageString(const char *szLanguage, char *sTarget, int iMaxLen)
	{
	char langindex[4];
	const char *cptr; 
	// No good parameters
	if (!szLanguage || !sTarget || !Data) return FALSE;
	// Search for two-letter language identifier in text body, i.e. "DE:"
	SCopy(szLanguage,langindex,2); SAppend(":",langindex);	
	if (cptr = SSearch(Data,langindex))
		{
		// Return the according string
		SCopyUntil(cptr,sTarget,0x0D,iMaxLen);
		return TRUE;
		}
	// Language string not found
	return FALSE;
	}

void C4ComponentHost::Close()
	{
	if (!hDialog) return;
#ifdef _WIN32
	EndDialog(hDialog,1);
#endif
	hDialog=NULL;
	}

void C4ComponentHost::InitDialog(HWND hDlg)
	{
#if defined(C4ENGINE) && defined(_WIN32)
	hDialog=hDlg;
	// Set text
	SetWindowText(hDialog,Name);
	SetDlgItemText(hDialog,IDOK,LoadResStr(IDS_BTN_OK));
	SetDlgItemText(hDialog,IDCANCEL,LoadResStr(IDS_BTN_CANCEL));
	if (Data)	SetDlgItemText(hDialog,IDC_EDITDATA,Data);
#endif
	}

const char *C4ComponentHost::GetData()
	{
	return Data;
	}

BOOL C4ComponentHost::SetLanguageString(const char *szLanguage, const char *szString)
	{
	// Safety
	if (!szLanguage || !szString) return FALSE;
	// Allocate temp buffer
	char *cpBuffer = new char [Size+SLen(szLanguage)+1+SLen(szString)+2+1];
	cpBuffer[0]=0;
	// Copy all lines except language line
	const char *cpPos=Data;
	while (cpPos && *cpPos)
		{
		if (!SEqual2(cpPos,szLanguage))
			{
			SCopyUntil( cpPos, cpBuffer+SLen(cpBuffer), 0x0A );
			if (cpBuffer[0]) if (cpBuffer[SLen(cpBuffer)-1]!=0x0D) SAppendChar(0x0D,cpBuffer);
			SAppendChar(0x0A,cpBuffer);
			}
		cpPos = SAdvancePast(cpPos,0x0A);
		}
	// Append new language line
	SAppend(szLanguage,cpBuffer);
	SAppendChar(':',cpBuffer);
	SAppend(szString,cpBuffer);
	// Set new data, delete temp buffer
	delete [] Data; Data=NULL;
	Data = new char [SLen(cpBuffer)+1];
	SCopy(cpBuffer,Data);
	Size=SLen(Data);
	delete [] cpBuffer;
	// Success
	Modified=TRUE;
	return TRUE;
	}
