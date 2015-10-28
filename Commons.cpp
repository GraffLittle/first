//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Commons.h"
#include "Projct.h"
#include <Registry.hpp>
#include <FileCtrl.hpp>

#pragma package(smart_init)

#ifdef BUILDER_WITH_MONITOR
  #include "MainUnitM.h"
#else
  #include "MainUnit.h"
#endif

AnsiString DefaultCompilerOptions = "SRC CD HLA";
AnsiString DefaultAsmOptions = "SET (HLARGE) CA";
AnsiString DefaultHexOptions = "FLASH (0x00)";
AnsiString DefaultEditor = "notepad";
AnsiString KeilPath = "C:\\Keil\\C166";
AnsiString MatlabRoot = "C:\\MatlabR11";

AnsiString LastPrjDir = "";
AnsiString HexFlashName = "";

TStringList *EarlyOpened;

int RightWidth = 100, BottomHeight = 100;

_EXCEPTION_POINTERS *exxp = NULL;
EXCEPTION_RECORD er;
CONTEXT cntxt;

int (__stdcall *OptCodeFunc)( const char* srcFile,
	bool bReplace, bool bLocalize, bool bShift,
  CanOptimizeFunc canOptimizeFunc, LogFunc logFunc );
int (__stdcall *GetVarNamesFunc)( const char* srcFile, char** varnames );

int WhatDoYouDo_forAllInOne = -1;

