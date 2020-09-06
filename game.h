#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#ifdef GLES
#include <SDL_opengles2.h>
#endif

#ifdef _WIN32
#include <windows.h>            // Header File For Windows
//#include <GL\gl.h>            // Header File For The OpenGL32 Library
//#include <GL\glu.h>            // Header File For The GLu32 Library
#endif

#ifdef __APPLE__
//#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
typedef unsigned char BYTE;
#define TRUE 1
#define FALSE 0

// TODO: Replace these with SDL equivalents -tkidd
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_ESCAPE         0x1B
#define VK_RETURN         0x0D
#endif

#ifdef GLES
#define glColor4fv(a) glColor4f(a[0], a[1], a[2], a[3])
#define glColor3fv(a) glColor4f(a[0], a[1], a[2], 1.0f)
#define glColor3f(a,b,c) glColor4f(a, b, c, 1.0f)
#endif

#include <stdio.h>			// Header File For Standard Input/Output
#include <stdlib.h>
#include <math.h>				// Header File For Math Library
#include "glext.h"

#include "3dglasses.h"
#include "renderflags.h"
#include "cvars.h"
#include "menus.h"
#include "tokenize.h"
#include "timer.h"
#include "console.h"
#include "command.h"
#include "commands.h"

	//extern HWND		hWnd;

	void AdjustRockColor(const int&);

	extern BYTE keys[];
	extern BYTE lastkeys[];

	extern BOOL g_bExitApp;
	extern BOOL g_bShowMenu;
	extern BOOL g_bStartGame;
	extern BOOL g_bCheatsEnabled;

	extern float g_fNow;

	extern cvar_t g_cvResolution;
	extern cvar_t g_cvColorDepth;
	extern cvar_t g_cvGammaGamma;
	extern cvar_t g_cvGammaBrightness;
	extern cvar_t g_cvGammaContrast;

	extern cvar_t g_cvFullscreen;
	extern cvar_t g_cvCopernicusMode;
	extern cvar_t g_cvSpaceMode;
	extern cvar_t g_cvDifficulty;
	extern cvar_t g_cvSpaceSize;
	
	extern cvar_t g_cvInfiniteLives;
	extern cvar_t g_cvInfiniteShields;
	extern cvar_t g_cvInvincibility;

	extern cvar_t g_cvDeveloper;
	extern cvar_t g_cvLightingEnabled;
	extern cvar_t g_cvBlendingEnabled;
	extern cvar_t g_cvTexturesEnabled;
	extern cvar_t g_cvWireframeEnabled;
	extern cvar_t g_cvBoundingBoxesEnabled;
	extern cvar_t g_cvFpsEnabled;
	extern cvar_t g_cvVsyncEnabled;

	BOOL SetVsync(const BOOL);

	extern cvar_t g_cvJoystickEnabled;
	extern cvar_t g_cvForceFeedbackEnabled;
	extern cvar_t g_cvJoystickDeadzone;
	extern BOOL g_bDirectInputEnabled;
	extern BOOL g_bForceFeedbackCapable;
	
	extern cvar_t g_cvSoundEnabled;
	extern cvar_t g_cvVolume;

	extern cvar_t g_cvShipRotateAdjust;
	
	extern cvar_t g_cvMouseSensitivity;
	extern cvar_t g_cvMouseEnabled;

	void SetSound(const BOOL);
	void SetMouse(const BOOL);
	void SetVolume(float NewValue);
	void SetGamma(void);
	
	void UnacquireMouse();
	
	#define KeyDown(x) ((keys[x] && !lastkeys[x]) && keys[x])
	#define KeyUp(x) ((keys[x] && !lastkeys[x]) && !keys[x])
	
	extern const int NUM_KEY_BINDS;
	extern char g_asKeyBindsText[][40];
	extern cvar_t *g_acvKeyBinds[];

	// Function definitions
	GLvoid glPrint(int, const GLfloat&, const GLfloat&, const char*);
	GLvoid glPrintVar(int, const GLfloat&, const GLfloat&, const char*, ...);
	GLvoid glPrintCentered(int, const GLfloat&, const char*);
	GLvoid glPrintCenteredVar(int, const GLfloat&, const char*, ...);
	void UpdateMessage(const char*, ...);
	void PauseGame(const BOOL&);
	BOOL InitializeGame(void);
	void TerminateGame(void);
	
	float GetRefreshTimingRate(void);

	// Sound stuff (for menus)
	void PlayMenuBeepSound();
	void PlayMenuLowSound();
	void PlayMenuExplosionSound();

	// 3d glasses stuff
	extern cvar_t g_cv3dglasses;
	int GetScreenWidth(void);
	int GetScreenHeight(void);

#endif

