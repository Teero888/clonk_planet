/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* The C4Action class is merely a simple data structure */

#include <C4Include.h>

C4Action::C4Action() { Default(); }

C4Action::~C4Action() {}

void C4Action::Default() {
  Act = ActIdle;
  ZeroMem(Name, C4D_MaxIDLen + 1);
  Dir = DIR_None;
  ComDir = COMD_None;
  Time = 0;
  Data = 0;
  Target = Target2 = NULL;
  Phase = PhaseDelay = 0;
  Facet.Default();
  FacetX = FacetY = 0;
  t_attach = CNAT_None;
}
