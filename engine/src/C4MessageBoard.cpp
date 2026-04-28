/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Fullscreen startup log and chat type-in */

#include <C4Include.h>

C4MessageBoard::C4MessageBoard() { Default(); }

C4MessageBoard::~C4MessageBoard() { Clear(); }

void C4MessageBoard::Default() {
  Message[0] = 0;
  TypeInBuf[0] = 0;
  for (int cnt = 0; cnt < C4MSGB_BackBufferMax; cnt++)
    BackBuffer[cnt][0] = 0;
  Active = FALSE;
  pCursor = NULL;
  pLog = NULL;
  Delay = 0;
  Fader = 0;
  TypeIn = FALSE;
  DelayFactor = 2;
  TypeInCommand = FALSE;
  Output.Default();
  Startup = FALSE;
  Process = 0;
}

void C4MessageBoard::Clear() { Active = FALSE; }

void C4MessageBoard::Execute() {
  if (!Active || !pCursor)
    return;

  // Advance fader (if not startup or typein)
  int iFadeRange = Engine.DDraw.TextHeight();
  if (!Startup && !TypeIn) {
    if (Fader > 0)
      Fader = Max(Fader - 2, 0);
    if (Fader < 0)
      Fader = Max(Fader - 2, -iFadeRange);
    if (Fader == 0) {
      if (Delay > 0)
        Delay--;
      if (Delay == 0)
        Fader = -1;
    }
    // Current process halts message
    if (Process)
      Fader = -1;
  }

  // Fade next message
  if (Fader == -iFadeRange)
    if (pCursor[0]) {
      SCopyUntil(pCursor, Message, '|', C4MaxMessage);
      Delay = SLen(Message) * DelayFactor;
      Fader = iFadeRange;
      pCursor = SAdvancePast(pCursor, '|');
    } else
      Message[0] = 0;

  // Draw
  Draw(Output);
}

void C4MessageBoard::Init(const char *szLog, C4Facet &cgo, BOOL fStartup) {
  Active = TRUE;
  pLog = pCursor = szLog;
  Output = cgo;
  Startup = fStartup;
}

void C4MessageBoard::Draw(C4Facet &cgo) {
  if (!Active)
    return;

  // Clipper
  Engine.DDraw.SetPrimaryClipper(cgo.X, cgo.Y, cgo.X + cgo.Wdt - 1, cgo.Y + cgo.Hgt - 1);

  // Startup: full log transparent
  if (Startup) {
    // Background (loader)
    Game.GraphicsSystem.fctLoader.Draw(cgo, FALSE);
    // Scrolling text
    int iLinesVisible = cgo.Hgt / Engine.DDraw.TextHeight();
    int iFirstLine = Max(SCharCount('|', pLog) - iLinesVisible, 0);
    int iFirstChar = 0;
    if (iFirstLine)
      iFirstChar = SCharPos('|', pLog, iFirstLine) + 1;
    char *cpTerminate = NULL;
    // Append process text
    if (Process) {
      cpTerminate = (char *)(pLog + iFirstChar + SLen(pLog + iFirstChar));
      if (pLog[0])
        if (cpTerminate[-1] == '|')
          cpTerminate--;
      sprintf(cpTerminate, " %i%%", Process);
    }
    // Draw text
    Engine.DDraw.TextOut(pLog + iFirstChar, cgo.Surface, cgo.X, cgo.Y);
    // Truncate process text
    if (cpTerminate) {
      cpTerminate[0] = '|';
      cpTerminate[1] = 0;
    }
    // Startup draw enforces page flip
    Engine.DDraw.PageFlip();
  }

  // Game running: message fader
  else {
    int iColor = FWhite;
    // Background
    Engine.DDraw.BlitSurfaceTile(Game.GraphicsResource.fctBackground.Surface, cgo.Surface, cgo.X, cgo.Y, cgo.Wdt, cgo.Hgt);
    // TypeIn
    if (TypeIn) {
      // Type-in text
      sprintf(OSTR, ">%s_", TypeInBuf);
      // Command color
      if (TypeInCommand)
        iColor = FRed;
      // Draw
      Engine.DDraw.StringOut(OSTR, cgo.Surface, cgo.X, cgo.Y, iColor);
    }
    // Message
    else {
      // Player message color
      C4Player *pPlr = GetMessagePlayer(Message);
      if (pPlr)
        iColor = FPlayer + pPlr->Color;
      // Draw
      if (/*(Fader==0) &&*/ Process) {
        sprintf(OSTR, "%s %i%%", Message, Process);
        Engine.DDraw.StringOut(OSTR, cgo.Surface, cgo.X, cgo.Y + Fader, iColor);
      } else
        Engine.DDraw.StringOut(Message, cgo.Surface, cgo.X, cgo.Y + Fader, iColor);
    }
  }

  // No clipper
  Engine.DDraw.NoPrimaryClipper();
}

