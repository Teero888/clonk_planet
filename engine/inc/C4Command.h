/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* The command stack controls an object's complex and independent behavior */

const int C4CMD_None = 0, C4CMD_Follow = 1, C4CMD_MoveTo = 2, C4CMD_Enter = 3,
          C4CMD_Exit = 4, C4CMD_Grab = 5, C4CMD_Build = 6, C4CMD_Throw = 7,
          C4CMD_Chop = 8, C4CMD_UnGrab = 9, C4CMD_Jump = 10, C4CMD_Wait = 11,
          C4CMD_Get = 12, C4CMD_Put = 13, C4CMD_Drop = 14, C4CMD_Dig = 15,
          C4CMD_Activate = 16, C4CMD_PushTo = 17, C4CMD_Construct = 18,
          C4CMD_Transfer = 19, C4CMD_Attack = 20, C4CMD_Context = 21,
          C4CMD_Buy = 22, C4CMD_Sell = 23, C4CMD_Acquire = 24,
          C4CMD_Energy = 25, C4CMD_Retry = 26, C4CMD_Home = 27, C4CMD_Call = 28;

const int C4CMD_First = C4CMD_Follow, C4CMD_Last = C4CMD_Call;

const char *CommandName(int iCommand);
WORD CommandNameID(int iCommand);
int CommandByName(const char *szCommand);

class C4Command {
public:
  C4Command();
  ~C4Command();

public:
  C4Object *cObj;
  int Command;
  int Tx, Ty;
  C4Object *Target, *Target2;
  int Data;
  int UpdateInterval;
  BOOL Evaluated, PathChecked, Finished;
  int Failures, Retries, Permit;
  char *Text;
  C4Command *Next;

public:
  void Set(int iCommand, C4Object *pObj, C4Object *pTarget, int iTx, int iTy,
           C4Object *pTarget2, int iData, int iUpdateInterval, BOOL fEvaluated,
           int iRetries, const char *szText);
  void Clear();
  void Execute();
  void ClearPointers(C4Object *pObj);
  void Default();
  void EnumeratePointers();
  void DenumeratePointers();
  BOOL Read(const char *szSource);
  BOOL Write(char *sTarget, int iIndex);

protected:
  void Call();
  void Home();
  void Retry();
  void Energy();
  void Fail();
  void Acquire();
  void Sell();
  void Buy();
  void Attack();
  void Transfer();
  void Construct();
  void Finish(BOOL fSuccess = FALSE);
  void Follow();
  void MoveTo();
  void Enter();
  void Exit();
  void Grab();
  void UnGrab();
  void Throw();
  void Chop();
  void Build();
  void Jump();
  void Wait();
  void Get();
  void Put();
  void Drop();
  void Dig();
  void Activate();
  void PushTo();
  void Context();
  int CallFailed();
  BOOL JumpControl();
  BOOL InitEvaluation();
};
