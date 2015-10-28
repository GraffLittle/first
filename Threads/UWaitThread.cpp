//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "UWaitThread.h"
#include "MainUnit.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TWaitThread::TWaitThread( bool CreateSuspended,
  HANDLE NewDataEvent1, HANDLE NewDataEvent2,
  HANDLE ServerSemaph, HANDLE ClientSemaph,
  TIECBase *ServerObject, TIECBase *ClientObject, TStrings *Out)
  : TThread(CreateSuspended)
{
  _NewDataEvent1 = NewDataEvent1;
  _NewDataEvent2 = NewDataEvent2;
  _ServerSemaph = ServerSemaph;
  _ClientSemaph = ClientSemaph;
  _ServerObject = ServerObject;
  _ClientObject = ClientObject;
  _Out = Out;

  FreeOnTerminate = true;
}
//---------------------------------------------------------------------------
AnsiString SysMess2String(TIECSystemMessage *sm)
{
  TDateTime DT;
  DT.Val = sm->DateTime;
  return DT.DateTimeString() + " : system message 0x" +
    IntToHex((int)sm->Code,8) + " (" + sm->Comment + ")";
}
//---------------------------------------------------------------------------
void __fastcall TWaitThread::Execute()
{
  HANDLE h_arr[4] = {
    _NewDataEvent1, _NewDataEvent2, _ServerSemaph, _ClientSemaph};
  char buf[1024];
  while (!Terminated) {
    //WaitForSingleObject(_NewDataEvent,INFINITE);
    DWORD res = WaitForMultipleObjects( 4, h_arr, FALSE, 1000); //INFINITE);
    if (res == WAIT_TIMEOUT) continue;
    if (res == WAIT_OBJECT_0) { // new server data
      _ServerObject->ReadBuffer( buf, 1024);
      _Out->Add(buf);
      } // if wait obj 0
    if (res == (WAIT_OBJECT_0+1)) { // new client data
      _ClientObject->ReadBuffer( buf, 1024);
      _Out->Add(buf);
      } // if wait obj 1
    if (res == (WAIT_OBJECT_0+2)) // server system message
      _Out->Add( SysMess2String( _ServerObject->GetLastSystemMessage() ));
    if (res == (WAIT_OBJECT_0+3)) // client system message
      _Out->Add( SysMess2String( _ClientObject->GetLastSystemMessage() ));
    } // while
}
//---------------------------------------------------------------------------

