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

        [rootVC.view addSubview:[rootVC actionButtonWithRect:[rootVC.view frame]]];
        [rootVC.view addSubview:[rootVC joyStickWithRect:[rootVC.view frame]]];
        [rootVC.view addSubview:[rootVC enterButtonWithRect:[rootVC.view frame]]];
        [rootVC.view addSubview:[rootVC escapeButtonWithRect:[rootVC.view frame]]];
    #endif
}
