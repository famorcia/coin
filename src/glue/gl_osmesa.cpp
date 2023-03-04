/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include "glue/gl_osmesa.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>

#include <Inventor/C/basic.h>
#include <Inventor/C/glue/dl.h>
#include <Inventor/C/errors/debugerror.h>
#include <Inventor/C/tidbits.h>

#include "glue/dlp.h"
#include "glue/glp.h"

/* ********************************************************************** */
#ifndef  HAVE_OSMESA

/* Dummy versions of the functions, when built without GLX: */

void osmesaglue_init(cc_glglue * w)
{
  w->glx.version.major = -1;
  w->glx.version.minor = 0;
  w->glx.isdirect = FALSE;

  w->glx.serverversion = NULL;
  w->glx.servervendor = NULL;
  w->glx.serverextensions = NULL;
  w->glx.clientversion = NULL;
  w->glx.clientvendor = NULL;
  w->glx.clientextensions = NULL;
  w->glx.glxextensions = NULL;
}

void * osmesaglue_getprocaddress(const cc_glglue * glue, const char * fname) { return NULL; }
int osmesaglue_ext_supported(const cc_glglue * w, const char * extension) { return 0; }

void * osmesaglue_context_create_offscreen(unsigned int width, unsigned int height) { assert(FALSE); return NULL; }
SbBool osmesaglue_context_make_current(void * ctx) { assert(FALSE); return FALSE; }
void osmesaglue_context_reinstate_previous(void * ctx) { assert(FALSE); }
void osmesaglue_context_destruct(void * ctx) { assert(FALSE); }

SbBool osmesaglue_context_pbuffer_max(void * ctx, unsigned int * lims) { assert(FALSE); return FALSE; }

#else /* HAVE_OSMESA */

/* ********************************************************************** */

#include <GL/osmesa.h>

/*
 * This is the cleanup part of the X11/Xmd.h conflict fix hack set up
 * above.  2003-06-25 larsa
 */
#ifdef COIN_DEFINED_BOOL
#undef BOOL
#undef COIN_DEFINED_BOOL
#endif /* COIN_DEFINED_BOOL */
#ifdef COIN_DEFINED_INT32
#undef INT32
#undef COIN_DEFINED_INT32
#endif /* COIN_DEFINED_INT32 */


/* ********************************************************************** */

/* Sanity checks for enum extension value assumed to be equal to the
 * final / "proper" / standard OpenGL enum values. (If not, we could
 * end up with hard-to-find bugs because of mismatches with the
 * compiled values versus the runtime values.)
 *
 * This doesn't really _fix_ anything, it is just meant as an aid to
 * smoke out platforms where we're getting unexpected enum values.
 */

#ifdef GLX_RENDER_TYPE_SGIX
#if GLX_RENDER_TYPE != GLX_RENDER_TYPE_SGIX
#error dangerous enum mismatch
#endif /* cmp */
#endif /* GLX_RENDER_TYPE_SGIX */

#ifdef GLX_DRAWABLE_TYPE_SGIX
#if GLX_DRAWABLE_TYPE != GLX_DRAWABLE_TYPE_SGIX
#error dangerous enum mismatch
#endif /* cmp */
#endif /* GLX_DRAWABLE_TYPE_SGIX */

#ifdef GLX_RGBA_TYPE_SGIX
#if GLX_RGBA_TYPE != GLX_RGBA_TYPE_SGIX
#error dangerous enum mismatch
#endif /* cmp */
#endif /* GLX_RGBA_TYPE_SGIX */

#ifdef GLX_RGBA_BIT_SGIX
#if GLX_RGBA_BIT != GLX_RGBA_BIT_SGIX
#error dangerous enum mismatch
#endif /* cmp */
#endif /* GLX_RGBA_BIT_SGIX */

#ifdef GLX_PBUFFER_BIT_SGIX
#if GLX_PBUFFER_BIT != GLX_PBUFFER_BIT_SGIX
#error dangerous enum mismatch
#endif /* cmp */
#endif /* GLX_PBUFFER_BIT_SGIX */

/* ********************************************************************** */

struct osmesaglue_contextdata {
    OSMesaContext ctx;
    void *buffer;
    int width;
    int height;

    OSMesaContext storedcontext;
    void *storedcontext_buffer;
    int storedcontext_type;
    int storedcontext_width;
    int storedcontext_height;


    osmesaglue_contextdata(int w, int h)
            :ctx(0)
            ,buffer(0)
            ,width(w)
            ,height(h)
            ,storedcontext(0){
    }

    ~osmesaglue_contextdata() {
        delete [] (GLubyte*)(buffer);
        if(ctx) {
            OSMesaDestroyContext(ctx);
        }
    }

};

#include <iostream>

#define PRINT_ME() \
if (coin_glglue_debug()) {\
cc_debugerror_postinfo("osmesaglue", "%s",__PRETTY_FUNCTION__); }\
do {} while (0)

void *
osmesaglue_getprocaddress(const cc_glglue * glue_in, const char * fname)
{
    PRINT_ME();
    return 0;
}

