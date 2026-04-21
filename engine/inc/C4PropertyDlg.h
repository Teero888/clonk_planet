/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Console mode dialog for object properties and script interface */

class C4PropertyDlg  
	{
	friend BOOL CALLBACK PropertyDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
	public:
		C4PropertyDlg();
		~C4PropertyDlg();
	public:
		HWND hDialog;
	public:
		void Default();
		void Clear();
		void Execute();
		void ClearPointers(C4Object *pObj);
		void UpdateInputCtrl(C4Object *pObj);
		BOOL Open();
		BOOL Update();
		BOOL Update(C4ObjectList &rSelection);
	protected:
		C4ID idSelectedDef;
		C4ObjectList Selection;
	};
