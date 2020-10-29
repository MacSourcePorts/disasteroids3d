//
//  sys_ios.m
//  disasteroids3d-iOS
//
//  Created by Tom Kidd on 8/26/20.
//  Copyright © 2020 Tom Kidd. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <SDL_syswm.h>
#include "sys_ios.h"
//#include "game.h"

#if TARGET_OS_TV
#import "disasteroids3d_tvOS-Swift.h"
#else
#import "disasteroids3d_iOS-Swift.h"
#endif

UIViewController* GetSDLViewController(SDL_Window *sdlWindow) {
    SDL_SysWMinfo systemWindowInfo;
    SDL_VERSION(&systemWindowInfo.version);
    if ( ! SDL_GetWindowWMInfo(sdlWindow, &systemWindowInfo)) {
        // error handle?
        return nil;
    }
    UIWindow *appWindow = systemWindowInfo.info.uikit.window;
    UIViewController *rootVC = appWindow.rootViewController;
    return rootVC;
}

void Sys_AddControls(SDL_Window *sdlWindow) {
#if !TARGET_OS_TV
    // adding on-screen controls -tkidd
    SDL_uikitviewcontroller *rootVC = (SDL_uikitviewcontroller *)GetSDLViewController(sdlWindow);
    NSLog(@"root VC = %@",rootVC);

    [rootVC.view addSubview:[rootVC fireButtonWithRect:[rootVC.view frame]]];
    [rootVC.view addSubview:[rootVC thrustButtonWithRect:[rootVC.view frame]]];
    [rootVC.view addSubview:[rootVC enterButtonWithRect:[rootVC.view frame]]];
    [rootVC.view addSubview:[rootVC rotateLeftButtonWithRect:[rootVC.view frame]]];
    [rootVC.view addSubview:[rootVC rotateRightButtonWithRect:[rootVC.view frame]]];
    [rootVC.view addSubview:[rootVC shieldsButtonWithRect:[rootVC.view frame]]];
#endif
}

void Sys_ToggleControls(SDL_Window *sdlWindow, BOOL g_bGameOver) {
#if !TARGET_OS_TV
    SDL_uikitviewcontroller *rootVC = (SDL_uikitviewcontroller *)GetSDLViewController(sdlWindow);
    [rootVC toggleControls:g_bGameOver];
#endif
}

const char* Sys_GetSaveGamePath() {
    
#if !TARGET_OS_TV
    const char* path = [[[[NSFileManager defaultManager] URLForDirectory:NSDocumentDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:true error:nil] path] UTF8String];
#else
    const char* path = [[[[NSFileManager defaultManager] URLForDirectory:NSCachesDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:true error:nil] path] UTF8String];
#endif
    return path;
}
