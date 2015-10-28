//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "u_gl_panel.h"
//#include "MainUnit.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//
/*
static inline void ValidCtrCheck(TGLDrawPanel *)
{
  new TGLDrawPanel(NULL);
}
*/
//---------------------------------------------------------------------------
__fastcall TGLDrawPanel::TGLDrawPanel( TComponent* Owner,
  Tfogard_pol_model_calc *a_calc)
  : TPanel(Owner), ShowAxes(false), calc(a_calc)
{
  Align = alClient;
  //ControlStyle = ControlStyle >> csCaptureMouse;
  BevelOuter = bvNone;
  FullRepaint = false;

  //Parent = (TWinControl*)Owner;

  OnResize = PanelResize;
  //OnKeyDown = PanelKeyDown;

  //SendToBack();
  //BringToFront();

  ShowAxes = true;

  P0[0] = 0.0f; P0[1] = 0.0f; P0[2] = 0.0f;
  optZ = 15.0f;
  if (calc) // find max Z to move back
  {
    if(calc->free_volume.Count>0)
    {
    P0[0] = calc->model_3d.min_max[1]+calc->model_3d.min_max[0];
    P0[1] = calc->model_3d.min_max[3]+calc->model_3d.min_max[2];
    P0[2] = calc->model_3d.min_max[5]+calc->model_3d.min_max[4];
    /*
    optZ = P0[0];
    if (optZ < P0[1]) optZ = P0[1];
    if (optZ < P0[2]) optZ = P0[2];  */

    //optZ /= 1.5;
    P0[0] /= 2; P0[1] /= 2; P0[2] /= 2;

    //моя коляка
    optZ = sqrt(P0[0]*P0[0]+P0[1]*P0[1]+P0[2]*P0[2]);
    };
  };

  X = 0.0f; Y = 0.0f; Z = optZ;
  rotX = -45.0f; rotY = 0.0f; rotZ = -135.0f;

  vX[0] = 1.0f; vX[1] = 0.0f; vX[2] = 0.0f;
  vY[0] = 0.0f; vY[1] = 1.0f; vY[2] = 0.0f;
  vZ[0] = 0.0f; vZ[1] = 0.0f; vZ[2] = 1.0f;

  off0[0] = 0.0f; off0[1] = 0.0f; off0[2] = 0.0f;

  speed_weel=500.0;
}
//---------------------------------------------------------------------------
__fastcall TGLDrawPanel::~TGLDrawPanel(void)
{
  try {
    wglMakeCurrent( ghDC, ghRC);
  } catch(...) {  }
  try { wglMakeCurrent( NULL, NULL); } catch(...) { }
  if (ghRC) try { wglDeleteContext(ghRC); } catch(...) { }
}
//---------------------------------------------------------------------------
/*
namespace GLDrawPanel
{
  void __fastcall PACKAGE Register()
  {
     TComponentClass classes[1] = {__classid(TGLDrawPanel)};
     RegisterComponents("Little More", classes, 0);
  }
}
*/
//---------------------------------------------------------------------------
BOOL TGLDrawPanel::bSetupPixelFormat(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd, *ppfd;
    int pixelformat;
    ppfd = &pfd;
    ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    ppfd->nVersion = 1;
    ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    //ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER|PFD_TYPE_RGBA|PFD_DEPTH_DONTCARE;
    ppfd->dwLayerMask = PFD_MAIN_PLANE;
    ppfd->iPixelType = PFD_TYPE_RGBA;
    ppfd->cColorBits = 16;
    ppfd->cDepthBits = 16;
    ppfd->cAccumBits = 0;
    ppfd->cStencilBits = 0;
    if ((pixelformat = ChoosePixelFormat(hdc, ppfd)) == 0)
    {
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
        return FALSE;
    }
    if (SetPixelFormat(hdc, pixelformat, ppfd) == FALSE)
    {
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
        return FALSE;
    }
    return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::CreateParams(TCreateParams &Params)
{
  TPanel::CreateParams(Params);
  Params.WindowClass.style |= CS_OWNDC;
  //Params.WindowClass.style &= ~WS_EX_CONTROLPARENT;
}
//---------------------------------------------------------------------------
GLvoid __fastcall TGLDrawPanel::DrawScene(GLvoid)
{
    if (!ghDC) return;
  	wglMakeCurrent( ghDC, ghRC);

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushMatrix(); 	// It is important to push the Matrix before calling
			// glRotatef and glTranslatef

		//glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();

		//gluLookAt( X, Y, Z, X, Y, Z-optZ, 0.0, 1.0, 0.0);

    //glRotatef(rotX,1.0,0.0,0.0); // Rotate on x
    //glRotatef(rotY,0.0,1.0,0.0); // Rotate on y
    //glRotatef(rotY,0.0,cos(rotX * M_PI / 180.0),sin(rotX * M_PI / 180.0));
    //glRotatef(rotZ,0.0,0.0,1.0); // Rotate on z

    //glTranslatef(X, Y, Z); 	// Translates the screen left or right,
			// up or down or zoom in zoom out

    if (ShowAxes)
    {
      glTranslated( -off0[0], -off0[1], -off0[2]);
      // Draw the positive side of the lines x,y,z
      glBegin(GL_LINES);
        glColor4f( 0.5, 0.5, 0.5, 0.9f);
        glVertex3f(0,0,0);
        glVertex3f(10,0,0);
        glVertex3f(0,0,0);
        glVertex3f(0,10,0);
        glVertex3f(0,0,0);
        glVertex3f(0,0,10);
      glEnd();

      // Dotted lines for the negative sides of x,y,z
      glEnable(GL_LINE_STIPPLE); 	// Enable line stipple to use a dotted pattern for the lines
      glLineStipple(1, 0x0101); 	// Dotted stipple pattern for the lines
      glBegin(GL_LINES);
        glColor4f( 0.5, 0.5, 0.5, 0.9f);
        glVertex3f(-10,0,0);
        glVertex3f(0,0,0);
        glVertex3f(0,0,0);
        glVertex3f(0,-10,0);
        glVertex3f(0,0,0);
        glVertex3f(0,0,-10);
      glEnd();
      glDisable(GL_LINE_STIPPLE); 	// Disable the line stipple

      glTranslated( off0[0], off0[1], off0[2]);
    } // ShowAxes

    if (calc) glTranslatef( -P0[0], -P0[1], -P0[2]);
    if (DisplayFunc) DisplayFunc();
    if (calc) glTranslatef( P0[0], P0[1], P0[2]);

    glPopMatrix(); 		// Don't forget to pop the Matrix
    SwapBuffers(ghDC);
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::PanelResize( TObject *Sender )
{
    if (!ghDC) return;
    wglMakeCurrent( ghDC, ghRC);

    int w = ClientWidth;
    int h = ClientHeight;
    glViewport ( 0, 0, (GLsizei) w, (GLsizei) h); // Set the viewport

    glMatrixMode (GL_PROJECTION); 	// Set the Matrix mode
    glLoadIdentity ();
    gluPerspective(75, (GLfloat) w /(GLfloat) h , 0.10, 10000.0);

    glMatrixMode(GL_MODELVIEW);
    DrawScene();
}
//---------------------------------------------------------------------------
GLvoid __fastcall TGLDrawPanel::Initialize(GLvoid)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    //glAlphaFunc( GL_LESS, GL_GREATER);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_ALPHA_TEST);

   	glBlendFunc(GL_ONE,GL_ONE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
    glEnable(GL_BLEND);
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::FindNormals( bool anX, bool anY, bool anZ)
{
  if (!ghDC) return;
  wglMakeCurrent( ghDC, ghRC);

  try {
    GLdouble modelMatrix[16], projMatrix[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLdouble v0[3], w[3];
    gluUnProject( ClientWidth/2, ClientHeight/2, 0,
      modelMatrix, projMatrix, viewport, &v0[0], &v0[1], &v0[2]);
    if (anX)
    {
      gluUnProject( 2, ClientHeight/2, 0,
        modelMatrix, projMatrix, viewport, &w[0], &w[1], &w[2]);
      vX[0] = w[0]-v0[0]; vX[1] = w[1]-v0[1]; vX[2] = w[2]-v0[2];
    }
    if (anY)
    {
      gluUnProject( ClientWidth/2, 2, 0,
        modelMatrix, projMatrix, viewport, &w[0], &w[1], &w[2]);
      vY[0] = w[0]-v0[0]; vY[1] = w[1]-v0[1]; vY[2] = w[2]-v0[2];
    }
    if (anZ)
    {
      gluUnProject( ClientWidth/2, ClientHeight/2, 100,
        modelMatrix, projMatrix, viewport, &w[0], &w[1], &w[2]);
      vZ[0] = w[0]-v0[0]; vZ[1] = w[1]-v0[1]; vZ[2] = w[2]-v0[2];
    }
  }
  catch (...)
  {
    vX[0] = 1.0f; vX[1] = 0.0f; vX[2] = 0.0f;
    vY[0] = 0.0f; vY[1] = 1.0f; vY[2] = 0.0f;
    vZ[0] = 0.0f; vZ[1] = 0.0f; vZ[2] = 1.0f;
  }
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::CenterToPoint( GLfloat pX, GLfloat pY, GLfloat pZ )
{
    if (!ghDC) return;
    wglMakeCurrent( ghDC, ghRC);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt( 0.0, 0.0, optZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef( -45.0, 1.0, 0.0, 0.0);
    glRotatef( -135.0, 0.0, 0.0, 1.0);
    off0[0] = P0[0]-pX; off0[1] = P0[1]-pY; off0[2] = P0[2]-pZ;
    glTranslatef( off0[0], off0[1], off0[2]);
    DrawScene();
}
//---------------------------------------------------------------------------
// Keyboard & Mouse
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::KeyPress(wchar_t &Key)
{
  if (!ghDC) return;
  wglMakeCurrent( ghDC, ghRC);

  FindNormals( true, true, true);
  double tick_count=300;
  switch (Key)
  {
  case L'a' : // Translate on x axis
  case L'A' :
  case L'ф' :
  case L'Ф' :
  {
    int oldTick=A_KeyTick;
    A_KeyTick = GetTickCount();
    GLfloat d = 0.9;
    if( (A_KeyTick-oldTick)>0 )
    {
      d *= tick_count/(A_KeyTick-oldTick);
    }
    glTranslatef( -d*vX[0], -d*vX[1], -d*vX[2]);
    off0[0] -= d*vX[0]; off0[1] -= d*vX[1]; off0[2] -= d*vX[2];
    break;
  }
  case L'd' : // Translate on x axis (opposite)
  case L'D' :
  case L'в' :
  case L'В' :
  {
    int oldTick=D_KeyTick;
    D_KeyTick = GetTickCount();
    GLfloat d = 0.9;
    if( (D_KeyTick-oldTick)>0 )
    {
      d *= tick_count/(D_KeyTick-oldTick);
    }
    glTranslatef( d*vX[0], d*vX[1], d*vX[2]);
    off0[0] += d*vX[0]; off0[1] += d*vX[1]; off0[2] += d*vX[2];
    break;
  }
  case L'w' : // Translate on y axis
  case L'W' :
  case L'ц' :
  case L'Ц' :
  {
    int oldTick=W_KeyTick;
    W_KeyTick = GetTickCount();
    GLfloat d = 0.9;
    if( (W_KeyTick-oldTick)>0 )
    {
      d *= tick_count/(W_KeyTick-oldTick);
    }
    glTranslatef( d*vZ[0], d*vZ[1], d*vZ[2]);
    off0[0] += d*vZ[0]; off0[1] += d*vZ[1]; off0[2] += d*vZ[2];
		break;
  }
  case L's' : // Translate on y axis (opposite)
  case L'S' :
  case L'ы' :
  case L'Ы' :
  {
    int oldTick=S_KeyTick;
    S_KeyTick = GetTickCount();
    GLfloat d = 0.9;
    if( (S_KeyTick-oldTick)>0 )
    {
      d *= tick_count/(S_KeyTick-oldTick);
    }
    glTranslatef( -d*vZ[0], -d*vZ[1], -d*vZ[2]);
    off0[0] -= d*vZ[0]; off0[1] -= d*vZ[1]; off0[2] -= d*vZ[2];
		break;
  }
  case L'o' : // Centralize it!
  case L'O' :
  case L'щ' :
  case L'Щ' :
  {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt( 0.0, 0.0, optZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef( -45.0, 1.0, 0.0, 0.0);
    glRotatef( -135.0, 0.0, 0.0, 1.0);
    off0[0] = 0.0f; off0[1] = 0.0f; off0[2] = 0.0f;
		break;
  }
  case L'e' : // Rotate clockwise
  case L'E' :
  case L'у' :
  case L'У' :
    glTranslated( -off0[0], -off0[1], -off0[2]);
    glRotatef( 1.0, vZ[0], vZ[1], vZ[2]);
    glTranslated( off0[0], off0[1], off0[2]);
		break;
  case L'q' : // Rotate clockwise
  case L'Q' :
  case L'й' :
  case L'Й' :
    glTranslated( -off0[0], -off0[1], -off0[2]);
    glRotatef( -1.0, vZ[0], vZ[1], vZ[2]);
    glTranslated( off0[0], off0[1], off0[2]);
		break;
  case L'+' :
  {
  	speed_weel=speed_weel*2.0;
    break;
  }
  case L'-' :
  {
  	speed_weel=speed_weel/2.0;
    break;
  }
	}
  DrawScene();
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::MouseDown( TMouseButton Button,
  TShiftState Shift, int X, int Y)
{
    if (Shift.Contains(ssRight)) j_set_value(Cursor, crSizeAll);
    mX = X;
    mY = Y;
    SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::MouseUp( TMouseButton Button,
	TShiftState Shift, int aX, int aY)
{
  j_set_value(Cursor, crDefault);
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::MouseMove( TShiftState Shift, int X, int Y)
{
    if (!ghDC) return;
    wglMakeCurrent( ghDC, ghRC);

    if (Shift.Contains(ssLeft))
		{
      int dx = X - mX;
      int dy = Y - mY;
			rotY += dx;
			rotX += dy;
        glTranslated( -off0[0], -off0[1], -off0[2]);
      FindNormals();
      if (dy) glRotatef( -dy, vX[0], vX[1], vX[2]);
      if (dx) glRotatef( -dx, vY[0], vY[1], vY[2]);
        glTranslated( off0[0], off0[1], off0[2]);
      DrawScene();
      mX = X;
      mY = Y;
    }
    else if (Shift.Contains(ssRight))
		{
      j_set_value(Cursor, crSizeAll);
      int dx = X - mX;
      int dy = Y - mY;
      FindNormals();
      if (dx) glTranslatef( -dx*vX[0], -dx*vX[1], -dx*vX[2]);
      if (dy) glTranslatef( dy*vY[0], dy*vY[1], dy*vY[2]);
      DrawScene();
      mX = X;
      mY = Y;
    }
}
//---------------------------------------------------------------------------
bool __fastcall TGLDrawPanel::DoMouseWheel(Classes::TShiftState Shift,
    int WheelDelta, const Types::TPoint &MousePos)
{
    if (!ghDC) return false;
    wglMakeCurrent( ghDC, ghRC);

    int oldMouseWheelTick=MouseWheelTick;
    MouseWheelTick = GetTickCount();
    GLfloat d = 0.9;
    if( (MouseWheelTick-oldMouseWheelTick)>0 )
    {
      d *= speed_weel/(MouseWheelTick-oldMouseWheelTick);
    }

    GLfloat dlt = (WheelDelta < 0) ? d : -d;
    Z += dlt;

    FindNormals( false, false, true);
    glTranslatef( dlt*vZ[0], dlt*vZ[1], dlt*vZ[2]);
    //off0[0] += dlt*vZ[0]; off0[1] += dlt*vZ[1]; off0[2] += dlt*vZ[2];

    DrawScene();
    return true;
}
//---------------------------------------------------------------------------
// Window functions
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::SetDisplayFunc(TDisplayFunc DF)
{
  DisplayFunc = DF;
  RecreateWnd();
}
//---------------------------------------------------------------------------

void __fastcall TGLDrawPanel::OnMessage(TMessage & Message)
{
  if( Message.WParamLo==0 )
  {
    Hide();
    Show();
  }
  else
    PanelResize(this);
}
//---------------------------------------------------------------------------

void __fastcall TGLDrawPanel::Paint(void)
{
  DrawScene();
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::CreateWnd(void)
{
  TPanel::CreateWnd();

  //bool was = (ghDC != NULL);
  ghDC = Canvas->Handle;
  if (!bSetupPixelFormat(ghDC)) {
      return;
      }
  ghRC = wglCreateContext(ghDC);
  if (!ghRC) {
    	ShowMessage(":-)~ hrc == NULL");
      return;
      }
  if (!wglMakeCurrent(ghDC, ghRC)) {
    	ShowMessage("Could not MakeCurrent");
      return;
      }
  Initialize();
  PanelResize(NULL);

  //if (!was)
  {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt ( 0.0, 0.0, optZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glRotatef( -45.0, 1.0, 0.0, 0.0);
    glRotatef( -135.0, 0.0, 0.0, 1.0);
  }
}
//---------------------------------------------------------------------------
void __fastcall TGLDrawPanel::DestroyWnd(void)
{
  try {
    wglMakeCurrent( ghDC, ghRC);
  } catch(...) {  }
  try { wglMakeCurrent( NULL, NULL); } catch(...) { }
  if (ghRC) try { wglDeleteContext(ghRC); } catch(...) { }

  TPanel::DestroyWnd();
}
//---------------------------------------------------------------------------

