/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Holds a single text file component from a group */

class C4ComponentHost {
  friend BOOL CALLBACK ComponentDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

public:
  C4ComponentHost();
  ~C4ComponentHost();

protected:
  char *Data;
  int Size;
  BOOL Modified;
  char Name[_MAX_FNAME + 1];
  char Filename[_MAX_FNAME + 1];
  HWND hDialog;

public:
  void Default();
  void Clear();
  void Open();
  void InitDialog(HWND hDlg);
  const char *GetData();
  virtual void Close();
  BOOL Load(const char *szName, C4Group &hGroup, const char *szFilename, const char *szLanguage = NULL);
  BOOL Save(C4Group &hGroup);
  BOOL GetLanguageString(const char *szLanguage, char *sTarget, int iMaxLen);
  BOOL SetLanguageString(const char *szLanguage, const char *szString);
};