//---------------------------------------------------------------------------
TExceptionMessages ExceptMess, *ExcMess = &ExceptMess;
//---------------------------------------------------------------------------
// class TExceptionMessages
//---------------------------------------------------------------------------
AnsiString ContextFlagsString(DWORD ContextFlags)
{
  AnsiString Ans = "";
  if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) // SS:SP, CS:IP, FLAGS, BP
    Ans += " CONTEXT_CONTROL ";
  if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) // AX, BX, CX, DX, SI, DI
    Ans += " CONTEXT_INTEGER ";
  if ((ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) // DS, ES, FS, GS
    Ans += " CONTEXT_SEGMENTS ";
  if ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) // 387 state
    Ans += " CONTEXT_FLOATING_POINT ";
  if ((ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) // DB 0-3,6,7
    Ans += " CONTEXT_DEBUG_REGISTERS ";
  if ((ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS) // cpu specific extensions
    Ans += " CONTEXT_EXTENDED_REGISTERS ";
  return Ans;
}
//---------------------------------------------------------------------------
AnsiString ExceptionString(DWORD ExceptionCode)
{
  AnsiString Ans = "";
  switch (ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION : Ans =
      "The thread tried to read from or write to a virtual address\n"
      "for which it does not have the appropriate access.";
      break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED : Ans =
      "The thread tried to access an array element that is out of bounds\n"
      "and the underlying hardware supports bounds checking.";
      break;
    case EXCEPTION_BREAKPOINT : Ans =
      "A breakpoint was encountered.";
      break;
    case EXCEPTION_DATATYPE_MISALIGNMENT : Ans =
      "The thread tried to read or write data that is misaligned on hardware\n"
      "that does not provide alignment. For example, 16-bit values must be aligned\n"
      "on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.";
      break;
    case EXCEPTION_FLT_DENORMAL_OPERAND : Ans =
      "One of the operands in a floating-point operation is denormal.\n"
      "A denormal value is one that is too small to represent\n"
      "as a standard floating-point value.";
      break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO : Ans =
      "The thread tried to divide a floating-point value by a floating-point\n"
      "divisor of zero.";
      break;
    case EXCEPTION_FLT_INEXACT_RESULT : Ans =
      "The result of a floating-point operation cannot be represented\n"
      "exactly as a decimal fraction.";
      break;
    case EXCEPTION_FLT_INVALID_OPERATION : Ans =
      "This exception represents any floating-point exception\n"
      "not included in this list.";
      break;
    case EXCEPTION_FLT_OVERFLOW : Ans =
      "The exponent of a floating-point operation is greater than the magnitude\n"
      "allowed by the corresponding type.";
      break;
    case EXCEPTION_FLT_STACK_CHECK : Ans =
      "The stack overflowed or underflowed as the result\n"
      "of a floating-point operation.";
      break;
    case EXCEPTION_FLT_UNDERFLOW : Ans =
      "The exponent of a floating-point operation is less than the magnitude\n"
      "allowed by the corresponding type.";
      break;
    case EXCEPTION_ILLEGAL_INSTRUCTION : Ans =
      "The thread tried to execute an invalid instruction.";
      break;
    case EXCEPTION_IN_PAGE_ERROR : Ans =
      "The thread tried to access a page that was not present, and the system\n"
      "was unable to load the page. For example, this exception might occur\n"
      "if a network connection is lost while running a program over the network.";
      break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO : Ans =
      "The thread tried to divide an integer value by an integer divisor of zero.";
      break;
    case EXCEPTION_INT_OVERFLOW : Ans =
      "The result of an integer operation caused a carry out\n"
      "of the most significant bit of the result.";
      break;
    case EXCEPTION_INVALID_DISPOSITION : Ans =
      "An exception handler returned an invalid disposition to the exception\n"
      "dispatcher. Programmers using a high-level language such as C\n"
      "should never encounter this exception.";
      break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION : Ans =
      "The thread tried to continue execution after a noncontinuable\n"
      "exception occurred.";
      break;
    case EXCEPTION_PRIV_INSTRUCTION : Ans =
      "The thread tried to execute an instruction whose operation\n"
      "is not allowed in the current machine mode.";
      break;
    case EXCEPTION_SINGLE_STEP : Ans =
      "A trace trap or other single-instruction mechanism signaled\n"
      "that one instruction has been executed.";
      break;
    case EXCEPTION_STACK_OVERFLOW : Ans =
      "The thread used up its stack.";
      break;
    //default : Ans = "Unknown Exception Code";   // __FILE__
  }
  return Ans;
}
//---------------------------------------------------------------------------
TTreeNode* __fastcall TExceptionMessages::AddFFL( AnsiString Str,
  AnsiString File, AnsiString Func, int Line)
{
  MainForm->StatBar->Panels->Items[2]->Text = "E";

  if (!OutTree) {
    AnsiString Mess = "[" + DateTimeToStr(Now()) + "] : " + Str +
      "\nFile = " + ExtractFileName(File) + " (" + IntToStr(Line) + ")" +
      "\nFunction = " + Func;
    ShowMessage(Mess);
    return NULL;
    }

  TTreeNode *TN = OutTree->Items->Add( NULL,
    "[" + DateTimeToStr(Now()) + "] : " + Str);
  OutTree->Items->AddChild( TN, AnsiString("File = ") + ExtractFileName(File) +
    " (" + IntToStr(Line) + ")");
  OutTree->Items->AddChild( TN, "Function = " + Func);

  if (exxp) {
    OutTree->Items->AddChild( TN, ExceptionString(er.ExceptionCode));
    TTreeNode *tn1 = OutTree->Items->AddChild( TN, "Context");
    OutTree->Items->AddChild( tn1, "Flags = 0x" +
      IntToHex((int)cntxt.ContextFlags,8) + ", " +
      ContextFlagsString(cntxt.ContextFlags));
    if ((cntxt.ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {
      TTreeNode *tn2 = OutTree->Items->AddChild( tn1, "Control");
      OutTree->Items->AddChild( tn2, "Ebp = 0x" + IntToHex((int)cntxt.Ebp,8));
      OutTree->Items->AddChild( tn2, "Eip = 0x" + IntToHex((int)cntxt.Eip,8));
      OutTree->Items->AddChild( tn2, "SegCs = 0x" + IntToHex((int)cntxt.SegCs,8));
      OutTree->Items->AddChild( tn2, "EFlags = 0x" + IntToHex((int)cntxt.EFlags,8));
      OutTree->Items->AddChild( tn2, "Esp = 0x" + IntToHex((int)cntxt.Esp,8));
      OutTree->Items->AddChild( tn2, "SegSs = 0x" + IntToHex((int)cntxt.SegSs,8));
      }
    if ((cntxt.ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {
      TTreeNode *tn2 = OutTree->Items->AddChild( tn1, "Integer");
      OutTree->Items->AddChild( tn2, "Edi = 0x" + IntToHex((int)cntxt.Edi,8));
      OutTree->Items->AddChild( tn2, "Esi = 0x" + IntToHex((int)cntxt.Esi,8));
      OutTree->Items->AddChild( tn2, "Ebx = 0x" + IntToHex((int)cntxt.Ebx,8));
      OutTree->Items->AddChild( tn2, "Edx = 0x" + IntToHex((int)cntxt.Edx,8));
      OutTree->Items->AddChild( tn2, "Ecx = 0x" + IntToHex((int)cntxt.Ecx,8));
      OutTree->Items->AddChild( tn2, "Eax = 0x" + IntToHex((int)cntxt.Eax,8));
      }
    if ((cntxt.ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) {
      TTreeNode *tn2 = OutTree->Items->AddChild( tn1, "Segments");
      OutTree->Items->AddChild( tn2, "SegGs = 0x" + IntToHex((int)cntxt.SegGs,8));
      OutTree->Items->AddChild( tn2, "SegFs = 0x" + IntToHex((int)cntxt.SegFs,8));
      OutTree->Items->AddChild( tn2, "SegEs = 0x" + IntToHex((int)cntxt.SegEs,8));
      OutTree->Items->AddChild( tn2, "SegDs = 0x" + IntToHex((int)cntxt.SegDs,8));
      }
    if ((cntxt.ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) {
      TTreeNode *tn2 = OutTree->Items->AddChild( tn1, "Floating point");
      OutTree->Items->AddChild( tn2, "Control word = 0x" + IntToHex((int)cntxt.FloatSave.ControlWord,8));
      OutTree->Items->AddChild( tn2, "Status word = 0x" + IntToHex((int)cntxt.FloatSave.StatusWord,8));
      OutTree->Items->AddChild( tn2, "Tag word = 0x" + IntToHex((int)cntxt.FloatSave.TagWord,8));
      OutTree->Items->AddChild( tn2, "Error offset = 0x" + IntToHex((int)cntxt.FloatSave.ErrorOffset,8));
      OutTree->Items->AddChild( tn2, "Error selector = 0x" + IntToHex((int)cntxt.FloatSave.ErrorSelector,8));
      OutTree->Items->AddChild( tn2, "Data offset = 0x" + IntToHex((int)cntxt.FloatSave.DataOffset,8));
      OutTree->Items->AddChild( tn2, "Data selector = 0x" + IntToHex((int)cntxt.FloatSave.DataSelector,8));
      OutTree->Items->AddChild( tn2, "Cr0NpxState = 0x" + IntToHex((int)cntxt.FloatSave.Cr0NpxState,8));
      }
    if ((cntxt.ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) {
      TTreeNode *tn2 = OutTree->Items->AddChild( tn1, "Debug registers");
      OutTree->Items->AddChild( tn2, "Dr0 = 0x" + IntToHex((int)cntxt.Dr0,8));
      OutTree->Items->AddChild( tn2, "Dr1 = 0x" + IntToHex((int)cntxt.Dr1,8));
      OutTree->Items->AddChild( tn2, "Dr2 = 0x" + IntToHex((int)cntxt.Dr2,8));
      OutTree->Items->AddChild( tn2, "Dr3 = 0x" + IntToHex((int)cntxt.Dr3,8));
      OutTree->Items->AddChild( tn2, "Dr6 = 0x" + IntToHex((int)cntxt.Dr6,8));
      OutTree->Items->AddChild( tn2, "Dr7 = 0x" + IntToHex((int)cntxt.Dr7,8));
      }
    if ((cntxt.ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS) {
      OutTree->Items->AddChild( tn1, "Extended registers");
      }
    exxp = NULL;
    }

  return TN;
}
//---------------------------------------------------------------------------
void __fastcall TExceptionMessages::Add( AnsiString Str, AnsiString Date,
  AnsiString Time, AnsiString File, AnsiString Func, int Line)
{
  AddFFL( Str, File, Func, Line);
}
//---------------------------------------------------------------------------
void __fastcall TExceptionMessages::AddL( AnsiString Str, AnsiString File,
  AnsiString Func, int Line)
{
  AddFFL( Str, File, Func, Line);
}
//---------------------------------------------------------------------------
void __fastcall TExceptionMessages::AddObj( AnsiString Str, AnsiString File,
  AnsiString Func, int Line, TObject *Obj)
{
  TTreeNode *Node = AddFFL( Str, File, Func, Line);
  if (!Node) return;

  TComponent *CM = dynamic_cast<TComponent*>(Obj);
  if (CM) {
    OutTree->Items->AddChild( Node,
      "Object = " + CM->Name + " (" + Obj->ClassName() + ")");
    if (CM->Owner)
      OutTree->Items->AddChild( Node, "Object Owner = " + CM->Owner->Name +
        " (" + CM->Owner->ClassName() + ")");
  } else
    OutTree->Items->AddChild( Node, "Object Class Name = " + Obj->ClassName());
}
//---------------------------------------------------------------------------
void __fastcall TExceptionMessages::AddAdd( AnsiString Str, AnsiString File,
  AnsiString Func, int Line, AnsiString Additional)
{
  TTreeNode *Node = AddFFL( Str, File, Func, Line);
  if (!Node) return;
  OutTree->Items->AddChild( Node, "Additional info = " + Additional);
}
//---------------------------------------------------------------------------
// Common functions
//---------------------------------------------------------------------------
AnsiString __fastcall GetRegistryValue(AnsiString KeyName, AnsiString Value)
{
  AnsiString S = "";
  __TRY
  TRegistry *Registry = new TRegistry;
  try {
    Registry->RootKey = HKEY_CLASSES_ROOT; //HKEY_LOCAL_MACHINE;
    Registry->OpenKey(KeyName,false);
    S = Registry->ReadString(Value);
  } __finally {
    delete Registry;
    }
  __CATCH
  return S;
}//---------------------------------------------------------------------------
AnsiString __fastcall GetMatlab5Root(void)
{
   AnsiString S = "";
   __TRY
   TRegistry *RR = new TRegistry();
   try
    {

    // Для Matlab 5.3
     RR->RootKey = HKEY_CLASSES_ROOT;
     if (RR->OpenKey("\\Matlab.Application.Single.5\\CLSID",false))
       S = RR->ReadString("");
     if (RR->OpenKey("\\CLSID\\"+S+"\\LocalServer32",false))
       S = RR->ReadString("");
     int ps = S.AnsiPos("matlab.exe");
     if (ps) {
      S = S.SubString( 1, ps-2);
      // S содержит размещение matlab.exe (полный путь)
      S = ExtractFileDir(S);
      }
    }
      __finally {
      delete RR;
      }
  if (!DirectoryExists(S)) {
  //  S = "C:\\MATLABR11";
      S = "";
    }
  __CATCH
  return S;
}
//---------------------------------------------------------------------------
AnsiString __fastcall GetMatlab6Root(void)
{
   AnsiString S = "";
   __TRY
   TRegistry *RR = new TRegistry();
   try
    {
    // Для Matlab 6.5

     RR->RootKey = HKEY_CLASSES_ROOT;
     if (RR->OpenKey("\\Matlab.Application.Single.6\\CLSID",false))
       S = RR->ReadString("");
     if (RR->OpenKey("\\CLSID\\"+S+"\\LocalServer32",false))
       S = RR->ReadString("");
     int ps = S.AnsiPos("matlab.exe");
     if (ps) {
      S = S.SubString( 1, ps-2);
     // S содержит размещение matlab.exe (полный путь)
      S = ExtractFileDir(S);
      S = ExtractFileDir(S);
      }
    }
      __finally {
      delete RR;
      }
  if (!DirectoryExists(S)) {
    S = "";
    }
  __CATCH
  return S;
}
//---------------------------------------------------------------------------
void __fastcall LastErrorMessage(AnsiString Capt, int ErrorNumber)
{
  __TRY
  char *lpMsgBuf;
  DWORD errnum;
  if (ErrorNumber != -1)
    errnum = ErrorNumber;
  else
    errnum = GetLastError();
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    errnum,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0, NULL
    );
  MessageBox( NULL, lpMsgBuf, Capt.c_str(), MB_OK | MB_ICONINFORMATION);
  LocalFree( lpMsgBuf );
  __CATCH
}
//---------------------------------------------------------------------------
AnsiString ResStr(UINT resID)
{
  char buff[1024] = "";
  __TRY
  int rs = LoadString( NULL, resID, buff, 1024);
  if (!rs) LastErrorMessage("Load Resource String error");
  __CATCH
  return AnsiString(buff);
}
//---------------------------------------------------------------------------
bool __fastcall DosCmdExec(LPCSTR lpCmdLine)
{
  PROCESS_INFORMATION ProcInf;
  STARTUPINFO StInf = {
    sizeof(STARTUPINFO),
    0,0,"Hello",
    0,0,100,100,0,0,0,
    STARTF_USESHOWWINDOW,
    SW_HIDE,
    0,0,0,0,0
    };

  BOOL ErrCode =
    CreateProcess(
      NULL,
      (char*)lpCmdLine,
      NULL,
      NULL,
      FALSE,
      0,
      NULL,
      NULL,
      &StInf,
      &ProcInf
    );
  if (!ErrCode) {
    LastErrorMessage("CreateProcess");
    return false;
    }
  HANDLE hP = ProcInf.hProcess;
  DWORD ExCode;
  do {
    ErrCode = GetExitCodeProcess( hP, &ExCode);
    } while (ExCode == STILL_ACTIVE);
  if (!ErrCode) {
    LastErrorMessage("GetExitCodeProcess");
    return false;
    }
  return true;
}
//---------------------------------------------------------------------------
TStringList* StringToTokiens( AnsiString Words, AnsiString Dlmtr )
{
  AnsiString Work = Words.Trim();
  int spos;
  TStringList *Making = new TStringList;

  while ((spos = Work.AnsiPos(Dlmtr)) != 0) {
    Making->Add(Work.SubString( 1, spos-1));
    Work = Work.SubString( spos+1, Work.Length() - spos);
    Work = Work.Trim();
  } // while
  if (Work != "")
    Making->Add(Work);

  return Making;
}
//---------------------------------------------------------------------------
void StringToTokiensA( AnsiString Words, AnsiString Dlmtr, TStrings *Making )
{
  AnsiString Work = Words.Trim();
  int spos;
  while ((spos = Work.AnsiPos(Dlmtr)) != 0) {
    Making->Add(Work.SubString( 1, spos-1));
    Work = Work.SubString( spos+1, Work.Length() - spos);
    Work = Work.Trim();
  } // while
  if (Work != "")
    Making->Add(Work);
}
//---------------------------------------------------------------------------
TStringList* StringToTok2( AnsiString Words,
  AnsiString Dlmtr1, AnsiString Dlmtr2 )
{
  AnsiString Work = Words.Trim();
  int spos;
  TStringList *Making = new TStringList;

  int spos1 = Work.AnsiPos(Dlmtr1);
  int spos2 = Work.AnsiPos(Dlmtr2);
  while (spos1 || spos2) {
 //   int spos = 0;
    if (!spos1 && spos2) spos = spos2;
    else if (!spos2 && spos1) spos = spos1;
    else spos = (spos1 < spos2) ? spos1 : spos2;
    Making->Add(Work.SubString( 1, spos-1));
    Work = Work.SubString( spos+1, Work.Length() - spos);
    Work = Work.Trim();
    spos1 = Work.AnsiPos(Dlmtr1);
    spos2 = Work.AnsiPos(Dlmtr2);
  } // while
  if (Work != "")
    Making->Add(Work);

  return Making;
}
//---------------------------------------------------------------------------
AnsiString LastWord( AnsiString Words )
{
  TStrings *SL = StringToTok2(Words," ","\t");
  if (SL->Count)
    return SL->Strings[SL->Count - 1];
  else return "";
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define PrjDir "$(PRJ)"
#define MseDir "$(MSE)"
#define DatDir "$(DAT)"
//---------------------------------------------------------------------------
AnsiString __fastcall DirFromAbbr( AnsiString AbbrDir, AnsiString ProjDir )
// Получает имя директории из  $(PRJ)\Obj (или типа того)
{
  AbbrDir = ExcludeTrailingBackslash(AbbrDir);
  AnsiString Subs = AbbrDir.SubString( 1, 6);
  if (Subs == PrjDir) {
    AbbrDir.Delete(1,6);
    if (ProjDir == "")
      return ExtractFileDir(Project->NameOfFile) + AbbrDir;
    else
      return ProjDir + AbbrDir;
  } else if (Subs == MseDir) {
    char buff[301];
    AbbrDir.Delete(1,6);
    if (GetMSEPathStr( pcProgram, buff, 300))
      return buff + AbbrDir;
    return ExtractFileDir(Application->ExeName) + AbbrDir;
  } else if (Subs == DatDir) {
    char buff[301];
    AbbrDir.Delete(1,6);
    if (GetMSEPathStr( pcData, buff, 300))
      return buff + AbbrDir;
    return ExtractFileDir(Application->ExeName) + "\\Data" + AbbrDir;
  } else
    return AbbrDir;
}
//---------------------------------------------------------------------------
void __fastcall BreakDirs( AnsiString Str, TStrings *Outs )
{
  Outs->Clear();
  StringToTokiensA( Str, ";", Outs);
  for (int i = 0; i < Outs->Count; i++)
    Outs->Strings[i] = DirFromAbbr(Outs->Strings[i]);
}
//---------------------------------------------------------------------------
AnsiString __fastcall GetFullName( AnsiString SrcDir, AnsiString RelatFile)
// Merges two parts of full file name
// SrcDir =     'C:\...\Directory'
// RelatFile =  '..\Dir1\FileName.Ext' or 'Dir2\FileName.Ext'
{
  AnsiString WasDir = GetCurrentDir();
  if (!SetCurrentDir(SrcDir)) {
    //AnsiString Mess = "Can't set current directory to " + SrcDir;
    //MessageBox( NULL, Mess.c_str(), "Error", MB_OK | MB_ICONERROR);
    }
  AnsiString Ans = ExpandUNCFileName(RelatFile);
  SetCurrentDir(WasDir);
  return Ans;
}
//---------------------------------------------------------------------------
AnsiString __fastcall GetRelativeName( AnsiString FullName, AnsiString DirPart)
// Breaks full file name on two parts - DirPart and Relative Name
{
  return ExtractRelativePath( IncludeTrailingBackslash(DirPart), FullName);
}
//---------------------------------------------------------------------------
#include <shlobj.h>
int CALLBACK BrowseCallbackProc(  HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
 {
  LPITEMIDLIST pidlBrowse=(LPITEMIDLIST)lParam;
//  if (pidlBrowse)
   {
    char *Buffer=(char *)lpData;
    switch(uMsg)
     {
      case BFFM_INITIALIZED:
       {
          SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)Buffer);
          SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)Buffer);
       }
      break;
      case BFFM_SELCHANGED:
       {
        if (SHGetPathFromIDList(pidlBrowse,Buffer)&& *Buffer)
          SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)Buffer);
         else
          {
           SendMessage(hwnd,BFFM_ENABLEOK,0,FALSE);
           SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,
            (LPARAM)"Object is not a folder");
          }

       }
      break;
      case BFFM_VALIDATEFAILED :
       {
        return 1;
       }
     }
   }
  return 0;
 }
//---------------------------------------------------------------------------
AnsiString GetFolderName(HWND hWnd, AnsiString Text, AnsiString OldFolder,UINT ulFlags)
 {
  LPMALLOC pMalloc;
  AnsiString S=OldFolder;
  if (SUCCEEDED(SHGetMalloc(&pMalloc)))
   {
    LPITEMIDLIST pidlDesktop;
    if (SUCCEEDED(SHGetSpecialFolderLocation(Application->Handle,CSIDL_DESKTOP,&pidlDesktop)))
     {
      char *Buffer=(char *)pMalloc->Alloc(MAX_PATH);
      if (Buffer)
       {
        BROWSEINFO bi;
        strcpy(Buffer,OldFolder.c_str());
        bi.hwndOwner=hWnd;
        bi.pidlRoot=pidlDesktop;
        bi.pszDisplayName=Buffer;
        bi.lpszTitle=Text.c_str();
        bi.ulFlags=ulFlags;//BIF_DONTGOBELOWDOMAIN+BIF_RETURNONLYFSDIRS+BIF_STATUSTEXT;
        bi.lpfn=&BrowseCallbackProc;
        bi.lParam=(LPARAM)Buffer;
        bi.iImage=0;

        LPITEMIDLIST pidlBrowse=SHBrowseForFolder(&bi);
        if (pidlBrowse)
         {
          if (SHGetPathFromIDList(pidlBrowse,Buffer)&& *Buffer)
           {
            S=AnsiString (Buffer);
           }
          pMalloc->Free(pidlBrowse);
         }
        pMalloc->Free(Buffer);
       }
      pMalloc->Free(pidlDesktop);
     }
     pMalloc->Release();
   }
  return S;
 }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//#include "DllHeaderOne.h"
#include <float.h> // for _control87
//---------------------------------------------------------------------------
    /*
     *     Public vars
     */
//Engine *c_ep;
char c_buffer[1001];
bool c_runing = false;

//---------------------------------------------------------------------------
/*
void __fastcall ExecuteMatlabString(AnsiString Str)
{
  try {

      HINSTANCE LibOne=LoadLibrary("libeng.dll");      
      HINSTANCE LibTwo=LoadLibrary("libmx.dll"); 

      mengOpen = (uengOpen*) GetProcAddress(LibOne,"engOpen");
      mengEvalString = (uengEvalString*) GetProcAddress(LibOne,"engEvalString");
      mengGetArray = (uengGetArray*) GetProcAddress(LibOne,"engGetArray");
      mmxGetString = (umxGetString*) GetProcAddress(LibTwo,"mxGetString");
      mengOutputBuffer = (uengOutputBuffer*) GetProcAddress(LibOne,"engOutputBuffer");
      mengClose = (uengClose*) GetProcAddress(LibOne,"engClose");

  
    do {
      if (!c_runing) {
        _control87(MCW_EM,MCW_EM);
        if ((c_ep = mengOpen("/O")) == NULL) {
          LastErrorMessage("Engine not Opened");
          c_runing = false;
          return;
        }
        c_runing = true;
      }
      mengOutputBuffer(c_ep, c_buffer, 1000);

      AnsiString Cmd = "qwertyuiop = '_qwertyuiop_'";
      mxArray *title;
      char bb[301];
      int cnt = 0;
      do {
        mengEvalString( c_ep, Cmd.c_str());
        mxArray *title = mengGetArray(c_ep, "qwertyuiop");
//        mxArray *title = engGetVariable(c_ep, "qwertyuiop"); // для Matlab 6.5
        mmxGetString(title, bb, 300);
      } while ((strcmp("_qwertyuiop_", bb)) && (cnt++ < 100));
      if (cnt > 100)
        c_runing = false;
        //c_ep = engOpen("/O");
    } while (!c_runing);

  	mengEvalString( c_ep, Str.c_str());

  } __finally {
    }
}
*/
//---------------------------------------------------------------------------
/*
void __fastcall CloseMatlabEngine(void)
{
  __TRY
  if (c_runing) {

//    asm {
//      pop ecx
//      pop ecx
//      }

    mengClose(c_ep);

//    asm {
//      pop ecx
//      }

    c_runing = false;
    c_ep = NULL;
  }
  __CATCH
}
*/
//---------------------------------------------------------------------------
/*
AnsiString HexBinErrorString(unsigned long ErrCode)
{
  AnsiString Ans = "Error";
  __TRY
  switch (ErrCode) {
    case 0: Ans = "No errors."; break;
    case NO_COLON: Ans = "Нет двоеточия."; break;
    case WRONG_SYMBOL: Ans = "Ошибочный символ."; break;
    case CHECKSUM_ERROR: Ans = "Ошибка контрольной суммы."; break;
    case WRONG_LENGTH: Ans = "Неправильная длина."; break;
    case WRONG_ADDRESS: Ans = "Неправильный адрес."; break;
    case UNKNOWN_TYPE: Ans = "Неизвестный тип."; break;
    case INCOMPATIBLE_TYPE: Ans = "Несовместимый тип."; break;

    case FILE_OPEN_ERROR: Ans = "Ошибка открытия файла."; break;
    case FILE_CLOSE_ERROR: Ans = "Ошибка закрытия файла."; break;

    case MALLOC_ERROR: Ans = "Ошибка выделения памяти."; break;
    }
  __CATCH
  return Ans;
}
*/
//---------------------------------------------------------------------------
AnsiString __fastcall SlashNto13(AnsiString Origin)
{
  AnsiString Ans = "";
  __TRY
  for (int i = 1; i <= Origin.Length(); i++) {
    if ((Origin[i] == '\\') && (i != Origin.Length())) {
      if (Origin[i+1] == 'n') { Ans = Ans + '\n'; i++;
      } else if (Origin[i+1] == 'r') { Ans = Ans + '\r'; i++;
      } else if (Origin[i+1] == 't') { Ans = Ans + '\t'; i++;
      } else Ans = Ans + Origin[i];
    } else Ans = Ans + Origin[i];
  }
  __CATCH
  return Ans;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

bool IsMonitorBusy = false;
bool GetBusy(void) { return IsMonitorBusy; }
void SetBusy(bool NewBusy) { IsMonitorBusy = NewBusy; }

typedef void (*StatFunc)(unsigned);

bool mDownloadData
(
 void*         pBuff,  /*Буфер для загружаемых данных*/
 unsigned long ulAddr, /*Начальный адрес для загрузки*/
 unsigned long ulSize, /*Размер загружаемых данных*/
 unsigned      uMax   /*Максимальный размер загружаемых данных
                         за одну передачу: 1..6 байт*/
)
{
  while (IsMonitorBusy) {}
  IsMonitorBusy = true;
  bool res = DownloadData( pBuff, ulAddr, ulSize, uMax);
  IsMonitorBusy = false;
  return res;
}

bool mUploadData
(
 void*         pBuff,      /*Буфер для выгрузки данных*/
 unsigned long ulAddr,     /*Начальный адрес для выгрузки*/
 unsigned long ulSize,     /*Размер выгружаемых данных*/
 unsigned      uMax       /*Максимальный размер выгружаемых данных
                             за одну передачу: 1..5 байт*/
)
{
  while (IsMonitorBusy) {}
  IsMonitorBusy = true;
  bool res = UploadData( pBuff, ulAddr, ulSize, uMax);
  IsMonitorBusy = false;
  return res;
}

bool mUploadDataStat
(
 void*         pBuff,      /*Буфер для выгрузки данных*/
 unsigned long ulAddr,     /*Начальный адрес для выгрузки*/
 unsigned long ulSize,     /*Размер выгружаемых данных*/
 unsigned      uMax,       /*Максимальный размер выгружаемых данных
                             за одну передачу: 1..5 байт*/
 StatFunc      func
)
{
  while (IsMonitorBusy) {}
  IsMonitorBusy = true;
  bool res = UploadDataStat( pBuff, ulAddr, ulSize, uMax, func);
  IsMonitorBusy = false;
  return res;
}

bool mDownloadDataStat
(
void*         pBuff,  /*Буфер для загружаемых данных*/
unsigned long ulAddr,
unsigned long ulSize, /*Размер загружаемых данных*/
unsigned      uMax,   /*Максимальный размер загружаемых данных
                        за одну передачу: 1..6 байт*/
StatFunc      func
)
{
  while (IsMonitorBusy) {}
  IsMonitorBusy = true;
  bool res = DownloadDataStat( pBuff, ulAddr, ulSize, uMax, func);
  IsMonitorBusy = false;
  return res;
}
//---------------------------------------------------------------------------
// MSE format to ASAP format
AnsiString __fastcall ConvertMseFormat(AnsiString Src)
{
  AnsiString Ans = Src.SubString(2,Src.Length()-2); // without '%' and 'f'
  __TRY
  int p1, p2;
  int sres = sscanf( Ans.c_str(), "%u.%u", &p1, &p2);
  if (!sres) {
    MessageBox( NULL, ("Incorrect format: " + Src).c_str(), "Error",
      MB_OK | MB_ICONERROR);
    p1 = 1;
    p2 = 0;
  } else if (sres == 1) {
    MessageBox( NULL, ("Incorrect format: " + Src).c_str(), "Error",
      MB_OK | MB_ICONERROR);
    p2 = 0;
    }
  Ans = "%" + IntToStr(p1 + p2 + (p2 ? 1 : 0)) + "." + IntToStr(p2);
  __CATCH
  return Ans;
}
//---------------------------------------------------------------------------
#pragma inline
void __fastcall ReadFloatWord(void)
{
  asm {
    FSTCW CtlWord
    FWAIT
    }
}
//---------------------------------------------------------------------------

void __fastcall SetFloatWord(void)
{
  asm {
    FLDCW CtlWord
    FWAIT
    }
}

//---------------------------------------------------------------------------
// About platform
//---------------------------------------------------------------------------
AnsiString PlatformFile = "Keil.dll";
AnsiString PlatformName = "KEIL";

HINSTANCE hinstPlatform = NULL;
p_IsItPlatformDll pIsItPlatformDll = NULL;
p_PlatformDllName pPlatformDllName = NULL;
p_HeaderInsert pHeaderInsert = NULL;
p_GetMap pGetMap = NULL;
p_GetLibPublics pGetLibPublics = NULL;
p_AsmParameters pAsmParameters = NULL;
p_C_Parameters pC_Parameters = NULL;
p_LinkParameters pLinkParameters = NULL;
p_HexParameters pHexParameters = NULL;
p_MakeSets pMakeSets = NULL;
p_MakeBuildRules pMakeBuildRules = NULL;
p_MakePrivateCRules pMakePrivateCRules = NULL;
p_MakePrivateAsmRules pMakePrivateAsmRules = NULL;
p_MakeDefaultRules pMakeDefaultRules = NULL;
p_MakeModelRules pMakeModelRules = NULL;
p_GetVECTAB pGetVECTAB = NULL;
//---------------------------------------------------------------------------
bool __fastcall LoadPlatform(AnsiString FileName)
{
  __TRY
  if (hinstPlatform) {
    FreeLibrary(hinstPlatform);
    hinstPlatform = NULL;
    }

  hinstPlatform = LoadLibrary(FileName.c_str());
  if (!hinstPlatform) {
    AddToLog("Can't load library " + FileName,clRed,fsBold);
    return false;
    }

    
  pIsItPlatformDll =
    (p_IsItPlatformDll)GetProcAddress( hinstPlatform, "IsItPlatformDll");
  pPlatformDllName =
    (p_PlatformDllName)GetProcAddress( hinstPlatform, "PlatformDllName");
  pHeaderInsert =
    (p_HeaderInsert)GetProcAddress( hinstPlatform, "HeaderInsert");
  pGetMap =
    (p_GetMap)GetProcAddress( hinstPlatform, "GetMap");
  pGetVECTAB = (p_GetVECTAB)GetProcAddress( hinstPlatform, "GetVECTAB");  
  pGetLibPublics =
    (p_GetLibPublics)GetProcAddress( hinstPlatform, "GetLibPublics");
  pAsmParameters =
    (p_AsmParameters)GetProcAddress( hinstPlatform, "AsmParameters");
  pC_Parameters =
    (p_C_Parameters)GetProcAddress( hinstPlatform, "C_Parameters");
  pLinkParameters =
    (p_LinkParameters)GetProcAddress( hinstPlatform, "LinkParameters");
  pHexParameters =
    (p_HexParameters)GetProcAddress( hinstPlatform, "HexParameters");
  pMakeSets =
    (p_MakeSets)GetProcAddress( hinstPlatform, "MakeSets");
  pMakeBuildRules =
    (p_MakeBuildRules)GetProcAddress( hinstPlatform, "MakeBuildRules");
  pMakePrivateCRules =
    (p_MakePrivateCRules)GetProcAddress( hinstPlatform, "MakePrivateCRules");
  pMakePrivateAsmRules =
    (p_MakePrivateAsmRules)GetProcAddress( hinstPlatform, "MakePrivateAsmRules");
  pMakeDefaultRules =
    (p_MakeDefaultRules)GetProcAddress( hinstPlatform, "MakeDefaultRules");
  pMakeModelRules =
    (p_MakeModelRules)GetProcAddress( hinstPlatform, "MakeModelRules");

  if (!pIsItPlatformDll || !pPlatformDllName || !pHeaderInsert || !pGetMap
    || !pGetLibPublics || !pAsmParameters || !pC_Parameters || !pLinkParameters
    || !pHexParameters || !pMakeBuildRules || !pMakeSets
    || !pMakePrivateCRules || !pMakePrivateAsmRules || !pMakeDefaultRules
    || !pMakeModelRules) {
    FreeLibrary(hinstPlatform);
    hinstPlatform = NULL;
    return false;
    }

  PlatformFile = ExtractFileName(FileName);
  PlatformName = pPlatformDllName();
  __CATCH
  return true;
}
//---------------------------------------------------------------------------
// end: About platform
//---------------------------------------------------------------------------

