
// 3dgame.cpp
// June 17, 2001
//
// This code is different from the standard game.cpp in that it contains
// code to handle rendering to anaglyph rendering using red/blue glasses
// 
// game.cpp is obsolete since I've made other updates to the code, although
// red/blue support isn't in here by default.  
//

/*

Disasteroids 3D
Copyright (c) 2001 Thom Wetzel, Jr. and LMNOpc

This software is provided 'as-is', without any express 
or implied warranty. In no event will the authors be held 
liable for any damages arising from the use of this software.

*/

#define APP_TITLE "Disasteroids 3D"

#define VERSION "VERSION 1.4 BETA (2002.02.24)"
const float g_VersionNumber = 1.4f;

const char csConfigFilename[] = "Disasteroids3D.cfg";
const char csConsoleDumpFilename[] = "Disasteroids3d.log";

// Reserving channels for being able to stop/pan them later
const int channelBulletFiringSound = 1;
const int channelSmallExplodeSound = 2;
const int channelMediumExplodeSound = 3;
const int channelLargeExplodeSound = 4;
const int channelShipThrustSound = 5;
const int channelLSaucerSound = 6;
const int channelSaucerFireSound = 7;

#include "game.h"


// Game data
#include "modeldata.h"		// Model data
#include "resource.h"		// Resource file data

// Lookup table code
#include "LookupTables.h"

#ifdef IOS
#include "sys_ios.h"
#endif

// SDL variables
SDL_Window* SDL_window = NULL;
static SDL_GLContext SDL_glContext = NULL;

BOOL	g_bActive		= TRUE;			// Window active flag

BYTE	keys[256];					// Array Used For The Keyboard Routine
BYTE	lastkeys[256];				// Array to use to determine when a key is first pressed

// Resolution stuff
typedef struct resinfo_t
{
	int width;
	int height;
} resinfo;
const int RESOLUTIONS_COUNT = 5;
resinfo g_resolutions[RESOLUTIONS_COUNT] = {{ 640, 480 }, { 800, 600 }, { 1024, 768 }, {1280, 1024}, {1600, 1200}};
cvar_t g_cvResolution = {"Resolution", "0", TRUE};
int m_screenwidth = 0;
int m_screenheight = 0;

// 3d mode
cvar_t g_cv3dglasses = {"3DGlasses", "0", TRUE};


// Console variables
cvar_t g_cvFullscreen			= {"Fullscreen",		"2",		TRUE};
cvar_t g_cvSpaceMode				= {"SpaceMode",		"2",		TRUE};
cvar_t g_cvCopernicusMode		= {"CopernicusMode", "0",		TRUE};


// Color depth stuff
const int COLORDEPTHS_COUNT = 2;
int colordepths[COLORDEPTHS_COUNT] = { 16, 32 };
cvar_t g_cvColorDepth = {"ColorDepth", "0", TRUE};

// Gamma ramp stuff
static Uint16 old_gamma_ramp[3][256];
cvar_t g_cvGammaGamma		= {"GammaGamma",			"0.5", TRUE};
cvar_t g_cvGammaBrightness = {"GammaBrightness",	"0.5", TRUE};
cvar_t g_cvGammaContrast	= {"GammaContrast",		"0.5", TRUE};


// Difficulty settings
const int EASY_DIFFICULTY		= 0;
const int NORMAL_DIFFICULTY	= 1;
const int HARD_DIFFICULTY		= 2;
cvar_t g_cvDifficulty			= {"Difficulty",		"1",		TRUE};
int g_nDifficulty					= 0;

// Cheats
BOOL	 g_bCheatsEnabled			= FALSE;
cvar_t g_cvInfiniteLives		= {"InfiniteLives",	"0",		FALSE};
cvar_t g_cvInfiniteShields		= {"InfiniteShields","0",		FALSE};
cvar_t g_cvInvincibility		= {"Invincibility",	"0",		FALSE};

// Storage For Textures
const int NUM_TEXTURES = 14;
GLuint texture[NUM_TEXTURES];

// Font list
GLuint font_list;

// Open GL lists
const int NUM_LISTS = 14;
GLuint player_list;
GLuint rock_list;
GLuint shot_list;
GLuint shields_list;
GLuint debris_list;
GLuint blast_list;
GLuint smoke_list;
GLuint lsaucer_list;
GLuint sfire_list;
GLuint background_list;
GLuint logo_list;
GLuint bigspace_list;							// Added By NeHe (04/30/00)
GLuint extraship_list;
GLuint extraship5_list;

cvar_t g_cvDeveloper					= {"Developer",				"1",  TRUE};	// Developer mode flag
cvar_t g_cvLightingEnabled			= {"LightingEnabled",		"-1", FALSE};	// Lighting enabled flag
cvar_t g_cvBlendingEnabled			= {"BlendingEnabled",		"-1", FALSE};	// Blending enabled flag
cvar_t g_cvTexturesEnabled			= {"TexturesEnabled",		"-1", FALSE};	// Textures enabled flag
cvar_t g_cvWireframeEnabled		= {"WireframeEnabled",		"0",	FALSE};	// Wireframe mode flag
cvar_t g_cvBoundingBoxesEnabled	= {"BoundingBoxesEnabled",	"0",	FALSE};	// Bounding box display flag
cvar_t g_cvFpsEnabled				= {"ShowFPS",					"0",	FALSE};	// Display frames per second flag
cvar_t g_cvVsyncEnabled				= {"VsyncEnabled",			"0",	TRUE};	// Vsync enabled flag
cvar_t g_cvSpaceSize					= {"SpaceSize",				"1",	TRUE};	// Space size value

const float g_cvMaxFps					= 60.0f;

BOOL	g_bExitApp					= FALSE;		// Global loop ending variable

BOOL	g_bStartGame				= FALSE;		// Flag to use to trigger the start of a new game from the menus
BOOL	g_bGameOver					= TRUE;		// Game over flag
BOOL	g_bGamePaused				= FALSE;		// Game paused flag
BOOL	g_bShowMenu					= FALSE;

int	g_nFps						= 0;			// Frames per second variable
float	g_fGameLength				= 0;			// Application run time variable
int	g_nLevelNum					= 0;			// Level number indicator

float g_fSwapBufferInterval	= 0;			// Vsync timing variable

// Default lighting setup for light 1
GLfloat LightAmbient[]	= { 0.05f, 0.05f, 0.05f, 1.0f };
GLfloat LightDiffuse[]	= { 0.3f, 0.3f, 0.7f, 1.0f }; 
GLfloat LightPosition[]	= { 7.0f, 7.0f, 5.0f, 1.0f };

/* V 1.2 

  I changed all these from consts to straight floats so that
  I can now change the space size in the game
*/
float WORLD_WIDTH			=  44.0f;		// Holds The Width Of Our World
float WORLD_HALFWIDTH	=  22.0f;		// Holds Half The Width Of Our World
float WORLD_HEIGHT		=  32.0f;		// Holds The Height Of Our World
float WORLD_HALFHEIGHT	=  16.0f;		// Holds Half The Height Of Our World
float WORLD_DEPTH			= -20.0f;	// The distance all the objects are from the camera

float SCREEN_WIDTH		=  22.0f;
float	SCREEN_HALFWIDTH	=  11.0f;
float SCREEN_HEIGHT		=  16.0f;
float	SCREEN_HALFHEIGHT	=   8.0f;
float SCREEN_DEPTH		= -20.0f;

// Different actors have different behaviors and 
// rendering characteristics.  
const int ACTOR_NONE					= 0;
const int ACTOR_PLAYER				= 1;
const int ACTOR_ROCK					= 2;
const int ACTOR_SHOT					= 3;
const int ACTOR_SHIELDS				= 4;
const int ACTOR_DEBRIS				= 5;
const int ACTOR_BLAST				= 6;
const int ACTOR_PARTICLE			= 7;
const int ACTOR_FLAMINGPARTICLE	= 8;
const int ACTOR_LSAUCER				= 9;
const int ACTOR_SFIRE				= 10;
const int ACTOR_FLAMES				= 11;


// This is the maximum number of elements
// in the actor array.
const int MAX_ACTORS					= 210;


// Define how actors[] is split up
//
// Make sure the 3D objects are before the 2D
// so that things are rendered correctly
//
// In the future, I'm going to do this with
// pointers, but for now, this works.
const int IDX_NO_ACTOR			= 0;	// No actor can have index 0
const int IDX_FIRST_ROCK		= 1;
const int IDX_LAST_ROCK			= 29;
const int IDX_LSAUCER			= 30;
const int IDX_PLAYER1			= 31;
const int IDX_PLAYER2			= 32;
const int IDX_FIRST_DEBRIS		= 33;
const int IDX_LAST_DEBRIS		= 116;
const int IDX_FIRST_BLAST		= 117;
const int IDX_LAST_BLAST		= 126;
const int IDX_FIRST_PARTICLE	= 127;
const int IDX_LAST_PARTICLE	= 186;
const int IDX_FIRST_SHOT		= 187;	// Make sure all the shots are lumped together
const int IDX_LAST_SHOT			= 198;
const int IDX_FIRST_SFIRE		= 199;
const int IDX_LAST_SFIRE		= 208;	// End of shots
const int IDX_SHIELDS			= 209;

// Game timing variables
float g_fNow			= 0;		// Holds current game Tick
float g_fNowOffset	= 0;		// Holds number of Ticks game was paused


// Color structure
typedef struct rgbcolor_t {
	GLbyte r;		// Red intensity (0-255)
	GLbyte g;		// Green intensity (0-255)
	GLbyte b;		// Blue intensity (0-255)
} udtRgbColor;


// Actor definition
typedef struct actor_t {

	// All of the following properties are used
	// by all the actors
	int		type;				// This value should be set to one of the ACTORs above
	BOOL		active;			// Is this actor active (TRUE/FALSE)
	GLfloat	x;					// X position of the actor
	GLfloat	y;					// Y position of the actor
	GLfloat	z;					// Z position of the actor
	GLfloat	rx;				// X rotation of the actor
	GLfloat	ry;				// Y rotation of the actor
	GLfloat	rz;				// Z rotation of the actor
	GLfloat	vx;				// X velocity of the actor
	GLfloat	vy;				// Y velocity of the actor
	GLfloat	vz;				// Z velocity of the actor
	GLuint	displaylistid;	// Defines the display list id for this actor
	udtRgbColor color;		// Defines the color for this actor
	float		LifeStartTime;	// Start time of actor's life cycle
	float		LifeEndTime;	// End time of the actor's life cycle
	float		LifePercent;	// Percentage of life remaing for actor
	float		size;				// Visual scaling factor (different size rocks/blasts)

	// These properties are shared by actors that collide 
	//
	long		PointValue;		// Value awarded to player when this actor is destroyed
	float		xcolldist;		// X collision distance (bounding box x radius)
	float		ycolldist;		// Y collision distance (bounding box y radius)


	// These properties are unique to the player(s)
	//
	BOOL		ShieldsUp;
	GLfloat	ShieldsPower;
	long		Lives;
	long		NextBonus;
	long		Score;
	float		ShotDelay;
	float		InvincibleEndTime;
	BOOL		Hyperspacing;
	float		HyperspaceInterval;
	float		HyperspaceDelay;

	// These properties are unique to the saucer(s)
	//
	float		ChangeDirectionDelay;

	// These properties are unique to particle-emitters
	float		ParticleSpawnDelay;	// Time until next particle should be spawned

	
} actorinfo;

actorinfo actors[MAX_ACTORS];
actorinfo *player;


// Precomputed values used throughout
const float pi						= 3.1415926535f;
const float inversepi			= 0.3183098861f;
const float convertdegtorad	= 0.0174532925f;


// Background motion velocity
GLfloat g_fBackgroundRot = 0.0f;


// Sound stuff (mostly ripped from DirectX SDK Duel and modified)
BOOL g_bDirectSoundEnabled = FALSE;
cvar_t g_cvSoundEnabled					= {"SoundEnabled", "2", TRUE};
cvar_t g_cvVolume							= {"Volume", "1.0", TRUE};
Mix_Chunk* g_pBulletFiringSound	= NULL;
Mix_Chunk* g_pSmallExplodeSound	= NULL;
Mix_Chunk* g_pMediumExplodeSound  = NULL;
Mix_Chunk* g_pLargeExplodeSound	= NULL;
Mix_Chunk* g_pShipThrustSound		= NULL;
Mix_Chunk* g_pThumpHiSound			= NULL;
Mix_Chunk* g_pThumpLoSound			= NULL;
Mix_Chunk* g_pLSaucerSound			= NULL;
Mix_Chunk* g_pSaucerFireSound		= NULL;
Mix_Chunk* g_pLifeSound				= NULL;
Mix_Chunk* g_pMenuBeepSound			= NULL;
const int NUM_SHIP_TYPES				= 9;
BOOL g_bThrustSoundOn					= FALSE;
const float cStereoSeperation = 0.1f;

// DirectInput stuff (ripped from DirectX SDK FFDonuts and modified)
BOOL g_bDirectInputEnabled							= FALSE;
SDL_Joystick* g_pdidJoystick = NULL;
SDL_Haptic* g_pdidFFJoystick = NULL;
unsigned long                g_dwForceFeedbackGain	= 100L;
cvar_t g_cvJoystickEnabled			= {"JoystickEnabled", "-1", FALSE};
cvar_t g_cvForceFeedbackEnabled	= {"ForcefeedbackEnabled", "0", TRUE};
cvar_t g_cvJoystickDeadzone		= {"JoystickDeadzone", "0.2", TRUE};
BOOL g_bForceFeedbackCapable	= FALSE;
const int JOYSTICK_LEFT			= 1;
const int JOYSTICK_RIGHT		= 2;
const int JOYSTICK_UP			= 4;
const int JOYSTICK_DOWN			= 8;
const int JOYSTICK_FIRE			= 16;
const int JOYSTICK_SHIELDS		= 32;
const int JOYSTICK_HYPERSPACE	= 64;

// SDL Input stuff
void handleKey(SDL_KeyboardEvent key, BOOL keydown);
void playExplodeFFEffect();

// Mouse variables
// TODO: Mouse later -tkidd
//LPDIRECTINPUTDEVICE	g_pdidMouse	= NULL;
//DIMOUSESTATE g_dims;
cvar_t g_cvMouseEnabled				= {"MouseEnabled", "-1", TRUE};
cvar_t g_cvMouseSensitivity		= {"MouseSensitivity", "0.5", TRUE};

// Other control stuff
cvar_t g_cvShipRotateAdjust		= { "ShipRotateAdjust", "0.5", TRUE};


// Variables for dealing with highscore initial entry
// High score variables and initialization
const int MAX_HIGHSCORES = 10;

typedef struct _highscoreinfo {
	char		Initials[4];
	long		Score;
	int		Level;
	char		Difficulty;		// E, N, H
	char		SpaceSize;		// S, M, L

} highscoreinfo;

// Init the highscores with my initials -- I'm a WINNER!!!! :)
highscoreinfo	highscores[MAX_HIGHSCORES] = {
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M', 
		"TCW", 1000, 1, 'N', 'M' };
BOOL	g_bEnteringHighScoreInitials		= FALSE;
char	g_nHighScoreInitialsIdx				= 0;
const char g_cLegalHighScoreChars[38]	= " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
highscoreinfo *playerhighscore			= NULL;

const long HIGHSCORE_ENTRY_DELAY			= 2;
long	g_lHighScoreEntryDelayEndTime		= 0;


// Message display variables
const long MESSAGE_LIFE = 3;
long g_lMessageEndTime	= 0;
char g_sMessage[256]		= "";


// Key binds
const int NUM_KEY_BINDS		= 8;

char g_asKeyBindsText[NUM_KEY_BINDS][40] = 
{
	"THRUST", "SHIELDS", "HYPERSPACE", "ROTATE LEFT", "ROTATE RIGHT", "FIRE", "START GAME", "PAUSE GAME"
};
cvar_t g_cvKeyThrust[2]			= {{"KeyBind_Thrust", "38", TRUE},	{"KeyBind_ThrustAlt", "0", TRUE}};
cvar_t g_cvKeyShields[2]		= {{"KeyBind_Shields", "40", TRUE},	{"KeyBind_ShieldsAlt", "0", TRUE}};
cvar_t g_cvKeyHyperspace[2]	= {{"KeyBind_Hyperspace", "104", TRUE}, {"KeyBind_HyperspaceAlt", "0", TRUE}};
cvar_t g_cvKeyRotateLeft[2]	= {{"KeyBind_RotateLeft", "37", TRUE},	{"KeyBind_RotateLeftAlt", "0", TRUE}};
cvar_t g_cvKeyRotateRight[2]	= {{"KeyBind_RotateRight", "39", TRUE}, {"KeyBind_RotateRightAlt", "0",	TRUE}};
cvar_t g_cvKeyFire[2]			= {{"KeyBind_Fire", "32", TRUE}, {"KeyBind_FireAlt", "0", TRUE}};
cvar_t g_cvKeyStartGame[2]		= {{"KeyBind_StartGame", "49", TRUE}, {"KeyBind_StartGameAlt", "0", TRUE}};
cvar_t g_cvKeyPause[2]			= {{"KeyBind_Pause", "112", TRUE}, {"KeyBind_PauseAlt", "0", TRUE}};

// Array of keybind console variables.  This is used for the menu system.
// In the future, I need to create a keybind type that contains both the 
// description and cvar of each bind, but this will do... for now.
cvar_t *g_acvKeyBinds[NUM_KEY_BINDS * 2] = 
{
	&g_cvKeyThrust[0],		&g_cvKeyThrust[1],
	&g_cvKeyShields[0],		&g_cvKeyShields[1],
	&g_cvKeyHyperspace[0],	&g_cvKeyHyperspace[1],
	&g_cvKeyRotateLeft[0],	&g_cvKeyRotateLeft[1],
	&g_cvKeyRotateRight[0], &g_cvKeyRotateRight[1],
	&g_cvKeyFire[0],			&g_cvKeyFire[1],
	&g_cvKeyStartGame[0],	&g_cvKeyStartGame[1],
	&g_cvKeyPause[0],			&g_cvKeyPause[1]
};


// Pre-Copernicus Mode
float g_fBackgroundX = 0;
float g_fBackgroundY = 0;
BOOL  g_bStarsActive = FALSE;
const int NUM_STARS = 100;
const float STAR_WORLD_WIDTH = SCREEN_WIDTH + 15.0f;
const float STAR_WORLD_HALFWIDTH = STAR_WORLD_WIDTH * 0.5f;
const float STAR_WORLD_HEIGHT = SCREEN_HEIGHT + 15.0f;
const float STAR_WORLD_HALFHEIGHT = STAR_WORLD_HEIGHT * 0.5f;
const float MIN_STAR_Z = SCREEN_DEPTH - 30.0f;
const float MAX_STAR_Z = SCREEN_DEPTH + 0.0f;
const float MIN_STAR_SIZE = 0.15f;
const float MAX_STAR_SIZE = 0.35f;
typedef struct _starinfo {
	GLfloat	x;					// X position of the actor
	GLfloat	y;					// Y position of the actor
	GLfloat	z;					// Z position of the actor
	GLfloat	size;				// Size of star
} starinfo;
starinfo stars[NUM_STARS];

void handleKey(SDL_KeyboardEvent key, BOOL keydown)
{
	if (key.keysym.scancode == SDL_SCANCODE_GRAVE) {
		keys[126] = keydown; // "~"
	}
	else if (key.keysym.sym >= SDLK_SPACE && key.keysym.sym < SDLK_DELETE) {
		keys[key.keysym.sym] = keydown;
	}
	else if (key.keysym.sym == SDLK_RETURN || key.keysym.sym == SDLK_BACKSPACE || key.keysym.sym == SDLK_ESCAPE) {
		keys[key.keysym.sym] = keydown;
	}
	else if (key.keysym.scancode == SDL_SCANCODE_LEFT) {
		if (g_cvKeyRotateLeft[0].value)
			keys[int(g_cvKeyRotateLeft[0].value)] = keydown;
		if (g_cvKeyRotateLeft[1].value)
			keys[int(g_cvKeyRotateLeft[1].value)] = keydown;
	}
	else if (key.keysym.scancode == SDL_SCANCODE_RIGHT) {
		if (g_cvKeyRotateRight[0].value)
			keys[int(g_cvKeyRotateRight[0].value)] = keydown;
		if (g_cvKeyRotateRight[1].value)
			keys[int(g_cvKeyRotateRight[1].value)] = keydown;
	}
	else if (key.keysym.scancode == SDL_SCANCODE_UP) {
		if (g_cvKeyThrust[0].value)
			keys[int(g_cvKeyThrust[0].value)] = keydown;
		if (g_cvKeyThrust[1].value)
			keys[int(g_cvKeyThrust[1].value)] = keydown;
	}
	else if (key.keysym.scancode == SDL_SCANCODE_DOWN) {
		if (g_cvKeyShields[0].value)
			keys[int(g_cvKeyShields[0].value)] = keydown;
		if (g_cvKeyShields[1].value)
			keys[int(g_cvKeyShields[1].value)] = keydown;
	}
}

void playExplodeFFEffect() {
	if (SDL_HapticRumblePlay(g_pdidFFJoystick, 0.75, 500) != 0)
	{
		printf("Warning: Unable to play rumble! %s\n", SDL_GetError());
	}
}

//--------------------------------------------------------
// TCW
//
// Send TRUE to Pause to pause game, send FALSE
// to resume game
//--------------------------------------------------------
void PauseGame(const BOOL& Start)
{
	static float fStartTime = 0;

	if (Start)
	{
		if (!g_bGamePaused) 
		{
			fStartTime = TimerGetTime();
			g_bGamePaused = TRUE;

			// Unacquire mouse
			// TODO: Mouse later -tkidd
			//if (NULL != g_pdidMouse)
			//	g_pdidMouse->Unacquire();
		}
	}
	else
	{
		if (g_bGamePaused)
		{
			g_fNowOffset += TimerGetTime() - fStartTime;
			g_bGamePaused = FALSE;

			// Reacquire mouse
			// TODO: Mouse later -tkidd
			/*if (NULL != g_pdidMouse)
			{
				g_pdidMouse->Unacquire();
				g_pdidMouse->Acquire();
			}*/

		}
	}
}


//--------------------------------------------------------
// TCW
// Get an random integer within a specified range
//--------------------------------------------------------
int randInt(const int& min, const int& max) {
    return ((rand()%(int)(((max) + 1)-(min)))+ (min)); 
}

//--------------------------------------------------------
// Get a random float within a specified range
//--------------------------------------------------------
float randFloat(const float& min, const float& max) {
	float range = max - min;
	float num = range * rand() / RAND_MAX;
	return (num + min);
}

