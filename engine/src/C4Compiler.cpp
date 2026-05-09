/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Scan ini-type text component files to run-time data structures */

#include <C4Include.h>

const int MaxRefBuf = 100, MaxLineBuf = 4096, MaxCompileBuf = 20000;

BOOL C4Compiler::CompileLine(C4CompilerValue *pRefs, const char *szSection, const char *szLine, BYTE *pTarget, const char *szSecExt) {
  C4CompilerValue *cRef;
  char szRefName[MaxRefBuf + 1];
  char szSegment[MaxRefBuf + 1];
  int cnt;

  if (!pRefs || !szSection || !szLine || !pTarget)
    return FALSE;

  // No good value line
  if (!SCharCount('=', szLine))
    return FALSE;
  // Get ref name
  SCopyUntil(szLine, szRefName, '=', MaxRefBuf);
  szLine = SAdvancePast(szLine, '=');

  // Find reference in correct section
  char csec[MaxRefBuf + 1];
  csec[0] = 0;
  for (cRef = pRefs; cRef->Type != C4CV_End; cRef++) {
    // New section
    if (cRef->Type == C4CV_Section) {
      sprintf(csec, cRef->Name, szSecExt);
    }
    // Value
    else {
      // Match section, match value
      if (SEqual(szSection, csec))
        if (SEqual(szRefName, cRef->Name))
          break;
    }
  }
  if (!cRef)
    return FALSE; // Reference not found

  // Convert and set referenced value
  pTarget += cRef->Offset;
  switch (cRef->Type) {

  case C4CV_String:
    SCopy(szLine, (char *)pTarget, cRef->Size);
    break;

  case C4CV_Id:
    for (cnt = 0; (cnt < cRef->Size) && SCopySegment(szLine, cnt, szSegment, ',', C4MaxName); cnt++) {
      C4ID id = (SEqual(szSegment, "NONE") || (SLen(szSegment) != 4)) ? C4ID_None : C4Id(szSegment);
      memcpy(pTarget + cnt * sizeof(C4ID), &id, sizeof(C4ID));
    }
    break;

  case C4CV_Integer:
    for (cnt = 0; (cnt < cRef->Size) && SCopySegment(szLine, cnt, szSegment, ',', C4MaxName); cnt++) {
      int val;
      if (sscanf(szSegment, "%i", &val) == 1)
        memcpy(pTarget + cnt * sizeof(int), &val, sizeof(int));
    }
    break;

  case C4CV_Variable:
    for (cnt = 0; (cnt < cRef->Size) && SCopySegment(szLine, cnt, szSegment, ',', C4MaxName); cnt++) {
      long val;
      if (sscanf(szSegment, "%li", &val) == 1)
        memcpy(pTarget + cnt * sizeof(long), &val, sizeof(long));
    }
    break;

  case C4CV_NameList:
    ((C4NameList *)pTarget)->Read(szLine, 0);
    break;

  case C4CV_IdList:
    ((C4IDList *)pTarget)->Read(szLine, 0);
    break;

#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // - - - - - - - -

  case C4CV_ObjectList:
    // Read string of enumerated pointers for post-compile
    ((C4ObjectList *)pTarget)->ReadEnumerated(szLine);
    break;

#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - - - - - - - - -
  }

  return TRUE;
}

BOOL C4Compiler::CompileStructure(C4CompilerValue *pRefs, const char *szSource, void *vpTarget, const char *szSecExt) {
  char section[MaxRefBuf + 1];
  char valueline[MaxLineBuf + 1];
  const char *cpos;

  if (!pRefs || !szSource || !vpTarget)
    return FALSE;

  BYTE *pTarget = (BYTE *)vpTarget;

  section[0] = 0;
  cpos = szSource;

  // Scan source text
  while (*cpos) {

    // Section
    if (*cpos == '[') {
      // Set current section name
      SCopyUntil(cpos + 1, section, ']', MaxRefBuf);
      // Advance to next line
      cpos = SAdvancePast(cpos, 0x0A);
    }

    // Value
    else if (IsIdentifier(*cpos)) {
      // Get value line
      SCopyUntil(cpos, valueline, 0x0D, MaxLineBuf);
      // Compile line
      CompileLine(pRefs, section, valueline, pTarget, szSecExt);
      // Advance to next line
      cpos = SAdvancePast(cpos, 0x0A);
    }

    // No good line
    else {
      // Advance to next line
      cpos = SAdvancePast(cpos, 0x0A);
    }
  }

  return TRUE;
}

