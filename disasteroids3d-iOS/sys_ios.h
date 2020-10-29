//
//  sys_ios.h
//  disasteroids3d
//
//  Created by Tom Kidd on 8/26/20.
//  Copyright © 2020 Tom Kidd. All rights reserved.
//

#ifndef sys_ios_h
#define sys_ios_h
#   include <SDL_video.h>

void Sys_AddControls(SDL_Window *sdlWindow);
void Sys_ToggleControls(SDL_Window *sdlWindow, BOOL g_bGameOver);
const char* Sys_GetSaveGamePath();

#endif /* sys_ios_h */
