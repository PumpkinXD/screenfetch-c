
/*

Copyright 1985, 1986, 1987, 1991, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/

/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SCREENFETCH_C_X11_NATIVE_FUNCTIONS_H
#define SCREENFETCH_C_X11_NATIVE_FUNCTIONS_H

#include <stddef.h>
#include <stdint.h>
#include <uchar.h>


/**
 * x11 and glx
 * 
*/

//x11 typedef, from x.h, xlib.h and Xutil.h
typedef char *XPointer;
typedef unsigned long XID;
typedef XID Colormap;
typedef XID Window;
typedef XID GLXDrawable;
typedef unsigned long VisualID;
typedef struct _XDisplay Display;
typedef struct _XGC *GC;

typedef struct _XExtData {
  int number;             /* number returned by XRegisterExtension */
  struct _XExtData *next; /* next item on list of data for structure */
  int (*free_private)(    /* called to free private storage */
                      struct _XExtData *extension);
  XPointer private_data; /* data private to this extension. */
} XExtData;
typedef struct {
  XExtData *ext_data; /* hook for extension to hang data */
  VisualID visualid;  /* visual id of this visual */
#if defined(__cplusplus) || defined(c_plusplus)
  int c_class; /* C++ class of screen (monochrome, etc.) */
#else
  int class; /* class of screen (monochrome, etc.) */
#endif
  unsigned long red_mask, green_mask, blue_mask; /* mask values */
  int bits_per_rgb;                              /* log base 2 of distinct color values */
  int map_entries;                               /* color map entries */
} Visual;
typedef struct {
  int depth;       /* this depth (Z) of the depth */
  int nvisuals;    /* number of Visual types at this depth */
  Visual *visuals; /* list of visuals possible at this depth */
} Depth;
typedef struct {
  XExtData *ext_data;        /* hook for extension to hang data */
  struct _XDisplay *display; /* back pointer to display structure */
  Window root;               /* Root window id. */
  int width, height;         /* width and height of screen */
  int mwidth, mheight;       /* width and height of  in millimeters */
  int ndepths;               /* number of depths possible */
  Depth *depths;             /* list of allowable depths on the screen */
  int root_depth;            /* bits per pixel */
  Visual *root_visual;       /* root visual */
  GC default_gc;             /* GC for the root root visual */
  Colormap cmap;             /* default color map */
  unsigned long white_pixel;
  unsigned long black_pixel; /* White and Black pixel values */
  int max_maps, min_maps;    /* max and min color maps */
  int backing_store;         /* Never, WhenMapped, Always */
  /*Bool*/int save_unders;
  long root_input_mask; /* initial root input mask */
} Screen;
typedef struct {
  XExtData *ext_data; /* hook for extension to hang data */
  int depth;          /* depth of this image format */
  int bits_per_pixel; /* bits/pixel at this depth */
  int scanline_pad;   /* scanline must padded to this multiple */
} ScreenFormat;
typedef struct {
  Visual *visual;
  VisualID visualid;
  int screen;
  int depth;
#if defined(__cplusplus) || defined(c_plusplus)
  int c_class; /* C++ */
#else
  int class;
#endif
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
  int colormap_size;
  int bits_per_rgb;
} XVisualInfo;

typedef struct
#ifdef XLIB_ILLEGAL_ACCESS
    _XDisplay
#endif
{
  XExtData *ext_data; /* hook for extension to hang data */
  struct _XPrivate *private1;
  int fd; /* Network socket. */
  int private2;
  int proto_major_version; /* major version of server's X protocol */
  int proto_minor_version; /* minor version of servers X protocol */
  char *vendor;            /* vendor of the server hardware */
  XID private3;
  XID private4;
  XID private5;
  int private6;
  XID (*resource_alloc)
  (/* allocator function */
   struct _XDisplay *);
  int byte_order;              /* screen byte order, LSBFirst, MSBFirst */
  int bitmap_unit;             /* padding and data requirements */
  int bitmap_pad;              /* padding requirements on bitmaps */
  int bitmap_bit_order;        /* LeastSignificant or MostSignificant */
  int nformats;                /* number of pixmap formats in list */
  ScreenFormat *pixmap_format; /* pixmap format list */
  int private8;
  int release; /* release of the server */
  struct _XPrivate *private9, *private10;
  int qlen;                        /* Length of input event queue */
  unsigned long last_request_read; /* seq number of last event read */
  unsigned long request;           /* sequence number of last request. */
  XPointer private11;
  XPointer private12;
  XPointer private13;
  XPointer private14;
  unsigned max_request_size; /* maximum number 32 bit words in request*/
  struct _XrmHashBucketRec *db;
  int (*private15)(struct _XDisplay *);
  char *display_name;          /* "host:display" string used on this connect*/
  int default_screen;          /* default screen for operations */
  int nscreens;                /* number of screens on this server*/
  Screen *screens;             /* pointer to list of screens */
  unsigned long motion_buffer; /* size of motion buffer */
  unsigned long private16;
  int min_keycode; /* minimum defined keycode */
  int max_keycode; /* maximum defined keycode */
  XPointer private17;
  XPointer private18;
  int private19;
  char *xdefaults; /* contents of defaults from server */
                   /* there is more to this structure, but it is private to Xlib */
}
#ifdef XLIB_ILLEGAL_ACCESS
Display,
#endif
    *_XPrivDisplay;

//glx typedef, from gl/gl.h and gl/glx.h
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned int GLenum;
typedef struct __GLXcontextRec *GLXContext;


//consts
#define None 0L /* universal null resource or null atom */

#define GLX_RGBA 4
#define GLX_DOUBLEBUFFER 5
#define GLX_DEPTH_SIZE 12
#define GL_RENDERER 0x1F01

//marco "functions"
#define DefaultScreen(dpy) (((_XPrivDisplay)(dpy))->default_screen)
#define ScreenOfDisplay(dpy, scr) (&((_XPrivDisplay)(dpy))->screens[scr])
#define DefaultRootWindow(dpy) (ScreenOfDisplay(dpy, DefaultScreen(dpy))->root)



//x11 and glx functions
typedef Display *(*fnXOpenDisplay)(const char *display_name);
typedef int(*fnXFree)(void* data);
typedef int(*fnXCloseDisplay)(Display* display);

// XVisualInfo* glXChooseVisual( Display *dpy, int screen,int *attribList )
typedef XVisualInfo *(*fnglXChooseVisual)(Display *dpy, int screen, int *attribList);

// GLXContext glXCreateContext( Display *dpy, XVisualInfo *vis,GLXContext shareList, Bool direct );
typedef GLXContext (*fnglXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList,
                                         int direct);
//Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx);
typedef int (*fnglXMakeCurrent)(Display *dpy, GLXDrawable drawable, GLXContext ctx);

// GLAPI const GLubyte * GLAPIENTRY glGetString( GLenum name );
typedef const GLubyte *(*__attribute__((visibility("default"))) fnglGetString)(GLenum name);

//void glXDestroyContext(Display *dpy, GLXContext ctx);
typedef void (*fnglXDestroyContext)(Display *dpy, GLXContext ctx);

#endif // SCREENFETCH_C_X11_NATIVE_FUNCTIONS_H

