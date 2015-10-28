//---------------------------------------------------------------------------

#ifndef MainUnitH
#define MainUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include "VaClasses.hpp"
#include "VaComm.hpp"
//---------------------------------------------------------------------------
// for COM
//---------------------------------------------------------------------------
struct structMessageFile
{
  UINT       message_ID;
  UINT       message_Size; // размера сообщени€ в байтах
  SYSTEMTIME date_time;
  BYTE       incoming_message; // 0-вход€щее, 1 - исход€щее
  structMessageFile()
  {
    message_ID = message_Size = 0;
    incoming_message = 0;
  }
};
//---------------------------------------------------------------------------
struct structMessage: public structMessageFile
{
  char *message_body;
  structMessage()
  {
    message_body = NULL;
  }
  ~structMessage()
  {
    delete[] message_body;
    message_body = NULL;
  }
  void setMessage( const UINT &_id_message, const char *_message,
    const UINT &_size_message, const SYSTEMTIME &_datetime )
  {
    if ( message_body != NULL ) return;
    message_body = new char[ _size_message ];
    memcpy( message_body, _message, _size_message );
    message_ID   = _id_message;
    message_Size = _size_message;
    memcpy( &date_time, &_datetime, sizeof( SYSTEMTIME ) );
  }
};
//---------------------------------------------------------------------------
// Main form
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
  TGroupBox *GroupBox2;
  TMemo *Memo1;
  TButton *Button1;
  TButton *Button3;
  TTimer *CheckTimer;
  TStringGrid *ActGrid;
  TTimer *COMTimer;
  TComboBox *COMsBox;
  TVaComm *vaComm;
  TButton *OpenCOMBtn;
  TLabel *Label1;
  TStringGrid *ValsGrid;
  TButton *NullBtn;
  TButton *Button2;
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall Button1Click(TObject *Sender);
  void __fastcall Button2Click(TObject *Sender);
  void __fastcall Button3Click(TObject *Sender);
  void __fastcall CheckTimerTimer(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall OpenCOMBtnClick(TObject *Sender);
  void __fastcall COMTimerTimer(TObject *Sender);
  void __fastcall NullBtnClick(TObject *Sender);
private:
  void __fastcall FillActGrid(bool Full = false);
  void __fastcall FillValsGrid(bool Full = false);
  void ValidateDIDevInfo(LPCDIDEVICEINSTANCE DIDevInfo);
  void CheckInput(void);
  void PaintChart(void);

  double _kurs, _tang, _kren, _H;
  double _cX, _cY, _vX, _vY, _vZ;

  void __fastcall WritePack2COM(void);

private:
  HANDLE hMapFile;
  LPVOID lpMapAddress;
  CRITICAL_SECTION m_AccessCriticalSection;
  void __fastcall WriteToMapAddress( char *_data, UINT _size);
  void __fastcall StartDataFileMaping(void);
  void __fastcall CloseDataFileMaping(void);

public:
  __fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
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
#define FILE_MAP_OBJ_TEXT TEXT("MyFileMappingObject")
#define FILE_MAP_OBJ_SIZE 0x00FF
typedef struct _tagInOutData{
  /*
  union _time {
    LONGLONG L;
    SYSTEMTIME systime;
    } time;
  */
  long lNum;
  long lSize;
  char ch[];
  }
    TInOutData;
//---------------------------------------------------------------------------
#endif

