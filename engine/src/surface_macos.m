#include "ungrund.h"
#include <webgpu/webgpu.h>

#if defined(__APPLE__)

#import <Foundation/Foundation.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

void* ug_create_metal_layer(void* ns_window_ptr) {
    NSWindow* ns_window = (__bridge NSWindow*)ns_window_ptr;
    
    // Set the content view to use a layer
    [ns_window.contentView setWantsLayer:YES];
    
    // Create a Metal layer
    CAMetalLayer* metal_layer = [CAMetalLayer layer];
    
    // Set the layer on the content view
    [ns_window.contentView setLayer:metal_layer];
    
    // Return the metal layer as a void pointer
    return (__bridge void*)metal_layer;
}

#endif

