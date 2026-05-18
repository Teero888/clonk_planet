/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Log file handling */

#include <C4Include.h>

const int C4LogSize = 30000;
const int C4LogMsgLen = 500;
char *C4LogBuf = NULL;
time_t C4LogStartTime = 0;

#include <thread>
extern std::thread::id g_mainThreadId;

void ClearC4Log() {
  if (C4LogBuf)
    delete[] C4LogBuf;
  C4LogBuf = NULL;
}

BOOL Log(const char *szMessage) {
  if (!szMessage)
    return FALSE;

  // Pass on to console
  Console.Out(szMessage);

  char szMessageBuf[C4LogMsgLen + 1];
  // Set start time
  if (!C4LogStartTime)
    time(&C4LogStartTime);
  // Allocate buffer if necessary
  if (!C4LogBuf) {
    if (!(C4LogBuf = new char[C4LogSize]))
      return FALSE;
    C4LogBuf[0] = 0;
  }
  // Add message
  SCopy(szMessage, szMessageBuf, C4LogMsgLen);
  if (SLen(C4LogBuf) + SLen(szMessageBuf) + 2 < C4LogSize) {
    SAppend(szMessageBuf, C4LogBuf);
    SAppendChar('|', C4LogBuf);
  }

  // Immediate save
  FILE *fhnd;
  if (fhnd = fopen(C4CFN_Log, "a+")) {
    fputs(szMessageBuf, fhnd);
    fputs("\n", fhnd);
    fflush(fhnd);
    fclose(fhnd);
  }

  // Notify startup message board (main thread only!)
  if (std::this_thread::get_id() == g_mainThreadId)
    Game.GraphicsSystem.MessageBoard.LogNotify();

  return TRUE;
}

void SaveC4Log(const char *szFileName) {
  char buf[C4LogMsgLen + 1];
  FILE *fhnd;
  if (!(fhnd = fopen(szFileName, "w")))
    return;
  for (int cnt = 0; SCopySegment(C4LogBuf, cnt, buf, '|'); cnt++)
    if ((fputs(buf, fhnd) < 0) || (fputs("\n", fhnd) < 0)) {
      fclose(fhnd);
      return;
    }
  fclose(fhnd);
}

void CloseLog(const char *szFileName) { ClearC4Log(); }
// TODO: make this debug mode only again
BOOL NetLog(const char *szMessage) {
  if (!szMessage)
    return FALSE;
  char buf[C4LogMsgLen + 10 + 1];
  sprintf(buf, " Net: %s", szMessage);
  printf("%s\n", buf);
  return Log(buf);
}

void LogProcess(int iValue) { Game.GraphicsSystem.MessageBoard.NotifyProcess(iValue); }

BOOL LogProcessCallback(const char *szFormat, int iValue) {
  Game.GraphicsSystem.MessageBoard.NotifyProcess(iValue);
  return TRUE;
}
