/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to initialize configuration and execute the game */

#include <C4Include.h>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

C4Application::C4Application() {
  hInstance = NULL;
  hWindow = NULL;
  Active = FALSE;
  Fullscreen = FALSE;
}

C4Application::~C4Application() {
  Config.General.Definitions[0] = 0;
  Config.Save();
  CloseLog(Config.AtLogPath(C4CFN_Log));
}

extern std::thread::id g_mainThreadId;

BOOL C4Application::Init(HINSTANCE hInst, int nCmdShow, char *szCmdLine) {
  printf("Application::Init: %s\n", szCmdLine);
  g_mainThreadId = std::this_thread::get_id();

  // Set instance
  hInstance = hInst;

  // Config check
  Config.Init();
  Config.Load();

  // Init C4Group
  C4Group_SetMaker(Config.General.Name);
  C4Group_SetProcessCallback(&ProcessCallback);
  C4Group_SetTempPath(Config.General.TempPath);

  // Set resource for strings (and extended flag for second language)
  SetStringResource(hInstance, SEqualNoCase(Config.General.Language, "US"));

#ifdef C4SHAREWARE
  // Set external unscramble function for scrambled resource strings
  void UnscrambleString(char *szString);
  SetResourceStringUnscramble(UnscrambleString);
#endif

  // Set unregistered user name
  if (!Config.Registered())
    C4Group_SetMaker(LoadResStr(IDS_PRC_UNREGUSER));

  // Fullscreen mode switch (ignored, we force windowed-fullscreen on Linux)
#ifndef _WIN32
  Fullscreen = TRUE;
#else
  Fullscreen = FALSE;
#endif

  // Init carrier window
  printf("Console.Init...\n");
  if (!(hWindow = Console.Init(hInstance))) {
    Clear();
    return FALSE;
  }
#ifndef _WIN32
  FullScreen.Init(hInstance);
#endif
  // Init engine
  printf("Engine.Init...\n");
  if (!Engine.Init(hInstance, hWindow, Fullscreen)) {
    Clear();
    return FALSE;
  }
  // Execute command line
  printf("Console.CommandLine...\n");
  if (!Console.CommandLine(szCmdLine))
    return FALSE;

  return TRUE;
}

void C4Application::Run() {
  // Main message loop
  while (TRUE)
    switch (HandleMessage()) {
    case 0:
      Execute();
      break;
    case 1:
      break;
    case 2:
      return;
    }
}

void C4Application::Clear() { Engine.Clear(); }

void C4Application::Execute() {
#ifndef _WIN32
  // Drive engine flags on Linux
  static uint32_t lastTick = 0;
  uint32_t now = glfwGetTime() * 1000;
  if (now - lastTick >= 28) // ~35 FPS, matching CRITICAL_MSEC
  {
    Game.GameGo = TRUE;
    Game.GraphicsSystem.GraphicsGo = TRUE;
    lastTick = now;
  }
#endif

  // Fullscreen mode
  if (Fullscreen)
    FullScreen.Execute();
  // Console mode
  else {
    Console.Execute();
    // In windowed mode on Linux, we still want the game to run and render to
    // our window
    FullScreen.Execute();
  }
}

void C4Application::Quit() { PostQuitMessage(0); }

BOOL C4Application::ProcessCallback(const char *szMessage, int iProcess) {
  Console.Out(szMessage);
  return TRUE;
}

// Forward declare internal window handle from StdDDraw.cpp
extern "C" void *GetGLFWWindow();

