/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Sings up or retrieves game information from a master server */

class C4MasterServerClient  
	{
	friend C4Network;
	public:
		C4MasterServerClient();
		~C4MasterServerClient();
	protected:
		volatile bool Terminate;
		volatile bool Reference;
		volatile bool AdvanceReference;
		HANDLE hThread; DWORD idThread;
		HWND hWnd;
		int KeepPeriod,ReferencePeriod;
		char Address[_MAX_PATH+1];
		char LocalAddress[_MAX_PATH+1];
		char ReferenceFilename[_MAX_PATH+1];
		char Directory[_MAX_PATH+1];
	public:
		void Default();
		void Clear();
		void ForceReference();
		BOOL Init(HWND hwnd, const char *szAddress, const char *szDirectory, int iKeepPeriod, int iReferencePeriod);
	protected:
		int Run();
		BOOL Send(const char *szAction, const char *szFilename=".ip", BYTE *bpData=NULL, int iSize=0);
		void Message(WORD idMsg, const char *szMsgPar=NULL);
	private:
		static DWORD WINAPI ThreadFunction(LPVOID lpPar);
	};
