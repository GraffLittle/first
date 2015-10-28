//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "WaitDataUnit.h"
#include "UWokerWithLA.h"
extern TWorkWithLA *objWorkerLA;
#pragma package(smart_init)
//---------------------------------------------------------------------------
EXCEPTION_POINTERS *exxp;
EXCEPTION_RECORD er;
CONTEXT cntxt;
//---------------------------------------------------------------------------
AnsiString __fastcall LastErrorMessage(AnsiString Capt, int ErrorNumber = -1)
{
  AnsiString res = "";
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
  if (Capt != "")
    MessageBox( NULL, lpMsgBuf, Capt.c_str(), MB_OK | MB_ICONINFORMATION);
  res = lpMsgBuf; 
  LocalFree( lpMsgBuf );
  __CATCH
  return res;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TWaitDataThread::TWaitDataThread(bool CreateSuspended)
  : TThread(CreateSuspended)
{
  _size = 0;
  FirstByteOfData = NULL;
  OnTerminate = Term;
  FreeOnTerminate = true;

  ::InitializeCriticalSection(&m_AccessCriticalSection);
}
//---------------------------------------------------------------------------
void __fastcall TWaitDataThread::Execute()
{
  __TRY
  HANDLE hMapFile = NULL;
  LPVOID lpMapAddress = NULL;

  while (!hMapFile) {
    hMapFile = OpenFileMapping(FILE_MAP_READ, // read permission
      FALSE,                         // Do not inherit the name
      FILE_MAP_OBJ_TEXT);            // of the mapping object.
    }
  /*
  if (!hMapFile)
  {
    LastErrorMessage("Could not open file mapping object.");
    throw Exception("hMapFile == NULL");
  }
  */

  lpMapAddress = MapViewOfFile(hMapFile, // handle to mapping object
    FILE_MAP_READ,                     // read permission
    0,                                 // max. object size
    0,                                 // size of hFile
    0);                                // map entire file

  if (!lpMapAddress)
  {
    //ErrorHandler("Could not map view of file.");
    LastErrorMessage("Could not map view of file.");
    throw Exception("lpMapAddress == NULL");
  }

  FirstByteOfData = (TInOutData*)lpMapAddress;

  long i = 0;
  while (!Terminated) {
    if (FirstByteOfData->lNum != i) {
      ::EnterCriticalSection(&m_AccessCriticalSection);
      i = FirstByteOfData->lNum;
      _size = FirstByteOfData->lSize;
      memcpy( _data, FirstByteOfData->ch, _size);
      ::LeaveCriticalSection(&m_AccessCriticalSection);
      Synchronize(UpdateCaption);
      }
    }
  __CATCH
}
//---------------------------------------------------------------------------
void __fastcall TWaitDataThread::UpdateCaption(void)
{
  if (objWorkerLA && objWorkerLA->objCOMM)
    objWorkerLA->objCOMM->NewMessage(_data,_size);
}
//---------------------------------------------------------------------------
void __fastcall TWaitDataThread::Term(System::TObject* Sender)
{
  ::DeleteCriticalSection(&m_AccessCriticalSection);
  //MainForm->Button1->Enabled = true;
}
//---------------------------------------------------------------------------

