//---------------------------------------------------------------------------
#ifndef u_gl_panelH
#define u_gl_panelH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
#include <GL/gl.h>
#include <GL/glu.h>
//---------------------------------------------------------------------------
#include "u_fogard_pol_model.h"
//---------------------------------------------------------------------------
typedef void __fastcall(__closure *TDisplayFunc)(void);

class TGLDrawPanel : public TPanel
{
private:
    BOOL bSetupPixelFormat(HDC hdc);

protected:
    void __fastcall PanelResize( TObject *Sender );
    void __fastcall PanelKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);

    GLfloat X; // Translate screen to x direction (left or right)
    GLfloat Y; // Translate screen to y direction (up or down)
    GLfloat Z; // Translate screen to z direction (zoom in or out)
    GLfloat rotX; // Rotate screen on x axis
    GLfloat rotY; // Rotate screen on y axis
    GLfloat rotZ; // Rotate screen on z axis
    int mX; // mouse X
    int mY; // mouse Y

    bool ShowAxes;
    GLfloat optZ; // Optimal Z moving back to view entire model
    // normal vectors - gorizontal (X) ans vertical (Y) and Z
    GLdouble vX[3], vY[3], vZ[3];
    // find vX[3], vY[3], vZ[3]
    void __fastcall FindNormals( bool anX = true, bool anY = true,
      bool anZ = false);
    GLfloat P0[3]; // Object center
    GLdouble off0[3]; // Off of (0,0,0)

    TDisplayFunc DisplayFunc;
 	  Tfogard_pol_model_calc *calc;

    int MouseWheelTick;
    int A_KeyTick, D_KeyTick, W_KeyTick, S_KeyTick;

public:

    HDC   ghDC;
    HGLRC ghRC;
    //
    double speed_weel;
    //+-

  __fastcall TGLDrawPanel( TComponent* Owner,
    Tfogard_pol_model_calc *a_calc = NULL);
  __fastcall ~TGLDrawPanel(void);
  __fastcall GLvoid DrawScene(GLvoid);
  __fastcall GLvoid Initialize(GLvoid);

	virtual void __fastcall Paint(void);
	virtual void __fastcall CreateWnd(void);
  virtual void __fastcall DestroyWnd(void);
  virtual void __fastcall CreateParams(TCreateParams &Params);

  DYNAMIC void __fastcall MouseDown(TMouseButton Button,
    Classes::TShiftState Shift, int X, int Y);
  DYNAMIC void __fastcall MouseUp( TMouseButton Button,
	TShiftState Shift, int aX, int aY);
  DYNAMIC void __fastcall MouseMove(Classes::TShiftState Shift, int X, int Y);
  DYNAMIC bool __fastcall DoMouseWheel(Classes::TShiftState Shift,
    int WheelDelta, const Types::TPoint &MousePos);
  DYNAMIC void __fastcall KeyPress(wchar_t &Key);

  void __fastcall SetDisplayFunc(TDisplayFunc DF);

  void __fastcall CenterToPoint( GLfloat pX, GLfloat pY, GLfloat pZ);

  void __fastcall OnMessage(TMessage & Message);
  BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_SETFOCUS, TMessage, OnMessage)
  END_MESSAGE_MAP(TComponent)

};
//---------------------------------------------------------------------------
#endif

