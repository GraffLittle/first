//---------------------------------------------------------------------------
#ifndef UWaitThreadH
#define UWaitThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "iec.h"
//---------------------------------------------------------------------------
class TWaitThread : public TThread
{
private:
  TIECBase *_ServerObject;
  TIECBase *_ClientObject;
  HANDLE _NewDataEvent1;
  HANDLE _NewDataEvent2;
  HANDLE _ServerSemaph;
  HANDLE _ClientSemaph;
  TStrings *_Out;
protected:
  void __fastcall Execute();
public:
  __fastcall TWaitThread( bool CreateSuspended,
    HANDLE NewDataEvent1, HANDLE NewDataEvent2,
    HANDLE ServerSemaph, HANDLE ClientSemaph,
    TIECBase *ServerObject, TIECBase *ClientObject,
    TStrings *Out);
};
//---------------------------------------------------------------------------
#endif

