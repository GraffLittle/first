//---------------------------------------------------------------------------
#define INITGUID
#include <vcl.h>
#pragma hdrstop

//#include <objbase.h>                  //Функции и стандартные интерфейсы библиотеки COM
#include <dinput.h>                     //Объявление интерфейсов и констант DirectInput

#include <string.h>
#include <tchar.h>

#include <math.h>

//---------------------------------------------------------------------------
//#include <minmax.h>
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
//---------------------------------------------------------------------------
#include "MainUnit.h"
#include "UOGL.h"
#include "CommMessages.h"
#include "ModelUnit.h"
#include "DevTypeString.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VaClasses"
#pragma link "VaComm"
#pragma resource "*.dfm"
TMainForm *MainForm;
IDirectInput8 *DIHandle;                //Объект IDirectInput8
HRESULT res;                            //Результат выполнения функции

typedef struct _TDI8DevRepresent
{
  IDirectInputDevice8 *DIDevHandle;      //Устройство
  DIDEVICEINSTANCE DevInfo;              //Информация о устройстве
  _TDI8DevRepresent(LPCDIDEVICEINSTANCE DIDevDesc){
    DIDevHandle = NULL;
    memcpy(&DevInfo, DIDevDesc, sizeof(DIDEVICEINSTANCE));
  }
}
  TDI8DevRepresent;

BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
//---------------------------------------------------------------------------
TDI8DevRepresent *DevRepresent = NULL;
//---------------------------------------------------------------------------
#define LENGTH_DEV_NAME 40
#define BUTTON_DOWN 0x80
#define NUM_OF_ACTIONS 14

// Game action constants
// If you make changes to this list, make a corresponding change to the
// NUM_OF_ACTIONS global constant.
enum GAME_ACTIONS {
  KREN = 0,      // Separate inputs are needed in this case for
  TANG,          //   Walk/Left/Right because the joystick uses an
  KURS,          //   axis to report both left and right
  POWER,
  FIRE,
  FIRE2,
  QUIT,

  JB12,
  JB7,
  JB3,
  JB4,
  JB5,
  JB6,

  J_POV
};

// Convenience wrapper for device pointers
struct DeviceState
{
  LPDIRECTINPUTDEVICE8 pDevice;   // Pointer to the device
  TCHAR szName[LENGTH_DEV_NAME];  // Friendly name of the device
  bool  bAxisRelative;            // Relative x-axis data flag
  DWORD dwNullInput[NUM_OF_ACTIONS];  // Arrays of the current input values and
  DWORD dwInput[NUM_OF_ACTIONS];  // Arrays of the current input values and
  DWORD dwPaint[NUM_OF_ACTIONS];  //   values when last painted
  bool  bMapped[NUM_OF_ACTIONS];  // Flags whether action was successfully
};                                  //   mapped to a device object

DeviceState g_aDevice = {0};

bool NullInput = true;

// Friendly names for action constants are used by DirectInput for the
// built-in configuration UI
const TCHAR *ACTION_NAMES[NUM_OF_ACTIONS] =
{
  TEXT("Kren"),
  TEXT("Tangazh"),
  TEXT("Kourse"),
  TEXT("Power"),
  TEXT("Fire"),
  TEXT("Fire2"),
  TEXT("Quit"),

  TEXT("JB12"),
  TEXT("JB7"),
  TEXT("JB3"),
  TEXT("JB4"),
  TEXT("JB5"),
  TEXT("JB6"),
  
  TEXT("J_POV")
};

// Map the game actions to the genre and keyboard constants.
DIACTION g_adiaActionMap[] =
{
  // Device input (joystick, etc.) that is pre-defined by DInput according
  // to genre type.
  // The genre for this app is Action->Flight Simulator - Combat Helicopter.
  { KREN,             DIAXIS_FLYINGH_BANK,        0,  ACTION_NAMES[KREN], },
  { TANG,             DIAXIS_FLYINGH_PITCH,       0,  ACTION_NAMES[TANG], },
  { KURS,             DIAXIS_FLYINGH_TORQUE,      0,  ACTION_NAMES[KURS], },
  { POWER,            DIAXIS_FLYINGH_COLLECTIVE,  0,  ACTION_NAMES[POWER], },
  { FIRE,             DIBUTTON_FLYINGH_FIRE,      0,  ACTION_NAMES[FIRE], },
  { FIRE2,            DIBUTTON_FLYINGH_WEAPONS, 0,  ACTION_NAMES[FIRE2], },
  // Buttons
  { JB12, DIBUTTON_FLYINGH_MENU, 0,  ACTION_NAMES[JB12], },
  { JB3,  DIBUTTON_FLYINGH_TARGET, 0,  ACTION_NAMES[JB3], },
  { JB4,  DIBUTTON_FLYINGH_COUNTER, 0,  ACTION_NAMES[JB4], },
  { JB5,  DIBUTTON_FLYINGH_FASTER_LINK, 0,  ACTION_NAMES[JB5], },
  { JB6,  DIBUTTON_FLYINGH_GEAR, 0,  ACTION_NAMES[JB6], },
  { JB7,  DIBUTTON_FLYINGH_FIRESECONDARY, 0,  ACTION_NAMES[JB7], },
  // Point-of-view indicator
  { J_POV,  DIHATSWITCH_FLYINGH_GLANCE, 0,  ACTION_NAMES[J_POV], }
};