BOOL C4Compiler::DecompileStructure(C4CompilerValue *pRefs, void *vpData, void *vpDefault, char **ppOutput, int *ipSize, const char *szSecExt) {
  char ostr[MaxLineBuf + 1], ostr2[MaxLineBuf + 1];
  char secname[MaxRefBuf + 1] = "[Unknown]";
  BOOL secput = FALSE;
  int cnt;
  BOOL fDiff;
  int iDiff;
  BOOL fOwnBuffer = FALSE;

  if (!pRefs || !vpData || !vpDefault)
    return FALSE;

  BYTE *pData = (BYTE *)vpData;
  BYTE *pDefault = (BYTE *)vpDefault;

  // No buffer provided: create output buffer (static max size)
  if (!(*ppOutput)) {
    *ppOutput = new char[MaxCompileBuf];
    *ppOutput[0] = 0;
    fOwnBuffer = TRUE;
  }

  // Create output text by processing all variable refs
  for (C4CompilerValue *cRef = pRefs; cRef->Type != C4CV_End; cRef++)
    switch (cRef->Type) {

    case C4CV_Section:
      // Prepare section name
      secname[0] = 0;
      if (*ppOutput[0])
        SAppend(LineFeed, secname);
      sprintf(ostr, "[%s]", cRef->Name);
      sprintf(ostr2, ostr, szSecExt);
      SAppend(ostr2, secname);
      SAppend(LineFeed, secname);
      secput = FALSE;
      break;

    case C4CV_String:
      // Compare to default
      if (SEqual((char *)(pData + cRef->Offset), (char *)(pDefault + cRef->Offset)))
        break;
      // Section name check
      if (!secput) {
        SAppend(secname, *ppOutput);
        secput = TRUE;
      }
      // Append to output
      SAppend(cRef->Name, *ppOutput);
      SAppend("=", *ppOutput);
      SAppend((char *)(pData + cRef->Offset), *ppOutput);
      SAppend(LineFeed, *ppOutput);
      break;

    case C4CV_Id:
      // Compare to default
      fDiff = FALSE;
      for (cnt = 0; cnt < cRef->Size; cnt++) {
        C4ID idData, idDefault;
        memcpy(&idData, pData + cRef->Offset + cnt * sizeof(C4ID), sizeof(C4ID));
        memcpy(&idDefault, pDefault + cRef->Offset + cnt * sizeof(C4ID), sizeof(C4ID));
        if (idData != idDefault)
          fDiff = TRUE;
      }
      if (!fDiff)
        break;
      // Section name check
      if (!secput) {
        SAppend(secname, *ppOutput);
        secput = TRUE;
      }
      // Append to output
      SAppend(cRef->Name, *ppOutput);
      SAppend("=", *ppOutput);
      for (cnt = 0; cnt < cRef->Size; cnt++) {
        if (cnt)
          SAppend(",", *ppOutput);
        C4ID idData;
        memcpy(&idData, pData + cRef->Offset + cnt * sizeof(C4ID), sizeof(C4ID));
        GetC4IdText(idData, ostr);
        SAppend(ostr, *ppOutput);
      }
      SAppend(LineFeed, *ppOutput);
      break;

    case C4CV_Integer:
      // Compare to default
      iDiff = 0;
      for (cnt = 0; cnt < cRef->Size; cnt++) {
        int iData, iDefault;
        memcpy(&iData, pData + cRef->Offset + cnt * sizeof(int), sizeof(int));
        memcpy(&iDefault, pDefault + cRef->Offset + cnt * sizeof(int), sizeof(int));
        if (iData != iDefault)
          iDiff = cnt + 1;
      }
      if (!iDiff)
        break;
      // Section name check
      if (!secput) {
        SAppend(secname, *ppOutput);
        secput = TRUE;
      }
      // Append to output
      SAppend(cRef->Name, *ppOutput);
      SAppend("=", *ppOutput);
      for (cnt = 0; cnt < iDiff; cnt++) {
        if (cnt)
          SAppend(",", *ppOutput);
        int iData;
        memcpy(&iData, pData + cRef->Offset + cnt * sizeof(int), sizeof(int));
        sprintf(ostr, "%i", iData);
        SAppend(ostr, *ppOutput);
      }
      SAppend(LineFeed, *ppOutput);
      break;

    case C4CV_Variable:
      // Compare to default
      iDiff = 0;
      for (cnt = 0; cnt < cRef->Size; cnt++) {
        long lData, lDefault;
        memcpy(&lData, pData + cRef->Offset + cnt * sizeof(long), sizeof(long));
        memcpy(&lDefault, pDefault + cRef->Offset + cnt * sizeof(long), sizeof(long));
        if (lData != lDefault)
          iDiff = cnt + 1;
      }
      if (!iDiff)
        break;
      // Section name check
      if (!secput) {
        SAppend(secname, *ppOutput);
        secput = TRUE;
      }
      // Append to output
      SAppend(cRef->Name, *ppOutput);
      SAppend("=", *ppOutput);
      for (cnt = 0; cnt < iDiff; cnt++) {
        if (cnt)
          SAppend(",", *ppOutput);
        long lData;
        memcpy(&lData, pData + cRef->Offset + cnt * sizeof(long), sizeof(long));
        sprintf(ostr, "%li", lData);
        SAppend(ostr, *ppOutput);
      }
      SAppend(LineFeed, *ppOutput);
      break;

    case C4CV_NameList:
      // Access data
      C4NameList *npData, *npDefault;
      npData = (C4NameList *)(pData + cRef->Offset);
      npDefault = (C4NameList *)(pDefault + cRef->Offset);
      // Compare to default
      if (*npData == *npDefault)
        break;
      // Section name check
      if (!secput) {
        SAppend(secname, *ppOutput);
        secput = TRUE;
      }
      // Append to output
      SAppend(cRef->Name, *ppOutput);
      SAppend("=", *ppOutput);
      npData->Write(ostr);
      SAppend(ostr, *ppOutput);
      SAppend(LineFeed, *ppOutput);
      break;

    case C4CV_IdList:
      // Access data
      C4IDList *ilpData, *ilpDefault;
      ilpData = (C4IDList *)(pData + cRef->Offset);
      ilpDefault = (C4IDList *)(pDefault + cRef->Offset);
      // Compare to default
      if (*ilpData == *ilpDefault)
        break;
      // Section name check
      if (!secput) {
        SAppend(secname, *ppOutput);
        secput = TRUE;
      }
      // Append to output
      SAppend(cRef->Name, *ppOutput);
      SAppend("=", *ppOutput);
      ilpData->Write(ostr);
      SAppend(ostr, *ppOutput);
      SAppend(LineFeed, *ppOutput);
      break;

#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // - - - - - - - -

    case C4CV_ObjectList:
      // Access data
      C4ObjectList *olpData;
      olpData = (C4ObjectList *)(pData + cRef->Offset);
      // Check empty
      if (olpData->IsClear())
        break;
      // Section name check
      if (!secput) {
        SAppend(secname, *ppOutput);
        secput = TRUE;
      }
      // Append to output
      SAppend(cRef->Name, *ppOutput);
      SAppend("=", *ppOutput);
      olpData->Write(ostr);
      SAppend(ostr, *ppOutput);
      SAppend(LineFeed, *ppOutput);
      break;

#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - - - - - - - - -
    }

  // Self-allocated buffer: finish with eof
  if (fOwnBuffer) {
    SAppend(EndOfFile, *ppOutput);
    *ipSize = SLen(*ppOutput);
  }

  return TRUE;
}
