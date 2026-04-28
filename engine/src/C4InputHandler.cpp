/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles separate input streams from network clients (asynchronous mode only)
 */

#include <C4Include.h>

C4InputHandler::C4InputHandler() { Default(); }

C4InputHandler::~C4InputHandler() { Clear(); }

void C4InputHandler::Default() {
  Active = false;
  Terminate = false;
  hThread = NULL;
  idThread = 0;
  hHostThread = NULL;
  idHostThread = 0;
  hWnd = NULL;
  FirstStream = NULL;
  NewStream = NULL;
}

void C4InputHandler::Clear() {
  Active = false;

  // Close handler thread
  if (hThread) {
    // Ask thread to terminate nicely
    Terminate = true;
    DWORD dwExit = STILL_ACTIVE;
    // Wait 3 seconds for thread to close
    for (int cnt = 0; cnt < 6; cnt++) {
      if (!GetExitCodeThread(hThread, &dwExit) || (dwExit != STILL_ACTIVE))
        break;
      Sleep(500);
    }
    // Kill thread
    if (dwExit == STILL_ACTIVE)
      TerminateThread(hThread, 999);
  }
  hThread = NULL;

  // Terminate host thread
  if (hHostThread)
    TerminateThread(hHostThread, 999);
  hHostThread = NULL;

  // Close and delete all streams
  C4InputStream *pStream, *pNext;
  for (pStream = FirstStream; pStream; pStream = pNext) {
    pNext = pStream->Next;
    delete pStream;
  }
  FirstStream = NULL;
}

BOOL C4InputHandler::Init(HWND hwnd) {

  // Store data
  Active = true;
  hWnd = hwnd;

  // Launch host thread
  if (!(hHostThread = CreateThread(NULL, 0, &HostThreadFunction, this, 0, &idHostThread)))
    return FALSE;

  // Launch handling thread
  if (!(hThread = CreateThread(NULL, 0, &ThreadFunction, this, 0, &idThread)))
    return FALSE;

  // Success
  return TRUE;
}

DWORD WINAPI C4InputHandler::ThreadFunction(void *lpPar) {
  C4InputHandler *pInstance = (C4InputHandler *)lpPar;
  return pInstance->Run();
}

DWORD WINAPI C4InputHandler::HostThreadFunction(void *lpPar) {
  C4InputHandler *pInstance = (C4InputHandler *)lpPar;

  C4Stream *pListener = new C4Stream;
  C4Stream *pStrm = NULL;
  int iResult;

  // Host thread loop
  while (!pInstance->Terminate) {
    // Await client & connect
    iResult = pListener->Connect(Config.Network.LocalName, C4STRM_Listener, NULL, &pStrm, C4PORT_Input);
    if (iResult == C4STRM_Ok) {
      // Wait for handler thread to take over new stream
      while (pInstance->NewStream)
        Sleep(500);
      // Store new stream for handler
      pInstance->NewStream = pStrm;
    }
  }

  return 123;
}

void C4InputHandler::Message(WORD idMsg, const char *szMsgPar) { PostMessage(hWnd, WM_USER_LOG, idMsg, (LPARAM)szMsgPar); }

int C4InputHandler::Run() {
  C4InputStream *pStrm, *pPrev, *pNext;
  C4Packet Packet;
  C4Control *pInput = NULL;

  while (!Terminate) {

    Sleep(100);

    // Add new incoming stream
    if (NewStream) {
      pStrm = new C4InputStream;
      pStrm->Set(NewStream);
      pStrm->Next = FirstStream;
      FirstStream = pStrm;
      NewStream = NULL;
    }

    // Execute input streams
    pPrev = pNext = NULL;
    for (pStrm = FirstStream; pStrm; pStrm = pNext) {
      // Store next
      pNext = pStrm->Next;
      // Stream open: check & read
      if (pStrm->Stream->Good()) {
        if (pStrm->Stream->Readable())
          if (pStrm->Stream->ReceivePacket(C4PK_Control, Packet) == C4STRM_Ok) {
            if (!pInput)
              pInput = new C4Control;
            pInput->AddData(Packet.Data, Packet.Size);
          }
        // Store prev
        pPrev = pStrm;
      }
      // Stream no good: delete
      else {
        if (pPrev)
          pPrev->Next = pNext;
        else
          FirstStream = pNext;
        delete pStrm;
      }
    }

    // Send input to main thread
    if (pInput) {
      SendMessage(hWnd, WM_NET_INPUT, 0, (LPARAM)pInput);
      pInput = NULL;
    }
  }

  // Terminated
  return TRUE;
}

C4InputStream::C4InputStream() { Default(); }

C4InputStream::~C4InputStream() { Clear(); }

void C4InputStream::Default() {
  Stream = NULL;
  Next = NULL;
}

void C4InputStream::Clear() {
  if (Stream)
    delete Stream;
  Stream = NULL;
}

void C4InputStream::Set(C4Stream *pStream) { Stream = pStream; }