inline DWORD GetNumOfMappings() { return sizeof(g_adiaActionMap)/sizeof(DIACTION); }

// This GUID must be unique for every game, and the same for every instance of
// this app. The GUID allows DirectInput to remember input settings.
// {3AFABAD0-D2C0-4514-B47E-65FEF9B5142F}
const GUID g_guidApp = { 0x3afabad0, 0xd2c0, 0x4514, { 0xb4, 0x7e, 0x65, 0xfe, 0xf9, 0xb5, 0x14, 0x2F } };

// Global variables
LPDIRECTINPUT8  g_pDI         = NULL;  // DirectInput access pointer
DIACTIONFORMAT  g_diaf        = {0};   // DIACTIONFORMAT structure, used for
                                       //   enumeration and viewing config
// Function prototypes
BOOL CALLBACK EnumDevicesCallback( LPCDIDEVICEINSTANCE, LPDIRECTINPUTDEVICE8, DWORD, DWORD, LPVOID );
//---------------------------------------------------------------------------
// MainForm
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
        : TForm(Owner)
{
  _kurs = 0, _tang = 0, _kren = 0, _H = 200;
  _cX = _cY = _vX = _vY = _vZ = 0.0;
  _cX = 6322222;
  _cY = 6622222;

  CoInitialize(NULL);             //Инициализация библиотеки COM

  hMapFile = NULL;
  lpMapAddress = NULL;
  ::InitializeCriticalSection(&m_AccessCriticalSection);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
  //Создание объекта DirectInput8
  res = CoCreateInstance(CLSID_DirectInput8,
          NULL,
          CLSCTX_INPROC_SERVER,
          IID_IDirectInput8,
          (void**)&DIHandle);
  if(FAILED(res)){
    Application->MessageBox("Ошибка создания объекта DirectInput8", "error", MB_OK|MB_ICONERROR);
    Application->Terminate();
  }

  //Инициализация объекта DirectInput8
  res = DIHandle->Initialize(HInstance, DIRECTINPUT_VERSION);
  if(res != DI_OK){
    Application->MessageBox("Ошибка инициализации DirectInput8", "warning", MB_OK|MB_ICONWARNING);
  }
  g_pDI = DIHandle;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
  TStringList *sl = new TStringList;
  __try {
    COMsBox->Items->Clear();
    vaComm->GetComPortNames(sl);
    COMsBox->Items->AddStrings(sl);
    COMsBox->ItemIndex = 0;
  } __finally { delete sl; }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  COMTimer->Enabled = false;

  DevRepresent->DIDevHandle->Release();

  DIHandle->Release();            //Удаление объекта
  CoUninitialize();               //Закрытие библиотеки COM

  vaComm->Close();

  CloseDataFileMaping();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FillActGrid(bool Full)
{
  if (Full) {
    ActGrid->RowCount = NUM_OF_ACTIONS;
    for( int i=0; i < NUM_OF_ACTIONS; i++ )
      ActGrid->Cells[0][i] = ACTION_NAMES[i];
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FillValsGrid(bool Full)
{
  if (Full) {
    ValsGrid->Cells[0][0] = "cX";
    ValsGrid->Cells[0][1] = "cY";
    ValsGrid->Cells[0][2] = "H";
    ValsGrid->Cells[0][3] = "Kurs";
    ValsGrid->Cells[0][4] = "Tang";
    ValsGrid->Cells[0][5] = "Kren";
    ValsGrid->Cells[0][6] = "vX";
    ValsGrid->Cells[0][7] = "vY";
    ValsGrid->Cells[0][8] = "vZ";
    }
  ValsGrid->Cells[1][0] = AnsiString::FormatFloat("0.000",_cX);
  ValsGrid->Cells[1][1] = AnsiString::FormatFloat("0.000",_cY);
  ValsGrid->Cells[1][2] = AnsiString::FormatFloat("0.000",_H);
  ValsGrid->Cells[1][3] = AnsiString::FormatFloat("0.000",_kurs);
  ValsGrid->Cells[1][4] = AnsiString::FormatFloat("0.000",_tang);
  ValsGrid->Cells[1][5] = AnsiString::FormatFloat("0.000",_kren);
  ValsGrid->Cells[1][6] = AnsiString::FormatFloat("0.000",_vX);
  ValsGrid->Cells[1][7] = AnsiString::FormatFloat("0.000",_vY);
  ValsGrid->Cells[1][8] = AnsiString::FormatFloat("0.000",_vZ);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Button1Click(TObject *Sender)
{
  // Setup action format for the actual gameplay
  ZeroMemory( &g_diaf, sizeof(DIACTIONFORMAT) );
  g_diaf.dwSize          = sizeof(DIACTIONFORMAT);
  g_diaf.dwActionSize    = sizeof(DIACTION);
  g_diaf.dwDataSize      = GetNumOfMappings() * sizeof(DWORD);
  g_diaf.guidActionMap   = g_guidApp;
  g_diaf.dwGenre         = DIVIRTUAL_FLYING_HELICOPTER;
  g_diaf.dwNumActions    = GetNumOfMappings();
  g_diaf.rgoAction       = g_adiaActionMap;
  g_diaf.lAxisMin        = -100;
  g_diaf.lAxisMax        = 100;
  g_diaf.dwBufferSize    = 16;
  _tcscpy( g_diaf.tszActionMap, _T("Flight Simulator - Combat Helicopter") );

  res = res | DIHandle->EnumDevices( DI8DEVTYPE_JOYSTICK,
    //DI8DEVCLASS_ALL, //DI8DEVTYPE_KEYBOARD,
    DIEnumDevicesCallback,
    this,                                  //Указатель на форму
    DIEDFL_ATTACHEDONLY); //DIEDFL_ALLDEVICES);
  if(res != DI_OK){
    Application->MessageBox("Ошибка определения устройств DirectInput8", "error", MB_OK|MB_ICONERROR);
  }

  FillActGrid(true);
  FillValsGrid(true);
  ValidateDIDevInfo(&(DevRepresent->DevInfo));

  oglForm->Show();
  oglForm->Width = oglForm->Width + 1;

  Button2->Enabled = true;
  Button3->Enabled = true;
  CheckTimer->Enabled = true;
  OpenCOMBtn->Enabled = true;
  NullBtn->Enabled = true;
}
//---------------------------------------------------------------------------
// Callbacks
//---------------------------------------------------------------------------
BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
  TMainForm *psForm = (TMainForm*)pvRef;

  DevRepresent = new TDI8DevRepresent(lpddi);

  MainForm->GroupBox2->Caption = lpddi->tszInstanceName;

  res = DIHandle->CreateDevice(DevRepresent->DevInfo.guidInstance, &DevRepresent->DIDevHandle, NULL);
  if(res != DI_OK){
    Application->MessageBox("Ошибка создания устройства", "warning", MB_OK|MB_ICONWARNING);
    return DIENUM_CONTINUE;
  }

  return EnumDevicesCallback( &(DevRepresent->DevInfo),
    DevRepresent->DIDevHandle,
    0, 0, 0);
}
//---------------------------------------------------------------------------
// Name: EnumDevicesCallback
// Desc: Callback function for EnumDevices. This particular function stores
//       a list of all currently attached devices for use on the input chart.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumDevicesCallback( LPCDIDEVICEINSTANCE lpddi,
                                  LPDIRECTINPUTDEVICE8 lpdid, DWORD dwFlags,
                                  DWORD dwRemaining, LPVOID pvRef )
{
  HRESULT hr;

  // Build the action map against the device
  if( FAILED(hr = lpdid->BuildActionMap( &g_diaf, NULL, DIDBAM_DEFAULT )) )
      // There was an error while building the action map. Ignore this
      // device, and contine with the enumeration
      return DIENUM_CONTINUE;


  // Inspect the results
  for( UINT i=0; i < g_diaf.dwNumActions; i++ )
  {
      DIACTION *dia = &(g_diaf.rgoAction[i]);

      if( dia->dwHow != DIAH_ERROR && dia->dwHow != DIAH_UNMAPPED )
          g_aDevice.bMapped[dia->uAppData] = TRUE;
  }

  // Set the action map
  if( FAILED(hr = lpdid->SetActionMap( &g_diaf, NULL, DIDSAM_DEFAULT )) )
  {
      // An error occured while trying the set the action map for the
      // current device. Clear the stored values, and continue to the
      // next device.
      ZeroMemory( g_aDevice.bMapped,
           sizeof(g_aDevice.bMapped) );
      return DIENUM_CONTINUE;
  }

  // The current device has been successfully mapped. By storing the
  // pointer and informing COM that we've added a reference to the
  // device, we can use this pointer later when gathering input.
  g_aDevice.pDevice = lpdid;
  lpdid->AddRef();

  // Store the device's friendly name for display on the chart.
  _tcsncat( g_aDevice.szName, lpddi->tszInstanceName, LENGTH_DEV_NAME-5 );

  if( _tcslen( lpddi->tszInstanceName ) > LENGTH_DEV_NAME-5 )
      _tcscat( g_aDevice.szName, TEXT("...") );

  // Store axis absolute/relative flag
  DIPROPDWORD dipdw;
  dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
  dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
  dipdw.diph.dwObj        = 0;
  dipdw.diph.dwHow        = DIPH_DEVICE;

  hr = lpdid->GetProperty( DIPROP_AXISMODE, &dipdw.diph );
  if (SUCCEEDED(hr))
      g_aDevice.bAxisRelative = ( DIPROPAXISMODE_REL == dipdw.dwData );

  // Ask for the next device
  return DIENUM_STOP;
}
//-----------------------------------------------------------------------------
/*Обновление информации о устройстве : */
void TMainForm::ValidateDIDevInfo(LPCDIDEVICEINSTANCE DIDevInfo)
{
  Memo1->Clear();

  Memo1->Lines->Add(AnsiString(".guidInstance = ") + GUIDToString(DIDevInfo->guidInstance));
  Memo1->Lines->Add(AnsiString(".guidProduct  = ") + GUIDToString(DIDevInfo->guidProduct));
  Memo1->Lines->Add(AnsiString(".dwDevType = ") + DIDevInfo->dwDevType);
  Memo1->Lines->Add(".dwDevType string - " + DeviceTypeAndSubtype(DIDevInfo->dwDevType));
  Memo1->Lines->Add(AnsiString(".tszInstanceName  = ") + DIDevInfo->tszInstanceName);
  Memo1->Lines->Add(AnsiString(".tszProductName  = ") + DIDevInfo->tszProductName);
  Memo1->Lines->Add(AnsiString(".guidFFDriver  = ") + GUIDToString(DIDevInfo->guidFFDriver));
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Button2Click(TObject *Sender)
{
  DevRepresent->DIDevHandle->RunControlPanel(Handle, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Button3Click(TObject *Sender)
{
  // The user has clicked on the "View Configuration" button.
  // DirectInput has supports a native UI for viewing or
  // changing the current device mappings.
  DICONFIGUREDEVICESPARAMS diCDParams;

  ZeroMemory( &diCDParams, sizeof(DICONFIGUREDEVICESPARAMS) );
  diCDParams.dwSize = sizeof(DICONFIGUREDEVICESPARAMS);
  diCDParams.dwcFormats = 1;
  diCDParams.lprgFormats = &g_diaf;
  diCDParams.hwnd = Handle;

  g_pDI->ConfigureDevices( NULL, &diCDParams, DICD_DEFAULT, NULL );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CheckTimerTimer(TObject *Sender)
{
  CheckInput();
}
//---------------------------------------------------------------------------
// Other
//---------------------------------------------------------------------------
// Name: CheckInput
// Desc: Poll the attached devices and update the display
//-----------------------------------------------------------------------------
void TMainForm::CheckInput(void)
{
  HRESULT hr;

  LPDIRECTINPUTDEVICE8 pdidDevice = g_aDevice.pDevice;
  DIDEVICEOBJECTDATA rgdod[NUM_OF_ACTIONS];
  DWORD dwItems = NUM_OF_ACTIONS;

  hr = pdidDevice->Acquire();
  if( FAILED(hr) ) return;
  hr = pdidDevice->Poll();
  if( FAILED(hr) ) return;
  hr = pdidDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
                                  rgdod, &dwItems, 0 );
  if( FAILED(hr) ) return;

  for( DWORD j=0; j<dwItems; j++ )
  {
    UINT_PTR dwAction = rgdod[j].uAppData;
    DWORD dwData = rgdod[j].dwData;

    if (dwAction == POWER) dwData = -dwData;
    if (NullInput) g_aDevice.dwNullInput[dwAction] = dwData;
    g_aDevice.dwInput[dwAction] = dwData;
  }
  NullInput = false;

  // Paint new data
  PaintChart();
}
//-----------------------------------------------------------------------------
// Name: PaintChart
// Desc: Output the device/action chart
//-----------------------------------------------------------------------------
void TMainForm::PaintChart(void)
{
  // ____________________________________________________________________
  // BEGIN paint stored data

  // Cycle through each game action
  for( int i=0; i < NUM_OF_ACTIONS; i++ )
  {
    DWORD dwData = g_aDevice.dwInput[i] - g_aDevice.dwNullInput[i];
    // If the action data has not changed since the last painting, we
    // can skip to the next action to avoid flicker
    if( !g_aDevice.bMapped[i] )
      ActGrid->Cells[1][i] = "X";
    else
    if( dwData != g_aDevice.dwPaint[i] ) {
      ActGrid->Cells[1][i] = (int)(dwData);
      g_aDevice.dwPaint[i] = dwData;
      }
    switch (i) {
      case KREN: _kren += ((int)dwData) * 0.1; //0.45;
        break;
      case TANG: _tang += ((int)dwData) * 0.1; //0.45;
        break;
      case KURS: _kurs += ((int)dwData) * 0.1; //0.45;
        break;
      case POWER: _H += ((int)dwData) * 0.1; //0.5;
        break;
      case FIRE:
        break;
      case JB12: case QUIT: if (dwData) PostQuitMessage(0);
        break;
      case JB7: break;
      case JB3: break;
      case JB4: break;
      case JB5: break;
      case JB6: break;
      case J_POV: break;
      default: ; //can't be
      } // switch (i)
  } // for i
  // ____________________________________________________________________
  // END paint stored data

  _kurs += 3.0 * sin(_kren * 3.14159265358979323846 / 180.0) * dt;

  if (_H < 20) _H = 20;
  if (_kurs < 0) _kurs += 360;
  else if (_kurs > 360) _kurs -= 360;
  if (_tang < -60) _tang = -60;
  else if (_tang > 60) _tang = 60;
  if (_kren < -60) _kren = -60;
  else if (_kren > 60) _kren = 60;
  oglForm->SetFlyParams( _H, _kurs, _tang, _kren);

  // mine Flying model
  double ugol = _kurs * M_PI / 180.0;
  _vX = -_tang * sin(ugol) + _kren * cos(ugol);
  _vY = -_tang * cos(ugol) - _kren * sin(ugol);
  _cX += _vY * dt; // на карте поменяны X Y
  _cY += _vX * dt;
  // end mine

  /*
  //DWORD dwData = g_aDevices[iDevice].dwInput[i];
  //g_aDevices[1].dwInput[KREN_LEFT]

  // BEGIN: Flying Model
  X_tang = 12 * (-g_aDevices[1].dwInput[TANG_DOWN] + g_aDevices[1].dwInput[TANG_UP]);
  X_bok = 1 * (-g_aDevices[1].dwInput[KREN_LEFT] + g_aDevices[1].dwInput[KREN_RIGHT]);
  X_taga = 1 * (-g_aDevices[1].dwInput[POWER_DOWN] + g_aDevices[1].dwInput[POWER_UP]);;
  //Rud=0;
  //V0=100;
  H0 = _H;
  kurs0 = _kurs;
  X0 = _cX;
  Y0 = _cY;

pr(
  // in
  X0,Y0,kurs0,H0,V0,X_tang,X_bok,X_taga,te,
  X1,X2,X3,X4,X5,X6,X7,X8,X9,X10,X11,X12,Rud,
  // out
  te,X1,X2,X3,X4,X5,X6,X7,X8,X9,X10,X11,X12,Rud,al,tang,V,H,kurs,X,Y);

  //V0 = V;
  _cX = X;
  _cY = Y;
  _H = H;
  _kurs = kurs;
  _tang = tang;
  // END: Flying Model
  */

  FillValsGrid();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define MAX_MESSAGE_LENGHT 4096
int PackNum = 0;

void __fastcall TMainForm::WritePack2COM(void)
{
  SYSTEMTIME _st;
  double _values[25];
  setmem( _values, 25*sizeof(double), 0);
  UINT _id_message = 160;
  char _message[MAX_MESSAGE_LENGHT]; // = "";

  _values[8] = _cX;
  _values[9] = _cY;
  _values[10] = _H;
  _values[11] = _kurs;
    //_values[11] = _values[11]-((int)_values[11]/360)*360;
  _values[12] = _tang;
  _values[13] = _kren;
  _values[14] = _vX;
  _values[15] = _vY;
  _values[16] = _vZ;

  ::GetSystemTime(&_st);

  _message[0] = 0;
  DWORD dwBytes = COMMS_code_message( _id_message, _message, _values );
  if (vaComm->PortNum) vaComm->WriteBuf( _message, dwBytes);
  WriteToMapAddress( _message, dwBytes);
  /*
  structMessage *newMess = new structMessage;
  __try {
    newMess->setMessage( _id_message, _message, dwBytes, _st);
    vaComm->WriteBuf( newMess, dwBytes);
  __finally {
    delete newMess;
    }
  */
  Label1->Caption = IntToStr(PackNum) + " : " + vaComm->WriteBufUsed();
  PackNum++;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OpenCOMBtnClick(TObject *Sender)
{
  __TRY
  COMTimer->Enabled = false;
  vaComm->Close();
  AnsiString _pn = COMsBox->Items->Strings[COMsBox->ItemIndex];
  vaComm->PortNum = StrToInt(_pn.SubString(4,2));
  vaComm->Open();
  __CATCH
  Button1->Enabled = false;
  Button2->Enabled = false;
  Button3->Enabled = false;
  COMTimer->Enabled = true;
  StartDataFileMaping();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::COMTimerTimer(TObject *Sender)
{
  WritePack2COM();
}
//---------------------------------------------------------------------------
// DataFileMaping
//---------------------------------------------------------------------------
long iI = 0L;

EXCEPTION_POINTERS *exxp;
EXCEPTION_RECORD er;
CONTEXT cntxt;

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
void __fastcall TMainForm::StartDataFileMaping(void)
{
  __TRY
  hMapFile = CreateFileMapping(
    INVALID_HANDLE_VALUE, // current file handle (operating-system paging file)
    NULL,                          // default security
    PAGE_READWRITE,                // read/write permission
    0,                             // max. object size
    FILE_MAP_OBJ_SIZE,             // size of hFile
    FILE_MAP_OBJ_TEXT);            // name of mapping object

  if (!hMapFile)
  {
    //ErrorHandler("Could not create file mapping object.");
    LastErrorMessage("Could not create file mapping object.");
    throw Exception("hMapFile == NULL");
  }
  if (hMapFile != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
    return;
    }

  lpMapAddress = MapViewOfFile(hMapFile, // handle to mapping object
    FILE_MAP_ALL_ACCESS,               // read/write permission
    0,                                 // max. object size
    0,                                 // size of hFile
    0);                                // map entire file

  if (!lpMapAddress)
  {
    //ErrorHandler("Could not map view of file.");
    LastErrorMessage("Could not map view of file.");
    throw Exception("lpMapAddress == NULL");
  }

  WriteToMapAddress("",1);
  __CATCH
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CloseDataFileMaping(void)
{
  __TRY
  if (!lpMapAddress || !hMapFile)
    return;
  if (!UnmapViewOfFile(lpMapAddress))
    LastErrorMessage("!UnmapViewOfFile()");
  CloseHandle(hMapFile);
  ::DeleteCriticalSection(&m_AccessCriticalSection);
  __CATCH
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::WriteToMapAddress( char *_data, UINT _size)
{
  __TRY
  if (!lpMapAddress) return;
  TInOutData *FirstByteOfData = (TInOutData*)lpMapAddress;
  ::EnterCriticalSection(&m_AccessCriticalSection);
  memcpy( &(FirstByteOfData->ch), _data, _size);
  FirstByteOfData->lNum = iI++;
  FirstByteOfData->lSize = _size;
  ::LeaveCriticalSection(&m_AccessCriticalSection);
  __CATCH
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::NullBtnClick(TObject *Sender)
{
  for( int i=0; i < NUM_OF_ACTIONS; i++ )
    g_aDevice.dwNullInput[i] = g_aDevice.dwInput[i];
}
//---------------------------------------------------------------------------