int
osmesaglue_ext_supported(const cc_glglue * w, const char * extension)
{
    PRINT_ME();
    return 0;
}

void
osmesaglue_init(cc_glglue * w)
{
    PRINT_ME();
}


/* ********************************************************************** */

/* Create and return a handle to an offscreen OpenGL buffer.

   Where p-buffer support is available that will be used instead of a
   standard offscreen GLX context, as it should render much faster
   (due to hardware acceleration).

   See: http://www.oss.sgi.com/projects/ogl-sample/registry/SGIX/pbuffer.txt

   The initial pbuffer implementation was contributed by Tamer Fahmy
   and Hannes Kaufmann.
*/
void *
osmesaglue_context_create_offscreen(unsigned int width, unsigned int height)
{
    osmesaglue_contextdata* osmesa_ctx = new osmesaglue_contextdata(width, height);

    // osmesa_ctx->ctx = OSMesaCreateContextExt( OSMESA_RGBA, 16, 0, 0, NULL );
    osmesa_ctx->ctx = OSMesaCreateContext(OSMESA_RGBA, NULL );
    if(!osmesa_ctx->ctx) {
        if (coin_glglue_debug()) {
            cc_debugerror_postinfo("osmesaglue_context_create_offscreen",
                                   "can not make current context");
        }

        delete osmesa_ctx;
        return 0;
    }
    if (coin_glglue_debug()) {
        cc_debugerror_postinfo("osmesaglue_context_create_offscreen",
                               "current ctx:%p",
                               osmesa_ctx->ctx);
    }

    /* Allocate the image buffer */
    osmesa_ctx->buffer = new GLubyte [ width * height * 4 ];
    return (osmesa_ctx);
}

void*
osmesaglue_current_context() {
    OSMesaContext ctx=OSMesaGetCurrentContext();
    if (coin_glglue_debug()) {
        cc_debugerror_postinfo("osmesaglue_current_context",
                               "current ctx:%p",
                               ctx);
    }
    return (ctx);
};

SbBool
osmesaglue_context_make_current(void * ctx)
{
    osmesaglue_contextdata* osmesa_ctx = static_cast<osmesaglue_contextdata*>(ctx);
    if (coin_glglue_debug()) {
        cc_debugerror_postinfo("osmesaglue_context_make_current",
                               "current ctx:%p",
                               ctx);
    }

    SbBool result = TRUE;
    if(!osmesa_ctx) {
        result = FALSE;
        if (coin_glglue_debug()) {
            cc_debugerror_postinfo("osmesaglue_context_make_current",
                                   "not osmesa context!");
        }
    } else {
        OSMesaContext current_context =  OSMesaGetCurrentContext();
        if (coin_glglue_debug()) {
            cc_debugerror_postinfo("osmesaglue_context_make_current",
                                   "current_context ctx:%p",
                                   current_context);
        }
#if 0
        if((current_context != 0) && (current_context != osmesa_ctx->storedcontext)) {
            OSMesaGetColorBuffer(osmesa_ctx->storedcontext,
                                 &osmesa_ctx->storedcontext_width,
                                 &osmesa_ctx->storedcontext_height,
                                 &osmesa_ctx->storedcontext_type,
                                 &osmesa_ctx->storedcontext_buffer);
        }
#endif
        /* Bind the buffer to the context and make it current */
        if (!OSMesaMakeCurrent(osmesa_ctx->ctx,
                               osmesa_ctx->buffer,
                               GL_UNSIGNED_BYTE,
                               osmesa_ctx->width,
                               osmesa_ctx->height)) {
            result = FALSE;
            if (coin_glglue_debug()) {
                cc_debugerror_postinfo("osmesaglue_context_make_current",
                                       "can not make current context %p",
                                       osmesa_ctx->ctx);
            }
        }
    }
    return (result);
}

void
osmesaglue_context_destruct(void * ctx) {

    osmesaglue_contextdata * context = (struct osmesaglue_contextdata *)ctx;
    if (coin_glglue_debug()) {
        cc_debugerror_postinfo("osmesaglue_context_destruct",
                               "destroy context %p", context->ctx);
    }
    delete context;
    context = 0;
}

void osmesaglue_context_reinstate_previous(void * ctx) {
    osmesaglue_contextdata * context = (struct osmesaglue_contextdata *)ctx;

    if (context->storedcontext) {
        if (coin_glglue_debug()) {
            cc_debugerror_postinfo("osmesaglue_context_reinstate_previous",
                                   "restoring context %p to be current "
                                   "(drawable==%p)",
                                   context->storedcontext);
        }
        /*
        OSMesaMakeCurrent(context->storedcontext,
                          context->storedcontext_buffer,
                          context->storedcontext_type,
                          context->storedcontext_width,
                          context->storedcontext_height);
                          */
    }
}

/* ********************************************************************** */

void osmesaglue_cleanup(void)
{
    PRINT_ME();
}

// Not p-buffer with osmesa
SbBool
osmesaglue_context_pbuffer_max(void * , unsigned int *) {
    return FALSE;
}

#endif /* HAVE_GLX */
