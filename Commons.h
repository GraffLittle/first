//---------------------------------------------------------------------------

#ifndef CommonsH
#define CommonsH

#include <ComCtrls.hpp>
#include "streses.h"
#include "optimization.h"
#include "..\Commons\MSE_new\inc\xtrndefs.h"
#include "..\Commons\monitor\HexBin.h"
#include "..\Commons\monitor\Monitor.h"
#include "Platform.h"
//---------------------------------------------------------------------------
extern int (__stdcall *OptCodeFunc)( const char* srcFile,
	bool bReplace, bool bLocalize, bool bShift,
  CanOptimizeFunc canOptimizeFunc, LogFunc logFunc );
extern int (__stdcall *GetVarNamesFunc)( const char* srcFile, char** varnames );
//---------------------------------------------------------------------------
AnsiString __fastcall GetRegistryValue(AnsiString KeyName, AnsiString Value);
AnsiString __fastcall GetMatlab5Root(void);
AnsiString __fastcall GetMatlab6Root(void);

void __fastcall LastErrorMessage(AnsiString Capt = "", int ErrorNumber = -1);
AnsiString ResStr(UINT resID);
bool __fastcall DosCmdExec(LPCSTR lpCmdLine);
AnsiString __fastcall DirFromAbbr( AnsiString AbbrDir, AnsiString ProjDir = "" );
void __fastcall BreakDirs( AnsiString Str, TStrings *Outs );
AnsiString __fastcall GetRelativeName( AnsiString FullName, AnsiString DirPart);
AnsiString __fastcall GetFullName( AnsiString SrcDir, AnsiString RelatFile);
AnsiString GetFolderName(HWND hWnd, AnsiString Text, AnsiString OldFolder,
  UINT ulFlags);
void __fastcall ExecuteMatlabString(AnsiString Str);
void __fastcall CloseMatlabEngine(void);
AnsiString __fastcall SlashNto13(AnsiString Origin);
//---------------------------------------------------------------------------
// Класс сделан Алексеем Сиротиным для работы с 5 и 6 Matlab как с Com сервером
 class TMatlab
{
  Variant Matlab;
public:
  void start(int Version);   // Function
  void start(void);
  void show(void); // только для 6-ой версии
  void hide(void); // только для 6-ой версии
  void maximize(void);
  void minimize(void);
  AnsiString execute(AnsiString Command);  // TVarData
  void quit(void);
};

class TExceptionMessages
{
private:
  TTreeNode* __fastcall AddFFL( AnsiString Str, AnsiString File,
    AnsiString Func, int Line);
protected:
public:
  void __fastcall Add( AnsiString Str, AnsiString Date, AnsiString Time,
    AnsiString File, AnsiString Func, int Line);
  void __fastcall AddL( AnsiString Str, AnsiString File, AnsiString Func,
    int Line);
  void __fastcall AddObj( AnsiString Str, AnsiString File, AnsiString Func,
    int Line, TObject *Obj);
  void __fastcall AddAdd( AnsiString Str, AnsiString File, AnsiString Func,
    int Line, AnsiString Additional);

  TTreeView *OutTree;
};

extern TExceptionMessages *ExcMess; // Program Exceptions messages
//---------------------------------------------------------------------------
#define __TRY0 try {
#define __CATCH0 } catch(Exception *E) { \
    ExcMess->AddL(E->Message,__FILE__,__FUNC__,__LINE__); }

/*
#define __TRY try { try {
#define __CATCH } catch(Exception *E) { \
    ExcMess->AddL(E->Message,__FILE__,__FUNC__,__LINE__); } \
    } catch(...) { \
      ExcMess->AddL("Unknown (non-VCL) exception",__FILE__,__FUNC__,__LINE__); }
*/

extern _EXCEPTION_POINTERS *exxp;
extern EXCEPTION_RECORD er;
extern CONTEXT cntxt;

#define __TRY try { try {
#define __CATCH } catch(Exception *E) { \
    ExcMess->AddL(E->Message,__FILE__,__FUNC__,__LINE__); } \
    } __except((int)(exxp = GetExceptionInformation())) { \
      er = *(exxp->ExceptionRecord); cntxt = *(exxp->ContextRecord); \
      ExcMess->AddL("Non-VCL exception",__FILE__,__FUNC__,__LINE__); }

//---------------------------------------------------------------------------
#define REGSTR_PATH_CONF TEXT("Software\\Elcar\\Builder\\2.0\\")
#define REGSTR_PATH_HF TEXT("Software\\Elcar\\HF\\2.0\\")
#define REGSTR_PATH_DUMPING TEXT("Software\\Elcar\\Dumping\\1.0\\")