//
// This source code, along with the object definitions were generated
// by 3D Explorer.  I made some modifications to each to allow me to 
// load both player and rock object data.  After the player and rock
// are setup, shots and debris are hand-coded and VERY impessive (that's sarcasm, folks)
//
GLint Gen3DObjectList()
{

#ifndef GLES
	int i =0;
	int j =0;

	// Create display lists
	player_list = glGenLists(NUM_LISTS);
    
	// Player
	glNewList(player_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
        glBegin (GL_TRIANGLES);
        int vtxIndex = 0;
        for(i=0;i<sizeof(player_face_indicies)/sizeof(player_face_indicies[0]);i++)
			{
				for(j=0;j<3;j++)
				{
					int vi = player_face_indicies[i][j];
					int ni = player_face_indicies[i][j+3];
					int ti = player_face_indicies[i][j+6];
					glNormal3f (player_normals[ni][0],player_normals[ni][1],player_normals[ni][2]);
					glTexCoord2f(player_textures[ti][0],player_textures[ti][1]);
					glVertex3f (player_verticies[vi][0],player_verticies[vi][1],player_verticies[vi][2]);
				}
			}
    glEnd ();
	glEndList();

	// Read rock data from arrays
	rock_list = player_list + 1;
	glNewList(rock_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[6]);
		glBegin (GL_TRIANGLES);
			for(i=0;i<sizeof(rock_face_indicies)/sizeof(rock_face_indicies[0]);i++)
			{
				for(j=0;j<3;j++)
				{
					int vi=rock_face_indicies[i][j];
					int ni=rock_face_indicies[i][j+3];
					int ti=rock_face_indicies[i][j+6];
					glNormal3f (rock_normals[ni][0],rock_normals[ni][1],rock_normals[ni][2]);
					glTexCoord2f(rock_textures[ti][0],rock_textures[ti][1]); 
					glVertex3f (rock_verticies[vi][0],rock_verticies[vi][1],rock_verticies[vi][2]);
				}
			}
		glEnd ();
	glEndList();

	// Draw shot as textured quad
	shot_list = rock_list + 1;
	glNewList(shot_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f, 0.5f, 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f,-0.5f, 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,-0.5f, 0.0f);
		glEnd();

	glEndList();

	// Draw shields as textured quad
	shields_list = shot_list + 1;
	glNewList(shields_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,-1.0f, 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);
		glEnd();
	glEndList();

	// Debris - textured triangle
	debris_list = shields_list + 1;
	glNewList(debris_list, GL_COMPILE);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex3f( 0.15f,  0.15f,  0.0f);
			glVertex3f(-0.15f,  0.15f,  0.0f);
			glVertex3f( 0.15f, -0.15f,  0.0f);
			glVertex3f(-0.15f, -0.15f,  0.0f);
		glEnd();
	glEndList();

	// Blast
	blast_list = debris_list + 1;
	glNewList(blast_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[3]);
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  0.0f);
		glEnd();
	glEndList();
		
	// Smoke - textured quad
	smoke_list = blast_list + 1;
	glNewList(smoke_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.25f,  0.25f,  0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.25f,  0.25f,  0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.25f, -0.25f,  0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.25f, -0.25f,  0.0f);
		glEnd();
	glEndList();

	// Read ufo data from arrays
	lsaucer_list = smoke_list + 1;
	glNewList(lsaucer_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[11]);
		glBegin (GL_TRIANGLES);
		for(i=0;i<sizeof(ufo_face_indicies)/sizeof(ufo_face_indicies[0]);i++)
			{
				for(j=0;j<3;j++)
				{
					int vi = ufo_face_indicies[i][j];
					int ni = ufo_face_indicies[i][j+3];
					int ti = ufo_face_indicies[i][j+6];
					glNormal3f  (ufo_normals[ni][0]  ,ufo_normals[ni][1],  ufo_normals[ni][2]);
					glTexCoord2f(ufo_textures[ti][0] ,ufo_textures[ti][1]);
					glVertex3f  (ufo_verticies[vi][0],ufo_verticies[vi][1],ufo_verticies[vi][2]);
				}
			}
		glEnd ();
	glEndList();

	// Draw sfire as textured quad
	sfire_list = lsaucer_list + 1;
	glNewList(sfire_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,-1.0f, 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);
		glEnd();
	glEndList();

	// Draw background as textured quad
	background_list = sfire_list + 1;
	glNewList(background_list, GL_COMPILE);
		
		// Upper left
		glBindTexture(GL_TEXTURE_2D, texture[7]);
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f        , SCREEN_HEIGHT * 1.4f, 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-SCREEN_WIDTH, SCREEN_HEIGHT * 1.4f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.0f        , 0.0f                , 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-SCREEN_WIDTH, 0.0f                , 0.0f);
		glEnd();
		// Upper right
		glBindTexture(GL_TEXTURE_2D, texture[8]);
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( SCREEN_WIDTH, SCREEN_HEIGHT * 1.4f, 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f        , SCREEN_HEIGHT * 1.4f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( SCREEN_WIDTH, 0.0f                , 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f        , 0.0f                , 0.0f);
		glEnd();
		
		// Lower left
		glBindTexture(GL_TEXTURE_2D, texture[9]);
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.0f        , 0.0f                , 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-SCREEN_WIDTH, 0.0f                , 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.0f        ,-SCREEN_HEIGHT * 1.4f, 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-SCREEN_WIDTH,-SCREEN_HEIGHT * 1.4f, 0.0f);
		glEnd();
		
		// Lower right
		glBindTexture(GL_TEXTURE_2D, texture[10]);
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( SCREEN_WIDTH, 0.0f                , 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f        , 0.0f                , 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( SCREEN_WIDTH,-SCREEN_HEIGHT * 1.4f, 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f        ,-SCREEN_HEIGHT * 1.4f, 0.0f);
		glEnd();
	glEndList();

	// Logo
	logo_list = background_list + 1;
	glNewList(logo_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[12]);
		glBegin(GL_QUADS);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(0.0f,-0.5f); glVertex3f(-2.0f,-0.5f, 0.0f);
			glTexCoord2f(1.0f,-0.5f); glVertex3f( 0.0f,-0.5f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.0f, 0.5f, 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 0.5f, 0.0f);
		glEnd();
		glBegin(GL_QUADS);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(0.0f,-1.0f); glVertex3f(-0.0f,-0.5f, 0.0f);
			glTexCoord2f(1.0f,-1.0f); glVertex3f( 2.0f,-0.5f, 0.0f);
			glTexCoord2f(1.0f,-0.5f); glVertex3f( 2.0f, 0.5f, 0.0f);
			glTexCoord2f(0.0f,-0.5f); glVertex3f(-0.0f, 0.5f, 0.0f);
		glEnd();
		
	glEndList();
	
// ******************** Modified By NeHe (04/30/00) ********************
	
	// Big Space
	bigspace_list = logo_list + 1;
	glNewList(bigspace_list, GL_COMPILE);
		glColor4ub(255, 255, 255, 128);
		glTranslatef(-SCREEN_WIDTH*2,-SCREEN_HEIGHT*2,0.0f);
		glCallList(background_list);
		glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
		glCallList(background_list);
		glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
		glCallList(background_list);
		glTranslatef(-SCREEN_WIDTH*2,0.0f,0.0f);

		glTranslatef(-SCREEN_WIDTH*2,+SCREEN_HEIGHT*2,0.0f);
		glCallList(background_list);
		glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
		glCallList(background_list);
		glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
		glCallList(background_list);
		glTranslatef(-SCREEN_WIDTH*2,0.0f,0.0f);

		glTranslatef(-SCREEN_WIDTH*2,+SCREEN_HEIGHT*2,0.0f);
		glCallList(background_list);
		glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
		glCallList(background_list);
		glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
		glCallList(background_list);
		glTranslatef(-SCREEN_WIDTH*2,0.0f,0.0f);
	glEndList();

// ******************** Modified By NeHe (04/30/00) ********************

	
	// Extra ship
	extraship_list = bigspace_list + 1;
	glNewList(extraship_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[13]);
		glBegin(GL_QUADS);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.0f);
			glTexCoord2f(0.5f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.0f);
			glTexCoord2f(0.5f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.0f);
		glEnd();
	glEndList();

	// Five extra ships
	extraship5_list = extraship_list + 1;
	glNewList(extraship5_list, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture[13]);
		glBegin(GL_QUADS);
			glNormal3f( 0.0f, 0.0f, 1.0f);
			glTexCoord2f(0.5f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.0f);
			glTexCoord2f(0.5f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.0f);
		glEnd();
    glEndList();
#endif

	return player_list;
}

