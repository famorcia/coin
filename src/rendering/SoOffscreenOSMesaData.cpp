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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_OSMESA

#include "SoOffscreenOSMesaData.h"
#include "Inventor/C/glue/gl.h"
#include "glue/gl_osmesa.h"

#include <Inventor/errors/SoDebugError.h>

// Pixels-pr-mm.
// TODO: add resolution
SbVec2f
SoOffscreenOSMesaData::getResolution(void)
{
    return SbVec2f(72.0f / 25.4f, 72.0f / 25.4f); // fall back to 72dpi
}

void
SoOffscreenOSMesaData::init(cc_glglue_offscreen_cb_functions* func) {
    func->create_offscreen = osmesaglue_context_create_offscreen;
    func->make_current = osmesaglue_context_make_current;
    func->current_context = osmesaglue_current_context;
    func->reinstate_previous = osmesaglue_context_reinstate_previous;
    func->destruct = osmesaglue_context_destruct;
    cc_glglue_context_set_offscreen_cb_functions(func);
}

void
SoOffscreenOSMesaData::finish() {
    cc_glglue_context_set_offscreen_cb_functions(NULL);
}


#endif // HAVE_OSMESA