extern AnsiString DefaultCompilerOptions;
extern AnsiString DefaultAsmOptions;
extern AnsiString DefaultHexOptions;
extern AnsiString DefaultEditor;
extern AnsiString KeilPath;
extern AnsiString MatlabRoot;

extern AnsiString LastPrjDir;
extern AnsiString HexFlashName;

#define MAX_RECENT_COUNT 5
extern TStringList *EarlyOpened;

extern int ColW[6];
extern int RightWidth, BottomHeight;

/*
// Boot load parameters
extern AnsiString StartHex;
extern AnsiString LoadHex;
extern AnsiString LPTPortName;
extern unsigned KLineBaud;
extern unsigned CANLineBaud;
extern unsigned LPT_ID;
extern unsigned CRO_ID;
extern unsigned DTO_ID;
extern unsigned long StartLoadAddress;
extern unsigned CANAddress;
extern unsigned CAN_Chanel;
extern char IdBytes[256];
extern unsigned char IDsize;
extern AnsiString IdBytesStr;
*/
// Hex-file load parameters
extern unsigned long  HexOffset;
extern unsigned       HexFill;
extern unsigned       HexAlign;

//void Bytes2String(void);
//void String2Bytes(void);

//extern unsigned pCTR; // Указатель на счетчик команд
extern bool AnErrorOccured;

extern bool BootLoadMade;
extern bool BeforeProgramming;

typedef void (*CallBackFunction)(unsigned);
extern CallBackFunction CBFunc;

extern int WhatDoYouDo_forAllInOne;

void __fastcall AddToLog(AnsiString Text);
void __fastcall AddToLog(AnsiString Text,TColor mColor,TFontStyle mStyle);

void __fastcall ProcessPosition(int Pos);

extern unsigned short CtlWord;
void __fastcall ReadFloatWord(void);
void __fastcall SetFloatWord(void);
//---------------------------------------------------------------------------
//AnsiString HexBinErrorString(unsigned long ErrCode);
//---------------------------------------------------------------------------

bool mDownloadData
(
 void*         pBuff,  /*Буфер для загружаемых данных*/
 unsigned long ulAddr, /*Начальный адрес для загрузки*/
 unsigned long ulSize, /*Размер загружаемых данных*/
 unsigned      uMax   /*Максимальный размер загружаемых данных
                         за одну передачу: 1..6 байт*/
);
bool mUploadData
(
 void*         pBuff,      /*Буфер для выгрузки данных*/
 unsigned long ulAddr,     /*Начальный адрес для выгрузки*/
 unsigned long ulSize,     /*Размер выгружаемых данных*/
 unsigned      uMax       /*Максимальный размер выгружаемых данных
                             за одну передачу: 1..5 байт*/
);
bool mUploadDataStat
(
 void*         pBuff,      /*Буфер для выгрузки данных*/
 unsigned long ulAddr,     /*Начальный адрес для выгрузки*/
 unsigned long ulSize,     /*Размер выгружаемых данных*/
 unsigned      uMax,       /*Максимальный размер выгружаемых данных
                             за одну передачу: 1..5 байт*/
 void (*)(unsigned)
);
bool mDownloadDataStat
(
void*         pBuff,  /*Буфер для загружаемых данных*/
unsigned long ulAddr,
unsigned long ulSize, /*Размер загружаемых данных*/
unsigned      uMax,   /*Максимальный размер загружаемых данных
                        за одну передачу: 1..6 байт*/
void (*ShowProcess)(unsigned)
);

//---------------------------------------------------------------------------
AnsiString __fastcall ConvertMseFormat(AnsiString Src);

//---------------------------------------------------------------------------
// About platform
extern AnsiString PlatformFile;
extern AnsiString PlatformName;

bool __fastcall LoadPlatform(AnsiString FileName);

extern HINSTANCE hinstPlatform;
extern p_IsItPlatformDll pIsItPlatformDll;
extern p_PlatformDllName pPlatformDllName;
extern p_HeaderInsert pHeaderInsert;
extern p_GetMap pGetMap;
extern p_GetLibPublics pGetLibPublics;
extern p_AsmParameters pAsmParameters;
extern p_C_Parameters pC_Parameters;
extern p_LinkParameters pLinkParameters;
extern p_HexParameters pHexParameters;
extern p_MakeSets pMakeSets;
extern p_MakeBuildRules pMakeBuildRules;
extern p_MakePrivateCRules pMakePrivateCRules;
extern p_MakePrivateAsmRules pMakePrivateAsmRules;
extern p_MakeDefaultRules pMakeDefaultRules;
extern p_MakeModelRules pMakeModelRules;
extern p_GetVECTAB pGetVECTAB;

// end: About platform
//---------------------------------------------------------------------------
#endif