#ifndef _WIN32
static int GLFWKeyToVK(int key) {
  switch (key) {
  case GLFW_KEY_ENTER:
    return VK_RETURN;
  case GLFW_KEY_ESCAPE:
    return VK_ESCAPE;
  case GLFW_KEY_BACKSPACE:
    return VK_BACK;
  case GLFW_KEY_TAB:
    return VK_TAB;
  case GLFW_KEY_SPACE:
    return VK_SPACE;
  case GLFW_KEY_UP:
    return VK_UP;
  case GLFW_KEY_DOWN:
    return VK_DOWN;
  case GLFW_KEY_LEFT:
    return VK_LEFT;
  case GLFW_KEY_RIGHT:
    return VK_RIGHT;
  case GLFW_KEY_INSERT:
    return VK_INSERT;
  case GLFW_KEY_DELETE:
    return VK_DELETE;
  case GLFW_KEY_PAGE_UP:
    return VK_PRIOR;
  case GLFW_KEY_PAGE_DOWN:
    return VK_NEXT;
  case GLFW_KEY_HOME:
    return VK_HOME;
  case GLFW_KEY_END:
    return VK_END;
  case GLFW_KEY_F1:
    return VK_F1;
  case GLFW_KEY_F2:
    return VK_F2;
  case GLFW_KEY_F3:
    return VK_F3;
  case GLFW_KEY_F4:
    return VK_F4;
  case GLFW_KEY_F5:
    return VK_F5;
  case GLFW_KEY_F6:
    return VK_F6;
  case GLFW_KEY_F7:
    return VK_F7;
  case GLFW_KEY_F8:
    return VK_F8;
  case GLFW_KEY_F9:
    return VK_F9;
  case GLFW_KEY_F10:
    return VK_F10;
  case GLFW_KEY_F11:
    return VK_F11;
  case GLFW_KEY_F12:
    return VK_F12;
  case GLFW_KEY_KP_0:
    return VK_NUMPAD0;
  case GLFW_KEY_KP_1:
    return VK_NUMPAD1;
  case GLFW_KEY_KP_2:
    return VK_NUMPAD2;
  case GLFW_KEY_KP_3:
    return VK_NUMPAD3;
  case GLFW_KEY_KP_4:
    return VK_NUMPAD4;
  case GLFW_KEY_KP_5:
    return VK_NUMPAD5;
  case GLFW_KEY_KP_6:
    return VK_NUMPAD6;
  case GLFW_KEY_KP_7:
    return VK_NUMPAD7;
  case GLFW_KEY_KP_8:
    return VK_NUMPAD8;
  case GLFW_KEY_KP_9:
    return VK_NUMPAD9;
  case GLFW_KEY_KP_ADD:
    return VK_ADD;
  case GLFW_KEY_KP_SUBTRACT:
    return VK_SUBTRACT;
  case GLFW_KEY_KP_MULTIPLY:
    return VK_MULTIPLY;
  case GLFW_KEY_KP_DECIMAL:
    return VK_DECIMAL;
  default:
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
      return key;
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
      return key;
    return 0;
  }
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode,
                              int action, int mods) {
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    int vk = GLFWKeyToVK(key);
    if (vk)
      Game.KeyboardInput(vk, (mods & GLFW_MOD_ALT) != 0);
  }
}

static void glfw_mouse_button_callback(GLFWwindow *window, int button,
                                       int action, int mods) {
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  WORD flags = 0;
  if (mods & GLFW_MOD_CONTROL)
    flags |= MK_CONTROL;
  if (mods & GLFW_MOD_SHIFT)
    flags |= MK_SHIFT;

  int c4button = C4MC_Button_None;
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS)
      c4button = C4MC_Button_LeftDown;
    else if (action == GLFW_RELEASE)
      c4button = C4MC_Button_LeftUp;
  } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    if (action == GLFW_PRESS)
      c4button = C4MC_Button_RightDown;
    else if (action == GLFW_RELEASE)
      c4button = C4MC_Button_RightUp;
  }

  if (c4button != C4MC_Button_None) {
    Game.MouseControl.Move(c4button, (int)xpos, (int)ypos, flags);
  }
}

static void glfw_cursor_pos_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  Game.MouseControl.Move(C4MC_Button_None, (int)xpos, (int)ypos, 0);
}
#endif

int C4Application::HandleMessage() {
#ifndef _WIN32
  GLFWwindow *window = (GLFWwindow *)GetGLFWWindow();
  if (window) {
    if (glfwWindowShouldClose(window))
      return 2;
    static bool callbacksSet = false;
    if (!callbacksSet) {
      glfwSetKeyCallback(window, glfw_key_callback);
      glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
      glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);
      callbacksSet = true;
    }
  }
  glfwPollEvents();
  return 0; // Drive Execute()
#else
  MSG msg;
  BOOL MsgDone;
  // Peek message
  if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    return 0;
  // Handle message
  if (!GetMessage(&msg, NULL, 0, 0))
    return 2;
  // Dialog message transfer
  MsgDone = FALSE;
  if (!MsgDone)
    if (Console.hDialog && IsDialogMessage(Console.hDialog, &msg))
      MsgDone = TRUE;
  if (!MsgDone)
    if (Console.PropertyDlg.hDialog &&
        IsDialogMessage(Console.PropertyDlg.hDialog, &msg))
      MsgDone = TRUE;
  if (!MsgDone) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  // Done
  return 1;
#endif
}
