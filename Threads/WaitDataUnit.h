//---------------------------------------------------------------------------

#ifndef WaitDataUnitH
#define WaitDataUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
#define FILE_MAP_OBJ_TEXT TEXT("MyFileMappingObject")
#define FILE_MAP_OBJ_SIZE 0x00FF
typedef struct _tagInOutData{
  long lNum;
  long lSize;
  char ch[];
  }
    TInOutData;
//---------------------------------------------------------------------------
class TWaitDataThread : public TThread
{
private:
  char _data[1024];
  UINT _size;
  TInOutData *FirstByteOfData;
  void __fastcall UpdateCaption(void);
  CRITICAL_SECTION m_AccessCriticalSection;
protected:
  void __fastcall Execute();
public:
  __fastcall TWaitDataThread(bool CreateSuspended);
  void __fastcall Term(System::TObject* Sender);
};
//---------------------------------------------------------------------------
#define __TRY0 try {
#define __CATCH0 } catch(Exception *E) { \
    ShowMessage(E->Message + "\nFile: " + __FILE__ + "\nFunction: " + __FUNC__ \
      + "\nLine: " + __LINE__); }

extern EXCEPTION_POINTERS *exxp;
extern EXCEPTION_RECORD er;
extern CONTEXT cntxt;

#define __TRY try { try {
#define __CATCH } catch(Exception *E) { \
    ShowMessage(E->Message + "\nFile: " + __FILE__ + "\nFunction: " + __FUNC__ \
       + "\nLine: " + __LINE__); } \
    } __except((int)(exxp = GetExceptionInformation())) { \
      er = *(exxp->ExceptionRecord); cntxt = *(exxp->ContextRecord); \
      ShowMessage(AnsiString("Non-VCL exception\nFile: ") + __FILE__ + \
        "\nFunction: " + __FUNC__ + "\nLine: " + __LINE__); }

//---------------------------------------------------------------------------
#endif

