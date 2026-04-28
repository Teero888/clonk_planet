/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles separate input streams from network clients (asynchronous mode only)
 */

class C4InputHandler;

class C4InputStream {
  friend C4InputHandler;

public:
  C4InputStream();
  ~C4InputStream();

protected:
  C4Stream *Stream;
  C4InputStream *Next;

public:
  void Default();
  void Clear();
  void Set(C4Stream *pStream);
};

class C4InputHandler {
public:
  C4InputHandler();
  ~C4InputHandler();

protected:
  volatile bool Terminate;
  bool Active;
  HWND hWnd;
  HANDLE hThread;
  DWORD idThread;
  HANDLE hHostThread;
  DWORD idHostThread;
  C4InputStream *FirstStream;
  C4Stream *NewStream;

public:
  BOOL Init(HWND hwnd);
  void Clear();
  void Default();

protected:
  int Run();
  void Message(WORD idMsg, const char *szMsgPar = NULL);

private:
  static DWORD WINAPI ThreadFunction(LPVOID lpPar);
  static DWORD WINAPI HostThreadFunction(LPVOID lpPar);
};