BOOL C4MessageBoard::CharIn(char cChar) {
  if (!Active)
    return FALSE;
  if (!TypeIn)
    return FALSE;

  switch (cChar) {
  case '\b': // Backspace
    if (TypeInBuf[0])
      TypeInBuf[SLen(TypeInBuf) - 1] = 0;
    break;
  case '\r': // Enter
    if (TypeInBuf[0]) {
      // Script command
      if (TypeInCommand) {
        // Create valid single statement
        int iResult;
        char buffer[1000];
        sprintf(buffer, "return(%s);", TypeInBuf);
        iResult = Game.CommandLine(buffer);
        // Result output
        sprintf(OSTR, "%s = %i", TypeInBuf, iResult);
        Log(OSTR);
      }
      // Network command
      else if (Game.Network.Command(TypeInBuf)) {

      }
      // Player message
      else
        Out(TypeInBuf);
    }
    // Store to back buffer
    StoreBackBuffer(TypeInBuf);
    // Close type in
    CloseTypeIn();
    break;
  case '\t':
  case '\n':
  case '|': // Ignore newlines, pipes
    break;
  default: // Enter character
    if (SLen(TypeInBuf) < C4MaxMessage - 1) {
      TypeInBuf[SLen(TypeInBuf) + 1] = 0;
      TypeInBuf[SLen(TypeInBuf)] = cChar;
    }
    break;
  }

  return TRUE;
}

BOOL C4MessageBoard::StartTypeIn(BOOL fCommand) {
  if (!Active)
    return FALSE;
  if (TypeIn)
    return FALSE;
  TypeIn = TRUE;
  TypeInCommand = fCommand;
  TypeInBuf[0] = 0;
  BackIndex = -1;
  return TRUE;
}

BOOL C4MessageBoard::CloseTypeIn() {
  if (!Active)
    return FALSE;
  if (!TypeIn)
    return FALSE;
  TypeIn = FALSE;
  return TRUE;
}

void C4MessageBoard::EnsureLastMessage() {
  // Ingore if startup or typein
  if (!Active || Startup || TypeIn)
    return;
  DelayFactor = 0;
  Process = 0;
  while (pCursor[0] || (Fader > 0))
    Game.GraphicsSystem.Execute();
  DelayFactor = 2;
  Delay = SLen(Message) * DelayFactor;
}

BOOL C4MessageBoard::Out(const char *szText) {
  if (!Active)
    return FALSE;
  char szBuffer[C4MaxMessage + 1 + 50];
  C4Player *pPlr = Game.Players.GetLocalByIndex(0);
  // Starts with /me?
  if (SEqual2(szText, "/me"))
    sprintf(szBuffer, "* %s %s", pPlr ? pPlr->Name : Game.Network.LocalName, szText + 4);
  else
    sprintf(szBuffer, "%s: %s", pPlr ? pPlr->Name : Game.Network.LocalName, szText);
  // Output to control queue
  Game.Input.AddMessage(szBuffer);
  return TRUE;
}

void C4MessageBoard::LogNotify() {
  // Not active
  if (!Active)
    return;
  // Reset process
  Process = 0;
  // Draw
  Draw(Output);
}

C4Player *C4MessageBoard::GetMessagePlayer(const char *szMessage) {
  // Scan message text for heading player name
  if (SCharCount(':', szMessage)) {
    SCopyUntil(szMessage, OSTR, ':');
    return Game.Players.GetByName(OSTR);
  }
  return NULL;
}

void C4MessageBoard::StoreBackBuffer(const char *szMessage) {
  if (!szMessage || !szMessage[0])
    return;
  int cnt;
  // Move up buffers
  for (cnt = C4MSGB_BackBufferMax - 1; cnt > 0; cnt--)
    SCopy(BackBuffer[cnt - 1], BackBuffer[cnt]);
  // Add message
  SCopy(szMessage, BackBuffer[0]);
}

BOOL C4MessageBoard::Control(WORD vk_code) {
  if (!Active)
    return FALSE;

  // Type in control
  if (TypeIn)
    switch (vk_code) {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - -
    case VK_UP:
      if (BackIndex < C4MSGB_BackBufferMax - 1)
        if (BackBuffer[BackIndex + 1][0]) {
          BackIndex++;
          SCopy(BackBuffer[BackIndex], TypeInBuf);
        }
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - -
    case VK_DOWN:
      if (BackIndex > -1) {
        BackIndex--;
        if (BackIndex >= 0)
          SCopy(BackBuffer[BackIndex], TypeInBuf);
        else
          TypeInBuf[0] = 0;
      }
      return TRUE;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - - - - - - - - - - -
    }

  // Back log control
  else
    switch (vk_code) {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - -
    case VK_SCROLL:
      if (Message[0]) {
        if (pCursor > pLog)
          pCursor--;
        while ((pCursor > pLog) && (pCursor[-1] != '|'))
          pCursor--;
      }
      if (pCursor > pLog)
        pCursor--;
      while ((pCursor > pLog) && (pCursor[-1] != '|'))
        pCursor--;
      SCopyUntil(pCursor, Message, '|', C4MaxMessage);
      Delay = SLen(Message) * DelayFactor;
      Fader = 0;
      pCursor = SAdvancePast(pCursor, '|');
      return TRUE;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - - - - - - - - - - -
    }

  return FALSE;
}

void C4MessageBoard::NotifyProcess(int iValue) {
  // Not active
  if (!Active)
    return;
  // Set process
  Process = iValue;
  // Draw
  static int iDelay = 0;
  if (Game.GraphicsSystem.GraphicsGo) {
    iDelay++;
    Game.GraphicsSystem.GraphicsGo = FALSE;
  }
  if (iDelay > 10) {
    iDelay = 0;
    Game.GraphicsSystem.Execute();
  }
}
