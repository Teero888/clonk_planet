/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Lots of constants */

//============================= Main
//=====================================================

const int C4MaxNameList = 10, C4MaxName = 30, C4MaxComment = 100,
          C4MaxDefString = 100, C4MaxTitle = 512, C4MaxMessage = 100,

          C4MaxVariable = 10,

          C4ViewDelay = 100, C4RetireDelay = 60,

          C4MaxColor = 8, C4MaxKey = 12, C4MaxKeyboardSet = 4,

          C4MaxControlRate = 20;

const int C4M_MaxName = 15, C4M_MaxDefName = 2 * C4M_MaxName + 1,
          C4M_ColsPerMat = 3, C4M_MaxTexIndex = 128;

const int C4S_MaxPlayer = 4;

const int C4D_MaxName = C4MaxName, C4D_MaxVertex = 30,
          C4D_MaxIDLen = C4D_MaxName;

const int C4SymbolSize = 35, C4SymbolBorder = 5;

const int C4P_MaxPosition = 4;

const int C4P_Control_None = -1, C4P_Control_Keyboard1 = 0,
          C4P_Control_Keyboard2 = 1, C4P_Control_Keyboard3 = 2,
          C4P_Control_Keyboard4 = 3, C4P_Control_GamePad1 = 4;

//============================= Engine Return Values
//======================================

const int C4XRV_Completed = 0, C4XRV_Failure = 1, C4XRV_Aborted = 2;

//============================= Object Character Flags
//====================================

const DWORD OCF_None = 0, OCF_All = ~OCF_None, OCF_Normal = 1,
            OCF_Construct = 1 << 1, OCF_Grab = 1 << 2, OCF_Carryable = 1 << 3,
            OCF_OnFire = 1 << 4, OCF_HitSpeed1 = 1 << 5, OCF_FullCon = 1 << 6,
            OCF_Inflammable = 1 << 7, OCF_Chop = 1 << 8, OCF_Rotate = 1 << 9,
            OCF_Exclusive = 1 << 10, OCF_Entrance = 1 << 11,
            OCF_HitSpeed2 = 1 << 12, OCF_HitSpeed3 = 1 << 13,
            OCF_Collection = 1 << 14, OCF_Living = 1 << 15,
            OCF_HitSpeed4 = 1 << 16, OCF_FightReady = 1 << 17,
            OCF_LineConstruct = 1 << 18, OCF_Prey = 1 << 19,
            OCF_AttractLightning = 1 << 20, OCF_NotContained = 1 << 21,
            OCF_CrewMember = 1 << 22, OCF_Edible = 1 << 23,
            OCF_InLiquid = 1 << 24, OCF_InSolid = 1 << 25, OCF_InFree = 1 << 26,
            OCF_Available = 1 << 27, OCF_PowerConsumer = 1 << 28,
            OCF_PowerSupply = 1 << 29, OCF_Container = 1 << 30;

//================================== Contact / Attachment
//==============================================

const BYTE CNAT_None = 0, CNAT_Left = 1, CNAT_Right = 2, CNAT_Top = 4,
           CNAT_Bottom = 8, CNAT_Center = 16;

//=============================== Keyboard Input Controls
//=====================================================

const int C4DoubleClick = 10;

const int CON_CursorLeft = 0, CON_CursorToggle = 1, CON_CursorRight = 2,
          CON_Throw = 3, CON_Up = 4, CON_Dig = 5, CON_Left = 6, CON_Down = 7,
          CON_Right = 8, CON_Menu = 9, CON_Special = 10, CON_Special2 = 11;

//=================================== Control Commands
//======================================================

const BYTE COM_Single = 64, COM_Double = 128;

const BYTE COM_None = 0;

const BYTE COM_Left = 1, COM_Right = 2, COM_Up = 3, COM_Down = 4, COM_Throw = 5,
           COM_Dig = 6,

           COM_Special = 7, COM_Special2 = 8,

           COM_Contents = 9,

           COM_Left_S = COM_Left | COM_Single,
           COM_Right_S = COM_Right | COM_Single, COM_Up_S = COM_Up | COM_Single,
           COM_Down_S = COM_Down | COM_Single,
           COM_Throw_S = COM_Throw | COM_Single,
           COM_Dig_S = COM_Dig | COM_Single,
           COM_Special_S = COM_Special | COM_Single,
           COM_Special2_S = COM_Special2 | COM_Single,

           COM_Left_D = COM_Left | COM_Double,
           COM_Right_D = COM_Right | COM_Double, COM_Up_D = COM_Up | COM_Double,
           COM_Down_D = COM_Down | COM_Double,
           COM_Throw_D = COM_Throw | COM_Double,
           COM_Dig_D = COM_Dig | COM_Double,
           COM_Special_D = COM_Special | COM_Double,
           COM_Special2_D = COM_Special2 | COM_Double;

const BYTE COM_CursorLeft = 30, COM_CursorRight = 31, COM_CursorToggle = 32;

const BYTE COM_CursorToggle_D = COM_CursorToggle | COM_Double;

const BYTE COM_Help = 35, COM_PlayerMenu = 36;

const BYTE COM_MenuEnter = 37, COM_MenuEnterAll = 38, COM_MenuClose = 39,

           COM_MenuMove = 40, COM_MenuLeft = 52, COM_MenuRight = 53,
           COM_MenuUp = 54, COM_MenuDown = 55, COM_MenuScrollLeft = 56,
           COM_MenuScrollRight = 57, COM_MenuScrollUp = 58,
           COM_MenuScrollDown = 59, COM_MenuSelect = 60, COM_MenuCloseDown = 61,
           COM_MenuDrag = 62, COM_MenuDragScroll = 63,

           COM_MenuNavigation1 = COM_MenuMove,
           COM_MenuNavigation2 = COM_MenuDragScroll;

//=================================== Owners
//==============================================

const int NO_OWNER = -1, ANY_OWNER = -2, BY_OWNER = 10000,
          BY_HOSTILE_OWNER = 20000;

//=================================== Versions
//=========================================

const int C4PVer1 = 0, C4PVer2 = 9;
const int C4OVer1 = 0, C4OVer2 = 7;