void draw_blast() {
    glBindTexture(GL_TEXTURE_2D, texture[3]);

    GLfloat nmlBlast[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxBlast[] = {
       1.0f,  1.0f,  0.0f,
      -1.0f,  1.0f,  0.0f,
       1.0f, -1.0f,  0.0f,
      -1.0f, -1.0f,  0.0f
    };
    GLfloat texBlast[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlBlast);
    glVertexPointer(3, GL_FLOAT, 0, vtxBlast);
    glTexCoordPointer(2, GL_FLOAT, 0, texBlast);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

// TODO: why are the shots so huge when called from a function like this? -tkidd
void draw_sfire() {
    glBindTexture(GL_TEXTURE_2D, texture[1]);

    GLfloat nmlSfire[] = {
            0.0f, 0.0f, 1.0f
    };
    GLfloat vtxSfire[] = {
       1.0f, 1.0f, 0.0f,
      -1.0f, 1.0f, 0.0f,
       1.0f,-1.0f, 0.0f,
      -1.0f,-1.0f, 0.0f
    };
    GLfloat texSfire[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlSfire);
    glVertexPointer(3, GL_FLOAT, 0, vtxSfire);
    glTexCoordPointer(2, GL_FLOAT, 0, texSfire);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void draw_smoke() {
    // Smoke - textured quad
    glBindTexture(GL_TEXTURE_2D, texture[4]);

    GLfloat nmlSmoke[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxSmoke[] = {
       0.25f,  0.25f,  0.0f,
      -0.25f,  0.25f,  0.0f,
       0.25f, -0.25f,  0.0f,
      -0.25f, -0.25f,  0.0f
    };
    GLfloat texSmoke[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlSmoke);
    glVertexPointer(3, GL_FLOAT, 0, vtxSmoke);
    glTexCoordPointer(2, GL_FLOAT, 0, texSmoke);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void draw_shields() {
    glBindTexture(GL_TEXTURE_2D, texture[2]);

    GLfloat vtxShields[] = {
       1.0f, 1.0f, 0.0f,
      -1.0f, 1.0f, 0.0f,
       1.0f,-1.0f, 0.0f,
      -1.0f,-1.0f, 0.0f
    };
    GLfloat texShields[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vtxShields);
    glTexCoordPointer(2, GL_FLOAT, 0, texShields);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void draw_debris() {
    GLfloat vtxDebris[] = {
         0.15f,  0.15f,  0.0f,
        -0.15f,  0.15f,  0.0f,
         0.15f, -0.15f,  0.0f,
        -0.15f, -0.15f,  0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vtxDebris);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void draw_ufo() {
    GLfloat vtxUfo[sizeof(ufo_face_indicies)];
    GLfloat nmlUfo[sizeof(ufo_face_indicies)];
    GLfloat texUfo[(sizeof(ufo_face_indicies) / 3) * 2];
    
    int vtxUfoIndex = 0;
    int nmlUfoIndex = 0;
    int texUfoIndex = 0;
    for(int i=0; i<sizeof(ufo_face_indicies)/sizeof(ufo_face_indicies[0]); i++)
    {
        for(int j=0; j<3; j++)
        {
            int vi = ufo_face_indicies[i][j];
            int ni = ufo_face_indicies[i][j+3];
            int ti = ufo_face_indicies[i][j+6];
            
            nmlUfo[nmlUfoIndex++] = ufo_normals[ni][0];
            nmlUfo[nmlUfoIndex++] = ufo_normals[ni][1];
            nmlUfo[nmlUfoIndex++] = ufo_normals[ni][2];

            texUfo[texUfoIndex++] = ufo_textures[ti][0];
            texUfo[texUfoIndex++] = ufo_textures[ti][1];

            vtxUfo[vtxUfoIndex++] = ufo_verticies[vi][0];
            vtxUfo[vtxUfoIndex++] = ufo_verticies[vi][1];
            vtxUfo[vtxUfoIndex++] = ufo_verticies[vi][2];
        }
    }
    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlUfo);
    glVertexPointer(3, GL_FLOAT, 0, vtxUfo);
    glTexCoordPointer(2, GL_FLOAT, 0, texUfo);
    glDrawArrays(GL_TRIANGLES,0, (sizeof(ufo_face_indicies)/sizeof(ufo_face_indicies[0]) * 3));

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void draw_player() {
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    GLfloat vtxPlayer[sizeof(player_face_indicies)];
    GLfloat nmlPlayer[sizeof(player_face_indicies)];
    GLfloat texPlayer[(sizeof(player_face_indicies) / 3) * 2];

    int vtxIndex = 0;
    int nmlIndex = 0;
    int texIndex = 0;
    for(int i=0; i<sizeof(player_face_indicies)/sizeof(player_face_indicies[0]); i++)
    {
        for(int j=0; j<3; j++)
        {
            int vi = player_face_indicies[i][j];
            int ni = player_face_indicies[i][j+3];
            int ti = player_face_indicies[i][j+6];
            
            nmlPlayer[nmlIndex++] = player_normals[ni][0];
            nmlPlayer[nmlIndex++] = player_normals[ni][1];
            nmlPlayer[nmlIndex++] = player_normals[ni][2];

            texPlayer[texIndex++] = player_textures[ti][0];
            texPlayer[texIndex++] = player_textures[ti][1];

            vtxPlayer[vtxIndex++] = player_verticies[vi][0];
            vtxPlayer[vtxIndex++] = player_verticies[vi][1];
            vtxPlayer[vtxIndex++] = player_verticies[vi][2];
        }
    }
    
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlPlayer);
    glVertexPointer(3, GL_FLOAT, 0, vtxPlayer);
    glTexCoordPointer(2, GL_FLOAT, 0, texPlayer);
    glDrawArrays(GL_TRIANGLES,0, (sizeof(player_face_indicies)/sizeof(player_face_indicies[0]) * 3));

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void draw_rock() {
    glBindTexture(GL_TEXTURE_2D, texture[6]);

    GLfloat vtxRock[sizeof(rock_face_indicies)];
    GLfloat nmlRock[sizeof(rock_face_indicies)];
    GLfloat texRock[(sizeof(rock_face_indicies) / 3) * 2];

    int vtxRockIndex = 0;
    int nmlRockIndex = 0;
    int texRockIndex = 0;
    for(int i=0; i<sizeof(rock_face_indicies)/sizeof(rock_face_indicies[0]); i++)
    {
        for(int j=0; j<3; j++)
        {
            int vi = rock_face_indicies[i][j];
            int ni = rock_face_indicies[i][j+3];
            int ti = rock_face_indicies[i][j+6];
            
            nmlRock[nmlRockIndex++] = rock_normals[ni][0];
            nmlRock[nmlRockIndex++] = rock_normals[ni][1];
            nmlRock[nmlRockIndex++] = rock_normals[ni][2];

            texRock[texRockIndex++] = rock_textures[ti][0];
            texRock[texRockIndex++] = rock_textures[ti][1];

            vtxRock[vtxRockIndex++] = rock_verticies[vi][0];
            vtxRock[vtxRockIndex++] = rock_verticies[vi][1];
            vtxRock[vtxRockIndex++] = rock_verticies[vi][2];
        }
    }

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlRock);
    glVertexPointer(3, GL_FLOAT, 0, vtxRock);
    glTexCoordPointer(2, GL_FLOAT, 0, texRock);
    glDrawArrays(GL_TRIANGLES,0, (sizeof(rock_face_indicies)/sizeof(rock_face_indicies[0]) * 3));

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void draw_logo() {
    glBindTexture(GL_TEXTURE_2D, texture[12]);
    GLfloat nmlLogo1[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxLogo1[] = {
      -2.0f,-0.5f, 0.0f,
       0.0f,-0.5f, 0.0f,
       0.0f, 0.5f, 0.0f,
      -2.0f, 0.5f, 0.0f
    };
    GLfloat texLogo1[] = {
      0.0f,-0.5f,
      1.0f,-0.5f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlLogo1);
    glVertexPointer(3, GL_FLOAT, 0, vtxLogo1);
    glTexCoordPointer(2, GL_FLOAT, 0, texLogo1);
    glDrawArrays(GL_TRIANGLE_FAN,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    GLfloat nmlLogo2[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxLogo2[] = {
      -0.0f,-0.5f, 0.0f,
       2.0f,-0.5f, 0.0f,
       2.0f, 0.5f, 0.0f,
      -0.0f, 0.5f, 0.0f
    };
    GLfloat texLogo2[] = {
      0.0f,-1.0f,
      1.0f,-1.0f,
      1.0f,-0.5f,
      0.0f,-0.5f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlLogo2);
    glVertexPointer(3, GL_FLOAT, 0, vtxLogo2);
    glTexCoordPointer(2, GL_FLOAT, 0, texLogo2);
    glDrawArrays(GL_TRIANGLE_FAN,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void draw_extraship5() {
    glBindTexture(GL_TEXTURE_2D, texture[13]);
    GLfloat nmlExtraShip5[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxExtraShip5[] = {
      -0.5f, -0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
       0.5f,  0.5f,  0.0f,
      -0.5f,  0.5f,  0.0f
    };
    GLfloat texExtraShip5[] = {
      0.5f, 0.0f,
      1.0f, 0.0f,
      1.0f, 1.0f,
      0.5f, 1.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlExtraShip5);
    glVertexPointer(3, GL_FLOAT, 0, vtxExtraShip5);
    glTexCoordPointer(2, GL_FLOAT, 0, texExtraShip5);
    glDrawArrays(GL_TRIANGLE_FAN,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void draw_extraship() {
    glBindTexture(GL_TEXTURE_2D, texture[13]);

    GLfloat nmlExtraShip[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxExtraShip[] = {
      -0.5f, -0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
       0.5f,  0.5f,  0.0f,
      -0.5f,  0.5f,  0.0f
    };
    GLfloat texExtraShip[] = {
      0.0f, 0.0f,
      0.5f, 0.0f,
      0.5f, 1.0f,
      0.0f, 1.0f
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlExtraShip);
    glVertexPointer(3, GL_FLOAT, 0, vtxExtraShip);
    glTexCoordPointer(2, GL_FLOAT, 0, texExtraShip);
    glDrawArrays(GL_TRIANGLE_FAN,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void draw_background() {
    // Upper left
    glBindTexture(GL_TEXTURE_2D, texture[7]);

    GLfloat nmlUpperLeft[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxUpperLeft[] = {
       0.0f        , SCREEN_HEIGHT * 1.4f, 0.0f,
      -SCREEN_WIDTH, SCREEN_HEIGHT * 1.4f, 0.0f,
       0.0f        , 0.0f                , 0.0f,
      -SCREEN_WIDTH, 0.0f                , 0.0f
    };
    GLfloat texUpperLeft[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlUpperLeft);
    glVertexPointer(3, GL_FLOAT, 0, vtxUpperLeft);
    glTexCoordPointer(2, GL_FLOAT, 0, texUpperLeft);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
        
    // Upper right
    glBindTexture(GL_TEXTURE_2D, texture[8]);

    GLfloat nmlUpperRight[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxUpperRight[] = {
      SCREEN_WIDTH, SCREEN_HEIGHT * 1.4f, 0.0f,
      0.0f        , SCREEN_HEIGHT * 1.4f, 0.0f,
      SCREEN_WIDTH, 0.0f                , 0.0f,
      0.0f        , 0.0f                , 0.0f
    };
    GLfloat texUpperRight[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlUpperRight);
    glVertexPointer(3, GL_FLOAT, 0, vtxUpperRight);
    glTexCoordPointer(2, GL_FLOAT, 0, texUpperRight);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    
#ifdef IOS
    // testing something - another background tile -tkidd
    
    // Upper left 2
    glBindTexture(GL_TEXTURE_2D, texture[7]);

    GLfloat nmlUpperLeft2[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxUpperLeft2[] = {
       0.0f        , SCREEN_HEIGHT * 1.4f, 0.0f,
      -SCREEN_WIDTH * 2.0f, SCREEN_HEIGHT * 1.4f, 0.0f,
       0.0f        , 0.0f                , 0.0f,
      -SCREEN_WIDTH * 2.0f, 0.0f                , 0.0f
    };
    GLfloat texUpperLeft2[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlUpperLeft2);
    glVertexPointer(3, GL_FLOAT, 0, vtxUpperLeft2);
    glTexCoordPointer(2, GL_FLOAT, 0, texUpperLeft2);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    
    // Upper right 2
    glBindTexture(GL_TEXTURE_2D, texture[8]);

    GLfloat nmlUpperRight2[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxUpperRight2[] = {
      SCREEN_WIDTH * 2.0f, SCREEN_HEIGHT * 1.4f, 0.0f,
      0.0f        , SCREEN_HEIGHT * 1.4f, 0.0f,
      SCREEN_WIDTH * 2.0f, 0.0f, 0.0f,
      0.0f        , 0.0f                , 0.0f
    };
    GLfloat texUpperRight2[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlUpperRight2);
    glVertexPointer(3, GL_FLOAT, 0, vtxUpperRight2);
    glTexCoordPointer(2, GL_FLOAT, 0, texUpperRight2);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    
    // Lower left 2
       glBindTexture(GL_TEXTURE_2D, texture[9]);

       GLfloat nmlLowerLeft2[] = {
           0.0f, 0.0f, 1.0f
       };
       GLfloat vtxLowerLeft2[] = {
          0.0f        , 0.0f                , 0.0f,
         -SCREEN_WIDTH  * 2.0f, 0.0f                , 0.0f,
          0.0f        ,-SCREEN_HEIGHT * 1.4f, 0.0f,
         -SCREEN_WIDTH  * 2.0f,-SCREEN_HEIGHT * 1.4f, 0.0f
       };
       GLfloat texLowerLeft2[] = {
         1.0f, 1.0f,
         0.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f
       };

       glEnableClientState(GL_VERTEX_ARRAY);
       glEnableClientState(GL_TEXTURE_COORD_ARRAY);
       glEnableClientState(GL_NORMAL_ARRAY);

       glNormalPointer(GL_FLOAT, 0, nmlLowerLeft2);
       glVertexPointer(3, GL_FLOAT, 0, vtxLowerLeft2);
       glTexCoordPointer(2, GL_FLOAT, 0, texLowerLeft2);
       glDrawArrays(GL_TRIANGLE_STRIP,0,4);

       glDisableClientState(GL_VERTEX_ARRAY);
       glDisableClientState(GL_TEXTURE_COORD_ARRAY);
       glDisableClientState(GL_NORMAL_ARRAY);
           
       // Lower right 2
       glBindTexture(GL_TEXTURE_2D, texture[10]);
       GLfloat nmlLowerRight2[] = {
           0.0f, 0.0f, 1.0f
       };
       GLfloat vtxLowerRight2[] = {
           SCREEN_WIDTH  * 2.0f, 0.0f                , 0.0f,
         0.0f        , 0.0f                , 0.0f,
         SCREEN_WIDTH  * 2.0f,-SCREEN_HEIGHT * 1.4f, 0.0f,
         0.0f        ,-SCREEN_HEIGHT * 1.4f, 0.0f
       };
       GLfloat texLowerRight2[] = {
         1.0f, 1.0f,
         0.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f
       };

       glEnableClientState(GL_VERTEX_ARRAY);
       glEnableClientState(GL_TEXTURE_COORD_ARRAY);
       glEnableClientState(GL_NORMAL_ARRAY);

       glNormalPointer(GL_FLOAT, 0, nmlLowerRight2);
       glVertexPointer(3, GL_FLOAT, 0, vtxLowerRight2);
       glTexCoordPointer(2, GL_FLOAT, 0, texLowerRight2);
       glDrawArrays(GL_TRIANGLE_STRIP,0,4);

       glDisableClientState(GL_VERTEX_ARRAY);
       glDisableClientState(GL_TEXTURE_COORD_ARRAY);
       glDisableClientState(GL_NORMAL_ARRAY);
#endif
        
    // Lower left
    glBindTexture(GL_TEXTURE_2D, texture[9]);

    GLfloat nmlLowerLeft[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxLowerLeft[] = {
       0.0f        , 0.0f                , 0.0f,
      -SCREEN_WIDTH, 0.0f                , 0.0f,
       0.0f        ,-SCREEN_HEIGHT * 1.4f, 0.0f,
      -SCREEN_WIDTH,-SCREEN_HEIGHT * 1.4f, 0.0f
    };
    GLfloat texLowerLeft[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlLowerLeft);
    glVertexPointer(3, GL_FLOAT, 0, vtxLowerLeft);
    glTexCoordPointer(2, GL_FLOAT, 0, texLowerLeft);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
        
    // Lower right
    glBindTexture(GL_TEXTURE_2D, texture[10]);
    GLfloat nmlLowerRight[] = {
        0.0f, 0.0f, 1.0f
    };
    GLfloat vtxLowerRight[] = {
      SCREEN_WIDTH, 0.0f                , 0.0f,
      0.0f        , 0.0f                , 0.0f,
      SCREEN_WIDTH,-SCREEN_HEIGHT * 1.4f, 0.0f,
      0.0f        ,-SCREEN_HEIGHT * 1.4f, 0.0f
    };
    GLfloat texLowerRight[] = {
      1.0f, 1.0f,
      0.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glNormalPointer(GL_FLOAT, 0, nmlLowerRight);
    glVertexPointer(3, GL_FLOAT, 0, vtxLowerRight);
    glTexCoordPointer(2, GL_FLOAT, 0, texLowerRight);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void draw_bigspace() {
        glColor4ub(255, 255, 255, 128);
        glTranslatef(-SCREEN_WIDTH*2,-SCREEN_HEIGHT*2,0.0f);
        draw_background();
        glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
        draw_background();
        glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
        draw_background();
        glTranslatef(-SCREEN_WIDTH*2,0.0f,0.0f);

        glTranslatef(-SCREEN_WIDTH*2,+SCREEN_HEIGHT*2,0.0f);
        draw_background();
        glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
        draw_background();
        glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
        draw_background();
        glTranslatef(-SCREEN_WIDTH*2,0.0f,0.0f);

        glTranslatef(-SCREEN_WIDTH*2,+SCREEN_HEIGHT*2,0.0f);
        draw_background();
        glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
        draw_background();
        glTranslatef(+SCREEN_WIDTH*2,0.0f,0.0f);
        draw_background();
        glTranslatef(-SCREEN_WIDTH*2,0.0f,0.0f);
}

#define SDL_LOCKIFMUST(s) (SDL_MUSTLOCK(s) ? SDL_LockSurface(s) : 0)
#define SDL_UNLOCKIFMUST(s) { if(SDL_MUSTLOCK(s)) SDL_UnlockSurface(s); }

int invert_surface_vertical(SDL_Surface* surface)
{
	Uint8* t;
	register Uint8* a, * b;
	Uint8* last;
	register Uint16 pitch;

	if (SDL_LOCKIFMUST(surface) < 0)
		return -2;

	/* do nothing unless at least two lines */
	if (surface->h < 2) {
		SDL_UNLOCKIFMUST(surface);
		return 0;
	}

	/* get a place to store a line */
	pitch = surface->pitch;
	t = (Uint8*)malloc(pitch);

	if (t == NULL) {
		SDL_UNLOCKIFMUST(surface);
		return -2;
	}

	/* get first line; it's about to be trampled */
	memcpy(t, surface->pixels, pitch);

	/* now, shuffle the rest so it's almost correct */
	a = (Uint8*)surface->pixels;
	last = a + pitch * (surface->h - 1);
	b = last;

	while (a < b) {
		memcpy(a, b, pitch);
		a += pitch;
		memcpy(b, a, pitch);
		b -= pitch;
	}

	/* in this shuffled state, the bottom slice is too far down */
	memmove(b, b + pitch, last - b);

	/* now we can put back that first row--in the last place */
	memcpy(last, t, pitch);

	/* everything is in the right place; close up. */
	free(t);
	SDL_UNLOCKIFMUST(surface);

	return 0;
}

/*  helper function (used in texture loading)
    returns next power of two greater than or equal to x
*/
int
nextPowerOfTwo(int x)
{
    int val = 1;
    while (val < x) {
        val *= 2;
    }
    return val;
}

int LoadGLTextures()		
{
/////////////////////
	SDL_Surface* Surface;
/////////////////////


	// NOTE TO OTHERS:
	//
	//		Does anyone know how to load 8-bit bitmaps from
	//		a resource file?  Right now all of the textures
	//		are 16-bit and really don't need to be.  Thanks!


	int		loop = 0;
	int		Status = TRUE;
	const char* Texture[] = {	
								"Res/ship.bmp",			//0
								"Res/Star.bmp",			//1
								"Res/shield.bmp",		//2
								"Res/blast.bmp",		//3
								"Res/smoke.bmp",		//4
								"Res/font.bmp",			//5
								"Res/rock.bmp",			//6
								"Res/space_ul.bmp",		//7 *
								"Res/space_ur.bmp",		//8 *
								"Res/space_ll.bmp",		//9 *
								"Res/space_lr.bmp",		//10 *						
								"Res/saucer.bmp",		//11
								"Res/logo.bmp",			//12 *
								"Res/extraship.bmp" };	//13

	glGenTextures(NUM_TEXTURES, &texture[0]);

	for (loop = 0; loop < NUM_TEXTURES; loop++)
	{
        printf("Texture: %s\n", Texture[loop]);
		Surface = SDL_LoadBMP(Texture[loop]);
		invert_surface_vertical(Surface);

		if (Surface)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glBindTexture(GL_TEXTURE_2D, texture[loop]);
			if ((loop > 6) && (loop < 11) || (loop == 12))
			{
#ifdef GLES
                int bpp;                    /* texture bits per pixel */
                Uint32 Rmask, Gmask, Bmask, Amask;  /* masks for pixel format passed into OpenGL */
                SDL_Surface *bmp_surface_rgba8888;  /* this serves as a destination to convert the BMP
                                                       to format passed into OpenGL */

                /* Grab info about format that will be passed into OpenGL */
                SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bpp, &Rmask, &Gmask,
                                           &Bmask, &Amask);
                /* Create surface that will hold pixels passed into OpenGL */
                bmp_surface_rgba8888 =
                    SDL_CreateRGBSurface(0, Surface->w, Surface->h, bpp, Rmask,
                                         Gmask, Bmask, Amask);
                /* Blit to this surface, effectively converting the format */
                SDL_BlitSurface(Surface, NULL, bmp_surface_rgba8888, NULL);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             nextPowerOfTwo(Surface->w),
                             nextPowerOfTwo(Surface->h),
                             0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Surface->w, Surface->h, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, bmp_surface_rgba8888->pixels);
#else
                glTexImage2D(GL_TEXTURE_2D, 0, 3, Surface->w, Surface->h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, Surface->pixels);
#endif
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			}
			else
			{
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, true);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
#ifdef GLES
                int bpp;                    /* texture bits per pixel */
                Uint32 Rmask, Gmask, Bmask, Amask;  /* masks for pixel format passed into OpenGL */
                SDL_Surface *bmp_surface_rgba8888;  /* this serves as a destination to convert the BMP
                                                       to format passed into OpenGL */

                /* Grab info about format that will be passed into OpenGL */
                SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bpp, &Rmask, &Gmask,
                                           &Bmask, &Amask);
                /* Create surface that will hold pixels passed into OpenGL */
                bmp_surface_rgba8888 =
                    SDL_CreateRGBSurface(0, Surface->w, Surface->h, bpp, Rmask,
                                         Gmask, Bmask, Amask);
                /* Blit to this surface, effectively converting the format */
                SDL_BlitSurface(Surface, NULL, bmp_surface_rgba8888, NULL);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             nextPowerOfTwo(Surface->w),
                             nextPowerOfTwo(Surface->h),
                             0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Surface->w, Surface->h, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, bmp_surface_rgba8888->pixels);
#else
                glTexImage2D(GL_TEXTURE_2D, 0, 3, Surface->w, Surface->h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, Surface->pixels);
#endif
			}
		}
		else
		{
			Status=FALSE;
		}
	}
	return Status;
}



// NeHe
GLvoid BuildFont(GLvoid)								// Build Our Font Display List
{
	float	cx;												// Holds Our X Character Coord
	float	cy;												// Holds Our Y Character Coord
	int loop = 0;

#if 0
	font_list=glGenLists(256);							// Creating 256 Display Lists
#endif
	glBindTexture(GL_TEXTURE_2D, texture[5]);		// Select Our Font Texture
	for (loop=0; loop<256; loop++)					// Loop Through All 256 Lists
	{
		cx=float(loop%16)/16.0f;						// X Position Of Current Character
		cy=float(loop/16)/16.0f;						// Y Position Of Current Character

#if 0
		glNewList(font_list+loop,GL_COMPILE);		// Start Building A List
			glBegin(GL_TRIANGLE_STRIP);				// Use A Triangle Strip For Each Character
				glTexCoord2f(cx+0.0625f,1-cy);		// Texture Coord (Top Right)
				glVertex2f(1.0f,1.0f);					// Vertex Coord (Top Right)
				glTexCoord2f(cx,1-cy);					// Texture Coord (Top Left)
				glVertex2f(0.0f,1.0f);					// Vertex Coord (Top Left)
				glTexCoord2f(cx+0.0625f,1-cy-0.0625f);	// Texture Coord (Bottom Right)
				glVertex2f(1.0f,0.0f);					// Vertex Coord (Bottom Right)
				glTexCoord2f(cx,1-cy-0.0625f);		// Texture Coord (Bottom Left)
				glVertex2f(0.0f,0.0f);					// Vertex Coord (Bottom Left)
			glEnd();											// Done Building Our Quad (Character)
#else
            GLfloat vtx[] = {
             1.0f, 1.0f,
             0.0f, 1.0f,
             1.0f, 0.0f,
             0.0f, 0.0f
           };
           GLfloat tex[] = {
             cx+0.0625f, 1-cy,
             cx, 1-cy,
             cx+0.0625f, 1-cy-0.0625f,
             cx, 1-cy-0.0625f
           };
           
           glEnableClientState(GL_VERTEX_ARRAY);
           glEnableClientState(GL_TEXTURE_COORD_ARRAY);

           glVertexPointer(2, GL_FLOAT, 0, vtx);
           glTexCoordPointer(2, GL_FLOAT, 0, tex);
           glDrawArrays(GL_TRIANGLE_STRIP,0,4);

           glDisableClientState(GL_VERTEX_ARRAY);
           glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
           glTranslatef(0.6f,0.0f,0.0f);				// Move To The Right Of The Character
#if 0
		glEndList();										// Done Building The Display List
#endif
	}															// Loop Until All 256 Are Built
}

void print_character(int i) {
    float    cx;                                                // Holds Our X Character Coord
    float    cy;                                                // Holds Our Y Character Coord
    glBindTexture(GL_TEXTURE_2D, texture[5]);        // Select Our Font Texture

    cx=float(i%16)/16.0f;                        // X Position Of Current Character
    cy=float(i/16)/16.0f;                        // Y Position Of Current Character

    GLfloat vtx[] = {
        1.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };
    GLfloat tex[] = {
        cx+0.0625f, 1-cy,
        cx, 1-cy,
        cx+0.0625f, 1-cy-0.0625f,
        cx, 1-cy-0.0625f
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vtx);
    glTexCoordPointer(2, GL_FLOAT, 0, tex);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glTranslatef(0.6f,0.0f,0.0f);                // Move To The Right Of The Character
}


void print_string(char *s, int set) {
    size_t length = strlen(s);
    size_t i = 0;
    for (; i < length; i++) {
        //glCallList(s[i] - 17);
        print_character(s[i] - 32 + (128 * set));
    }
}

void print_string2(const char s[], int set) {
    
    size_t length = strlen(s);
    size_t i = 0;
    for (; i < length; i++) {
//        glCallList(s[i] - 17);
        print_character(s[i] - 32 + (128 * set));
    }
}

GLvoid KillFont(GLvoid)									// Delete The Font From Memory
{
#if 0
	glDeleteLists(font_list,256);						// Delete All 256 Display Lists
#endif
}



// (NeHe)
GLvoid glPrintVar(int set, const GLfloat& x, const GLfloat& y, const char *string, ...)
{
	char	text[256];					// Holds Our String
	va_list	ap;						// Pointer To List Of Arguments

	// Make sure set is valid
	if ((set > 1) || (set < 0))
		set=1;
	
	// Make sure string has length
	if (string == NULL)
		return;
	
	// Parse The String For Variables And Convert 
	// Symbols To Actual Numbers
	va_start(ap, string);
		vsprintf(text, string, ap);
	va_end(ap);

	// Reset The Projection Matrix
	glPushMatrix();

	// Setup rendering options
	renderenable(render_blend | render_texture);
	renderdisable(render_depthtest | render_lighting | render_wireframe);

    #if 0
        glListBase(font_list - 32 + (128 * set));                // Choose The Font Set (0 or 1)
    #endif
        glBindTexture(GL_TEXTURE_2D, texture[5]);                // Select Our Font Texture
    #if 0
        glTranslated(x, y, -30);                                    // Position The Text
        glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);    // Write The Text To The Screen
    #else
        glTranslatef(x, y, -30);                                    // Position The Text
        print_string(text, set);
    #endif

	glPopMatrix();

}


GLvoid glPrint(int set, const GLfloat& x, const GLfloat& y, const char *text)
{

	if (text==NULL)
		return;
	
	if (set>1)
		set=1;

	// Reset The Projection Matrix
	glPushMatrix();

	// Setup rendering options
	renderenable(render_blend | render_texture);
	renderdisable(render_depthtest | render_lighting | render_wireframe);

    #if 0
        glListBase(font_list - 32 + (128 * set));                // Choose The Font Set (0 or 1)
    #endif
        glBindTexture(GL_TEXTURE_2D, texture[5]);                // Select Our Font Texture
    #if 0
        glTranslated(x,y,-30);                                        // Position The Text
        glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);    // Write The Text To The Screen
    #else
        glTranslatef(x, y, -30);
        print_string2(text, set);
    #endif

	// Restore the matrix
	glPopMatrix();

}

GLvoid glPrintChar(const GLfloat& x, const GLfloat& y, int c)
{
    // Reset The Projection Matrix
    glPushMatrix();

    // Setup rendering options
    renderenable(render_blend | render_texture);
    renderdisable(render_depthtest | render_lighting | render_wireframe);

    #if 0
        glListBase(font_list - 32 + (128 * set));                // Choose The Font Set (0 or 1)
    #endif
        glBindTexture(GL_TEXTURE_2D, texture[5]);                // Select Our Font Texture
    #if 0
        glTranslated(x,y,-30);                                        // Position The Text
        glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);    // Write The Text To The Screen
    #else
        glTranslatef(x, y, -30);
        print_character(c);
    #endif

    // Restore the matrix
    glPopMatrix();

}


//
// TCW, but based 99% on code from Jeff
//
GLvoid glPrintCentered(int set, const GLfloat& y, const char *text)
{

	GLfloat x = 0;

	if (text == NULL)
		return;
	
	if (set>1)
		set=1;

	// Calc x
	x = -((GLfloat(strlen(text)) + 0.75f) * 0.3f);

	// Call glPrint
	glPrint(set, x, y, text);

}


//
// TCW, but based 99% on code from Jeff
//
GLvoid glPrintCenteredVar(int set, const GLfloat& y, const char *string, ...)
{

	if (set>1)
		set=1;

	char	text[256];									// Holds Our String
	va_list	ap;											// Pointer To List Of Arguments
	GLfloat x = 0;

	if (string==NULL)
		return;
	
	// Parse The String For Variables And Convert 
	// Symbols To Actual Numbers
	va_start(ap, string);
		vsprintf(text, string, ap);
	va_end(ap);

	// Calc x
	x = -((GLfloat(strlen(text)) + 0.75f) * 0.3f);

	// Call glPrint
	glPrint(set, x, y, text);

}


//--------------------------------------------------------
// TCW
// This routine gives me an easy way to post one line
// of messages along the bottom of the screen to notify
// the user when controls are enabled/disabled and 
// rendering options are toggled.
//--------------------------------------------------------
void UpdateMessage(const char *string, ...)
{

	char	text[256];
	va_list	ap;

	// Deactivate any messages if the 
	// message string is not specified
	if (string == NULL)
	{
		g_lMessageEndTime = 0;
		return;
	}
	
	// Parse The String For Variables And Convert 
	// Symbols To Actual Numbers
	va_start(ap, string);
		vsprintf(text, string, ap);
	va_end(ap);

	// Got rid of UpdateMessage messages
	con_printf(text);
}

// (NeHe: http://nehe.gamedev.net/article/replacement_for_gluperspective/21002/)
void perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
    const GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;

    //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    fH = tan( fovY / 360 * pi ) * zNear;
    fW = fH * aspect;

    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}


// (NeHe)
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);					// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);					// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	perspectiveGL(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);					// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}



BOOL GLSupportsExtension(char extName[])
{

	if (strstr((const char*)glGetString(GL_EXTENSIONS), extName) == NULL)
		return FALSE;

	return TRUE;
}


/* SetVsync

  This allows me to set/unset Vsync as necessary.  Since 
  Disasteroids' rendering isn't throttled, when Vsync is enabled,
  the game stutters in attempts to remain at a constant fps.

*/
/*

	Since very few video cards support this extension at 
	this time, this routine will be simplified to only 
	set the flags to the correct values, and this routine
	may be reinstated at a later time


typedef BOOL (APIENTRY * PFNWGLEXTSWAPINTERVALPROC) (GLint interval);
BOOL SetVsync(const BOOL value)
{
	
	PFNWGLEXTSWAPINTERVALPROC wglSwapIntervalEXT  = NULL;

	wglSwapIntervalEXT = (PFNWGLEXTSWAPINTERVALPROC) wglGetProcAddress("wglSwapIntervalEXT"); 
	if (wglSwapIntervalEXT != NULL)
	{
		if (value)
		{
			wglSwapIntervalEXT(0);
			Cvar_SetString("VsyncEnabled", "0");
			UpdateMessage("VSYNC DISABLED");
		}
		else		
		{
			wglSwapIntervalEXT(1);
			Cvar_SetString("VsyncEnabled", "-1");
			UpdateMessage("VSYNC ENABLED");
		}

		return TRUE;
	}

	UpdateMessage("OpenGL extension wglSwapIntervalEXT not supported");

	return FALSE;
}
*/

/*
	This routine DOES NOT actually play with a video card's vsync
	settings.

	What this routine DOES do is allow a video card with vsync to
	play the game when enabled by retrieving the refresh timing rate
	and setting the vsync enable flag, which tells the main loop
	to run in vsync mode.

  This is useful for those people running cards like the ATI Rage
  and Intel integrated 3d which don't allow a program to disable 
  vsync from the code.

*/

BOOL SetVsync(const BOOL value)
{
	if (value)
	{
		Cvar_SetString("VsyncEnabled", "0");
		// UpdateMessage("VSYNC DISABLED");
	}
	else
	{
		Cvar_SetString("VsyncEnabled", "-1");
		// UpdateMessage("VSYNC ENABLED");

		// Update the vsync timing offsets
		g_fSwapBufferInterval = GetRefreshTimingRate();

	}

	return TRUE;
}




/*
	
	SetGammaRamp(double gamma, double bright, double contrast)
	gamma [0,255] (increased gamma < 127, decreased gamma > 127)
	bright [0,255] (darker < 127, brighter > 127)
	contrast [0,255] (lower < 127, higher > 127)

	Thanks to 'DFrey' for posting this information in the 
	discussion boards on OpenGL.org.

*/

void SetGammaRamp(double gamma, double bright, double contrast)
{
	static Uint16 gamma_ramp[3][256];
	double v;
	const double ft = 2.0 / 255.0;
	static bool savedramp = false;

	// save old gamma ramp if this is first time modified	
	// FIXME: should get the gamma ramp anyway to see if it has been changed from what was	last set	
	if (!savedramp)
	{
		SDL_GetWindowGammaRamp(SDL_window, old_gamma_ramp[0], old_gamma_ramp[1], old_gamma_ramp[2]);
		savedramp = true;
	}
	
	for (int x = 0; x < 256; x++)
	{
		if (255 == gamma)
		{
			v = 0.0;
		}
		else
		{
			v = pow((double)x / 255, gamma / (255 - gamma));
		}
		
		v += ft * bright - 1.0;
		v += (ft * (double)x - 1.0) * (contrast / 255.0 -.5) * (ft * contrast + 1.0);
		v *= 65535;
		if (v<0)
		{
			v=0;
		}
		else if (v>65535)
		{
				v=65535;
		}
				
		gamma_ramp[0][x] = v;
		gamma_ramp[1][x] = v;
		gamma_ramp[2][x] = v;
	}
	
	SDL_SetWindowGammaRamp(SDL_window, gamma_ramp[0], gamma_ramp[1], gamma_ramp[2]);	
}


void SetGamma()
{

	double dblGamma		= double(127 + (+0.5 - g_cvGammaGamma.value)			* 127);
	double dblBrightness	= double(127 + (-0.5 + g_cvGammaBrightness.value)	* 127);
	double dblContrast	= double(127 + (-0.5 + g_cvGammaContrast.value)		* 127);

	// UpdateMessage("Gamma %f %f %f", dblGamma, dblBrightness, dblContrast);

	SetGammaRamp(dblGamma, dblBrightness, dblContrast);
}



//======================================================
// (NeHe)
// TCW
//	I made some modifications to load all of the
//	various elements that I'm using in the program
//	such as the lighting model, 3D object loading,
//	font building, etc.
//======================================================
int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{

	// Required routines to enable texture mapping
	if (!LoadGLTextures())	
		return FALSE;									// If Texture Didn't Load Return FALSE

	// Enable Texture Mapping
	renderenable(render_texture);

    #ifndef GLES
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    #endif

	glShadeModel(GL_FLAT);								// Enable Flat Shading
	
	glClearColor(0.0f, 1.0f, 0.0f, 0.0f);			// Black Background

    #ifdef GLES
        #define glClearDepth glClearDepthf
    #endif

    glClearDepth(1.0f);									// Depth Buffer Setup

	renderenable(render_depthtest);					// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	
	// Clear The Screen And The Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	
	// Attempt to use this to fix the lighting...
	glEnable(GL_NORMALIZE); 

	renderenable(render_lighting);					// Enable Lighting
	
	//	glEnable(GL_LIGHT0);								// Quick And Dirty Lighting (Assumes Light0 Is Set Up)
	
	// Fixed by NeHe
	glEnable(GL_COLOR_MATERIAL);
#ifndef GLES
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glMateriali(GL_FRONT,GL_SHININESS,128);
#endif
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	// Added for drawing transparent objects
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);					// Set The Blending Function For Translucency
	renderenable(render_blend);							// Enable Blending

	// Setup default light
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);		// Setup The Ambient Light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);		// Setup The Diffuse Light
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);	// Position The Light
	glEnable(GL_LIGHT1);								// Enable Light One
	
	// Build ObjectLists of 3d objects
	Gen3DObjectList();

	// Build the font
	//BuildFont();
	
	// Initialization Went OK
	return TRUE;
}



void LoadCfg()
{
	FILE *File = NULL;

	const int MAX_LINE_LEN = 80;
	char line[MAX_LINE_LEN];

	// Open configuration file
	File = fopen(csConfigFilename, "r");
	
	// Make sure file exists
	if (!File)
		return;

	// Read each line in the file until the end of the file
	while (fgets(line, MAX_LINE_LEN, File) != NULL)
	{
		
		// Each line in the .cfg file should contain two tokens;
		// the variable name and the string value.  Lines with more
		// than two tokens are ignored
		if (Tokenize(line) == 2)
		{
			// Set the cvar's string value
			if (!Cvar_SetString(g_Tokens[0], g_Tokens[1]))
			{
				/*
				// Display MessageBox with error message
				char sErrMsg[80];
				sprintf(sErrMsg, "Unknown Cvar '%s' cannot set value '%s'", g_Tokens[0], g_Tokens[1]);
				MessageBox(NULL, sErrMsg, "UNKNOWN CVAR", 0);
				*/
			}
		}
	}

	// Close File
	fclose(File);

}



// TCW
// Load highscores
BOOL LoadHighScores(char *Filename)				
{
	FILE *File = NULL;

	// Exit this function if Filename is not specified
	if (!Filename)
		return FALSE;
	
	// Open the file named Filename
	File = fopen(Filename,"r");

	// Process the file if it was opened successfully
	if (File)
	{
		fread(highscores, sizeof(highscoreinfo), MAX_HIGHSCORES, File);
		fclose(File);
		return TRUE;
	}

	return FALSE;
}


// TCW
// Save highscores
BOOL SaveHighScores(char *Filename)				
{
	FILE *File=NULL;

	if (g_bCheatsEnabled)
		return FALSE;
	
	if (!Filename)
		return FALSE;
	
	File=fopen(Filename,"w");
	if (File)
	{
		fwrite( highscores, sizeof(highscoreinfo), MAX_HIGHSCORES, File);
		fclose(File);
		return TRUE;
	}
	
	return FALSE;
}


/*

	I'm using this routine in place of glClear(GL_COLOR_BUFFER_BIT)
	where possible.  It seems to make a difference especially on 
	ATI cards

*/
GLvoid BlackOutScreen(GLvoid)
{
	// This will allow me to draw transparent black
	// shapes on the screen.  Without this the box 
	// isn't drawn because black is the transparent color
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Overwrite the current display with an untextured black square
	renderdisable(render_lighting | render_depthtest | render_texture | render_blend);
	glColor4ub(0, 0, 0, 0);
	const float fx = 0;
	const float fy = 0;
	const float fz = -4.0f;
	const float fs = 3.0f;
    #if 0
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2d(1,1); glVertex3f(fx + fs, fy + fs, fz);
        glTexCoord2d(0,1); glVertex3f(fx - fs, fy + fs, fz);
        glTexCoord2d(1,0); glVertex3f(fx + fs, fy - fs, fz);
        glTexCoord2d(0,0); glVertex3f(fx - fs, fy - fs, fz);
        glEnd();
    #else
        GLfloat vtx[] = {
            fx + fs, fy + fs, fz,
            fx - fs, fy + fs, fz,
            fx + fs, fy - fs, fz,
            fx - fs, fy - fs, fz
        };
        GLfloat tex[] = {
            1,1,
            0,1,
            1,0,
            0,0
        };
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vtx);
        glTexCoordPointer(2, GL_FLOAT, 0, tex);
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    #endif
	// Restore the blend func for translucency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}


void DrawGLBackground(GLvoid)
{
	int	i			= 0;
	float	fScalar	= 0;
	float	fx			= 0;
	float	fy			= 0;
	float	fz			= 0;
	float	fs			= 0;

	if (g_cvCopernicusMode.value != 0)
	{

		// Pre-Copernicus Mode

		// switch(g_nSpaceInMotion)
		switch((int)g_cvSpaceMode.value)
		{

		// Method zero draws the background twice,
		// once shifted for the player's movement,
		// then again further away and rotated 180
		// degrees.
		//
		// It looks really cool, but I don't know
		// when to reset the view, so I'm not going
		// to use it.
		case 0:

			// The screen needs to be cleared for this to work
			BlackOutScreen();
	
			glPushMatrix();

			renderenable(render_texture | render_blend);
			renderdisable(render_lighting | render_depthtest);
			
			fScalar = float(0.25f * sin(g_fNow));
			glTranslatef(fScalar, fScalar, -30.0f - fScalar);
			glRotatef(fScalar, fScalar, -fScalar, 1.0f);

// ******************** Modified By NeHe (04/30/00) ********************
			if (g_fBackgroundX > SCREEN_WIDTH*2)
			{
				g_fBackgroundX -= SCREEN_WIDTH*2;
			}
			if (g_fBackgroundX < -SCREEN_WIDTH*2)
			{
				g_fBackgroundX += SCREEN_WIDTH*2;
			}

			if (g_fBackgroundY > SCREEN_HEIGHT*2)
			{
				g_fBackgroundY -= SCREEN_HEIGHT*2;
			}
			if (g_fBackgroundY < -SCREEN_HEIGHT*2)
			{
				g_fBackgroundY += SCREEN_HEIGHT*2;
			}

			glTranslatef(g_fBackgroundX, g_fBackgroundY, 0);
			glColor4ub(255, 255, 255, 255);
			//glCallList(bigspace_list);
            draw_bigspace();

			renderenable(render_blend);

			glTranslatef(0, 0, 5.0f);
			glColor4ub(255, 255, 255, 128);
			glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
			//glCallList(bigspace_list);
            draw_bigspace();

			fScalar = -0.25f * (float)sin(g_fNow * 0.001f);
// ******************** Modified By NeHe (04/30/00) ********************

			glPopMatrix();

			break;

		// Draw background then stars
		// (The break is missing on purpose)
		case 1:
			// Draw background
			renderenable(render_texture);
			renderdisable(render_blend | render_lighting | render_depthtest);
			glColor4ub(255,255,255,128);
			fScalar = 0.25f * (float)sin(g_fNow);

			glPushMatrix();
				glTranslatef(fScalar, fScalar, -30.0f - fScalar);
				glRotatef(fScalar, fScalar, -fScalar, 1.0f);
				//glCallList(background_list);
                draw_background();
			glPopMatrix();

		// Draw stars
		case 2:
			

			// If the stars alone are being drawn, the old
			// background needs to be cleared
			if ((int)g_cvSpaceMode.value == 2) 
			{

// Change this next line to #if 0 to enable motion blur code
#if 1
				BlackOutScreen();

/*				
	Motion blur
*/
#else
				// This will allow me to draw transparent black
				// shapes on the screen.  Without this the box 
				// isn't drawn because black is the transparent color
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				// Overwrite the current display with an untextured black square
				renderdisable(render_lighting | render_depthtest | render_texture);
				renderenable(render_blend);
				glColor4ub(0, 0, 0, 25);
				fx = 0;
				fy = 0;
				fz = -4;
				fs = 3;
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2d(1,1); glVertex3f(fx + fs, fy + fs, fz);
					glTexCoord2d(0,1); glVertex3f(fx - fs, fy + fs, fz);
					glTexCoord2d(1,0); glVertex3f(fx + fs, fy - fs, fz);
					glTexCoord2d(0,0); glVertex3f(fx - fs, fy - fs, fz);
				glEnd();

				// Restore the blend func for translucency
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
#endif

			}


			// Setup global flag that tells main routine 
			// to recalc stars' positions next cycle
			g_bStarsActive = TRUE;

			// Triangle strips and glVertex3f used to 
			// speed up rendering.  See nehe.gamedev.net
			// lesson 19.
			renderenable(render_texture | render_blend );
			renderdisable(render_depthtest | render_lighting);
			glBindTexture(GL_TEXTURE_2D, texture[1]);
			glColor4ub(255, 255, 255, 240);
			for (i = 0; i < NUM_STARS; i++)
			{

				fx = stars[i].x;
				fy = stars[i].y;
				fz = stars[i].z;
				fs = stars[i].size;

#if 0
                glBegin(GL_TRIANGLE_STRIP);                        // Build Quad From A Triangle Strip
                    glTexCoord2d(1,1); glVertex3f(fx + fs, fy + fs, fz); // Top Right
                    glTexCoord2d(0,1); glVertex3f(fx - fs, fy + fs, fz); // Top Left
                    glTexCoord2d(1,0); glVertex3f(fx + fs, fy - fs, fz); // Bottom Right
                    glTexCoord2d(0,0); glVertex3f(fx - fs, fy - fs, fz); // Bottom Left
                glEnd();                                        // Done Building Triangle Strip
#else
                GLfloat vtx[] = {
                    fx + fs, fy + fs, fz,
                    fx - fs, fy + fs, fz,
                    fx + fs, fy - fs, fz,
                    fx - fs, fy - fs, fz
                };
                GLfloat tex[] = {
                    1,1,
                    0,1,
                    1,0,
                    0,0
                };
                
                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);

                glVertexPointer(3, GL_FLOAT, 0, vtx);
                glTexCoordPointer(2, GL_FLOAT, 0, tex);
                glDrawArrays(GL_TRIANGLE_STRIP,0,4);

                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif										// Done Building Triangle Strip
			}

			break;
		}
	}


	else
	{

		// Post-Copernicus Mode - Ship NOT center of game

		switch((int)g_cvSpaceMode.value)
		{
		case 0:
			renderenable(render_texture); 
			renderdisable(render_lighting | render_blend | render_depthtest);
			glColor4ub(255,255,255,255);
			glPushMatrix();
				glTranslatef(0.0f, 0.0f, -35.0f);
				//glCallList(background_list);
                draw_background();
			glPopMatrix();
			break;
		
		case 1:
			renderenable(render_texture);
			renderdisable(render_blend | render_lighting | render_depthtest);
			glColor4ub(255,255,255,255);
			glPushMatrix();
				fScalar = float(0.25f * sin(g_fNow));
				glTranslatef(fScalar, fScalar, -30.0f - fScalar);
				glRotatef(fScalar, fScalar, -fScalar, 1.0f);
				//glCallList(background_list);
                draw_background();
			glPopMatrix();
			break;

		case 2:

			// Clear screen
			BlackOutScreen();

			renderenable(render_texture);
			renderdisable(render_blend | render_lighting | render_depthtest);
			glColor4ub(255, 255, 255, 255);
			glPushMatrix();

			/* */
			glTranslatef(0.0f, 0.0f, -30.0f);
			glRotatef(180.0f * float(cos(g_fBackgroundRot/10.0f)), 0.0f, 0.0f, 1.0f);
			glTranslatef((3.0f*float(cos(g_fBackgroundRot)))+(2.1f*float(sin(g_fBackgroundRot*1.4f))),(2.8f*float(sin(g_fBackgroundRot)))+(1.3f*float(sin(g_fBackgroundRot*1.4f))),0.0f);
			glRotatef(10.0f*float(sin(g_fBackgroundRot*1.2f)),1.0f,0.0f,0.0f);
			//glCallList(background_list);
            draw_background();
			/* */

			glPopMatrix();
			break;
		}
	}

	glPopMatrix();
}


GLvoid SetupGLLights(GLvoid)
{

	int		i						= 0;
	int		nLightIdx			= 0;
	int		nLightCount			= 0;
	float		fRemainingLifePct	= 0;
	GLfloat	afLightData[4]		= {0.0f, 0.0f, 0.0f, 0.0f};
	BOOL		b3d					= FALSE;
	float		ftemp					= 0;
	
	static int nLastLightCount;

	// Setup 3d flag
	b3d = g_cv3dglasses.value != 0;

	// Setup LIGHT1
	if (b3d)
	{
		GLfloat LightAmbient[]	= { 0.05f, 0.05f, 0.05f, 1.0f };
		GLfloat LightDiffuse[]	= { 0.4f, 0.4f, 0.4f, 1.0f }; 
		GLfloat LightPosition[]	= { 7.0f, 7.0f, 5.0f, 1.0f };
	}
	else
	{
		GLfloat LightAmbient[]	= { 0.05f, 0.05f, 0.05f, 1.0f };
		GLfloat LightDiffuse[]	= { 0.3f, 0.3f, 0.7f, 1.0f }; 
		GLfloat LightPosition[]	= { 7.0f, 7.0f, 5.0f, 1.0f };
	}
	// Setup default light
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);		// Setup The Ambient Light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);		// Setup The Diffuse Light
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);	// Position The Light
	glEnable(GL_LIGHT1);								// Enable Light One

	// Position and enable lights for light sources
	nLightCount = 0;
	for (i = 0; i < MAX_ACTORS; i++) 
	{

		if (((actors[i].type == ACTOR_BLAST) || 
			(actors[i].type == ACTOR_SHOT)  ||
			(actors[i].type == ACTOR_SFIRE) ||
			(actors[i].type == ACTOR_PLAYER) ||
			(actors[i].type == ACTOR_LSAUCER)||
			(actors[i].type == ACTOR_SHIELDS)) && 
			(actors[i].active))
		{

			// Increment light index number
			nLightIdx = GL_LIGHT2 + nLightCount;

			// Make sure light index is in bounds
			if (nLightIdx < GL_LIGHT0 + GL_MAX_LIGHTS) 
			{

				afLightData[0] = actors[i].x;
				afLightData[1] = actors[i].y;
				afLightData[2] = actors[i].z + 1;
				afLightData[3] = 1.0f;
				glLightfv(nLightIdx, GL_POSITION, afLightData);

				switch(actors[i].type)
				{
				case ACTOR_SFIRE:
					
					if (b3d)
					{
						afLightData[0] = 0.66f;
						afLightData[1] = 0.66f;
						afLightData[2] = 0.66f;
						afLightData[3] = 1.0f;
					}
					else
					{
						afLightData[0] = 1.0f;
						afLightData[1] = 0.25f;
						afLightData[2] = 0.25f;
						afLightData[3] = 1.0f;
					}
					break;

				case ACTOR_BLAST:

					// Compute fLifePctRemaining
					fRemainingLifePct = 1.0f - actors[i].LifePercent;
					
					if (b3d)
					{
						ftemp = (0.75f * fRemainingLifePct) + 0.25f;
						afLightData[0] = ftemp;
						afLightData[1] = ftemp;
						afLightData[2] = ftemp;
						afLightData[3] = 1.0f;
					}
					else
					{
						afLightData[0] = (0.75f * fRemainingLifePct) + 0.25f;
						afLightData[1] = (0.75f * fRemainingLifePct) + 0.25f;
						afLightData[2] = 0.25f;
						afLightData[3] = 1.0f;
					}
					break;
				
				case ACTOR_SHIELDS:
					if (b3d)
					{
						ftemp = 1.0f * ((255 - player->ShieldsPower) / 255);
						afLightData[0] = ftemp;
						afLightData[1] = ftemp;
						afLightData[2] = ftemp;
						afLightData[3] = ftemp;
					}
					else
					{
						afLightData[0] = 1.0f * ((255 - player->ShieldsPower) / 255);
						afLightData[1] = 1.0f * (player->ShieldsPower / 255);
						afLightData[2] = 0.2f;
						afLightData[3] = 0.7f;
					}
					break;

				default:
					afLightData[0] = 0.2f;
					afLightData[1] = 0.2f;
					afLightData[2] = 0.2f;
					afLightData[3] = 1.0f;
					break;

				}

				// Setup and enable the light
				glLightfv(nLightIdx, GL_DIFFUSE, afLightData);

				// Don't enable lights that are already on
				if (nLastLightCount <= nLightCount)
					glEnable(nLightIdx);

				// Increment the lights counter
				nLightCount++;

			}
			else
			{
				if (i == IDX_FIRST_SHOT) 
				{
					glPrint(0, 0.0f, 0.0f, "MAX LIGHTS Exceeded");
					glPrintVar(0, 0.0f, 1.0f, "MAX_LIGHTS=%d nLightIdx=%d", GL_MAX_LIGHTS, nLightIdx);
				}
			}
		}
	}

	// Turn off any extra lights that were activated last
	// cycle that aren't needed now
	if (nLastLightCount > nLightCount)
	{
		for (i = nLightCount; i <= nLastLightCount; i++)
		{
			glDisable(GL_LIGHT2 + i);
		}
	}
	
	// Remember the number of lights used this 
	// cycle that need to be shut off next cycle
	nLastLightCount = nLightCount;

}


// TCW
// This is where the scene is drawn
//
int DrawGLScene(GLvoid)
{

	int		i						= 0;
	int		j						= 0;
	int		nLastType			= ACTOR_NONE;
	float		fScalar				= 0;
	float		fRemainingLifePct	= 0;
	float		fx						= 0;
	float		fy						= 0;
	float		fz						= 0;
	float		fs						= 0;
	char		c						= 0;

	BOOL		b3d					= FALSE;
	int		nLens					= 0;
	int		nMaxLens				= 0;

	const int EnableVomitComet = 0;


	// Only clear the depth bit.  The color bit is cleared
	// handled in the draw background routine when the last
	// frame isn't completely overdrawn
	glClear(GL_DEPTH_BUFFER_BIT);

	if (g_cv3dglasses.value) 
	{
		// Set flag
		b3d = TRUE;

		// Reset view - this is due to the 3d glasses code.  Bad.
		glLoadIdentity();

		// Clear screen
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Setup lens count
		nMaxLens = 1;
	
	}

	glPushMatrix();

	// Loop through lenses	
	for (nLens = 0; nLens <= nMaxLens; nLens++)
	{

		// Setup 3d lens
		if (b3d)
		{
			SetupLens(nLens);
		}
	
		// Rotate background around player (bleh!)
		if (EnableVomitComet && g_cvCopernicusMode.value)
		{
			glRotatef(-player->rz, 0.0f, 0.0f, 1.0f);
		}

		// Set global flag to indicate that the stars
		// aren't being rendered -- when the stars ARE
		// rendered, the flag will be set then to indicate
		// that they ARE :)
		g_bStarsActive = FALSE;

		// Set wireframe mode 
		renderset(render_wireframe, (g_cvWireframeEnabled.value != 0));
	
		// Draw background 
		if (g_cvTexturesEnabled.value)
			DrawGLBackground();

		// Setup light sources
		if (g_cvLightingEnabled.value)
			SetupGLLights();

		//	Position and draw actors
		for (i = 0; i < MAX_ACTORS; i++) {

			// If an actor is to be displayed, it must
			// be active
			if (actors[i].active) 
			{

				// Compute fLifePctRemaining
				fRemainingLifePct = 1.0f - actors[i].LifePercent;
				
				// Store current matrix
				glPushMatrix();

				// Move actor
				glTranslatef(actors[i].x, actors[i].y, actors[i].z);

				if (b3d)
					glColor4ub(255, 255, 255, 255);
				else
					glColor4ub(actors[i].color.r, actors[i].color.g, actors[i].color.b, 255);
				
				

				switch (actors[i].type) {
				
					// Two shots are drawn overlaying each other 
					// for twinkling effect
					case ACTOR_SHOT:
					case ACTOR_SFIRE:
						
						// Setup display characteristics for shots
						if (nLastType != actors[i].type) {
							
							// Setup rendering features
							renderset(render_blend, (g_cvBlendingEnabled.value != 0));
							renderset(render_texture, (g_cvTexturesEnabled.value != 0));
							renderdisable(render_lighting | render_depthtest);
						}

						
						if (actors[i].type == ACTOR_SHOT)
						{
							glColor4ub(255,255,255,254);
						}
						else
						{
							glScalef(0.7f, 0.7f, 0.7f);

							if (b3d)
								glColor4ub(192,192,192,254);
							else
								glColor4ub(255,0,0,254);
								
						}

						glRotatef(actors[i].rx, 0.0f, 0.0f, 1.0f);
						//glCallList(actors[i].displaylistid);
                        draw_sfire();

						// Draw friendly shots blue, enemy shots red
						if (b3d)
						{
							if (actors[i].type == ACTOR_SHOT)
								glColor4ub(96, 96, 96, 254);
							else 
								glColor4ub(128, 128, 128, 128);
						}
						else
						{
							if (actors[i].type == ACTOR_SHOT)
								glColor4ub(0, 0, 255, 254);
							else 
								glColor4ub(255, 255, 0, 128);
						}

						glTranslatef(0.0f, 0.0f, 0.1f);
						glRotatef(actors[i].rx * -2.0f, 0.0f, 0.0f, 1.0f);
						//glCallList(actors[i].displaylistid);
                        draw_sfire();

						break;

					case ACTOR_BLAST:
						
						// Setup display characteristics
						if (nLastType != actors[i].type) {
							
							// Setup rendering features
							renderset(render_blend, (g_cvBlendingEnabled.value != 0)); 
							renderset(render_texture, (g_cvTexturesEnabled.value != 0)); 
							renderdisable(render_lighting | render_depthtest); 
						}
						
						// Draw the blast twice for effect
						fScalar = (float)(actors[i].size + 2.0f * actors[i].LifePercent);
						glScalef(fScalar, fScalar, fScalar);

						if (b3d)
							glColor4ub(255, 255, 255, (GLuint)(fRemainingLifePct * 235.0f) + 20);
						else
							glColor4ub(actors[i].color.r, actors[i].color.g, actors[i].color.b, (GLuint)(fRemainingLifePct * 235.0f) + 20);

						glRotatef(actors[i].rx, 1.0f, 0.0f, 0.0f);
						//glCallList(actors[i].displaylistid);
                        draw_blast();
						
						glScalef(0.75f, 0.75f, 0.75f);

						if (b3d)
							glColor4ub(	(GLuint)(fRemainingLifePct * 128.0f) + 64, (GLuint)(fRemainingLifePct * 128.0f) + 64, (GLuint)(fRemainingLifePct * 128.0f) + 64, (GLuint)((fRemainingLifePct * 235.0f) + 20));
						else
							glColor4ub(	(GLuint)(fRemainingLifePct * 128.0f) + 64, (GLuint)(fRemainingLifePct * 64.0f), 0, (GLuint)((fRemainingLifePct * 235.0f) + 20));

                        //glCallList(actors[i].displaylistid);
                        draw_blast();
						glScalef(1.0f,1.0f,1.0f);

						break;

					// The flames use a different rendering technique
					// from the rest of the objects.  Rather than using
					// display lists, the flames' vertexes are calculated
					// and sent on the fly.  This should be quicker than
					// the other method because glLoadIdentity is only 
					// called once.
					case ACTOR_FLAMES: {
                        
                        // Setup rendering options
                        if (nLastType != actors[i].type) {

                            // Setup rendering features
                            renderset(render_blend,        (g_cvBlendingEnabled.value != 0));
                            renderset(render_texture,    (g_cvTexturesEnabled.value != 0));
                            renderdisable(render_lighting | render_depthtest);
                        
                            // Bind texture
                            glBindTexture(GL_TEXTURE_2D, texture[4]);

                        }

                        // Fade based on age
                        if (b3d)
                            glColor4ub(255, 255, 255, (GLint)(fRemainingLifePct * 180));
                        else
                            glColor4ub(255, (GLint)(fRemainingLifePct * 180), 0, 128);

                        // I'm cheating here a little by adding .1 to the z
                        // value.  This will force the flames to appear on top
                        // of the smoke so that the smoke won't fog out the
                        // flame effect.
                        fx = actors[i].x;
                        fy = actors[i].y;
                        fz = actors[i].z + 0.1f;
                        fs = actors[i].size * 0.75f;

#if 0
                        glBegin(GL_TRIANGLE_STRIP);
                            glTexCoord2d(1,1); glVertex3f(fx + fs, fy + fs, fz);
                            glTexCoord2d(0,1); glVertex3f(fx - fs, fy + fs, fz);
                            glTexCoord2d(1,0); glVertex3f(fx + fs, fy - fs, fz);
                            glTexCoord2d(0,0); glVertex3f(fx - fs, fy - fs, fz);
                        glEnd();
#else
                        GLfloat vtx2[] = {
                            fx + fs, fy + fs, fz,
                            fx - fs, fy + fs, fz,
                            fx + fs, fy - fs, fz,
                            fx - fs, fy - fs, fz
                        };
                        GLfloat tex2[] = {
                            1,1,
                            0,1,
                            1,0,
                            0,0
                        };

                        glEnableClientState(GL_VERTEX_ARRAY);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

                        glVertexPointer(3, GL_FLOAT, 0, vtx2);
                        glTexCoordPointer(2, GL_FLOAT, 0, tex2);
                        glDrawArrays(GL_TRIANGLE_STRIP,0,4);

                        glDisableClientState(GL_VERTEX_ARRAY);
                        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
                        break;
                    }

					case ACTOR_PARTICLE:
					case ACTOR_FLAMINGPARTICLE:

						// Setup rendering options
						if (nLastType != actors[i].type) {

							// Setup rendering features
							renderset(render_blend,		(g_cvBlendingEnabled.value != 0)); 
							renderset(render_texture,	(g_cvTexturesEnabled.value != 0)); 
							renderdisable(render_lighting | render_depthtest);

						}
						
						// Make actor's size at least one...
						if (actors[i].size <= 0)
						{
							actors[i].size = 1;
						}

						fScalar = (float)((actors[i].size + 1) * actors[i].LifePercent) + actors[i].size;
						glScalef(fScalar, fScalar, fScalar);
						
						// Rotate the actor (always faces the camera)
						glRotatef(actors[i].rz, 0.0f, 0.0f, 1.0f);
						
						// Fade based on age
						if (b3d)
							glColor4ub(128, 128, 128, (GLuint)((fRemainingLifePct) * 128.0f) + 64);
						else
							glColor4ub(actors[i].color.r, actors[i].color.g, actors[i].color.b, (GLuint)((fRemainingLifePct) * 128.0f) + 64);

						// Display object
						//glCallList(actors[i].displaylistid);
                        draw_smoke();

						// If flaming...
						if (actors[i].type == ACTOR_FLAMINGPARTICLE)
						{
							glScalef(0.3f, 0.3f, 0.3f);
							glRotatef(-actors[i].rz, 0.0f, 0.0f, 1.0f);

							if (b3d)
								glColor4ub(255,255,255,64);
							else
								glColor4ub(255,255,0,64);

							//glCallList(actors[i].displaylistid);
                            draw_smoke();
							glScalef(1.0f, 1.0f, 1.0f);
						}
						
						break;

					case ACTOR_SHIELDS:
						
						// Setup display characteristics for
						if (nLastType != actors[i].type) {
							
							// Setup rendering features
							renderset(render_blend,		(g_cvBlendingEnabled.value != 0));
							renderset(render_texture,	(g_cvTexturesEnabled.value != 0));
							renderenable(render_depthtest); 
							renderdisable(render_lighting);

						}
						
						if (b3d)
							glColor4ub(255, 255, 255, 254);
						else
							glColor4ub(actors[i].color.r, actors[i].color.g, actors[i].color.b, 254);

						glRotatef(actors[i].rx, 0.0f, 0.0f, 1.0f);
						glRotatef(actors[i].rz, 1.0f, 0.0f, 0.0f);
						//glCallList(actors[i].displaylistid);
                        draw_shields();
                        

						glTranslatef(0.0f, 0.0f, 0.1f);
						glRotatef(actors[i].rx * -2.0f, 0.0f, 0.0f, 1.0f);
						glRotatef(actors[i].rz * -2.0f, 1.0f, 0.0f, 0.0f);
						//glCallList(actors[i].displaylistid);
                        draw_shields();

						break;
				
					case ACTOR_DEBRIS:
						
						if (nLastType != actors[i].type) 
						{

							// Setup rendering features
							renderset(render_lighting,	(g_cvLightingEnabled.value != 0)); 
							renderset(render_blend,		(g_cvBlendingEnabled.value != 0)); 
							renderdisable(render_texture | render_depthtest);
						}
						
						if (actors[i].size) 
						{
							glScalef((float)(actors[i].size + 1), (float)(actors[i].size + 1), (float)(actors[i].size + 1));
						}
						
						glRotatef(actors[i].ry, 0.0f, 1.0f, 0.0f);
						glRotatef(actors[i].rx, 1.0f, 0.0f, 0.0f);
						glRotatef(actors[i].rz, 0.0f, 0.0f, 1.0f);
						
						if (b3d)
							glColor4ub(255, 255, 255, (GLuint)(128 * fRemainingLifePct) + 128);
						else
							glColor4ub(actors[i].color.r, actors[i].color.g, actors[i].color.b, (GLuint)(128 * fRemainingLifePct) + 128);
						
						//glCallList(actors[i].displaylistid);
                        draw_debris();

						glScalef(1.0f, 1.0f, 1.0f);
						
						break;
					
					case ACTOR_LSAUCER:
						
						// Setup display characteristics for shots
						if (nLastType != actors[i].type) {
							
							// Setup rendering features
							renderset(render_lighting,	(g_cvLightingEnabled.value != 0));
							renderset(render_texture,	(g_cvTexturesEnabled.value != 0));
							renderdisable(render_blend);
							renderenable(render_depthtest);
						}

						glScalef(2.0f, 2.0f, 2.0f);

						if (b3d)
							glColor4ub(255, 255, 255, 255);
						else
							glColor4ub(actors[i].color.r, actors[i].color.g, actors[i].color.b, 255);
						
						// Make the saucer wobble ;)
						glRotatef(-90.0f + (float)(sin(actors[i].x) * 15), 1.0f, 0.0f, 0.0f);

						glRotatef(actors[i].rz, 0.0f, 0.0f, 1.0f);

						//glCallList(actors[i].displaylistid);
                        draw_ufo();

						glScalef(1.0f, 1.0f, 1.0f);

						break;

					case ACTOR_PLAYER:
						
						if (nLastType != actors[i].type) {
							
							// Setup rendering features
							renderset(render_lighting,	(g_cvLightingEnabled.value != 0)); 
							renderset(render_texture,	(g_cvTexturesEnabled.value != 0)); 
							renderdisable(render_blend);
							renderenable(render_depthtest); 
						}

						if (actors[i].size)
							glScalef((float)(actors[i].size + 1), (float)(actors[i].size + 1), (float)(actors[i].size + 1));

						if (b3d)
							glColor4ub(255,255,255,255);

						glRotatef(actors[i].rz, 0.0f, 0.0f, 1.0f);
						glRotatef(actors[i].rx, 1.0f, 0.0f, 0.0f);
						glRotatef(actors[i].ry, 0.0f, 1.0f, 0.0f);
						//glCallList(actors[i].displaylistid);
                        draw_player();
						break;


					default:
						
						if (nLastType != actors[i].type) {
							
							// Setup rendering features
							renderset(render_lighting,	(g_cvLightingEnabled.value != 0)); 
							renderset(render_texture,	(g_cvTexturesEnabled.value != 0)); 
							renderdisable(render_blend);
							renderenable(render_depthtest); 
						}

						// 2002.09.14
						// Prevent rocks from overlapping
						if (actors[i].type == ACTOR_ROCK) 
							glClear(GL_DEPTH_BUFFER_BIT);
				

						if (actors[i].size)
							glScalef((float)(actors[i].size + 1), (float)(actors[i].size + 1), (float)(actors[i].size + 1));

						glRotatef(actors[i].ry, 0.0f, 1.0f, 0.0f);
						glRotatef(actors[i].rx, 1.0f, 0.0f, 0.0f);
						glRotatef(actors[i].rz, 0.0f, 0.0f, 1.0f);
                        
                        // TODO: Are there any other types that get called here?
                        //glCallList(actors[i].displaylistid);
                        if (actors[i].type == ACTOR_ROCK)
                            draw_rock();
                        
						break;
				}
				
				// Store the current actor's type
				nLastType = actors[i].type;

				// Restore previous matrix
				glPopMatrix();
			}
		}
	}

	// Restore color mask to full color
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

	// Draw bounding boxes
	if (g_cvBoundingBoxesEnabled.value) 
	{
		// Setup rendering features
		renderenable(render_blend);
		renderdisable(render_lighting | render_depthtest | render_texture);

		// Display bounding boxes translucent white
		glColor4ub(255, 255, 255, 60);
		
		// Draw the bounding box of each actor
		for (i = 0; i < MAX_ACTORS; i++) {
			if (actors[i].active)
			{
				switch (actors[i].type)
				{
					// All of these actors have bounding boxes...
					case ACTOR_PLAYER:
					case ACTOR_ROCK:
					case ACTOR_SHOT:
					case ACTOR_SHIELDS:
					case ACTOR_LSAUCER:
					case ACTOR_SFIRE:
							glPushMatrix();
							glTranslatef(actors[i].x, actors[i].y, actors[i].z);
#if 0
                            glBegin(GL_QUADS);
                                glVertex3f(-actors[i].xcolldist, -actors[i].ycolldist,  0.0f);
                                glVertex3f( actors[i].xcolldist, -actors[i].ycolldist,  0.0f);
                                glVertex3f( actors[i].xcolldist,  actors[i].ycolldist,  0.0f);
                                glVertex3f(-actors[i].xcolldist,  actors[i].ycolldist,  0.0f);
                            glEnd();
#else
                            GLfloat vtx[] = {
                              -actors[i].xcolldist, -actors[i].ycolldist,  0.0f,
                               actors[i].xcolldist, -actors[i].ycolldist,  0.0f,
                               actors[i].xcolldist,  actors[i].ycolldist,  0.0f,
                              -actors[i].xcolldist,  actors[i].ycolldist,  0.0f
                            };
                            
                            glEnableClientState(GL_VERTEX_ARRAY);

                            glVertexPointer(3, GL_FLOAT, 0, vtx);
                            glDrawArrays(GL_TRIANGLE_STRIP,0,4);

                            glDisableClientState(GL_VERTEX_ARRAY);
#endif
							glPopMatrix();
				}
			}
		}
	}

	glPopMatrix();


	// Un-rotate background before drawing the text
	if (EnableVomitComet && g_cvCopernicusMode.value)
		glRotatef(player->rz, 0.0f, 0.0f, 1.0f);

	// Fix the matrix for text
	if (b3d)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}



	// Don't display shields when the game's over
	if (!g_bGameOver)
	{
	
		// Write word 'Shields' -- duh.
		glColor4ub(255,255,255,255);
		glPrint(0, -16.0f, -10.25f, "SHIELDS");

		// Display shields
		for (i = 0; i <= 9; i++)
		{
			
			if (b3d)
				glColor4f(1.0f, 1.0f, 0, 1.0f);
			else
			{
				fScalar = float(i + 1) * 0.1f;
				glColor4f(1.0f - fScalar, fScalar, 0, 1.0f);
			}
            
            int ic = 0;

			if ((float)player->ShieldsPower / 255.0f < (float)i * 0.1f)
			{
				ic = 102;
			}
			else
			{
				ic = 103;
			}

            glPrintChar(-16.0f + (i * 0.6f), -11.0f, ic);
		}
	}

	// Set text color 
	glColor4ub(255,255,255,255);
	
	// Display score
	glPrint(0, -16.0f, 9.5f, "PLAYER 1");
	glPrintVar(0, -16.0f, 8.5f,"%d", player->Score);
	glPrintVar(0, -16.0f, 7.5f, "LEVEL %d", g_nLevelNum);

/*
	// Display FPS
	if (g_cvFpsEnabled.value != 0)
		glPrintVar(0,-16.0f,-12.0f, "FPS %5d IS WRONG -- Game Length %5.1f", g_nFps, g_fGameLength);
*/

	// Show message (if one exists)
	if ((g_lMessageEndTime != 0) && (g_lMessageEndTime >= g_fNow)) 
		glPrintCenteredVar(0, -11.0f, "%s", g_sMessage);

	// Print game over message on screen
	if (g_bGameOver) {

		if (!g_bShowMenu)
		{
							
			// Display highscore in the center of the top of the screen
			glPrintCenteredVar(0, 9.5f, "%3s %7d", highscores[0].Initials, highscores[0].Score);

			// If the user's entering their initials, don't rotate
			// through screens
			if ((g_bEnteringHighScoreInitials) || ((long)(g_fNow* 0.25f) % 3 == 0)) 
			{
				for (i = 0; i < MAX_HIGHSCORES; i++)
				{
					// If the user is entering their initials and 
					// playerhighscore's address is the same as the 
					// address of highscores[] at the current array
					// index, then draw the cursor
					if ((g_bEnteringHighScoreInitials) && (playerhighscore == &highscores[i]))  
					{
						// Store current matrix
						glPushMatrix();

						// Draw the cursor
						glColor3f(0.3f,0.0f,0.6f);
						glTranslatef(-6.4f + (g_nHighScoreInitialsIdx * 0.6f), 4.5f - i, -30.0f);
						glScalef(2.4f, 2.4f, 1.0f);
						renderenable(render_blend);
						renderdisable(render_lighting | render_depthtest | render_texture);
						//glCallList(debris_list);
                        draw_debris();
						glColor3f(1.0f,1.0f,1.0f);

						// Restore matrix
						glPopMatrix();
					}
					glPrintVar(0, -7.0f, 4.0f - i, "%3s %2d %7d   %c / %c", highscores[i].Initials, highscores[i].Level, highscores[i].Score, highscores[i].Difficulty, highscores[i].SpaceSize);
				}


				// Display message
				if (g_bEnteringHighScoreInitials)
				{
					glPrintCentered(0,  7.0f, "CONGRATULATIONS!");
					glPrintCentered(0,  6.0f, "YOU GOT A HIGH SCORE!");
					glPrintCentered(0, -8.0f, "TYPE IN YOUR INITIALS AND HIT ENTER WHEN DONE");
				}

			}
			else		
			{
				// Draw text
				glPrintCentered(0, -1.5f, "GAME OVER");
				glPrintCentered(0, -2.5f, "PRESS '1' TO START");
				
				glColor4ub(255, 255, 255, 160);
				glPrintCentered(0, - 7.25f, "PROGRAMMING / MODELS / TEXTURES");
				glPrintCentered(0, - 8.00f, "THOM WETZEL (www.LMNOpc.com)");
				
				glPrintCentered(0, - 9.25f, "HUGE THANKS TO JEFF MOLOFEE (nehe.gamedev.net)");
				glPrintCentered(0, -10.00f, "FOR OPENGL BASE CODE AND CODING ASSISTANCE!");


				glPrintCentered(0, -12.25f, "(C)2000 LMNOPC - THIS SOFTWARE MAY NOT BE SOLD");

				// Draw logo
				renderenable(render_blend | render_texture);
				renderdisable(render_lighting | render_depthtest);
				glPushMatrix();
				glTranslatef(0.0f, 0.45f, -4.5f);
				// glColor4ub(255, 255, 255, 160 + (char)(20.0f * sin(g_fNow / 0.250f)));
				glColor4ub(255, 255, 255, 255);
//				glCallList(logo_list);
                draw_logo();
				glPopMatrix();
			

			}
		}
	}
	else // if not game over
	{

		// Display highscore in the center of the top of the screen
		// If the current player's score is higher than the current
		// highscore, then show the player's score
		if (player->Score <= highscores[0].Score) 
			glPrintCenteredVar(0, 9.5f, "%3s %7d", highscores[0].Initials, highscores[0].Score);
		else
			glPrintCenteredVar(0, 9.5f, "??? %7d", player->Score);
	}

	// Print game over message
	if (g_bGamePaused && !g_bShowMenu)
		glPrintCenteredVar(0, 0, "GAME PAUSED -- PRESS '%c' TO CONTINUE", (int)g_cvKeyPause[0].value);
	
	// Print invincibility message
	if (player->InvincibleEndTime >= g_fNow)
		glPrintCenteredVar(0, 3, "INVINCIBLE FOR %d", int((player->InvincibleEndTime - g_fNow)) + 1);


	// Display extra men
	glColor4ub(235,235,255,255);
	renderenable(render_texture | render_blend);
	renderdisable(render_depthtest | render_lighting);
	i = player->Lives;
	glPushMatrix();
		glTranslatef(-9.0f, 7.25f,-25.0f);
		while (i > 6)
		{
			glTranslatef(1.0f, 0, 0);
			//glCallList(extraship5_list);
            draw_extraship5();
			i -= 5;
		}
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-9.0f, 8.25f,-25.0f);
		while (i > 1)
		{
			glTranslatef(1.0f, 0, 0);
			//glCallList(extraship_list);
            draw_extraship();
			i -= 1;
		}
	glPopMatrix();
	 

	// Draw menu
	if (g_bShowMenu)
		Menu_DrawGL();


	// Draw console
	glColor4ub(128, 128, 128, 255);
	con_draw(g_fNow);

	
	// Return success at the end of this function
	return TRUE;										
}

// NeHe
GLvoid KillGLWindow(GLvoid)						// Properly Kill The Window
{
	if (SDL_glContext)							// Do We Have A Rendering Context?
	{
		//if (!wglMakeCurrent(NULL,NULL))				// Are We Able To Release The DC And RC Contexts?
		//{
		//	MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		//}

		SDL_GL_DeleteContext(SDL_glContext);
		SDL_glContext = NULL;

	}

    SDL_DestroyWindow(SDL_window);
	SDL_window = NULL;
}



/*
 *	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height		- Height Of The GL Window Or Fullscreen Mode				*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	
*/
BOOL CreateGLWindow(char* title, int width, int height, int bits, BOOL fullscreenflag)
{
	int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
	SDL_DisplayMode desktopMode;
	Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    
#ifdef GLES
    // todo: figure out scaling so we can use  SDL_WINDOW_ALLOW_HIGHDPI
    flags = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
    x = 0;
    y = 0;
#endif

	// Destroy existing state if it exists
	if (SDL_glContext != NULL)
	{
		SDL_GL_DeleteContext(SDL_glContext);
		SDL_glContext = NULL;
	}

	if (SDL_window != NULL)
	{
		SDL_GetWindowPosition(SDL_window, &x, &y);
		SDL_DestroyWindow(SDL_window);
		SDL_window = NULL;
	}

	memset(&desktopMode, 0, sizeof(SDL_DisplayMode));

#ifndef GLES
	if (fullscreenflag)
	{
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	else
	{
		flags |= SDL_WINDOW_SHOWN;
	}
#endif

	int perChannelColorBits;

	if (bits == 24)
		perChannelColorBits = 8;
	else
		perChannelColorBits = 4;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, perChannelColorBits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, perChannelColorBits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, perChannelColorBits);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	if (g_cv3dglasses.value != 0)
	{
		SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
	}
    
#ifdef GLES
    // testing something -tkidd
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
//    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

	if ((SDL_window = SDL_CreateWindow(title, x, y, width, height, flags)) == NULL)
	{
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		return false;
	}

	//if (fullscreenflag)
	{
		SDL_DisplayMode mode;

		switch (bits)
		{
		case 16: mode.format = SDL_PIXELFORMAT_RGB565; break;
		case 32: mode.format = SDL_PIXELFORMAT_RGB24;  break;
		default: printf("bits is %d, can't fullscreen\n", bits); return false;
		}

		mode.w = width;
		mode.h = height;
		mode.refresh_rate = 60; //??
		mode.driverdata = NULL;

		if (SDL_SetWindowDisplayMode(SDL_window, &mode) < 0)
		{
			printf("SDL_SetWindowDisplayMode failed: %s\n", SDL_GetError());
			return false;
		}

		if ((SDL_glContext = SDL_GL_CreateContext(SDL_window)) == NULL)
		{
			printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
			SDL_DestroyWindow(SDL_window);
			SDL_window = NULL;
			return false;
		}

		glViewport(0, 0, width, height);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(SDL_window);

#ifndef GLES
		if (SDL_GL_SetSwapInterval(0) == -1)
		{
			printf("SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError());
		}
#endif

    
#ifdef IOS
        Sys_AddControls(SDL_window);
#endif
	}

	SDL_SetWindowInputFocus(SDL_window);
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())										// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
#ifdef _WIN32
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
#endif
		return FALSE;									// Return FALSE
	}
	
	return TRUE;										// Success
}


//--------------------------------------------------------
// TCW
//
// This routine returs the index of the first free actor
// in the specified range
//--------------------------------------------------------
inline int FindFreeActor(const int& StartIdx, const int& EndIdx) {
	
	int i = StartIdx - 1;
	int nMax = EndIdx;

	// Make sure EndIdx is within bounds
	if (nMax > MAX_ACTORS)
		nMax = MAX_ACTORS;

	// Loop to find available actor
	do 
	{
		if (!actors[++i].active)
			return i;
	} while(i < nMax);

	// No free actors were found
	return 0;
};


//-----------------------------------------------------------------------------
// Name: InitializeGameSounds()
// Desc: Ripped from Duel and modified to init my game sounds
//-----------------------------------------------------------------------------
bool InitializeGameSounds()
{
	//Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}

	// TODO: See if these can be loaded from the resources like before
	g_pShipThrustSound		= Mix_LoadWAV("Res/thrust.wav");
	g_pLSaucerSound			= Mix_LoadWAV("Res/lsaucer.wav");
	g_pBulletFiringSound		= Mix_LoadWAV("Res/fire.wav");
	g_pSmallExplodeSound		= Mix_LoadWAV("Res/explode3.wav");
	g_pMediumExplodeSound	= Mix_LoadWAV("Res/explode2.wav");
	g_pLargeExplodeSound		= Mix_LoadWAV("Res/explode1.wav");
	g_pSaucerFireSound		= Mix_LoadWAV("Res/sfire.wav");
	g_pLifeSound				= Mix_LoadWAV("Res/life.wav");
	g_pThumpHiSound			= Mix_LoadWAV("Res/thumphi.wav");
	g_pThumpLoSound			= Mix_LoadWAV("Res/thumplo.wav");
	g_pMenuBeepSound			= Mix_LoadWAV("Res/menubeep.wav");

    return true;
}


//
// NewValue needs to be a value between 0 and 1.0f
//
void SetVolume(float NewValue)
{

	int iVolume = NewValue * 128;

	Mix_VolumeChunk(g_pShipThrustSound, iVolume);
	Mix_VolumeChunk(g_pLSaucerSound, iVolume);
	Mix_VolumeChunk(g_pBulletFiringSound, iVolume);
	Mix_VolumeChunk(g_pSmallExplodeSound, iVolume);
	Mix_VolumeChunk(g_pMediumExplodeSound, iVolume);
	Mix_VolumeChunk(g_pLargeExplodeSound, iVolume);
	Mix_VolumeChunk(g_pSaucerFireSound, iVolume);
	Mix_VolumeChunk(g_pLifeSound, iVolume);
	Mix_VolumeChunk(g_pThumpHiSound, iVolume);
	Mix_VolumeChunk(g_pThumpLoSound, iVolume);
	Mix_VolumeChunk(g_pMenuBeepSound, iVolume);

}


//-----------------------------------------------------------------------------
// Name: CleanupGameSounds()
// Desc: Ripped from Duel and modified to cleanup my game sounds
//-----------------------------------------------------------------------------
void CleanupGameSounds()
{
   
	Mix_FreeChunk(g_pBulletFiringSound);
	Mix_FreeChunk(g_pSmallExplodeSound);
	Mix_FreeChunk(g_pMediumExplodeSound);
	Mix_FreeChunk(g_pLargeExplodeSound);
	Mix_FreeChunk(g_pShipThrustSound);
	Mix_FreeChunk(g_pThumpHiSound);
	Mix_FreeChunk(g_pThumpLoSound);
	Mix_FreeChunk(g_pLSaucerSound);
	Mix_FreeChunk(g_pSaucerFireSound);
	Mix_FreeChunk(g_pLifeSound);
	Mix_FreeChunk(g_pMenuBeepSound);

	g_pBulletFiringSound		= NULL;
	g_pSmallExplodeSound		= NULL;
	g_pMediumExplodeSound	= NULL;
	g_pLargeExplodeSound		= NULL;
	g_pShipThrustSound		= NULL;
	g_pThumpHiSound			= NULL;
	g_pThumpLoSound			= NULL;
	g_pLSaucerSound			= NULL;
	g_pSaucerFireSound		= NULL;
	g_pLifeSound				= NULL;
	g_pMenuBeepSound			= NULL;

	Mix_Quit();

}



/* 
	These functions are used in the menus to play
	sounds when the user navigates through the menu.

	Normally I would have the sound code right in
	the menus, but it would mean that a bunch of stuff
	that DirectSound requires to get going would 
	have to be referenced in menus.cpp.  This way,
	menus.cpp stays clean.
*/

void PlayMenuBeepSound()
{
	if (g_cvSoundEnabled.value)
		Mix_PlayChannel(-1, g_pMenuBeepSound, 0);
}

void PlayMenuLowSound()
{
	if (g_cvSoundEnabled.value) 
		Mix_PlayChannel(-1, g_pThumpLoSound, 0);
}

void PlayMenuExplosionSound()
{
	if (g_cvSoundEnabled.value)
		Mix_PlayChannel(channelLargeExplodeSound, g_pLargeExplodeSound, 0);
}


/*
	ResumeLoopingSounds
	KillLoopingSounds

  These are support functions that go along with SetSound
  and could be called from any where in the program where
  all the looping sounds in the game should be enabled/disabled
  en masse.
*/
void ResumeLoopingSounds(void)
{
	// Start playing looped sounds
	if (g_bThrustSoundOn)
		Mix_PlayChannel(channelShipThrustSound, g_pShipThrustSound, -1);
	if (actors[IDX_LSAUCER].active == TRUE)
		Mix_PlayChannel(channelLSaucerSound, g_pLSaucerSound, -1);
}	



void KillLoopingSounds(void)
{
	// Stop playing looped sounds
	if (g_bThrustSoundOn)
		Mix_HaltChannel(channelShipThrustSound);
	if (actors[IDX_LSAUCER].active == TRUE)
		Mix_HaltChannel(channelLSaucerSound);
}


/*
 This is used to enable/disable the sound.  It takes 
 care of everything from setting the SoundEnabled cvar
 to en/disabling the looping sounds.
*/
void SetSound(const BOOL value)
{
	if (g_bDirectSoundEnabled)
	{
		Cvar_SetValue("SoundEnabled", (FLOAT)value);

		if (g_cvSoundEnabled.value)
		{
			UpdateMessage("SOUND ENABLED");

			// Start playing looped sounds
			ResumeLoopingSounds();

		}
		else
		{
			UpdateMessage("SOUND DISABLED");

			// Stop playing looped sounds
			KillLoopingSounds();

		}
	}
	else
	{
		UpdateMessage("DIRECTSOUND INITIALIZATION FAILED");
	}
}

/* This is used to enable/disable the mouse.  It takes
	care of everything from setting the MouseEnabled cvar
	to un/acquiring the mouse.
*/
void SetMouse(const BOOL value)
{

	if (g_bDirectInputEnabled)
	{
		
		// Set cvar 
		Cvar_SetValue("MouseEnabled", (FLOAT)value);
	
		// Based on value, enable the mouse
		// TODO: Mouse later -tkidd
		//if (value)
		//{
		//	if (g_pdidMouse)
		//	{
		//		g_pdidMouse->Unacquire();
		//		g_pdidMouse->Acquire();
		//	}
		//	// UpdateMessage("MOUSE ENABLED");
		//}
		//else
		//{
		//	if (g_pdidMouse) 
		//	{
		//		g_pdidMouse->Unacquire();
		//	}
		//	// UpdateMessage("MOUSE DISABLED");
		//}
	}
}


/*
	This is used by the key bind menu to release the mouse
	once key binding is complete
*/
void UnacquireMouse()
{
	// TODO: Mouse later -tkidd
	//if (g_pdidMouse)
	//	g_pdidMouse->Unacquire();
}


//-----------------------------------------------------------------------------
// Name: DestroyInput()
// Desc: Ripped from FFDonuts and modified for my ff effects
//-----------------------------------------------------------------------------
void DestroyInput()
{
	// Unacquire and release joystick
	if( g_pdidJoystick )
	{
		SDL_JoystickClose(g_pdidJoystick);
		g_pdidJoystick = NULL;
	}

	if (g_pdidFFJoystick)
	{
		SDL_HapticClose(g_pdidFFJoystick);
	}

	// Unacquire and release mouse
	// TODO: Mouse later -tkidd
	//if (g_pdidMouse)
	//{
	//	g_pdidMouse->Unacquire();
	//	g_pdidMouse->Release();
	//	g_pdidMouse = NULL;
	//}
}

//-----------------------------------------------------------------------------
// Name: InitializeInput()
// Desc: Ripped from FFDonuts and modified
//-----------------------------------------------------------------------------
BOOL InitializeInput()
{
#ifdef IOS
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif
    
	// Initialize SDL Joystick
	if (SDL_Init(SDL_INIT_JOYSTICK) != 0)
	{
		printf("SDL_Init( SDL_INIT_JOYSTICK ) FAILED (%s)\n", SDL_GetError());
		return false;
	}

	// Initialize SDL haptics
	if (SDL_Init(SDL_INIT_HAPTIC) != 0)
	{
		printf("SDL_Init( SDL_INIT_HAPTIC ) FAILED (%s)\n", SDL_GetError());
		return false;
	}

	if (SDL_NumJoysticks() < 1)
	{
		printf("Warning: No joysticks connected!\n");
	}
	else
	{
		//Load joystick
		g_pdidJoystick = SDL_JoystickOpen(0);
		if (g_pdidJoystick == NULL)
		{
			printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get controller haptic device
			// Try to find and initialize force feedback
			g_pdidFFJoystick = SDL_HapticOpenFromJoystick(g_pdidJoystick);
			if (g_pdidFFJoystick == NULL)
			{
				printf("Warning: Controller does not support haptics! SDL Error: %s\n", SDL_GetError());				
			}
			else
			{
				//Get initialize rumble
				if (SDL_HapticRumbleInit(g_pdidFFJoystick) < 0)
				{
					printf("Warning: Unable to initialize rumble! SDL Error: %s\n", SDL_GetError());
				}
				else 
				{
					// Make the stick shake on startup
					playExplodeFFEffect();
				}
			}
		}
	}

    return true;
}

//-----------------------------------------------------------------------------
// Name: GetDeviceInput()
// Desc: Processes data from the input device.  Uses GetDeviceState().
//		 Ripped from FFDonuts and modified.  I also added mouse handling
//		code from one of the samples as well.
//-----------------------------------------------------------------------------
unsigned long GetDeviceInput()
{
    unsigned long       dwInput = 0;

	 // Make sure a joystick exists
	 if ((g_pdidJoystick != NULL) && (g_cvJoystickEnabled.value != 0))
	 {

/*
TODO:
	Implement joystick deadzone 
*/

//Analog joystick dead zone
		 const int JOYSTICK_DEAD_ZONE = 8000;

		// 2002.01.15 -- Forgot to remove this in V1.3
		// UpdateMessage("%d %d", dijs.lX, dijs.lY);
		
		 // Process device state

		 Sint16 x_move = SDL_JoystickGetAxis(g_pdidJoystick, 0);
		 Sint16 y_move = SDL_JoystickGetAxis(g_pdidJoystick, 1);

		 //Left of dead zone
		 if (x_move < -JOYSTICK_DEAD_ZONE)
		 {
			 dwInput |= JOYSTICK_LEFT;
		 }
		 //Right of dead zone
		 else if (x_move > JOYSTICK_DEAD_ZONE)
		 {
			 dwInput |= JOYSTICK_RIGHT;
		 }
		 //Below of dead zone
		 if (y_move < -JOYSTICK_DEAD_ZONE)
		 {
			 dwInput |= JOYSTICK_DOWN;
		 }
		 //Above of dead zone
		 else if (y_move > JOYSTICK_DEAD_ZONE)
		 {
			 dwInput |= JOYSTICK_UP;
		 }

		 if (SDL_JoystickGetButton(g_pdidJoystick, 0)) {
			 dwInput |= JOYSTICK_FIRE;
		 }
		 else if (SDL_JoystickGetButton(g_pdidJoystick, 1)) {
			 dwInput |= JOYSTICK_SHIELDS;
		 }
		 else if (SDL_JoystickGetButton(g_pdidJoystick, 2)) {
			 dwInput |= JOYSTICK_HYPERSPACE;
		 }
	 }
	 

	// Do mouse
	// TODO: Mouse later -tkidd
	// if ((g_pdidMouse != NULL) && (g_cvMouseEnabled.value != 0))
	//{
	//	// get the input's device state, and put the state in dims
	//	hr = g_pdidMouse->GetDeviceState(sizeof(DIMOUSESTATE), &g_dims);

	//	// If the mouse input is lost, try to reacquire it
	//	if (hr == DIERR_INPUTLOST)
	//	{
	//		// DirectInput is telling us that the input stream has
	//		// been interrupted.  We aren't tracking any state
	//		// between polls, so we don't have any special reset
	//		// that needs to be done.  We just re-acquire and
	//		// try again.
	//		g_pdidMouse->Acquire();

	//		// Nothing was read - exit until next loop
	//		return 0;
	//	}
	//}

/*
	// Mouse debug message
	UpdateMessage("dims = %d %d %c %c %c %c", g_dims.lX, 
			g_dims.lY, 
			g_dims.rgbButtons[0] & 0x80 ? '0' : ' ',
			g_dims.rgbButtons[1] & 0x80 ? '1' : ' ',
			g_dims.rgbButtons[2] & 0x80 ? '2' : ' ',
			g_dims.rgbButtons[3] & 0x80 ? '3' : ' ');
*/
	
	// Cheat and map mouse buttons to keys for user binds
	// TODO: Mouse later -tkidd
	// keys[252] = (g_dims.rgbButtons[0] & 0x80);
	//keys[253] = (g_dims.rgbButtons[1] & 0x80);
	//keys[254] = (g_dims.rgbButtons[2] & 0x80);
	//keys[255] = (g_dims.rgbButtons[3] & 0x80);
	
    // return the new device state
    return dwInput;
}

//--------------------------------------------------------
// TCW
// CollisionTest is a centralized collision detection
// that determines if two actors are colliding.  This 
// is being done to eliminate multiple instances of the
// same code 
//
// NOTE:  CollisionTest 
//
//--------------------------------------------------------
inline BOOL CollisionTest(const actorinfo& actor1, const actorinfo& actor2)
{

/* Long hand version
	
	float xdist			= 0.0f;
	float ydist			= 0.0f;
	float xcolldist	= 0.0f;
	float ycolldist	= 0.0f;

	// Calculate difference between actors' origins
	xdist = absFloat(actors[idx1].x - actors[idx2].x);
	ydist = absFloat(actors[idx1].y - actors[idx2].y);

	// Add actors' bounding box values
	xcolldist = actors[idx1].xcolldist + actors[idx2].xcolldist;
	ycolldist = actors[idx1].ycolldist + actors[idx2].ycolldist;
	
	// Objects are colliding if actor distance
	// is less than the sum of their bounding boxes
	if ((xdist <= xcolldist) && (ydist <= ycolldist))
		return TRUE;

	return FALSE;

*/

	// Objects are colliding if actor distance
	// is less than the sum of their bounding boxes
	return ((fabs(actor1.x - actor2.x) <= actor1.xcolldist + actor2.xcolldist) && (fabs(actor1.y - actor2.y) <= actor1.ycolldist + actor2.ycolldist));

}



//--------------------------------------------------------
// TCW
// Sets up and spawns saucer.  Before the saucer is 
// spawned, a check is made to make sure the saucer isn't
// spwning in the middle of a rock.
//--------------------------------------------------------
BOOL CreateSaucer(const long& IDX) 
{
	int i = 0;	

	// Randomly start at left or right edge of screen
	if (randInt(1, 100) < 50)
	{
		actors[IDX].x = WORLD_HALFWIDTH;
		actors[IDX].vx = -5.0f;
	}
	else
	{
		actors[IDX].x = -WORLD_HALFWIDTH;
		actors[IDX].vx = 5.0f;
	}

	// Start anywhere on y axis 
	actors[IDX].y = randFloat(-(float)WORLD_HALFHEIGHT, (float)WORLD_HALFHEIGHT);

	// Setup collision variables
	actors[IDX].xcolldist = 0.6f;
	actors[IDX].ycolldist = 0.25f;

	// If a collision is going to be immediate, 
	// don't spawn ufo
	for (i = IDX_FIRST_ROCK; i <= IDX_LAST_ROCK; i++)
	{
		if (CollisionTest(actors[IDX], actors[i]))
		{
			return FALSE;
		}
	}
	
	actors[IDX].active			= TRUE;
	actors[IDX].type				= ACTOR_LSAUCER;
	actors[IDX].displaylistid	= lsaucer_list;
	
	actors[IDX].z					= WORLD_DEPTH;
	actors[IDX].rx					= -95;
	actors[IDX].ry					= 0;
	actors[IDX].rz					= 0;

	actors[IDX].vy					= 5.0f;
	actors[IDX].vz					= 5.0f;

	actors[IDX].color.r			= (char)128;
	actors[IDX].color.g			= (char)128;
	actors[IDX].color.b			= (char)128;
	actors[IDX].size			= 1;
	actors[IDX].ShotDelay		= 0;

	actors[IDX].ChangeDirectionDelay = g_fNow + randFloat(0.5f, 2.0f);

	if (g_cvSoundEnabled.value)
	{
		Mix_PlayChannel(channelLSaucerSound, g_pLSaucerSound, -1);
	}

	return(TRUE);

}

//--------------------------------------------------------
// TCW
// This routine just deactivates the saucer actor and 
// turns off the saucer noise
//--------------------------------------------------------
void DestroySaucer(const long& IDX)
{
	actors[IDX].active = FALSE;

	if (g_cvSoundEnabled.value)
	{
		Mix_HaltChannel(channelLSaucerSound);
	}
}


//--------------------------------------------------------
// TCW
// This routine spawns the player, but first makes sure that
// the player won't be spawned into the center of an enemy.
//--------------------------------------------------------
BOOL SpawnPlayer(BOOL hyperspace = FALSE) 
{

	int i						= 0;
	int group				= 0;
	int lbound				= 0;
	int ubound				= 0;
	static float firsttime = 0;
	BOOL bSpawn			= FALSE;

	const float fDefaultCollDist = 0.4f;
	
	// Set default player properties 
	if (!hyperspace) {

		player->active	= FALSE;
		player->type	= ACTOR_PLAYER;
		player->x		= 0;
		player->y		= 0;
		player->z		= WORLD_DEPTH;
		player->rx		= 0;
		player->ry		= 0;
		player->rz		= 0;
		player->vx		= 0;
		player->vy		= 0;
		player->vz		= 0;
		player->displaylistid = player_list;
		player->color.r		= (GLuint)255;
		player->color.g		= (GLuint)255;
		player->color.b		= (GLuint)255;
		player->LifeEndTime	= 0;
		player->size		= 0;
		player->ShieldsUp	= FALSE;
		player->ShieldsPower= 255;
			
		// If the player isn't respawned within ???
		// seconds, then spawn player as invincible for
		// five seconds
		if (firsttime == 0) 
		{
			firsttime = g_fNow;
		}
		else
		{

			// 3 Seconds of collisions gets you invincible
			if (firsttime + 3 <= g_fNow)
			{
				// Setup player as active and invincible
				player->InvincibleEndTime = g_fNow + 5.0f;

				// Set flag telling program to spawn player
				bSpawn = TRUE;
			}
		}

	}

	// Randomly position player after hyperspacing
	else {
		player->x		= randFloat(-WORLD_HALFWIDTH  * 0.75f,  WORLD_HALFWIDTH * 0.75f);
		player->y		= randFloat(-WORLD_HALFHEIGHT * 0.75f, WORLD_HALFHEIGHT * 0.75f);
		player->rz		= randFloat(0, 360);
		player->vx		= 0;
		player->vy		= 0;
	}


	// The bSpawn flag would be set to TRUE if the
	// player couldn't be spawned for 5 seconds and
	// the collision detection isn't necessary because
	// the player's invincible anyway.
	if (!bSpawn)
	{
		
		// Make the x and ycolldist larger so that the 
		// player has a chance to react before being
		// crushed by a rock after spawning
		player->xcolldist	= fDefaultCollDist * 4;
		player->ycolldist	= fDefaultCollDist * 4;

		// If a collision is going to be immediate, 
		// don't spawn player
		for (group=0; group <= 2; group++)
		{
			switch (group)
			{
				case 0:
					lbound = IDX_FIRST_ROCK;
					ubound = IDX_LAST_ROCK;
					break;
				case 1:
					lbound = IDX_LSAUCER;
					ubound = IDX_LSAUCER;
					break;
				case 2:
					lbound = IDX_FIRST_SFIRE;
					ubound = IDX_LAST_SFIRE;
					break;
			}

			for (i = lbound; i < ubound; i++)
			{
				if (actors[i].active) 
				{
					if (CollisionTest(actors[i], *player))
					{
						return FALSE;
					}
				}
			}
		}

		// If the program made it this far, it's safe to 
		// spawn the player
		bSpawn = TRUE;
	}

	// bSpawn indicates that it's safe to spawn the 
	// player.
	if (bSpawn)
	{

		// Set the x and ycolldist to the REAL values
		player->xcolldist	= fDefaultCollDist;
		player->ycolldist	= fDefaultCollDist;

		// If no collisions were detected, then spawn the player
		player->active = TRUE;

		// Reset firsttime for next time player is spawned
		firsttime = 0;

		// Indicate success
		return TRUE;
	
	}
	else
	{
		return FALSE;
	}

}


//--------------------------------------------------------
// TCW
// Create debris -- makes specified number of 
// polys based on actor[] sent into routine
//
// This routine will be replaced by a particle system
//
//--------------------------------------------------------
void CreateDebris(actorinfo *BaseActor, int nAmt) {

	int i = 0;
	int j = 0;
	int nMax = IDX_LAST_DEBRIS - IDX_FIRST_DEBRIS;
	
	if (nAmt > nMax)
	{
		nAmt = nMax;
	}

	// Create debris
	for (i = 0; i < nAmt; i++) 
	{

		j = FindFreeActor(IDX_FIRST_DEBRIS, IDX_LAST_DEBRIS);
		if (j) 
		{
			
			actors[j] = *BaseActor;
			
			actors[j].active = TRUE;

			actors[j].size = 0;

			actors[j].type = ACTOR_DEBRIS;
			actors[j].displaylistid = debris_list;

			actors[j].LifeStartTime = g_fNow;
			actors[j].LifeEndTime = g_fNow + randFloat(0.25f, 1.0f);

			actors[j].rx = randFloat(0, 360);
			actors[j].ry = randFloat(0, 360);
			actors[j].rz = randFloat(0, 360);
			
			actors[j].vx *= randFloat(0.5f, 1.0f);
			actors[j].vy *= randFloat(0.5f, 1.0f);
			actors[j].vz = randFloat(1.0f, 2.4f);

			// Time until next debris is spawned (in seconds)
			actors[j].ParticleSpawnDelay = randFloat(0.01f, 0.05f);

		}
		// If j is unable to find any open actors,
		// don't waste any more time -- actors are booked!
		else
		{
			return;
		}
	}
}

//--------------------------------------------------------
// TCW
// This checks for collisions between a selected *actor
// and the elements in the actors[] array between 
// LBound and UBound.  This is done so that actors whose
// collisions don't matter aren't evaluated.
//
// This routine is also responsible for the effect of 
// a collision, such as object destruction, debris 
// creation, sound playing, etc.
//--------------------------------------------------------
void CollisionDetection(actorinfo *actor, const int& LBound, const int& UBound)
{

	int i = 0;
	int k = 0;

	actorinfo *obj[2] = {NULL, NULL};

	// Make sure the player is active -- 
	// a collision may have already been found
	if (actor->active)
	{

		// Check collisions between rocks and bullets/player
		i = LBound;
		do
		{
			if (actors[i].active) 
			{
				if (CollisionTest(*actor, actors[i]))
				{
					obj[0] = actor;
					obj[1] = &actors[i];
				}
			}
			i++;
		}
		while ((i <= UBound) && (obj[0] == NULL));
	}

	if (obj[0] != NULL)
	{
		for (i = 0; i <= 1; i++)
		{
			
			// Update the objects with the correct responses
			switch (obj[i]->type)
			{

			case ACTOR_SHOT:
			case ACTOR_SFIRE:
				obj[i]->active = FALSE;
				break;

			case ACTOR_PLAYER:

				// Shields up -- player okay
				if ((obj[i]->ShieldsUp) && (obj[i]->ShieldsPower > 0)) 
				{
					if (!g_cvInfiniteShields.value)
						obj[i]->ShieldsPower -= 25;
				}
				else if (g_cvInvincibility.value)
				{
					// Player is invincible -- no damage
				}
				else 
				{
					// Activate force feedback
					if ((g_cvJoystickEnabled.value != 0) && (g_cvForceFeedbackEnabled.value != 0))
					{
						playExplodeFFEffect();
					}

					// Player death sound
					if (g_cvSoundEnabled.value) 
					{
						Mix_PlayChannel(channelLargeExplodeSound, g_pLargeExplodeSound, 0);
					}
						
					// Player killed
					obj[i]->active = FALSE;

					// Create debris
					obj[i]->vx = randFloat(5.0f, 20.0f);
					obj[i]->vy = randFloat(5.0f, 20.0f);
					obj[i]->vz = randFloat(5.0f, 20.0f);
					CreateDebris(obj[i], 999);
						
					// Create blast
					k = FindFreeActor(IDX_FIRST_BLAST, IDX_LAST_BLAST);
					if (k) 
					{
						actors[k]				= *obj[i];
						actors[k].active		= TRUE;
						actors[k].type			= ACTOR_BLAST;
						actors[k].displaylistid = blast_list;
						actors[k].color.r		= (GLuint)255;
						actors[k].color.g		= (GLuint)255;
						actors[k].color.b		= (GLuint)255;
						actors[k].LifeStartTime = g_fNow;

						actors[k].LifeEndTime	= g_fNow + 1.5f;
						actors[k].size			= 2;

						if (g_cv3dglasses.value) 
						{
							actors[k].LifeEndTime	= g_fNow + 1.5f;
							actors[k].size = 2;
						}

						actors[k].rx			= randFloat(15, 75);
						actors[k].ry			= actors[k].rx;
					}
				}
				break;

			case ACTOR_ROCK:

				// Add points and play sound
				switch ((int)obj[i]->size) {
					
					// Large rock
					case 2:
						player->Score += 20;

						if (g_cvSoundEnabled.value) 
						{
							FLOAT fScreenXPos = actors[i].x / WORLD_HALFWIDTH;// *cStereoSeperation;
							float fRight = (fScreenXPos * 127) + 127;
							int iRight = (int)fRight;
							int iLeft = 254 - iRight;
							Mix_SetPanning(channelLargeExplodeSound, iLeft, iRight);
							Mix_PlayChannel(channelLargeExplodeSound, g_pLargeExplodeSound, 0);
						}
						
						// Activate force feedback
						if ((g_cvJoystickEnabled.value != 0) && (g_cvForceFeedbackEnabled.value != 0))
						{
							playExplodeFFEffect();
						}
						
						break;
					
					// Medium rock
					case 1:
						
						player->Score += 50;
						
						if (g_cvSoundEnabled.value)
						{
							FLOAT fScreenXPos = actors[i].x / WORLD_HALFWIDTH;// *cStereoSeperation;
							float fRight = (fScreenXPos * 127) + 127;
							int iRight = (int)fRight;
							int iLeft = 254 - iRight;
							Mix_SetPanning(channelMediumExplodeSound, iLeft, iRight);
							Mix_PlayChannel(channelMediumExplodeSound, g_pMediumExplodeSound, 0);
						}
						
						// Activate force feedback
						if ((g_cvJoystickEnabled.value != 0) && (g_cvForceFeedbackEnabled.value != 0))
							playExplodeFFEffect();

						break;

					// Small rock
					case 0:
						
						player->Score += 100;

						if (g_cvSoundEnabled.value)
						{
							FLOAT fScreenXPos = actors[i].x / WORLD_HALFWIDTH;// *cStereoSeperation;
							float fRight = (fScreenXPos * 127) + 127;
							int iRight = (int)fRight;
							int iLeft = 254 - iRight;
							Mix_SetPanning(channelSmallExplodeSound, iLeft, iRight);
							Mix_PlayChannel(channelSmallExplodeSound, g_pSmallExplodeSound, 0);
						}
						
						// Activate force feedback
						if ((g_cvJoystickEnabled.value != 0) && (g_cvForceFeedbackEnabled.value != 0))
							playExplodeFFEffect();

						break;
								
				}
							
				// Create blast
				k = FindFreeActor(IDX_FIRST_BLAST, IDX_LAST_BLAST);
				if (k) {
					actors[k]					= *obj[i];
					actors[k].active			= TRUE;
					actors[k].type				= ACTOR_BLAST;
					actors[k].displaylistid	= blast_list;
					actors[k].color.r			= randInt(200,255);
					actors[k].color.g			= randInt(200,255);
					actors[k].color.b			= randInt(0, 100);
					actors[k].LifeEndTime	= g_fNow + 0.4f;
					actors[k].LifeStartTime	= g_fNow;
				}


				// Decrement the size of the current rock
				obj[i]->size--;

				if (obj[i]->size >= 0)
				{

					// Spawn new rock using same settings as old rock
					k = FindFreeActor(IDX_FIRST_ROCK, IDX_LAST_ROCK);
					
					if (k) 
					{
						
						// Update original rock
						obj[i]->rx *= 2;
						obj[i]->ry *= 2;
						obj[i]->vx *= 2;
						obj[i]->vy *= 2;
						obj[i]->vz *= 2;
						obj[i]->xcolldist = 0.3f * (obj[i]->size + 1);
						obj[i]->ycolldist = obj[i]->xcolldist;

						// Set rock[k] to the same values as rock[i]
						actors[k]		= *obj[i];
						
						// Make new rock darker than source rock
						actors[k].color.r = (GLuint)(actors[k].color.r * 0.75f);
						actors[k].color.g = (GLuint)(actors[k].color.g * 0.75f);
						actors[k].color.b = (GLuint)(actors[k].color.b * 0.75f);
						
						// Make sure resulting rock isn't too dark
						AdjustRockColor(k);

						// Set negative angles for rock[k]
						actors[k].rx	= -obj[i]->rx;
						actors[k].ry	= -obj[i]->ry;
						actors[k].vx	= -obj[i]->vx;
						actors[k].vy	= -obj[i]->vy;

						// Set objects to +/- 60 degrees of the main rock's former heading
						obj[i]->rz		= obj[i]->rz + 60;
						actors[k].rz	= obj[i]->rz - 60;

					}
				}

				// Create debris
				CreateDebris(obj[i], int(8.0f * (obj[i]->size + 2.0f)));

				break;

				case ACTOR_LSAUCER:

					// Kill actor
					DestroySaucer(IDX_LSAUCER);

					// Activate force feedback
					if ((g_cvJoystickEnabled.value != 0) && (g_cvForceFeedbackEnabled.value != 0))
					{
						playExplodeFFEffect();
					}

					// Increase score and play explosion sound
					player->Score += 500;
					if (g_cvSoundEnabled.value) 
					{
						FLOAT fScreenXPos = actors[i].x / WORLD_HALFWIDTH;// *cStereoSeperation;
						float fRight = (fScreenXPos * 127) + 127;
						int iRight = (int)fRight;
						int iLeft = 254 - iRight;
						Mix_SetPanning(channelLargeExplodeSound, iLeft, iRight);
						Mix_PlayChannel(channelLargeExplodeSound, g_pLargeExplodeSound, 0);
					}
					
					// Create blast
					k = FindFreeActor(IDX_FIRST_BLAST, IDX_LAST_BLAST);
					if (k) {
						actors[k]					= *obj[i];
						actors[k].active			= TRUE;
						actors[k].type				= ACTOR_BLAST;
						actors[k].displaylistid	= blast_list;
						actors[k].color.r			= randInt(200,255);
						actors[k].color.g			= randInt(200,255);
						actors[k].color.b			= randInt(0, 100);
						actors[k].LifeEndTime	= g_fNow + 0.250f;
						actors[k].LifeStartTime	= g_fNow;
					}

					// Create debris
					CreateDebris(obj[i], 10);

				break;

				default:
					obj[i]->active = FALSE;
			}
		}
	}
	return;
}


/*

	GetRefreshTimingRate()

	This function returns the time between two SwapBuffers().
	The time includes a second call to SwapBuffers, so it is
	slightly inaccurate.
  
	This routine is the first step towards the intelligent
	refresh rate control.  

*/
float GetRefreshTimingRate(void)
{
	int nConsec = 0;
	float fTime = 0;
	float fLastTime = 0;
	int nAttempts = 0;

	// Give the program some method of escaping if the
	// refresh rates never agree
	while (nAttempts++ < 25)
	{

		// Swap buffers
		//SwapBuffers(hDC);
		SDL_GL_SwapWindow(SDL_window);

		// Get current time
		fTime = TimerGetTime();

		// Swap buffers again
		//SwapBuffers(hDC);
		SDL_GL_SwapWindow(SDL_window);

		// Calc time between swaps
		fTime = TimerGetTime() - fTime;

		// Compare the current value with the last 
		// one (with some variance) 
		if (fabs(fTime - fLastTime) < 0.0005f)
		{
			nConsec++;

			// Make sure that the last three timing 
			// agree to avoid accepting bad data
			if (nConsec >= 10)
				return fTime;

		}

		// Remember the current value of fTime for 
		// the next evaluation
		fLastTime = fTime;

	} 

	// Display message to let user know that this 
	// routine didn't work.
	UpdateMessage("GetRefreshTimingRate FAILED");

	return 0;
}

int GetScreenWidth()
{
	return m_screenwidth;
}

int GetScreenHeight()
{
	return m_screenheight;
}


// Game initialization 
BOOL InitializeGame(void)
{
	int nColorDepth = 0;

	// Use g_cvResolution to get window res 
	// 
	// Make sure that value of resolution is within bounds before 
	// setting values
	if ((g_cvResolution.value >= RESOLUTIONS_COUNT) || (g_cvResolution.value < 0))
	{
		Cvar_SetValue("Resolution", 0);
	}
	m_screenwidth = g_resolutions[(int)g_cvResolution.value].width;
	m_screenheight = g_resolutions[(int)g_cvResolution.value].height;

	// Use g_cvColorDepth to get window color depth
	// 
	// Make sure that value of ColorDepth is within bounds before setting values
	if ((g_cvColorDepth.value >= COLORDEPTHS_COUNT) || (g_cvColorDepth.value < 0))
	{
		Cvar_SetValue("ColorDepths", 0);
	}
	nColorDepth = colordepths[(int)g_cvColorDepth.value];

	if (!SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		const char* driverName;

		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			printf("SDL_Init( SDL_INIT_VIDEO ) FAILED (%s)\n", SDL_GetError());
		}

		if (SDL_Init(SDL_INIT_AUDIO) != 0)
		{
			printf("SDL_Init( SDL_INIT_AUDIO ) FAILED (%s)\n", SDL_GetError());
		}

		driverName = SDL_GetCurrentVideoDriver();
		printf("SDL using driver \"%s\"\n", driverName);
	}
		
#ifdef GLES
    // todo: figure out scaling so we can use  SDL_WINDOW_ALLOW_HIGHDPI
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    int Width = DM.w;
    int Height = DM.h;
    
    printf("SDL Display w: %i h: %i\n", Width, Height);
    
    m_screenheight = Height;
    m_screenwidth = Width;        
#endif

	// Create Our OpenGL Window -- quit if window was not created
	if (!CreateGLWindow(APP_TITLE, m_screenwidth, m_screenheight, nColorDepth, (BOOL)g_cvFullscreen.value))
		return FALSE;

	// Can play game without sound. Do not exit
	if(!InitializeGameSounds())
		g_bDirectSoundEnabled = FALSE;
	else 
		g_bDirectSoundEnabled = TRUE;

	// Default sound enabled on if the system
	// is sound capable
	if ((g_cvSoundEnabled.value == 2.0f) || !g_bDirectSoundEnabled)
	{
		Cvar_SetValue("SoundEnabled", (float)g_bDirectSoundEnabled);
	}

	// Set the sound's volume
	SetVolume(g_cvVolume.value);
		
	// Attempt to setup DirectInput.  Don't exit if it fails

	// TODO: Removing DirectInput, will replace with SDL -tkidd
	if( !InitializeInput())
	{
		g_bDirectInputEnabled = FALSE;
	}
	else
	{
		g_bDirectInputEnabled = TRUE;

		// Enable joystick by default
		Cvar_SetString("JoystickEnabled", "-1");
		
		// Attempt to activate force feedback and enable it if it's present
		// TODO: put this back when we get SDL input in place -tkidd
		//g_bForceFeedbackCapable = BOOL(HasForceFeedbackJoystick(hInstance) == TRUE);

		// If the joystick is capable of force feedback, enable it 
		Cvar_SetValue("ForceFeedbackEnabled", (g_bForceFeedbackCapable != 0));
		
	}

	// Setup rendering flags
	renderinit();

	// Setup Vsync
	SetVsync(!(int)Cvar_GetValue("VsyncEnabled"));

	// Get refresh timing rate for automatic frame rate control
	g_fSwapBufferInterval = GetRefreshTimingRate();

	// Set gamma
	SetGamma();

	// Return  success
	return TRUE;
}


// Kill GL lists, sounds, controls, window
void TerminateGame(void)
{
	// Kill GL fonts
	KillFont();

	// Kill display lists
#if 0
	glDeleteLists(player_list, NUM_LISTS);
#endif

	// Kill textures
	glDeleteTextures(NUM_TEXTURES, &texture[0]);

	// Turn off DirectSound
	CleanupGameSounds();

	// Turn off DirectInput
	DestroyInput();

	// Restore gamma ramp before exit
	SDL_SetWindowGammaRamp(SDL_window, old_gamma_ramp[0], old_gamma_ramp[1], old_gamma_ramp[2]);

	// Kill The Window
	KillGLWindow();

}


/*

  Used in AdjustRockColor to convert bytes into something usable

  I don't know if there's a built-in C++ function to do this,
  so I wrote this myself.  ;)

*/
unsigned int ConvertGLbyteToUint(const GLbyte& value)
{
	if (value < 0)
		return (unsigned int)abs(value) + 128;
	else
		return (unsigned int)value;
}


/* 
	This function makes sure that rocks don't get 
	too dark, while still maintaining the cool rock
	colors of previous versions of the game.
*/
void AdjustRockColor(const int& i)
{

	unsigned int r = 0;
	unsigned int g = 0;
	unsigned int b = 0;
	
	float fRGBSum = 0;
	float fRGBAvg = 0;
	float fPctR = 0;
	float fPctG = 0;
	float fPctB = 0;
	int	nTemp = 0;
	
	const float cMinAvg = 102.0f;
	const float cMinSum = cMinAvg * 3.0f;

	// Convert bytes to unsigned int values
	r = ConvertGLbyteToUint(actors[i].color.r);
	g = ConvertGLbyteToUint(actors[i].color.g);
	b = ConvertGLbyteToUint(actors[i].color.b);

	// Add values
	fRGBSum = float(r) + float(g) + float(b);
	fRGBAvg = fRGBSum / 3.0f;

	if (fRGBAvg < cMinAvg)
	{
		fPctR = r / fRGBSum;
		fPctG = g / fRGBSum;
		fPctB = b / fRGBSum;

		nTemp = int(cMinSum * fPctR);
		if (nTemp > 255) 
			nTemp = 255;
		actors[i].color.r = (GLbyte)nTemp;

		nTemp = int(cMinSum * fPctG);
		if (nTemp > 255)
			nTemp = 255;
		actors[i].color.g = GLbyte(nTemp);

		nTemp = int(cMinSum * fPctB);
		if (nTemp > 255)
			nTemp = 255;
		actors[i].color.b = (GLbyte)nTemp;

	}

}

// CVar code 
void InitCvars()
{

	Cvar_Register(&g_cvResolution);
	Cvar_Register(&g_cvColorDepth);

	Cvar_Register(&g_cvGammaGamma);
	Cvar_Register(&g_cvGammaBrightness);
	Cvar_Register(&g_cvGammaContrast);

	Cvar_Register(&g_cvFullscreen);
	Cvar_Register(&g_cvSpaceMode);
	Cvar_Register(&g_cvCopernicusMode);
	Cvar_Register(&g_cvDifficulty);

	Cvar_Register(&g_cvKeyThrust[0]);
	Cvar_Register(&g_cvKeyThrust[1]);
	Cvar_Register(&g_cvKeyShields[0]);
	Cvar_Register(&g_cvKeyShields[1]);
	Cvar_Register(&g_cvKeyHyperspace[0]);
	Cvar_Register(&g_cvKeyHyperspace[1]);
	Cvar_Register(&g_cvKeyRotateLeft[0]);
	Cvar_Register(&g_cvKeyRotateLeft[1]);
	Cvar_Register(&g_cvKeyRotateRight[0]);
	Cvar_Register(&g_cvKeyRotateRight[1]);
	Cvar_Register(&g_cvKeyFire[0]);
	Cvar_Register(&g_cvKeyFire[1]);
	Cvar_Register(&g_cvKeyStartGame[0]);
	Cvar_Register(&g_cvKeyStartGame[1]);

	printf("1: g_cvKeyPause[0].value: %f\n", g_cvKeyPause[0].value);

	Cvar_Register(&g_cvKeyPause[0]);

	printf("2: g_cvKeyPause[0].value: %f\n", g_cvKeyPause[0].value);

	Cvar_Register(&g_cvKeyPause[1]);

	Cvar_Register(&g_cvShipRotateAdjust);
	
	Cvar_Register(&g_cvMouseSensitivity);
	Cvar_Register(&g_cvMouseEnabled);

	Cvar_Register(&g_cvLightingEnabled);
	Cvar_Register(&g_cvBlendingEnabled);
	Cvar_Register(&g_cvTexturesEnabled);
	Cvar_Register(&g_cvWireframeEnabled);
	Cvar_Register(&g_cvBoundingBoxesEnabled);
	Cvar_Register(&g_cvFpsEnabled);
	Cvar_Register(&g_cvVsyncEnabled);
	Cvar_Register(&g_cvSpaceSize);

	Cvar_Register(&g_cvJoystickEnabled);
	Cvar_Register(&g_cvForceFeedbackEnabled);
	Cvar_Register(&g_cvJoystickDeadzone);
	
	Cvar_Register(&g_cvSoundEnabled);
	Cvar_Register(&g_cvVolume);

	Cvar_Register(&g_cvInfiniteLives);
	Cvar_Register(&g_cvInfiniteShields);
	Cvar_Register(&g_cvInvincibility);

	Cvar_Register(&g_cv3dglasses);

}

void InitActors() {

		// Initialize actors
	for (int i = 0; i < MAX_ACTORS; i++) 
	{
		actors[i].active			= FALSE;
		actors[i].type				= ACTOR_NONE;
		actors[i].displaylistid = NULL;
		actors[i].x					= 0;
		actors[i].y					= 0;
		actors[i].z					= WORLD_DEPTH;
		actors[i].rx				= 0;
		actors[i].ry				= 0;
		actors[i].rz				= 0;
		actors[i].vx				= 0;
		actors[i].vy				= 0;
		actors[i].vz				= 0;
		actors[i].color.r			= (char)255;
		actors[i].color.g			= (char)255;
		actors[i].color.b			= (char)255;
		actors[i].LifeStartTime = 0;
		actors[i].LifeEndTime	= 0;
		actors[i].LifePercent	= 0.0f;
		actors[i].size				= 0;
		actors[i].ShieldsUp		= FALSE;
		actors[i].ShieldsPower	= 0;
		actors[i].Lives			= 0;
		actors[i].NextBonus		= 0;
		actors[i].Score			= 0;
		actors[i].ShotDelay		= 0;
		actors[i].ChangeDirectionDelay = 0;
		actors[i].PointValue		= 0;
		actors[i].InvincibleEndTime = 0;
		actors[i].xcolldist		= 0;
		actors[i].ycolldist		= 0;
		actors[i].Hyperspacing	= FALSE;
		actors[i].HyperspaceInterval = 0;
		actors[i].HyperspaceDelay = 0;
	}
	
	
	// Setup player to point to the point where player1 is 
	// in the actors list
	player = &actors[IDX_PLAYER1];

	// Setup shields
	actors[IDX_SHIELDS].active = TRUE;
	actors[IDX_SHIELDS].type = ACTOR_SHIELDS;
	actors[IDX_SHIELDS].displaylistid = shields_list;
	actors[IDX_SHIELDS].x		= 0;
	actors[IDX_SHIELDS].y		= 0;
	actors[IDX_SHIELDS].z		= WORLD_DEPTH;
	actors[IDX_SHIELDS].rx		= 0;
	actors[IDX_SHIELDS].ry		= 0;
	actors[IDX_SHIELDS].rz		= 0;
	actors[IDX_SHIELDS].vx		= 0;
	actors[IDX_SHIELDS].vy		= 0;
	actors[IDX_SHIELDS].vz		= 0;
	actors[IDX_SHIELDS].color.r		= (char)255;
	actors[IDX_SHIELDS].color.g		= (char)255;
	actors[IDX_SHIELDS].color.b		= (char)255;
	actors[IDX_SHIELDS].LifeEndTime	= 0;
	actors[IDX_SHIELDS].size			= 0;
	actors[IDX_SHIELDS].ShieldsUp		= FALSE;
	actors[IDX_SHIELDS].ShieldsPower	= 0;
	actors[IDX_SHIELDS].xcolldist = 0.5f;
	actors[IDX_SHIELDS].ycolldist = 0.5f;
}



//--------------------------------------------------------
//
// TCW
// There's WAY too much code in WinMain.  All of the game 
// logic is contained in here.  Next game this routine
// won't contain anything game-related except a call to 
// a routine named game_main.
//
//--------------------------------------------------------

int main(int argc, char* args[])
{
	int		i				= 0;		// i,j,k are counter variables
	int		j				= 0;
	int		k				= 0;
	
	BOOL	bPlayerThrust	= FALSE; // Variable replaces the use of velocity
											// to flag when player is thrusting. 
											// Based on the amount of time between
											// frames, velocity would sometimes be zero.

	float fLastLoopTime	= 0;		// Fractional time when last loop was started
	float fDeltaTime		= 0;		// Change in fractional seconds since last iteration
	
	// Level variables
	BOOL	bAllEnemiesInactive	= TRUE;	// Init TRUE to reload all rocks
	float fLevelStartTime		= 0;		// Stores time on current level
	float	fTimeForNewRocks		= 1;		// Controls time between level clear and new level
	float	fPlayerRespawnTime	= 0;
	float	fThrustDebrisTime		= 0;

	// Enemy variables
	float fTimeForNewLSaucer	= 0;
	
	// Sound flags
	BOOL	bPlayThump			= FALSE;	// Toggles between hi and lo thump
	float	fNextThumpTime		= 0;		// Ticks when next thump will occur

	// FPS variables
//	long	lFramesRendered			= 0;
	float	fProgramStartTime			= 0;

	// DirectInput variables
	unsigned long	DirectInput					= 0;
//	DWORD	LastDirectInput			= 0;

	// Automatic framerate adjust variables
	const float DEFAULT_SWAPBUFFER_TIMING_ADJUST = 0.003f;
	float fTime						=0;
	float fLastSwapBufferTime = 0;
	float fSwapBufferTimingAdjust = DEFAULT_SWAPBUFFER_TIMING_ADJUST;
	int	nConsecutiveMissedSwapBuffers = 0;
	float fLastDrawGLSceneTime = 0;

    int nSpaceSize = 0;

	char sKeyBuffer[16]		= "";

	// Setup an array to keep track of when 
	// the last 10 hyperspaces took place
	// so that player can be penalized for
	// misuse of the hyperspace.  :)
	int	nHyperspaceUseCount = 0;
	float	afPreviousHyperspaceTicks[10] = {0,0,0,0,0,0,0,0,0,0};
	
	// Init developer mode flag
	Cvar_Register(&g_cvDeveloper);

	// Init timers
	TimerInit();

	// Initialize timing variables
	g_fNow = TimerGetTime();
	fProgramStartTime = g_fNow;

	// init console
	con_init();
	con_printf(VERSION);
	
	// Display version info on startup 
	UpdateMessage(VERSION);
	g_lMessageEndTime = (long)g_fNow + 5;

	// Initialize Cvars
	InitCvars();

	// Initialize commands
	InitializeCommands();

	// Load the game's configuration settings, which are 
	// stored as cvars.
	LoadCfg();

	// Prompt the user to select screen mode.  This is only done
	// once, the first time the program is run.  After that, the
	// program defaults to the console variable value.
	if (Cvar_GetValue("Fullscreen") == 2) 
	{
        // TODO: figure out macOS/SDL equivalent -tkidd
#ifdef _WIN32
		if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION) == IDNO)
#endif

#ifdef GLES
            Cvar_SetValue("Fullscreen", TRUE);
#else
			Cvar_SetValue("Fullscreen", FALSE);
#endif
#ifdef _WIN32
		else
			Cvar_SetValue("Fullscreen", TRUE);
#endif
	}

	// Setup graphics, controls and sounds
	if (!InitializeGame())
		return 0;

	// Release mouse until game actually starts
	UnacquireMouse();
	
	// Initialize Sin and Cos lookup tables
	InitLookupTables();

	// New random number seed
	srand((unsigned)time(NULL));

	// Load the highscores
	LoadHighScores("disasteroids3d.dat");

	// Initialze stars
	for (i = 0; i < NUM_STARS; i++)
	{
		stars[i].x = randFloat(-STAR_WORLD_HALFWIDTH, STAR_WORLD_HALFWIDTH);
		stars[i].y = randFloat(-STAR_WORLD_HALFHEIGHT, STAR_WORLD_HALFHEIGHT);
		stars[i].z = randFloat(MIN_STAR_Z, MAX_STAR_Z);
		stars[i].size = randFloat(MIN_STAR_SIZE, MAX_STAR_SIZE);
	}

	// To simplify this routine I moved the actor
	// initialization into its own routine
	InitActors();

	//Event handler
	SDL_Event e;

#ifndef IOS // don't want this on iPhone
	SDL_StartTextInput();
#endif

	// Main program loop
	while(!g_bExitApp)
	{

		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				g_bExitApp = true;
			}
            else if (e.type == SDL_TEXTINPUT) {
                keys[e.text.text[0]] = TRUE;
            }
			//Handle keypress with current mouse position
			else if (e.type == SDL_KEYDOWN)
			{
                // TODO: why is this not handled by the main loop? Maybe factor this out so we don't have to repeat it
                #ifdef IOS
                if (e.key.keysym.sym == SDLK_RETURN) {
                    g_bEnteringHighScoreInitials = FALSE;
                    playerhighscore = NULL;
                
                    // on iOS we can hide the keyboard on a "Return"
                    SDL_StopTextInput();
                    
                    PlayMenuExplosionSound();
                }
                else if (e.key.keysym.sym == SDLK_BACKSPACE)
                {
                    if (g_nHighScoreInitialsIdx > 0)
                    {
                        g_nHighScoreInitialsIdx--;
                        playerhighscore->Initials[g_nHighScoreInitialsIdx] = ' ';

                        PlayMenuBeepSound();
                    }
                }
                
                #endif

				handleKey(e.key, TRUE);
			}
			else if (e.type == SDL_KEYUP)
			{
				handleKey(e.key, FALSE);
			}
            else if (e.type == SDL_JOYBUTTONDOWN)
            {
                if (e.cbutton.button == 3) {
                    keys[int(g_cvKeyThrust[0].value)] = TRUE;
                }
            }
            else if (e.type == SDL_JOYBUTTONUP)
            {
                if (e.cbutton.button == 3) {
                    keys[int(g_cvKeyThrust[0].value)] = FALSE;
                }
            }
            else if (e.type == SDL_JOYDEVICEADDED)
            {
                SDL_GameController *toOpen = SDL_GameControllerOpen(e.cdevice.which);
                printf(
                    "Opened SDL_GameController ID #%i, %s\n",
                    e.cdevice.which,
                    SDL_GameControllerName(toOpen)
                );
            }
            else if (e.type == SDL_JOYAXISMOTION)
            {
                int sensitivity = 8000;
                
                if (e.jaxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
                {
                    if (e.jaxis.value < -sensitivity)
                    {
                        keys[int(g_cvKeyRotateLeft[0].value)] = TRUE;
                        keys[int(g_cvKeyRotateRight[0].value)] = FALSE;
                    }
                    else if (e.jaxis.value > sensitivity)
                    {
                        keys[int(g_cvKeyRotateLeft[0].value)] = FALSE;
                        keys[int(g_cvKeyRotateRight[0].value)] = TRUE;
                    }
                    else
                    {
                        keys[int(g_cvKeyRotateLeft[0].value)] = FALSE;
                        keys[int(g_cvKeyRotateRight[0].value)] = FALSE;
                    }
                }
                if (e.jaxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                    keys[int(g_cvKeyThrust[0].value)] = e.jaxis.value;
                }
                if (e.jaxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                    keys[int(g_cvKeyFire[0].value)] = e.jaxis.value;
                }
            }
        }
		// Update time
		if (!g_bGamePaused)
		{

			g_fNow = TimerGetTime() - g_fNowOffset;
			fDeltaTime = g_fNow - fLastLoopTime;

			if (fDeltaTime > 0)
				g_nFps = int(1.0f / fDeltaTime);

			fLastLoopTime = g_fNow;

		}

		// Compute average FPS
		g_fGameLength = g_fNow - fProgramStartTime;

		// Don't hog processor when game isn't active
#ifdef _WIN32
		if (!g_bActive)
		{
			WaitMessage();
		}
#endif

		// Swap buffers -- control max frame rate when
		// vsync is enabled
		if (g_cvVsyncEnabled.value) 
		{

			fTime = TimerGetTime();
			if (fTime - fLastSwapBufferTime >= g_fSwapBufferInterval - fSwapBufferTimingAdjust)
			{

				// Swap the buffers
				SDL_GL_SwapWindow(SDL_window);

				// Check to make sure that the SwapBuffers() call
				// wasn't too early and increment the consec missed
				// counter if so
				if (TimerGetTime() - fTime >= g_fSwapBufferInterval)
					nConsecutiveMissedSwapBuffers ++;
				else
					nConsecutiveMissedSwapBuffers = 0;

				// Adjust timing if five swap buffers are missed in a row
				if (nConsecutiveMissedSwapBuffers >= 5)
				{
					fSwapBufferTimingAdjust += 0.002f;
					nConsecutiveMissedSwapBuffers = 0;
				}

				// Make sure that SwapBufferTimingAdjust doesn't go screwy.
				// If the value is higher than fSwapBufferInterval, it's time
				// to reset the frame rate adjust system and try again
				if (fSwapBufferTimingAdjust >= g_fSwapBufferInterval * 0.75f)
				{
					g_fSwapBufferInterval = GetRefreshTimingRate();
					fSwapBufferTimingAdjust = DEFAULT_SWAPBUFFER_TIMING_ADJUST;
				}

				// Update FPS 
				if (g_cvFpsEnabled.value)
					UpdateMessage("FPS: %4.0f  Adjust=%f", 1 / (TimerGetTime() - fLastSwapBufferTime), fSwapBufferTimingAdjust);

				// Reset the buffer swap timer
				fLastSwapBufferTime = TimerGetTime();

			}
			
			//	The scene is drawn twice for every time
			// SwapBuffer is called.  This is done to keep
			// the number of tossed away frames to a minimum
			if (TimerGetTime() - fLastDrawGLSceneTime >= g_fSwapBufferInterval * 0.5f)
			{
				
				// Update last scene time
				fLastDrawGLSceneTime = TimerGetTime();

				// Draw the scene
				if (g_bActive)
				{
					if (!DrawGLScene()) 
					{
						g_bExitApp = TRUE;
					}
				}
			}

		} // if (g_cvVsyncEnabled.value)


		// Run full speed when the vsync is disabled
		else
		{

			// Only draw the scene when the window is active
			if (g_bActive)
			{
				if (!DrawGLScene())
				{
					g_bExitApp = TRUE;
				}
				else
				{
					//SwapBuffers(hDC);
					SDL_GL_SwapWindow(SDL_window);
				}
			}

			// Update FPS 
			if ((g_cvFpsEnabled.value) && (fDeltaTime))
				UpdateMessage("FPS: %4.0f", 1 / fDeltaTime);

		}


		// Get joystick and mouse inputs (if enabled)
		if (g_bDirectInputEnabled)
			DirectInput = GetDeviceInput();


		// Don't thump if the game's over
		if (!g_bGameOver)
		{
		
			// Setup the first thump
			if (!fNextThumpTime)
				fNextThumpTime = g_fNow + 1.0f;
			
			// Play the thump
			if ((g_fNow > fNextThumpTime) && (g_cvSoundEnabled.value)) {
				if (bPlayThump)
					Mix_PlayChannel(-1, g_pThumpLoSound, 0);
				else
					Mix_PlayChannel(-1, g_pThumpHiSound, 0);

				// Alternate thumps
				bPlayThump = !bPlayThump;
			
				// Set next thump to play in 1 second
				fNextThumpTime = g_fNow + 1.0f;

				// Make the beats speed up over time
				switch ((int)(g_fNow - fLevelStartTime) / 15) {
					case 0:	
						fNextThumpTime = g_fNow + 1.0f;
						break;
					
					case 1:
						fNextThumpTime = g_fNow + 0.75f;
						break;

					case 2:
						fNextThumpTime = g_fNow + 0.5f;
						break;

					default:
						fNextThumpTime = g_fNow + 0.25f;
						break;
				}
			}
		}



		// Process keystrokes

		// Update console
		con_update();
		
		// Lots of keyboard handling gets skipped when the console is active
		if (!g_con_active)
		{

			// Highscore initials entry
			if (g_bEnteringHighScoreInitials)
			{
				
				// Delay before accepting keystrokes so that 
				// player's last keystrokes aren't entered
				// as their highscore
				if (g_lHighScoreEntryDelayEndTime <= g_fNow)
				{

					// Check for backspace -- clears initials
					if (KeyDown(SDLK_BACKSPACE))
					{
						if (g_nHighScoreInitialsIdx > 0)
						{
							g_nHighScoreInitialsIdx--;
							playerhighscore->Initials[g_nHighScoreInitialsIdx] = ' ';

							PlayMenuBeepSound();
						}

					}
					// Check for enter
					else if (KeyDown(SDLK_RETURN))
					{
						g_bEnteringHighScoreInitials = FALSE;
						playerhighscore = NULL;
						PlayMenuExplosionSound();
					}
					// Check for legal initial chars
					// TODO: figure out the right way to do this with SDL -tkidd
					else if (g_nHighScoreInitialsIdx <= 2)
					{
						if (KeyDown(SDLK_SPACE))
						{
							playerhighscore->Initials[g_nHighScoreInitialsIdx] = g_cLegalHighScoreChars[0];
							g_nHighScoreInitialsIdx++;

							PlayMenuBeepSound();

							break;
						}

						int cpointer = 1;

						// numbers
						for (i = SDLK_0; i <= SDLK_9; i++) 
						{
							if (KeyDown(i))
							{
								playerhighscore->Initials[g_nHighScoreInitialsIdx] = g_cLegalHighScoreChars[cpointer];
								g_nHighScoreInitialsIdx++;

								PlayMenuBeepSound();

								break;
							}

							cpointer++;
						}

						cpointer = 11;

						for (i = SDLK_a; i <= SDLK_z; i++)
						{
							if (KeyDown(i))
							{
								playerhighscore->Initials[g_nHighScoreInitialsIdx] = g_cLegalHighScoreChars[cpointer];
								g_nHighScoreInitialsIdx++;

								PlayMenuBeepSound();

								break;
							}

							cpointer++;
						}
					}
				}
			} // if (g_bEnteringHighScoreInitials)

			// These keys don't work when the highscore is 
			// being entered
			else
			{

				// Certain keys only work between games
				if (g_bGameOver)
				{

					if (((keys[(int)g_cvKeyStartGame[0].value]) || (keys[(int)g_cvKeyStartGame[1].value]) || (g_bStartGame)) && !g_bShowMenu)
					{

						// Reset g_bStartGame
						g_bStartGame = FALSE;
					
						// Game's no longer over
						g_bGameOver = FALSE;

						// Restart game
						g_nLevelNum = 0;					// Reset level number
						fPlayerRespawnTime = 1;			// Respawn player ASAP
						player->Score = 0;				// Reset score

						// Store the current space size selection so that the 
						// highscore table is updated correctly at the end of 
						// the game
						nSpaceSize = int(g_cvSpaceSize.value);

						// Setup world and screen size 
						float fSpaceSizeMultiplier = 0;
						switch (nSpaceSize)
						{
						case 0:
							fSpaceSizeMultiplier = 0.5f;
							break;
						case 1:
							fSpaceSizeMultiplier = 1.0f;
							break;
						case 2:
							fSpaceSizeMultiplier = 1.3f;
							break;
						}
						
						WORLD_WIDTH			= 22	* fSpaceSizeMultiplier;
						WORLD_HALFWIDTH	= 11	* fSpaceSizeMultiplier;
						WORLD_HEIGHT		= 16	* fSpaceSizeMultiplier;
						WORLD_HALFHEIGHT	= 8 	* fSpaceSizeMultiplier;
						WORLD_DEPTH			= -20 * fSpaceSizeMultiplier;

						SCREEN_WIDTH		= 22	* fSpaceSizeMultiplier;
						SCREEN_HALFWIDTH	= 11	* fSpaceSizeMultiplier;
						SCREEN_HEIGHT		= 16	* fSpaceSizeMultiplier;
						SCREEN_HALFHEIGHT	= 8	* fSpaceSizeMultiplier;
						SCREEN_DEPTH		= -20 * fSpaceSizeMultiplier;

						// Reposition actors' depths
						for (i = 0; i < MAX_ACTORS; i++) {
							actors[i].z	= WORLD_DEPTH;
						}

						// Set current difficulty so that it can't 
						// be changed mid-game
						g_nDifficulty = (int)g_cvDifficulty.value;

						// Reset player lives
						if (g_nDifficulty == EASY_DIFFICULTY)
							player->Lives = 5;
						else
							player->Lives = 3;

						player->NextBonus = 10000;		// Reset next bonus

						// Kill rocks
						for (i = IDX_FIRST_ROCK; i < IDX_LAST_ROCK; i++) 
						{
							actors[i].active = FALSE;
						}

						// Kill shots
						for (i = IDX_FIRST_SHOT; i < IDX_LAST_SHOT; i++)
							actors[i].active = FALSE;

						// Kill saucer shots
						for (i = IDX_FIRST_SFIRE; i < IDX_LAST_SFIRE; i++)
							actors[i].active = FALSE;

						// Kill saucer
						actors[IDX_LSAUCER].active = FALSE;

						// Reset rocks ASAP
						fTimeForNewRocks = 1;

						// Acquire mouse if mouse mode is enabled
						SetMouse((BOOL)g_cvMouseEnabled.value);
												
					}
			
				} // if (g_bGameOver)

				// Certain keys only work during games
				else
				{
					if (KeyDown(int(g_cvKeyPause[0].value)) || KeyDown(int(g_cvKeyPause[1].value)))
					{
						printf("pause game hit\n");
						if (g_bGamePaused)
							PauseGame(FALSE);
						else
							PauseGame(TRUE);
					}
						
				}


				// ESC key shows game settings menu
				if (!g_bShowMenu)
				{
					if ((KeyDown(VK_ESCAPE)) && (g_bShowMenu == FALSE))
					{

						// Set the show menu flag
						g_bShowMenu = TRUE;

						// Pause the game if the menu is being
						// displayed mid-game
						if (!g_bGameOver)
							PauseGame(TRUE);

						// Initialize variables
						Menu_Init();
					}

					// *SHH* Activate the cheat menu
					if (	(sKeyBuffer[0] == 0x4d) &&
							(sKeyBuffer[1] == 0x4f) &&
							(sKeyBuffer[2] == 0x48) &&
							(sKeyBuffer[3] == 0x54) &&
							(sKeyBuffer[4] == 0x52) &&
							(sKeyBuffer[5] == 0x45) &&
							(sKeyBuffer[6] == 0x50) &&
							(sKeyBuffer[7] == 0x55) &&
							(sKeyBuffer[8] == 0x53))
					{
						// Set the show menu flag
						g_bShowMenu = TRUE;

						// Pause game
						if (!g_bGameOver)
							PauseGame(TRUE);

						// Init variables
						Menu_ShowCheatMenu();
					}

				}
				else
				{
					// When menu is exited, the function returns FALSE.
					// The game should be resumed and g_bShowMenu should
					// be reset
					if (Menu_Update() == FALSE)
					{
						// Reset the show menu flag
						g_bShowMenu = FALSE;

						// Resume game
						PauseGame(FALSE);

					}
				}


				// Don't allow player to hit movement keys while the 
				// game is paused
				if (!g_bGamePaused)
				{
					
					// Try cheesy fix to time problem when rotating
					if (fDeltaTime)
					{

						float fPlayerRxChange = 0;

						// Player turn left
						if ((keys[int(g_cvKeyRotateLeft[0].value)]) || (keys[int(g_cvKeyRotateLeft[1].value)]) || (DirectInput & JOYSTICK_LEFT))
						{
							player->rz += 1200.0f * fDeltaTime * g_cvShipRotateAdjust.value;
							fPlayerRxChange = 50.0f * fDeltaTime;
						}

						// Player turn right
						if ((keys[int(g_cvKeyRotateRight[0].value)]) || (keys[int(g_cvKeyRotateRight[1].value)]) || (DirectInput & JOYSTICK_RIGHT))
						{
							player->rz -= 1200.0f * fDeltaTime * g_cvShipRotateAdjust.value;
							fPlayerRxChange = -50.0f * fDeltaTime;
						}

						// Move player based on mouse position
						// TODO: Mouse later -tkidd
						/*if (g_cvMouseEnabled.value != 0)
						{
							if (g_dims.lX)
							{
								player->rz += float(-g_dims.lX) * 0.5f * g_cvMouseSensitivity.value;
								fPlayerRxChange = -g_dims.lX * 3.0f * fDeltaTime;
							}
						}*/


						// If fPlayerRxChange is 0, set it to a value 
						// that will return ry to zero
						if (fPlayerRxChange == 0)
						{
							if (player->rx < 0) 
								fPlayerRxChange += 250.0f * fDeltaTime;
							else if (player->rx > 0)
								fPlayerRxChange -= 250.0f * fDeltaTime;
						}
						
						// Adjust rx and make sure it stays in bounds
						player->rx += fPlayerRxChange;
						if (player->rx > 45.0f)
							player->rx = 45.0f;
						else if (player->rx < -45.0f)
							player->rx = -45.0f;

						// If rx is within five degrees of zero, force it
						// to zero to avoid player 'jitter'.
						if (fabs(player->rx) < 5.0f)
							player->rx = 0;

						// Reset RxChange
						fPlayerRxChange = 0;

					}

					// Player thrust
					bPlayerThrust = ((keys[int(g_cvKeyThrust[0].value)]) || (keys[int(g_cvKeyThrust[1].value)]) || (DirectInput & JOYSTICK_UP));

					// Activate infinite shields
					if (g_cvInfiniteShields.value)
					{
							player->ShieldsPower = 255;
					}

					// Activate shields when down arrow's down
					if ((keys[int(g_cvKeyShields[0].value)]) || (keys[int(g_cvKeyShields[1].value)]) || (DirectInput & JOYSTICK_SHIELDS))
					{
						if (player->active) 
						{
							player->ShieldsUp = TRUE;
							actors[IDX_SHIELDS].active = TRUE;
						}
					}
					else 
					{
						player->ShieldsUp = FALSE;
						actors[IDX_SHIELDS].active = FALSE;
					}


					// Hyperspace
					if (player->Hyperspacing == FALSE) 
					{

						// Check the hyperspace keys
						if (((keys[int(g_cvKeyHyperspace[0].value)]) || (keys[int(g_cvKeyHyperspace[1].value)]) || (DirectInput & JOYSTICK_HYPERSPACE)) && (player->active)) 
						{
							// Code to keep track of Hyperspace Abusers
							//
							// Shift hyperspace time array down one element
							// and count the number of times hyperspace has 
							// been used in the last ten seconds
							nHyperspaceUseCount = 0;
							for (i = 9; i >= 0; i--)
							{
								afPreviousHyperspaceTicks[i + 1] = afPreviousHyperspaceTicks[i];
								
								// Count uses of the hyperspace key in the last 10 seconds
								if ((g_fNow - afPreviousHyperspaceTicks[i] <= 10) && (afPreviousHyperspaceTicks[i] != 0))
								{
									nHyperspaceUseCount++;
								}
							}
							afPreviousHyperspaceTicks[0] = g_fNow;

							// Debug messages
							// UpdateMessage("HyperspaceUseCount = %d", nHyperspaceUseCount);
							// UpdateMessage("Now %f Last %f Diff %f", g_fNow, afPreviousHyperspaceTicks[1], g_fNow - afPreviousHyperspaceTicks[1]);

							// Set player flags to tell game that player
							// is hyperspacing and setup when he is to reappear
							player->Hyperspacing = TRUE;
							player->HyperspaceDelay = 0.5f;
							player->active = FALSE;

							// Spawn debris
							CreateDebris(player, 10);

						}
					}
					else 
					{
						// The Hyperspace duration is up -- spawn player
						if (player->HyperspaceDelay <= 0) 
						{
							// UpdateMessage("Hyperspace over - spawning player");
						
							// Hyperspace player
							if (SpawnPlayer(TRUE)) 
							{
								
								// Punish Hyperspace Abusers
								if ((nHyperspaceUseCount > randInt(0, 20)) && (nHyperspaceUseCount >= 3))
								{
									
									// Display message -- muhahahaha
									UpdateMessage("BOOM!  HYPERSPACE ABUSER!!!!");

									// Find rock and bash user into it -- BOOM
									for (i = IDX_FIRST_ROCK; i <= IDX_LAST_ROCK; i++)
									{
										if (actors[i].active) {
											player->x = actors[i].x;
											player->y = actors[i].y;
											break;
										}
									}
								}

								// Clear hyperspace flags
								player->Hyperspacing = FALSE;
								player->HyperspaceInterval = 0.05f;
							}
						}
						else 
						{
							player->HyperspaceDelay -= fDeltaTime;
						}
					}


					// Spawn shots 
					if (((keys[int(g_cvKeyFire[0].value)]) || (keys[int(g_cvKeyFire[1].value)]) || (DirectInput & JOYSTICK_FIRE)) && player->active)  
					{
						// Make sure enough time has passed between shots
						if (g_fNow > player->ShotDelay) 
						{

							// Find free actor
							i = FindFreeActor(IDX_FIRST_SHOT, IDX_LAST_SHOT);

							// A free shot space is available
							if (i) {

								// Init actor by setting all of its
								// properties equal to the player's
								actors[i] = actors[IDX_PLAYER1];

								actors[i].active = TRUE;
								actors[i].type = ACTOR_SHOT;
								
								// Make the shots spawn from the tip
								// of the ship instead of the center
								actors[i].x += (float)CosLookup(player->rz) * 0.5f;
								actors[i].y += (float)SinLookup(player->rz) * 0.5f;

								actors[i].vx = 20.1f;
								actors[i].vy = 20.1f;

								actors[i].vz = 0.0f;
								actors[i].displaylistid = shot_list;
								actors[i].LifeEndTime = g_fNow + 0.7f;
								
								actors[i].xcolldist = 0.2f; 
								actors[i].ycolldist = 0.2f;

								// Delay period between shots
								player->ShotDelay = g_fNow + 0.3f;

								// Play sound
								if (g_cvSoundEnabled.value)
								{
									FLOAT fScreenXPos = actors[i].x / WORLD_HALFWIDTH;// *cStereoSeperation;
									float fRight = (fScreenXPos * 127) + 127;
									int iRight = (int)fRight;
									int iLeft = 254 - iRight;
									Mix_SetPanning(channelBulletFiringSound, iLeft, iRight);
									Mix_PlayChannel(channelBulletFiringSound, g_pBulletFiringSound, 0);
								}
							}
						}
					}

					// Let the player fire more quickly when 
					// repeatedly hitting the fire button 
					// rather than holding it down
					if (!keys[int(g_cvKeyFire[0].value)] && !keys[int(g_cvKeyFire[1].value)] && !(DirectInput & JOYSTICK_FIRE))
						player->ShotDelay -= 400;
				
				} // if (!g_bGamePaused)

			} // if (g_bEnteringHighScoreInitials)

		} // if (!g_con_active)
		
		// Copy keys into lastkeys
		for (i = 0; i < 255; i++)
		{
			
			// Capture pressed keys into sKeyBuffer[]
			if ((lastkeys[i] != keys[i]) && (keys[i]))
			{
				for (j = 14; j >= 0; j--)
				{
					sKeyBuffer[j] = sKeyBuffer[j -1];
				}
				sKeyBuffer[15] = 0;
				sKeyBuffer[0] = i;
			}

			// Update lastkeys[]
			lastkeys[i] = keys[i];
		}


		// Init flag to try and find when all of the 
		// rocks have been shot
		bAllEnemiesInactive = TRUE;

		// Update actors (when the game isn't paused)
		if (!g_bGamePaused)
		{

			// I'm doing the player seperate from the rest of 
			// the actors so that I can update its position
			// first then move all of the other actors based
			// on its position.  This is necessary for the 
			// player-centric play mode
			if (player->active) {

				// Update all of the actors' LifePercent
				// if their LifeStartTime is given
				if (player->LifeStartTime > 0) 
					player->LifePercent = (float)((float)(g_fNow - player->LifeStartTime) / (float)(player->LifeEndTime - player->LifeStartTime));
				

				// Update player bonus
				if (player->Score >= player->NextBonus)
				{
					player->NextBonus += 10000;
					player->Lives++;

					if (g_cvSoundEnabled.value)
					{
						Mix_PlayChannel(-1, g_pLifeSound, 0);
					}
				}

				// Make sure the player's invincible when necessary
				if (player->InvincibleEndTime != 0)
				{
					if (player->InvincibleEndTime >= g_fNow)
					{
						player->ShieldsPower = 255;
						player->ShieldsUp = TRUE;
						actors[IDX_SHIELDS].active = TRUE;
					}
					else
					{
						actors[IDX_SHIELDS].active = FALSE;
						player->ShieldsPower = 255;
						player->InvincibleEndTime = 0;
					}
				}

				// Apply Player velocity 
				if (bPlayerThrust)
				{


#define use_old_thrust 0

#if use_old_thrust
					float velocity = 0.05f * fDeltaTime;
#else
					float velocity = 0.075f * fDeltaTime;
#endif

					// Set vx and vy
					player->vx += (float)CosLookup(player->rz) * velocity;
					player->vy += (float)SinLookup(player->rz) * velocity;

					// Zero velocity for next loop
					velocity = 0;

					// Spawn fire
					// 2002.09.30
					// I added the i loop so that more fire gets added when vsync is on.
					// I should add a vsync check variable here, but I'm feeling lazy.  ;)
					for (i = 0; i < randInt(1, 3); i++)
					{
						k = FindFreeActor(IDX_FIRST_PARTICLE, IDX_LAST_PARTICLE);
						if (k)
						{
							actors[k] = *player;
							actors[k].type = ACTOR_FLAMES;
							actors[k].displaylistid = smoke_list;
							actors[k].rz += 180 + randInt(-20, 20);
							actors[k].x += (float)CosLookup(actors[k].rz) * 0.55f;
							actors[k].y += (float)SinLookup(actors[k].rz) * 0.55f;
							actors[k].vx = 0.0005f;
							actors[k].vy = 0.0005f;
							actors[k].vz = 0;
							actors[k].LifeStartTime = g_fNow;
							// actors[k].LifeEndTime = g_fNow + randFloat(0.06f, 0.180f);
							actors[k].LifeEndTime = g_fNow + randFloat(0.10f, 0.20f);
							actors[k].size = 0.6f;
						}
					}

					// Spawn smoke
					if (fThrustDebrisTime < g_fNow)
					{

						k = FindFreeActor(IDX_FIRST_PARTICLE, IDX_LAST_PARTICLE);
						if (k)
						{
							// Don't allow more thrust for ... ms
							fThrustDebrisTime = g_fNow + randFloat(0.005f, 0.025f);

							// Base smoke on player
							actors[k] = *player;
							actors[k].type = ACTOR_PARTICLE;
							actors[k].displaylistid = smoke_list;
							actors[k].rz = actors[k].rz + 180 + randFloat(-15, 15);
							actors[k].x += (float)CosLookup(actors[k].rz) * 0.5f;
							actors[k].y += (float)SinLookup(actors[k].rz) * 0.5f;
							actors[k].vx = (GLfloat)fabs(actors[k].vx * 0.25f);
							actors[k].vy = (GLfloat)fabs(actors[k].vy * 0.25f);
							actors[k].vz = 0.01f;
							actors[k].color.r = actors[k].color.g = actors[k].color.b = randInt(16, 64);
							actors[k].LifeStartTime = g_fNow;
							actors[k].LifeEndTime = g_fNow + randFloat(0, 1.25f);
							actors[k].size = 1;
						}
					}

					// Play thrust sound
					if (g_cvSoundEnabled.value)
						if (g_bThrustSoundOn == FALSE)
							Mix_PlayChannel(channelShipThrustSound, g_pShipThrustSound, -1);
					g_bThrustSoundOn = TRUE;

				}
				else {

#if use_old_thrust 
					
					// Decelerate player ship
					//
					// Thanks to Marty R. for sending me this...
					//
					// velocity = velocity - (velocity * drag * elapsedTime)
					//
					// ... equation for controlling drift speed
					//
					player->vx = player->vx - (player->vx * 0.3f * fDeltaTime);
					player->vy = player->vy - (player->vy * 0.3f * fDeltaTime);
#endif

					// Stop sound of player thrust
					if (g_cvSoundEnabled.value)
					{
						Mix_HaltChannel(channelShipThrustSound);
					}
					
					// Flag thrust sound is now off
					g_bThrustSoundOn = FALSE;

				}

#if !use_old_thrust
				player->vx -= float(fabs(player->vx) * player->vx * 0.001f + player->vx * 0.5f) * fDeltaTime;
				player->vy -= float(fabs(player->vy) * player->vy * 0.001f + player->vy * 0.5f) * fDeltaTime;
#endif

				/*
				// This displays the player velocity for debug
				UpdateMessage("Vel x=%f y=%f", player->vx, player->vy);
				*/

				// Update player position
				player->x += player->vx * 200 * fDeltaTime;
				player->y += player->vy * 200 * fDeltaTime;

				// Handle screen wrap
				if (player->x > WORLD_HALFWIDTH) 
					player->x -= WORLD_WIDTH;
				else if (player->x < -WORLD_HALFWIDTH)
					player->x += WORLD_WIDTH;
				if (player->y > WORLD_HALFHEIGHT)
					player->y -= WORLD_HEIGHT;
				else if (player->y < -WORLD_HALFHEIGHT)
					player->y += WORLD_HEIGHT;
			}
		
			
			// Update each actor in the actors[] array
			for (i = 0; i < MAX_ACTORS; i++)
			{
				if ((actors[i].active) && (i != IDX_PLAYER1) && (i != IDX_PLAYER2)){

					// Update all of the actors' LifePercent
					// if their LifeStartTime is given
					if (actors[i].LifeStartTime > 0) 
					{
						actors[i].LifePercent = (float)((float)(g_fNow - actors[i].LifeStartTime) / (float)(actors[i].LifeEndTime - actors[i].LifeStartTime));
					}


					// Player is center of universe
					// if (g_bPreCopernicusMode)
					if (g_cvCopernicusMode.value)
					{
						actors[i].x -= player->x;
						actors[i].y -= player->y;
					}


					switch (actors[i].type) {

						case ACTOR_FLAMINGPARTICLE:
						case ACTOR_PARTICLE:
						case ACTOR_DEBRIS:
						case ACTOR_FLAMES:

							// Particles and debris MUST have LifeEndTime defined
							// or they won't make it past this line
							if (actors[i].LifeEndTime <= g_fNow)
							{
								actors[i].active = FALSE;
							}
							else
							{
								actors[i].x += (float)CosLookup(actors[i].rz) * (actors[i].vx + 2.0f) * fDeltaTime;
								actors[i].y += (float)SinLookup(actors[i].rz) * (actors[i].vy + 2.0f) * fDeltaTime;
								actors[i].z += (float)actors[i].vy * fDeltaTime;
								actors[i].rx += 100.0f * fDeltaTime;
								actors[i].ry += 100.0f * fDeltaTime;
							}

							// Debris can have smoke trails
							if (actors[i].type == ACTOR_DEBRIS)
							{
								
								// Spawn sparkling debris - 
								//		This is new code.  The old code was 
								//		written that at each cycle there was a 
								//		5% chance that the debris would spawn
								//		a particle.  Well now that the cycle runs
								//		10,000 times a frame, that was happening
								//		far more frequently than intended.  The
								//		new method spawns new debris after a 
								//		specified interval.  This is a concept that
								//		I was going to implement in my next game,
								//		but this was a good place to try it out.
								//		It's different from the rest of the timing
								//		code in the game in that it doesn't rely
								//		on g_fNow.  Implementing the during-game
								//		pause was difficult becuase every timing
								//		variable relied on g_fNow + some interval,
								//		and when the game pauses, g_fNow must be
								//		frozen (see pause code for more info).  
								//		Trust me, this is better ;)
								actors[i].ParticleSpawnDelay -= fDeltaTime;
								if (actors[i].ParticleSpawnDelay <= 0)
								{
									k = FindFreeActor(IDX_FIRST_PARTICLE, IDX_LAST_PARTICLE);
									if (k)
									{
										actors[k]			= actors[i];
										actors[k].active	= TRUE;
										actors[k].displaylistid = smoke_list;
										actors[k].type		= ACTOR_FLAMINGPARTICLE;
										actors[k].vx		*= 0.5;
										actors[k].vy		*= 0.5;
										actors[k].vz		= 0.5;
										
										actors[k].color.r	= randInt(48, 128);
										actors[k].color.g	= randInt(48, 128);
										actors[k].color.b	= 64;

										actors[k].LifeStartTime = g_fNow;
										actors[k].LifeEndTime	= g_fNow + randFloat(0.05f, 0.25f); // randFloat(0.050f, 0.100f);
									}

									// Reset spawn delay
									actors[i].ParticleSpawnDelay = randFloat(0.1f, 0.25f); // randFloat(0.01f, 0.04f);
								}
							}

							break;

						case ACTOR_LSAUCER:
							
							// Set flag to indicate that at least one 
							// enemy is alive
							bAllEnemiesInactive = FALSE;

							// Change saucer direction periodically
							if (actors[i].ChangeDirectionDelay <= g_fNow)
							{
								actors[i].ChangeDirectionDelay = g_fNow + randFloat(0.500f, 1.500f);
								actors[i].vy = -actors[i].vy;
							}

							// Update saucer position
							actors[i].x += actors[i].vx * fDeltaTime;
							actors[i].y += actors[i].vy * fDeltaTime;
							actors[i].rz += 500.0f * fDeltaTime;

							// Spawn shots in even intervals
							if (actors[i].ShotDelay <= g_fNow)
							{
								
								// Setup next shot
								actors[i].ShotDelay = g_fNow + 1;

								// Spawn shot

								// Find free actor
								j = FindFreeActor(IDX_FIRST_SFIRE, IDX_LAST_SFIRE);

								// A free shot space is available
								if (j) {
									actors[j].active = TRUE;
									actors[j].type	= ACTOR_SFIRE;
									actors[j].displaylistid = sfire_list;
									actors[j].x		= actors[i].x;
									actors[j].y		= actors[i].y;
									actors[j].z		= actors[i].z;
									actors[j].rx	= actors[i].rx;
									actors[j].ry	= actors[i].ry;
									actors[j].rz	= randFloat(0, 360);
									
									// Set saucer's shot speed based on difficulty
									if (g_nDifficulty == EASY_DIFFICULTY)
									{
										actors[j].vx	= 15.0f;
										actors[j].vy	= 15.0f;
										actors[j].vz	= 0.1f;
									}
									else
									{
										actors[j].vx	= 20.1f;
										actors[j].vy	= 20.1f;
										actors[j].vz	= 0.1f;
									}
									actors[j].LifeEndTime = g_fNow + 0.500f;

									actors[j].xcolldist = 0.2f;
									actors[j].ycolldist = 0.2f;

									// Play fire sound
									if (g_cvSoundEnabled.value)
									{
										FLOAT fScreenXPos = actors[i].x / WORLD_HALFWIDTH;// *cStereoSeperation;
										float fRight = (fScreenXPos * 127) + 127;
										int iRight = (int)fRight;
										int iLeft = 254 - iRight;
										Mix_SetPanning(channelSaucerFireSound, iLeft, iRight);
										Mix_PlayChannel(channelSaucerFireSound, g_pSaucerFireSound, 0);
									}
								}
							}

							// If the saucer goes off screen, kill it
							if ((actors[i].x < -WORLD_HALFWIDTH) || (actors[i].x >  WORLD_HALFWIDTH))
							{
								
								// Kill the current saucer
								DestroySaucer(i);

								// Setup next saucer delay
								fTimeForNewLSaucer = g_fNow + randFloat(2.5f, 5.0f);

							}

							break;


						case ACTOR_ROCK:

							// Set flag to indicate that at least one
							// rock is still active ;)
							bAllEnemiesInactive = FALSE;

							// Update rock position
							actors[i].x += (float)CosLookup(actors[i].rz) * actors[i].vx * fDeltaTime;
							actors[i].y += (float)SinLookup(actors[i].rz) * actors[i].vy * fDeltaTime;
							actors[i].rx += 100.0f * fDeltaTime;
							actors[i].ry += 100.0f * fDeltaTime;

							if (actors[i].size < 0) 
								actors[i].active = FALSE;
							break;


						case ACTOR_SFIRE:
						case ACTOR_SHOT:
							if (actors[i].LifeEndTime <= g_fNow)
							{
								actors[i].active = FALSE;
							}
							else 
							{
								actors[i].x += (float)CosLookup(actors[i].rz) * actors[i].vx * fDeltaTime;
								actors[i].y += (float)SinLookup(actors[i].rz) * actors[i].vy * fDeltaTime;
								actors[i].rx += 1000.0f * fDeltaTime;

							}
							break;

						
						case ACTOR_SHIELDS:
							if ((player->ShieldsUp) && (player->ShieldsPower > 0)) {
								actors[i].active = TRUE;
								actors[i].x = player->x;
								actors[i].y = player->y;
								actors[i].z = player->z;
								actors[i].color.r = 255 - (int)player->ShieldsPower;
								actors[i].color.g = (int)player->ShieldsPower;
								actors[i].color.b = 0;
								actors[i].rx += 100.0f * fDeltaTime;
								actors[i].rz += 100.0f * fDeltaTime;
								
								if (!g_cvInfiniteShields.value)
									player->ShieldsPower -= 100.0f * fDeltaTime;


							}
							else {
								actors[i].active = FALSE;
							}

							break;

						case ACTOR_BLAST:
							if (actors[i].LifeEndTime <= g_fNow)
							{
								actors[i].active = FALSE;
							}
							break;

						default:
							break;
					}
					

					// Screen wrap 
					//
					// Don't wrap DEBRIS or PARTICLES
					if ((actors[i].type == ACTOR_DEBRIS)  || (actors[i].type == ACTOR_PARTICLE))
					{
						if ((actors[i].x > SCREEN_HALFWIDTH) || 
							(actors[i].x < -SCREEN_HALFWIDTH) ||
							(actors[i].y >  SCREEN_HALFHEIGHT) ||
							(actors[i].y < -SCREEN_HALFHEIGHT))
								actors[i].active = FALSE;
					}
					// Wrap everything else
					else 
					{
						if (actors[i].x > WORLD_HALFWIDTH) 
							actors[i].x -= WORLD_WIDTH;
						else if (actors[i].x < -WORLD_HALFWIDTH)
							actors[i].x += WORLD_WIDTH;
						if (actors[i].y > WORLD_HALFHEIGHT)
							actors[i].y -= WORLD_HEIGHT;
						else if (actors[i].y < -WORLD_HALFHEIGHT)
							actors[i].y += WORLD_HEIGHT;
					}
				}
			} // for (i = 0; i < MAX_ACTORS; i++)


			// Player is center of universe
			if (g_cvCopernicusMode.value)
			{
				
				// Move the background's x and y
				// based on the player's position
				g_fBackgroundX -= player->x;
				g_fBackgroundY -= player->y;
				
				// Update the stars only if 
				// they're visible
				if (g_bStarsActive) 
				{
					for (i = 0; i < NUM_STARS; i++)
					{
						// Move stars
						stars[i].x -= player->x;
						stars[i].y -= player->y;

						// Wrap stars at star's world border
						if (stars[i].x > STAR_WORLD_HALFWIDTH)
							stars[i].x -= STAR_WORLD_WIDTH;
						if (stars[i].x < -STAR_WORLD_HALFWIDTH)
							stars[i].x += STAR_WORLD_WIDTH;
						if (stars[i].y > STAR_WORLD_HALFHEIGHT)
							stars[i].y -= STAR_WORLD_HEIGHT;
						if (stars[i].y < -STAR_WORLD_HALFHEIGHT)
							stars[i].y += STAR_WORLD_HEIGHT;
					}
				}
			
				// Set player at center of screen
				player->x = 0;
				player->y = 0;
			}

			// Check for player collisions between the various
			// objects in the game that can collide
			CollisionDetection(&actors[IDX_SHIELDS], IDX_FIRST_ROCK, IDX_LAST_ROCK);
			CollisionDetection(player, IDX_FIRST_ROCK, IDX_LAST_ROCK);
			CollisionDetection(player, IDX_FIRST_SFIRE, IDX_LAST_SFIRE);
			CollisionDetection(player, IDX_LSAUCER, IDX_LSAUCER);
			CollisionDetection(&actors[IDX_LSAUCER], IDX_FIRST_SHOT, IDX_LAST_SHOT);
			for (i = IDX_FIRST_ROCK; i<= IDX_LAST_ROCK; i++)
			{
				CollisionDetection(&actors[i], IDX_FIRST_SHOT, IDX_LAST_SHOT);
				CollisionDetection(&actors[i], IDX_FIRST_SFIRE, IDX_LAST_SFIRE);
				CollisionDetection(&actors[i], IDX_LSAUCER, IDX_LSAUCER);
			}


			// Respawn player after death
			if ((player->active == FALSE) && (!player->Hyperspacing)) {

				if (fPlayerRespawnTime == 0) {

					// Decrease lives
					if (!g_cvInfiniteLives.value)
						player->Lives--;

					// Delay 2 seconds before respawning player
					fPlayerRespawnTime = g_fNow + 2;

					// Stop thumping
					fNextThumpTime = 2000000000;

					// Stop thruster sound
					if (g_cvSoundEnabled.value)
					{
						Mix_HaltChannel(channelShipThrustSound);
					}
					g_bThrustSoundOn = FALSE;

					// If the player's out of lives,
					// game's over, man
					if (player->Lives <= 0)
					{
						// Set game over flag
						g_bGameOver = TRUE;

						// Stop looping sounds
						KillLoopingSounds();

						// Reset start game flag
						g_bStartGame = FALSE;

						// Check to see if current score is a high score
						for (i = 0; i < MAX_HIGHSCORES; i++)
						{
							if (player->Score > highscores[i].Score)
							{

								// Push all the other high scores
								// down one rank and elim the last
								for (j = MAX_HIGHSCORES - 2; j >= i; j--)
									highscores[j + 1] = highscores[j];

								// Found one!  Initialize variables and exit for loop
								g_bEnteringHighScoreInitials = TRUE;
                                                                
								g_nHighScoreInitialsIdx = 0;

								// Setup delay variables
								g_lHighScoreEntryDelayEndTime = (long)g_fNow + HIGHSCORE_ENTRY_DELAY;

								highscores[i].Score = player->Score;
								highscores[i].Level = g_nLevelNum;
								
								switch(nSpaceSize) {
								case 0:	
									highscores[i].SpaceSize = 'S';
									break;
								case 1:
									highscores[i].SpaceSize = 'M';
									break;
								case 2:
									highscores[i].SpaceSize = 'L';
									break;
								}

								switch(g_nDifficulty) {
								case 0:
									highscores[i].Difficulty = 'E';
									break;
								case 1:
									highscores[i].Difficulty = 'N';
									break;
								case 2:
									highscores[i].Difficulty = 'H';
									break;
								}

								sprintf(highscores[i].Initials, "   ");

								playerhighscore = &highscores[i];
                                
#ifdef IOS // on iOS we can show the keyboard now
                                    SDL_StartTextInput();
#endif
                                
								break;
							}
						}
					}
				}

				if ((fPlayerRespawnTime != 0) && (fPlayerRespawnTime < g_fNow) && (player->Lives > 0)) {

					// This function resets player at center
					// of screen as long as it doesn't spell
					// instant death.
					if (SpawnPlayer())
					{
						// Restart thumpss
						bPlayThump = !bPlayThump;
						fNextThumpTime = 1;

						// Reset Timeforplayerrespawn for next time player dies
						fPlayerRespawnTime = 0;

						// Reset saucer next time
						fTimeForNewLSaucer = 0;
					}					

				}

			}

			// Setup and respawn saucer
			if (!g_bGameOver)
			{

				// Reset timeout if it's zero
				if ((!fTimeForNewLSaucer) && (!actors[IDX_LSAUCER].active))
				{
					fTimeForNewLSaucer = g_fNow + randFloat(2.5f, 5.0f);
				}

				// On easy, the saucer doesn't spawn until level five
				if (!((g_nDifficulty == EASY_DIFFICULTY) && (g_nLevelNum < 5)))
				{
					// If the interval between saucers has passed, then 
					// spawn the saucer
					if ((fTimeForNewLSaucer) && (fTimeForNewLSaucer < g_fNow))
					{
						if (actors[IDX_LSAUCER].active == FALSE) 
						{
							if (CreateSaucer(IDX_LSAUCER))
								fTimeForNewLSaucer = 0;
						}
					}
				}
				else
					fTimeForNewLSaucer = 0;
			}
			

			// Spawn rocks with random positions and velocities
			// 
  			// In the future, this should happen after a delay
			// instead of right away
			if (bAllEnemiesInactive) {

				// When the board's done, set the delay to 
				// repopulate board and increment the board
				// number
				if (fTimeForNewRocks == 0) {
					
					fTimeForNewRocks = g_fNow + 2;

					// Stop thumping between levels
					fNextThumpTime = 2000000000;

					// Stop UFOs between levels
					fTimeForNewLSaucer = 2000000000;

				}
				
				// Check time...
				if ((fTimeForNewRocks != 0) && (fTimeForNewRocks <= g_fNow)) {

					// Increment the level number
					g_nLevelNum++;

					// Reset player's shields on easy difficulty
					if (g_nDifficulty == EASY_DIFFICULTY)
					{
						player->ShieldsPower = 255;
					}
					// Recharge them 20% of the level number on 
					// normal and hard difficulties
					else
					{
						// Recharge shields 20% of level number
						if (g_nLevelNum < 100)
						{
							float fPct = g_nLevelNum * 0.02f;
							i = int(255 * fPct);
							// UpdateMessage("Recharging %f%% == %d", fPct, i);
							if (player->ShieldsPower + i > 255)
							{
								player->ShieldsPower = 255;
							}
							else
							{
								player->ShieldsPower += i;
							}
						}
						else
						{
							player->ShieldsPower = 255;
						}
					}


					// Reset timefornewrocks for next time...
					fTimeForNewRocks = 0;

					// Set lLevelStartTime as g_lNow
					fLevelStartTime = g_fNow;

					// Reset thumping time
					fNextThumpTime = 0;

					// Reset UFO
					fTimeForNewLSaucer = g_fNow + randFloat(2.5f, 5.0f); 

					// Spawn rocks
					
					// Compute number of rocks to use for current level
					// and difficulty
					int ubound = 0;
					if (g_nDifficulty == HARD_DIFFICULTY)
					{
						ubound = IDX_FIRST_ROCK  + ((g_nLevelNum + 2) * 2);
					}
					else
					{
						ubound = IDX_FIRST_ROCK  + g_nLevelNum + 2;
					}
					
					if (ubound > IDX_LAST_ROCK)
					{
						ubound = IDX_LAST_ROCK;
					}

					for (i = IDX_FIRST_ROCK; i < ubound; i++)
					{
						actors[i].active = TRUE;
						actors[i].type = ACTOR_ROCK;

						// This will stick rocks close to the 
						// edges of the screen (in the x direction)
						actors[i].x = randFloat(0.0f, 2.0f + g_nLevelNum) - 1.0f - (g_nLevelNum * 0.5f) - WORLD_HALFWIDTH;
						actors[i].y = randFloat(0.0f, (float)WORLD_HEIGHT);
						actors[i].z = WORLD_DEPTH;
						
						actors[i].rx = randFloat(-360.0f, 360.0f);
						actors[i].ry = randFloat(-360.0f, 360.0f);
						actors[i].rz = randFloat(-360.0f, 360.0f);

						
						// Set rocks' speed based on difficulty
						switch (g_nDifficulty)
						{
						case EASY_DIFFICULTY:
							actors[i].vx = 1.5f; 
							actors[i].vy = 1.5f; 
							actors[i].vz = 1.5f; 
							break;

						case NORMAL_DIFFICULTY:
							actors[i].vx = 1.5f + 0.025f * g_nLevelNum; 
							actors[i].vy = 1.5f + 0.025f * g_nLevelNum; 
							actors[i].vz = 1.5f + 0.025f * g_nLevelNum; 
							break;

						case HARD_DIFFICULTY:
							actors[i].vx = 2.0f + 0.025f * g_nLevelNum;
							actors[i].vy = 2.0f + 0.025f * g_nLevelNum;
							actors[i].vz = 2.0f + 0.025f * g_nLevelNum;
							break;
						}

						// Set color of rock
						actors[i].color.r = randInt(96, 160);
						actors[i].color.g = randInt(96, 160);
						actors[i].color.b = randInt(96, 160);


						// Make sure rock isn't too dark
						AdjustRockColor(i);

						actors[i].displaylistid = rock_list;
						actors[i].size = 2;

						actors[i].xcolldist = 1.1f;
						actors[i].ycolldist = 1.1f;
					}
				}
			} // if (bAllEnemiesInactive) 

			// Update the background rotation (even during pause)
			g_fBackgroundRot += 0.5f * fDeltaTime;

		} // if (!g_bGamePaused)

	} // Message loop end


	
	//------------------------------------------------------
	// Shutdown
	//------------------------------------------------------

	// Save high score records
	SaveHighScores("disasteroids3d.dat");

	// Save Cvars
	Cvar_SaveAll(csConfigFilename);

	// Kill data, sounds, control, window
	TerminateGame();

	// Automatically save the console when running in developer mode
	if (g_cvDeveloper.value)
		con_dump(csConsoleDumpFilename);

	// Kill the console
	con_kill();

	// Exit The Program
	return 0;

}
