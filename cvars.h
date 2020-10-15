#ifndef CVARS_H
#define CVARS_H

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>

#include <SDL.h>

#if defined __APPLE__ || defined __linux__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define TRUE 1
#define FALSE 0
typedef bool BOOL;
typedef float FLOAT;
#endif

// cvar storage struct definition
typedef struct cvar_s {
	char	*name;
	char	*string;
	bool	persistant;
	float	value;
	struct cvar_s*	next;
} cvar_t;

// public functions
cvar_t *Cvar_FindVar(const char *name);
void Cvar_Register(cvar_t *variable);
bool Cvar_SaveAll(const char *Filename);
bool Cvar_SetString(const char *name, const char *newstring);
bool Cvar_SetValue(const char *name, float newvalue);
char *Cvar_GetString(const char *name);
float Cvar_GetValue(const char *name);

#endif
