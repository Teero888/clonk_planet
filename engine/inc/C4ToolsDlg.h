/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Drawing tools dialog for landscape editing in console mode */

const int C4TLS_Brush  = 0,
					C4TLS_Line	 = 1,
					C4TLS_Rect	 = 2,
					C4TLS_Fill	 = 3,
					C4TLS_Picker = 4;

const int C4TLS_GradeMax		 = 50,
					C4TLS_GradeMin		 = 1,
					C4TLS_GradeDefault = 5;

#define C4TLS_MatSky "Sky"

class C4ToolsDlg  
	{
	friend BOOL CALLBACK ToolsDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
	public:
		C4ToolsDlg();
		~C4ToolsDlg();
	public:
		HWND hDialog;
		int Tool;
		int Grade;
		BOOL ModeIFT;
		char Material[C4M_MaxName+1];
		char Texture[C4M_MaxName+1];
	protected:
		HBITMAP hbmBrush,hbmBrush2;
		HBITMAP hbmLine,hbmLine2;
		HBITMAP	hbmRect,hbmRect2;
		HBITMAP hbmFill,hbmFill2;
		HBITMAP hbmPicker,hbmPicker2;
		HBITMAP	hbmIFT;
		HBITMAP hbmNoIFT;
		HBITMAP hbmDynamic;
		HBITMAP hbmStatic;
		HBITMAP hbmExact;
	public:
		void Default();
		void Clear();
		void PopTextures();
		void PopMaterial();
		void ChangeGrade(int iChange);
		void UpdatePreview();
		BOOL Open();
		BOOL SetGrade(int iGrade);
		BOOL SetTool(int iTool);
		BOOL SetLandscapeMode(int iMode);
		BOOL SetIFT(BOOL fIFT);
		BOOL SelectTexture(const char *szTexture);
		BOOL SelectMaterial(const char *szMaterial);
	protected:
		void AssertValidTexture();
		void LoadBitmaps();
		void EnableControls();
		void UpdateIFTControls();
		void InitGradeCtrl();
		void InitMaterialCtrls();
		void UpdateLandscapeModeCtrls();
		void UpdateToolCtrls();
		void SetTexture(const char *szTexture);
		void SetMaterial(const char *szMaterial);
	};
