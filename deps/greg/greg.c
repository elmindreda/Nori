/* GREG - an OpenGL extension loader generator
 * Copyright © Camilla Berglund <dreda@dreda.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 */

#include <GREG/greg.h>

#include <string.h>
#include <stdio.h>

#if defined(_GREG_USE_EGL)
  #include <EGL/egl.h>
#elif defined(_GREG_USE_GLFW3)
  #include <GLFW/glfw3.h>
#elif defined(_GREG_USE_SDL2)
  #include <SDL/SDL.h>
#elif defined(_WIN32)
  #include <windows.h>
#elif defined(__linux__)
  #include <GL/glx.h>
#elif defined(__APPLE__)
  #include <CoreFoundation/CoreFoundation.h>
  #include <OpenGL/OpenGL.h>
#endif

typedef void (*GREGglproc)(void);

static struct
{
  int major;
  int minor;

#if defined(_GREG_USE_EGL)
#elif defined(_GREG_USE_GLFW3)
#elif defined(_GREG_USE_SDL2)
#elif defined(_WIN32)
  struct
  {
    HINSTANCE instance;
  } wgl;
#elif defined(__APPLE__)
  struct
  {
    void* framework;
  } nsgl;
#endif

} _greg;

int GREG_VERSION_1_0;
int GREG_VERSION_1_1;
int GREG_VERSION_1_2;
int GREG_VERSION_1_3;
int GREG_VERSION_1_4;
int GREG_VERSION_1_5;
int GREG_VERSION_2_0;
int GREG_VERSION_2_1;
int GREG_VERSION_3_0;
int GREG_VERSION_3_1;
int GREG_VERSION_3_2;


int GREG_ARB_texture_float;
int GREG_EXT_texture_filter_anisotropic;
int GREG_KHR_debug;


PFNGLACCUMPROC greg_glAccum;
PFNGLACTIVETEXTUREPROC greg_glActiveTexture;
PFNGLALPHAFUNCPROC greg_glAlphaFunc;
PFNGLARETEXTURESRESIDENTPROC greg_glAreTexturesResident;
PFNGLARRAYELEMENTPROC greg_glArrayElement;
PFNGLATTACHSHADERPROC greg_glAttachShader;
PFNGLBEGINPROC greg_glBegin;
PFNGLBEGINCONDITIONALRENDERPROC greg_glBeginConditionalRender;
PFNGLBEGINQUERYPROC greg_glBeginQuery;
PFNGLBEGINTRANSFORMFEEDBACKPROC greg_glBeginTransformFeedback;
PFNGLBINDATTRIBLOCATIONPROC greg_glBindAttribLocation;
PFNGLBINDBUFFERPROC greg_glBindBuffer;
PFNGLBINDBUFFERBASEPROC greg_glBindBufferBase;
PFNGLBINDBUFFERRANGEPROC greg_glBindBufferRange;
PFNGLBINDFRAGDATALOCATIONPROC greg_glBindFragDataLocation;
PFNGLBINDFRAMEBUFFERPROC greg_glBindFramebuffer;
PFNGLBINDRENDERBUFFERPROC greg_glBindRenderbuffer;
PFNGLBINDTEXTUREPROC greg_glBindTexture;
PFNGLBINDVERTEXARRAYPROC greg_glBindVertexArray;
PFNGLBITMAPPROC greg_glBitmap;
PFNGLBLENDCOLORPROC greg_glBlendColor;
PFNGLBLENDEQUATIONPROC greg_glBlendEquation;
PFNGLBLENDEQUATIONSEPARATEPROC greg_glBlendEquationSeparate;
PFNGLBLENDFUNCPROC greg_glBlendFunc;
PFNGLBLENDFUNCSEPARATEPROC greg_glBlendFuncSeparate;
PFNGLBLITFRAMEBUFFERPROC greg_glBlitFramebuffer;
PFNGLBUFFERDATAPROC greg_glBufferData;
PFNGLBUFFERSUBDATAPROC greg_glBufferSubData;
PFNGLCALLLISTPROC greg_glCallList;
PFNGLCALLLISTSPROC greg_glCallLists;
PFNGLCHECKFRAMEBUFFERSTATUSPROC greg_glCheckFramebufferStatus;
PFNGLCLAMPCOLORPROC greg_glClampColor;
PFNGLCLEARPROC greg_glClear;
PFNGLCLEARACCUMPROC greg_glClearAccum;
PFNGLCLEARBUFFERFIPROC greg_glClearBufferfi;
PFNGLCLEARBUFFERFVPROC greg_glClearBufferfv;
PFNGLCLEARBUFFERIVPROC greg_glClearBufferiv;
PFNGLCLEARBUFFERUIVPROC greg_glClearBufferuiv;
PFNGLCLEARCOLORPROC greg_glClearColor;
PFNGLCLEARDEPTHPROC greg_glClearDepth;
PFNGLCLEARINDEXPROC greg_glClearIndex;
PFNGLCLEARSTENCILPROC greg_glClearStencil;
PFNGLCLIENTACTIVETEXTUREPROC greg_glClientActiveTexture;
PFNGLCLIENTWAITSYNCPROC greg_glClientWaitSync;
PFNGLCLIPPLANEPROC greg_glClipPlane;
PFNGLCOLOR3BPROC greg_glColor3b;
PFNGLCOLOR3BVPROC greg_glColor3bv;
PFNGLCOLOR3DPROC greg_glColor3d;
PFNGLCOLOR3DVPROC greg_glColor3dv;
PFNGLCOLOR3FPROC greg_glColor3f;
PFNGLCOLOR3FVPROC greg_glColor3fv;
PFNGLCOLOR3IPROC greg_glColor3i;
PFNGLCOLOR3IVPROC greg_glColor3iv;
PFNGLCOLOR3SPROC greg_glColor3s;
PFNGLCOLOR3SVPROC greg_glColor3sv;
PFNGLCOLOR3UBPROC greg_glColor3ub;
PFNGLCOLOR3UBVPROC greg_glColor3ubv;
PFNGLCOLOR3UIPROC greg_glColor3ui;
PFNGLCOLOR3UIVPROC greg_glColor3uiv;
PFNGLCOLOR3USPROC greg_glColor3us;
PFNGLCOLOR3USVPROC greg_glColor3usv;
PFNGLCOLOR4BPROC greg_glColor4b;
PFNGLCOLOR4BVPROC greg_glColor4bv;
PFNGLCOLOR4DPROC greg_glColor4d;
PFNGLCOLOR4DVPROC greg_glColor4dv;
PFNGLCOLOR4FPROC greg_glColor4f;
PFNGLCOLOR4FVPROC greg_glColor4fv;
PFNGLCOLOR4IPROC greg_glColor4i;
PFNGLCOLOR4IVPROC greg_glColor4iv;
PFNGLCOLOR4SPROC greg_glColor4s;
PFNGLCOLOR4SVPROC greg_glColor4sv;
PFNGLCOLOR4UBPROC greg_glColor4ub;
PFNGLCOLOR4UBVPROC greg_glColor4ubv;
PFNGLCOLOR4UIPROC greg_glColor4ui;
PFNGLCOLOR4UIVPROC greg_glColor4uiv;
PFNGLCOLOR4USPROC greg_glColor4us;
PFNGLCOLOR4USVPROC greg_glColor4usv;
PFNGLCOLORMASKPROC greg_glColorMask;
PFNGLCOLORMASKIPROC greg_glColorMaski;
PFNGLCOLORMATERIALPROC greg_glColorMaterial;
PFNGLCOLORPOINTERPROC greg_glColorPointer;
PFNGLCOMPILESHADERPROC greg_glCompileShader;
PFNGLCOMPRESSEDTEXIMAGE1DPROC greg_glCompressedTexImage1D;
PFNGLCOMPRESSEDTEXIMAGE2DPROC greg_glCompressedTexImage2D;
PFNGLCOMPRESSEDTEXIMAGE3DPROC greg_glCompressedTexImage3D;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC greg_glCompressedTexSubImage1D;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC greg_glCompressedTexSubImage2D;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC greg_glCompressedTexSubImage3D;
PFNGLCOPYBUFFERSUBDATAPROC greg_glCopyBufferSubData;
PFNGLCOPYPIXELSPROC greg_glCopyPixels;
PFNGLCOPYTEXIMAGE1DPROC greg_glCopyTexImage1D;
PFNGLCOPYTEXIMAGE2DPROC greg_glCopyTexImage2D;
PFNGLCOPYTEXSUBIMAGE1DPROC greg_glCopyTexSubImage1D;
PFNGLCOPYTEXSUBIMAGE2DPROC greg_glCopyTexSubImage2D;
PFNGLCOPYTEXSUBIMAGE3DPROC greg_glCopyTexSubImage3D;
PFNGLCREATEPROGRAMPROC greg_glCreateProgram;
PFNGLCREATESHADERPROC greg_glCreateShader;
PFNGLCULLFACEPROC greg_glCullFace;
PFNGLDEBUGMESSAGECALLBACKPROC greg_glDebugMessageCallback;
PFNGLDEBUGMESSAGECALLBACKKHRPROC greg_glDebugMessageCallbackKHR;
PFNGLDEBUGMESSAGECONTROLPROC greg_glDebugMessageControl;
PFNGLDEBUGMESSAGECONTROLKHRPROC greg_glDebugMessageControlKHR;
PFNGLDEBUGMESSAGEINSERTPROC greg_glDebugMessageInsert;
PFNGLDEBUGMESSAGEINSERTKHRPROC greg_glDebugMessageInsertKHR;
PFNGLDELETEBUFFERSPROC greg_glDeleteBuffers;
PFNGLDELETEFRAMEBUFFERSPROC greg_glDeleteFramebuffers;
PFNGLDELETELISTSPROC greg_glDeleteLists;
PFNGLDELETEPROGRAMPROC greg_glDeleteProgram;
PFNGLDELETEQUERIESPROC greg_glDeleteQueries;
PFNGLDELETERENDERBUFFERSPROC greg_glDeleteRenderbuffers;
PFNGLDELETESHADERPROC greg_glDeleteShader;
PFNGLDELETESYNCPROC greg_glDeleteSync;
PFNGLDELETETEXTURESPROC greg_glDeleteTextures;
PFNGLDELETEVERTEXARRAYSPROC greg_glDeleteVertexArrays;
PFNGLDEPTHFUNCPROC greg_glDepthFunc;
PFNGLDEPTHMASKPROC greg_glDepthMask;
PFNGLDEPTHRANGEPROC greg_glDepthRange;
PFNGLDETACHSHADERPROC greg_glDetachShader;
PFNGLDISABLEPROC greg_glDisable;
PFNGLDISABLECLIENTSTATEPROC greg_glDisableClientState;
PFNGLDISABLEVERTEXATTRIBARRAYPROC greg_glDisableVertexAttribArray;
PFNGLDISABLEIPROC greg_glDisablei;
PFNGLDRAWARRAYSPROC greg_glDrawArrays;
PFNGLDRAWARRAYSINSTANCEDPROC greg_glDrawArraysInstanced;
PFNGLDRAWBUFFERPROC greg_glDrawBuffer;
PFNGLDRAWBUFFERSPROC greg_glDrawBuffers;
PFNGLDRAWELEMENTSPROC greg_glDrawElements;
PFNGLDRAWELEMENTSBASEVERTEXPROC greg_glDrawElementsBaseVertex;
PFNGLDRAWELEMENTSINSTANCEDPROC greg_glDrawElementsInstanced;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC greg_glDrawElementsInstancedBaseVertex;
PFNGLDRAWPIXELSPROC greg_glDrawPixels;
PFNGLDRAWRANGEELEMENTSPROC greg_glDrawRangeElements;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC greg_glDrawRangeElementsBaseVertex;
PFNGLEDGEFLAGPROC greg_glEdgeFlag;
PFNGLEDGEFLAGPOINTERPROC greg_glEdgeFlagPointer;
PFNGLEDGEFLAGVPROC greg_glEdgeFlagv;
PFNGLENABLEPROC greg_glEnable;
PFNGLENABLECLIENTSTATEPROC greg_glEnableClientState;
PFNGLENABLEVERTEXATTRIBARRAYPROC greg_glEnableVertexAttribArray;
PFNGLENABLEIPROC greg_glEnablei;
PFNGLENDPROC greg_glEnd;
PFNGLENDCONDITIONALRENDERPROC greg_glEndConditionalRender;
PFNGLENDLISTPROC greg_glEndList;
PFNGLENDQUERYPROC greg_glEndQuery;
PFNGLENDTRANSFORMFEEDBACKPROC greg_glEndTransformFeedback;
PFNGLEVALCOORD1DPROC greg_glEvalCoord1d;
PFNGLEVALCOORD1DVPROC greg_glEvalCoord1dv;
PFNGLEVALCOORD1FPROC greg_glEvalCoord1f;
PFNGLEVALCOORD1FVPROC greg_glEvalCoord1fv;
PFNGLEVALCOORD2DPROC greg_glEvalCoord2d;
PFNGLEVALCOORD2DVPROC greg_glEvalCoord2dv;
PFNGLEVALCOORD2FPROC greg_glEvalCoord2f;
PFNGLEVALCOORD2FVPROC greg_glEvalCoord2fv;
PFNGLEVALMESH1PROC greg_glEvalMesh1;
PFNGLEVALMESH2PROC greg_glEvalMesh2;
PFNGLEVALPOINT1PROC greg_glEvalPoint1;
PFNGLEVALPOINT2PROC greg_glEvalPoint2;
PFNGLFEEDBACKBUFFERPROC greg_glFeedbackBuffer;
PFNGLFENCESYNCPROC greg_glFenceSync;
PFNGLFINISHPROC greg_glFinish;
PFNGLFLUSHPROC greg_glFlush;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC greg_glFlushMappedBufferRange;
PFNGLFOGCOORDPOINTERPROC greg_glFogCoordPointer;
PFNGLFOGCOORDDPROC greg_glFogCoordd;
PFNGLFOGCOORDDVPROC greg_glFogCoorddv;
PFNGLFOGCOORDFPROC greg_glFogCoordf;
PFNGLFOGCOORDFVPROC greg_glFogCoordfv;
PFNGLFOGFPROC greg_glFogf;
PFNGLFOGFVPROC greg_glFogfv;
PFNGLFOGIPROC greg_glFogi;
PFNGLFOGIVPROC greg_glFogiv;
PFNGLFRAMEBUFFERRENDERBUFFERPROC greg_glFramebufferRenderbuffer;
PFNGLFRAMEBUFFERTEXTUREPROC greg_glFramebufferTexture;
PFNGLFRAMEBUFFERTEXTURE1DPROC greg_glFramebufferTexture1D;
PFNGLFRAMEBUFFERTEXTURE2DPROC greg_glFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURE3DPROC greg_glFramebufferTexture3D;
PFNGLFRAMEBUFFERTEXTURELAYERPROC greg_glFramebufferTextureLayer;
PFNGLFRONTFACEPROC greg_glFrontFace;
PFNGLFRUSTUMPROC greg_glFrustum;
PFNGLGENBUFFERSPROC greg_glGenBuffers;
PFNGLGENFRAMEBUFFERSPROC greg_glGenFramebuffers;
PFNGLGENLISTSPROC greg_glGenLists;
PFNGLGENQUERIESPROC greg_glGenQueries;
PFNGLGENRENDERBUFFERSPROC greg_glGenRenderbuffers;
PFNGLGENTEXTURESPROC greg_glGenTextures;
PFNGLGENVERTEXARRAYSPROC greg_glGenVertexArrays;
PFNGLGENERATEMIPMAPPROC greg_glGenerateMipmap;
PFNGLGETACTIVEATTRIBPROC greg_glGetActiveAttrib;
PFNGLGETACTIVEUNIFORMPROC greg_glGetActiveUniform;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC greg_glGetActiveUniformBlockName;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC greg_glGetActiveUniformBlockiv;
PFNGLGETACTIVEUNIFORMNAMEPROC greg_glGetActiveUniformName;
PFNGLGETACTIVEUNIFORMSIVPROC greg_glGetActiveUniformsiv;
PFNGLGETATTACHEDSHADERSPROC greg_glGetAttachedShaders;
PFNGLGETATTRIBLOCATIONPROC greg_glGetAttribLocation;
PFNGLGETBOOLEANI_VPROC greg_glGetBooleani_v;
PFNGLGETBOOLEANVPROC greg_glGetBooleanv;
PFNGLGETBUFFERPARAMETERI64VPROC greg_glGetBufferParameteri64v;
PFNGLGETBUFFERPARAMETERIVPROC greg_glGetBufferParameteriv;
PFNGLGETBUFFERPOINTERVPROC greg_glGetBufferPointerv;
PFNGLGETBUFFERSUBDATAPROC greg_glGetBufferSubData;
PFNGLGETCLIPPLANEPROC greg_glGetClipPlane;
PFNGLGETCOMPRESSEDTEXIMAGEPROC greg_glGetCompressedTexImage;
PFNGLGETDEBUGMESSAGELOGPROC greg_glGetDebugMessageLog;
PFNGLGETDEBUGMESSAGELOGKHRPROC greg_glGetDebugMessageLogKHR;
PFNGLGETDOUBLEVPROC greg_glGetDoublev;
PFNGLGETERRORPROC greg_glGetError;
PFNGLGETFLOATVPROC greg_glGetFloatv;
PFNGLGETFRAGDATALOCATIONPROC greg_glGetFragDataLocation;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC greg_glGetFramebufferAttachmentParameteriv;
PFNGLGETINTEGER64I_VPROC greg_glGetInteger64i_v;
PFNGLGETINTEGER64VPROC greg_glGetInteger64v;
PFNGLGETINTEGERI_VPROC greg_glGetIntegeri_v;
PFNGLGETINTEGERVPROC greg_glGetIntegerv;
PFNGLGETLIGHTFVPROC greg_glGetLightfv;
PFNGLGETLIGHTIVPROC greg_glGetLightiv;
PFNGLGETMAPDVPROC greg_glGetMapdv;
PFNGLGETMAPFVPROC greg_glGetMapfv;
PFNGLGETMAPIVPROC greg_glGetMapiv;
PFNGLGETMATERIALFVPROC greg_glGetMaterialfv;
PFNGLGETMATERIALIVPROC greg_glGetMaterialiv;
PFNGLGETMULTISAMPLEFVPROC greg_glGetMultisamplefv;
PFNGLGETOBJECTLABELPROC greg_glGetObjectLabel;
PFNGLGETOBJECTLABELKHRPROC greg_glGetObjectLabelKHR;
PFNGLGETOBJECTPTRLABELPROC greg_glGetObjectPtrLabel;
PFNGLGETOBJECTPTRLABELKHRPROC greg_glGetObjectPtrLabelKHR;
PFNGLGETPIXELMAPFVPROC greg_glGetPixelMapfv;
PFNGLGETPIXELMAPUIVPROC greg_glGetPixelMapuiv;
PFNGLGETPIXELMAPUSVPROC greg_glGetPixelMapusv;
PFNGLGETPOINTERVPROC greg_glGetPointerv;
PFNGLGETPOINTERVKHRPROC greg_glGetPointervKHR;
PFNGLGETPOLYGONSTIPPLEPROC greg_glGetPolygonStipple;
PFNGLGETPROGRAMINFOLOGPROC greg_glGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC greg_glGetProgramiv;
PFNGLGETQUERYOBJECTIVPROC greg_glGetQueryObjectiv;
PFNGLGETQUERYOBJECTUIVPROC greg_glGetQueryObjectuiv;
PFNGLGETQUERYIVPROC greg_glGetQueryiv;
PFNGLGETRENDERBUFFERPARAMETERIVPROC greg_glGetRenderbufferParameteriv;
PFNGLGETSHADERINFOLOGPROC greg_glGetShaderInfoLog;
PFNGLGETSHADERSOURCEPROC greg_glGetShaderSource;
PFNGLGETSHADERIVPROC greg_glGetShaderiv;
PFNGLGETSTRINGPROC greg_glGetString;
PFNGLGETSTRINGIPROC greg_glGetStringi;
PFNGLGETSYNCIVPROC greg_glGetSynciv;
PFNGLGETTEXENVFVPROC greg_glGetTexEnvfv;
PFNGLGETTEXENVIVPROC greg_glGetTexEnviv;
PFNGLGETTEXGENDVPROC greg_glGetTexGendv;
PFNGLGETTEXGENFVPROC greg_glGetTexGenfv;
PFNGLGETTEXGENIVPROC greg_glGetTexGeniv;
PFNGLGETTEXIMAGEPROC greg_glGetTexImage;
PFNGLGETTEXLEVELPARAMETERFVPROC greg_glGetTexLevelParameterfv;
PFNGLGETTEXLEVELPARAMETERIVPROC greg_glGetTexLevelParameteriv;
PFNGLGETTEXPARAMETERIIVPROC greg_glGetTexParameterIiv;
PFNGLGETTEXPARAMETERIUIVPROC greg_glGetTexParameterIuiv;
PFNGLGETTEXPARAMETERFVPROC greg_glGetTexParameterfv;
PFNGLGETTEXPARAMETERIVPROC greg_glGetTexParameteriv;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC greg_glGetTransformFeedbackVarying;
PFNGLGETUNIFORMBLOCKINDEXPROC greg_glGetUniformBlockIndex;
PFNGLGETUNIFORMINDICESPROC greg_glGetUniformIndices;
PFNGLGETUNIFORMLOCATIONPROC greg_glGetUniformLocation;
PFNGLGETUNIFORMFVPROC greg_glGetUniformfv;
PFNGLGETUNIFORMIVPROC greg_glGetUniformiv;
PFNGLGETUNIFORMUIVPROC greg_glGetUniformuiv;
PFNGLGETVERTEXATTRIBIIVPROC greg_glGetVertexAttribIiv;
PFNGLGETVERTEXATTRIBIUIVPROC greg_glGetVertexAttribIuiv;
PFNGLGETVERTEXATTRIBPOINTERVPROC greg_glGetVertexAttribPointerv;
PFNGLGETVERTEXATTRIBDVPROC greg_glGetVertexAttribdv;
PFNGLGETVERTEXATTRIBFVPROC greg_glGetVertexAttribfv;
PFNGLGETVERTEXATTRIBIVPROC greg_glGetVertexAttribiv;
PFNGLHINTPROC greg_glHint;
PFNGLINDEXMASKPROC greg_glIndexMask;
PFNGLINDEXPOINTERPROC greg_glIndexPointer;
PFNGLINDEXDPROC greg_glIndexd;
PFNGLINDEXDVPROC greg_glIndexdv;
PFNGLINDEXFPROC greg_glIndexf;
PFNGLINDEXFVPROC greg_glIndexfv;
PFNGLINDEXIPROC greg_glIndexi;
PFNGLINDEXIVPROC greg_glIndexiv;
PFNGLINDEXSPROC greg_glIndexs;
PFNGLINDEXSVPROC greg_glIndexsv;
PFNGLINDEXUBPROC greg_glIndexub;
PFNGLINDEXUBVPROC greg_glIndexubv;
PFNGLINITNAMESPROC greg_glInitNames;
PFNGLINTERLEAVEDARRAYSPROC greg_glInterleavedArrays;
PFNGLISBUFFERPROC greg_glIsBuffer;
PFNGLISENABLEDPROC greg_glIsEnabled;
PFNGLISENABLEDIPROC greg_glIsEnabledi;
PFNGLISFRAMEBUFFERPROC greg_glIsFramebuffer;
PFNGLISLISTPROC greg_glIsList;
PFNGLISPROGRAMPROC greg_glIsProgram;
PFNGLISQUERYPROC greg_glIsQuery;
PFNGLISRENDERBUFFERPROC greg_glIsRenderbuffer;
PFNGLISSHADERPROC greg_glIsShader;
PFNGLISSYNCPROC greg_glIsSync;
PFNGLISTEXTUREPROC greg_glIsTexture;
PFNGLISVERTEXARRAYPROC greg_glIsVertexArray;
PFNGLLIGHTMODELFPROC greg_glLightModelf;
PFNGLLIGHTMODELFVPROC greg_glLightModelfv;
PFNGLLIGHTMODELIPROC greg_glLightModeli;
PFNGLLIGHTMODELIVPROC greg_glLightModeliv;
PFNGLLIGHTFPROC greg_glLightf;
PFNGLLIGHTFVPROC greg_glLightfv;
PFNGLLIGHTIPROC greg_glLighti;
PFNGLLIGHTIVPROC greg_glLightiv;
PFNGLLINESTIPPLEPROC greg_glLineStipple;
PFNGLLINEWIDTHPROC greg_glLineWidth;
PFNGLLINKPROGRAMPROC greg_glLinkProgram;
PFNGLLISTBASEPROC greg_glListBase;
PFNGLLOADIDENTITYPROC greg_glLoadIdentity;
PFNGLLOADMATRIXDPROC greg_glLoadMatrixd;
PFNGLLOADMATRIXFPROC greg_glLoadMatrixf;
PFNGLLOADNAMEPROC greg_glLoadName;
PFNGLLOADTRANSPOSEMATRIXDPROC greg_glLoadTransposeMatrixd;
PFNGLLOADTRANSPOSEMATRIXFPROC greg_glLoadTransposeMatrixf;
PFNGLLOGICOPPROC greg_glLogicOp;
PFNGLMAP1DPROC greg_glMap1d;
PFNGLMAP1FPROC greg_glMap1f;
PFNGLMAP2DPROC greg_glMap2d;
PFNGLMAP2FPROC greg_glMap2f;
PFNGLMAPBUFFERPROC greg_glMapBuffer;
PFNGLMAPBUFFERRANGEPROC greg_glMapBufferRange;
PFNGLMAPGRID1DPROC greg_glMapGrid1d;
PFNGLMAPGRID1FPROC greg_glMapGrid1f;
PFNGLMAPGRID2DPROC greg_glMapGrid2d;
PFNGLMAPGRID2FPROC greg_glMapGrid2f;
PFNGLMATERIALFPROC greg_glMaterialf;
PFNGLMATERIALFVPROC greg_glMaterialfv;
PFNGLMATERIALIPROC greg_glMateriali;
PFNGLMATERIALIVPROC greg_glMaterialiv;
PFNGLMATRIXMODEPROC greg_glMatrixMode;
PFNGLMULTMATRIXDPROC greg_glMultMatrixd;
PFNGLMULTMATRIXFPROC greg_glMultMatrixf;
PFNGLMULTTRANSPOSEMATRIXDPROC greg_glMultTransposeMatrixd;
PFNGLMULTTRANSPOSEMATRIXFPROC greg_glMultTransposeMatrixf;
PFNGLMULTIDRAWARRAYSPROC greg_glMultiDrawArrays;
PFNGLMULTIDRAWELEMENTSPROC greg_glMultiDrawElements;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC greg_glMultiDrawElementsBaseVertex;
PFNGLMULTITEXCOORD1DPROC greg_glMultiTexCoord1d;
PFNGLMULTITEXCOORD1DVPROC greg_glMultiTexCoord1dv;
PFNGLMULTITEXCOORD1FPROC greg_glMultiTexCoord1f;
PFNGLMULTITEXCOORD1FVPROC greg_glMultiTexCoord1fv;
PFNGLMULTITEXCOORD1IPROC greg_glMultiTexCoord1i;
PFNGLMULTITEXCOORD1IVPROC greg_glMultiTexCoord1iv;
PFNGLMULTITEXCOORD1SPROC greg_glMultiTexCoord1s;
PFNGLMULTITEXCOORD1SVPROC greg_glMultiTexCoord1sv;
PFNGLMULTITEXCOORD2DPROC greg_glMultiTexCoord2d;
PFNGLMULTITEXCOORD2DVPROC greg_glMultiTexCoord2dv;
PFNGLMULTITEXCOORD2FPROC greg_glMultiTexCoord2f;
PFNGLMULTITEXCOORD2FVPROC greg_glMultiTexCoord2fv;
PFNGLMULTITEXCOORD2IPROC greg_glMultiTexCoord2i;
PFNGLMULTITEXCOORD2IVPROC greg_glMultiTexCoord2iv;
PFNGLMULTITEXCOORD2SPROC greg_glMultiTexCoord2s;
PFNGLMULTITEXCOORD2SVPROC greg_glMultiTexCoord2sv;
PFNGLMULTITEXCOORD3DPROC greg_glMultiTexCoord3d;
PFNGLMULTITEXCOORD3DVPROC greg_glMultiTexCoord3dv;
PFNGLMULTITEXCOORD3FPROC greg_glMultiTexCoord3f;
PFNGLMULTITEXCOORD3FVPROC greg_glMultiTexCoord3fv;
PFNGLMULTITEXCOORD3IPROC greg_glMultiTexCoord3i;
PFNGLMULTITEXCOORD3IVPROC greg_glMultiTexCoord3iv;
PFNGLMULTITEXCOORD3SPROC greg_glMultiTexCoord3s;
PFNGLMULTITEXCOORD3SVPROC greg_glMultiTexCoord3sv;
PFNGLMULTITEXCOORD4DPROC greg_glMultiTexCoord4d;
PFNGLMULTITEXCOORD4DVPROC greg_glMultiTexCoord4dv;
PFNGLMULTITEXCOORD4FPROC greg_glMultiTexCoord4f;
PFNGLMULTITEXCOORD4FVPROC greg_glMultiTexCoord4fv;
PFNGLMULTITEXCOORD4IPROC greg_glMultiTexCoord4i;
PFNGLMULTITEXCOORD4IVPROC greg_glMultiTexCoord4iv;
PFNGLMULTITEXCOORD4SPROC greg_glMultiTexCoord4s;
PFNGLMULTITEXCOORD4SVPROC greg_glMultiTexCoord4sv;
PFNGLNEWLISTPROC greg_glNewList;
PFNGLNORMAL3BPROC greg_glNormal3b;
PFNGLNORMAL3BVPROC greg_glNormal3bv;
PFNGLNORMAL3DPROC greg_glNormal3d;
PFNGLNORMAL3DVPROC greg_glNormal3dv;
PFNGLNORMAL3FPROC greg_glNormal3f;
PFNGLNORMAL3FVPROC greg_glNormal3fv;
PFNGLNORMAL3IPROC greg_glNormal3i;
PFNGLNORMAL3IVPROC greg_glNormal3iv;
PFNGLNORMAL3SPROC greg_glNormal3s;
PFNGLNORMAL3SVPROC greg_glNormal3sv;
PFNGLNORMALPOINTERPROC greg_glNormalPointer;
PFNGLOBJECTLABELPROC greg_glObjectLabel;
PFNGLOBJECTLABELKHRPROC greg_glObjectLabelKHR;
PFNGLOBJECTPTRLABELPROC greg_glObjectPtrLabel;
PFNGLOBJECTPTRLABELKHRPROC greg_glObjectPtrLabelKHR;
PFNGLORTHOPROC greg_glOrtho;
PFNGLPASSTHROUGHPROC greg_glPassThrough;
PFNGLPIXELMAPFVPROC greg_glPixelMapfv;
PFNGLPIXELMAPUIVPROC greg_glPixelMapuiv;
PFNGLPIXELMAPUSVPROC greg_glPixelMapusv;
PFNGLPIXELSTOREFPROC greg_glPixelStoref;
PFNGLPIXELSTOREIPROC greg_glPixelStorei;
PFNGLPIXELTRANSFERFPROC greg_glPixelTransferf;
PFNGLPIXELTRANSFERIPROC greg_glPixelTransferi;
PFNGLPIXELZOOMPROC greg_glPixelZoom;
PFNGLPOINTPARAMETERFPROC greg_glPointParameterf;
PFNGLPOINTPARAMETERFVPROC greg_glPointParameterfv;
PFNGLPOINTPARAMETERIPROC greg_glPointParameteri;
PFNGLPOINTPARAMETERIVPROC greg_glPointParameteriv;
PFNGLPOINTSIZEPROC greg_glPointSize;
PFNGLPOLYGONMODEPROC greg_glPolygonMode;
PFNGLPOLYGONOFFSETPROC greg_glPolygonOffset;
PFNGLPOLYGONSTIPPLEPROC greg_glPolygonStipple;
PFNGLPOPATTRIBPROC greg_glPopAttrib;
PFNGLPOPCLIENTATTRIBPROC greg_glPopClientAttrib;
PFNGLPOPDEBUGGROUPPROC greg_glPopDebugGroup;
PFNGLPOPDEBUGGROUPKHRPROC greg_glPopDebugGroupKHR;
PFNGLPOPMATRIXPROC greg_glPopMatrix;
PFNGLPOPNAMEPROC greg_glPopName;
PFNGLPRIMITIVERESTARTINDEXPROC greg_glPrimitiveRestartIndex;
PFNGLPRIORITIZETEXTURESPROC greg_glPrioritizeTextures;
PFNGLPROVOKINGVERTEXPROC greg_glProvokingVertex;
PFNGLPUSHATTRIBPROC greg_glPushAttrib;
PFNGLPUSHCLIENTATTRIBPROC greg_glPushClientAttrib;
PFNGLPUSHDEBUGGROUPPROC greg_glPushDebugGroup;
PFNGLPUSHDEBUGGROUPKHRPROC greg_glPushDebugGroupKHR;
PFNGLPUSHMATRIXPROC greg_glPushMatrix;
PFNGLPUSHNAMEPROC greg_glPushName;
PFNGLRASTERPOS2DPROC greg_glRasterPos2d;
PFNGLRASTERPOS2DVPROC greg_glRasterPos2dv;
PFNGLRASTERPOS2FPROC greg_glRasterPos2f;
PFNGLRASTERPOS2FVPROC greg_glRasterPos2fv;
PFNGLRASTERPOS2IPROC greg_glRasterPos2i;
PFNGLRASTERPOS2IVPROC greg_glRasterPos2iv;
PFNGLRASTERPOS2SPROC greg_glRasterPos2s;
PFNGLRASTERPOS2SVPROC greg_glRasterPos2sv;
PFNGLRASTERPOS3DPROC greg_glRasterPos3d;
PFNGLRASTERPOS3DVPROC greg_glRasterPos3dv;
PFNGLRASTERPOS3FPROC greg_glRasterPos3f;
PFNGLRASTERPOS3FVPROC greg_glRasterPos3fv;
PFNGLRASTERPOS3IPROC greg_glRasterPos3i;
PFNGLRASTERPOS3IVPROC greg_glRasterPos3iv;
PFNGLRASTERPOS3SPROC greg_glRasterPos3s;
PFNGLRASTERPOS3SVPROC greg_glRasterPos3sv;
PFNGLRASTERPOS4DPROC greg_glRasterPos4d;
PFNGLRASTERPOS4DVPROC greg_glRasterPos4dv;
PFNGLRASTERPOS4FPROC greg_glRasterPos4f;
PFNGLRASTERPOS4FVPROC greg_glRasterPos4fv;
PFNGLRASTERPOS4IPROC greg_glRasterPos4i;
PFNGLRASTERPOS4IVPROC greg_glRasterPos4iv;
PFNGLRASTERPOS4SPROC greg_glRasterPos4s;
PFNGLRASTERPOS4SVPROC greg_glRasterPos4sv;
PFNGLREADBUFFERPROC greg_glReadBuffer;
PFNGLREADPIXELSPROC greg_glReadPixels;
PFNGLRECTDPROC greg_glRectd;
PFNGLRECTDVPROC greg_glRectdv;
PFNGLRECTFPROC greg_glRectf;
PFNGLRECTFVPROC greg_glRectfv;
PFNGLRECTIPROC greg_glRecti;
PFNGLRECTIVPROC greg_glRectiv;
PFNGLRECTSPROC greg_glRects;
PFNGLRECTSVPROC greg_glRectsv;
PFNGLRENDERMODEPROC greg_glRenderMode;
PFNGLRENDERBUFFERSTORAGEPROC greg_glRenderbufferStorage;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC greg_glRenderbufferStorageMultisample;
PFNGLROTATEDPROC greg_glRotated;
PFNGLROTATEFPROC greg_glRotatef;
PFNGLSAMPLECOVERAGEPROC greg_glSampleCoverage;
PFNGLSAMPLEMASKIPROC greg_glSampleMaski;
PFNGLSCALEDPROC greg_glScaled;
PFNGLSCALEFPROC greg_glScalef;
PFNGLSCISSORPROC greg_glScissor;
PFNGLSECONDARYCOLOR3BPROC greg_glSecondaryColor3b;
PFNGLSECONDARYCOLOR3BVPROC greg_glSecondaryColor3bv;
PFNGLSECONDARYCOLOR3DPROC greg_glSecondaryColor3d;
PFNGLSECONDARYCOLOR3DVPROC greg_glSecondaryColor3dv;
PFNGLSECONDARYCOLOR3FPROC greg_glSecondaryColor3f;
PFNGLSECONDARYCOLOR3FVPROC greg_glSecondaryColor3fv;
PFNGLSECONDARYCOLOR3IPROC greg_glSecondaryColor3i;
PFNGLSECONDARYCOLOR3IVPROC greg_glSecondaryColor3iv;
PFNGLSECONDARYCOLOR3SPROC greg_glSecondaryColor3s;
PFNGLSECONDARYCOLOR3SVPROC greg_glSecondaryColor3sv;
PFNGLSECONDARYCOLOR3UBPROC greg_glSecondaryColor3ub;
PFNGLSECONDARYCOLOR3UBVPROC greg_glSecondaryColor3ubv;
PFNGLSECONDARYCOLOR3UIPROC greg_glSecondaryColor3ui;
PFNGLSECONDARYCOLOR3UIVPROC greg_glSecondaryColor3uiv;
PFNGLSECONDARYCOLOR3USPROC greg_glSecondaryColor3us;
PFNGLSECONDARYCOLOR3USVPROC greg_glSecondaryColor3usv;
PFNGLSECONDARYCOLORPOINTERPROC greg_glSecondaryColorPointer;
PFNGLSELECTBUFFERPROC greg_glSelectBuffer;
PFNGLSHADEMODELPROC greg_glShadeModel;
PFNGLSHADERSOURCEPROC greg_glShaderSource;
PFNGLSTENCILFUNCPROC greg_glStencilFunc;
PFNGLSTENCILFUNCSEPARATEPROC greg_glStencilFuncSeparate;
PFNGLSTENCILMASKPROC greg_glStencilMask;
PFNGLSTENCILMASKSEPARATEPROC greg_glStencilMaskSeparate;
PFNGLSTENCILOPPROC greg_glStencilOp;
PFNGLSTENCILOPSEPARATEPROC greg_glStencilOpSeparate;
PFNGLTEXBUFFERPROC greg_glTexBuffer;
PFNGLTEXCOORD1DPROC greg_glTexCoord1d;
PFNGLTEXCOORD1DVPROC greg_glTexCoord1dv;
PFNGLTEXCOORD1FPROC greg_glTexCoord1f;
PFNGLTEXCOORD1FVPROC greg_glTexCoord1fv;
PFNGLTEXCOORD1IPROC greg_glTexCoord1i;
PFNGLTEXCOORD1IVPROC greg_glTexCoord1iv;
PFNGLTEXCOORD1SPROC greg_glTexCoord1s;
PFNGLTEXCOORD1SVPROC greg_glTexCoord1sv;
PFNGLTEXCOORD2DPROC greg_glTexCoord2d;
PFNGLTEXCOORD2DVPROC greg_glTexCoord2dv;
PFNGLTEXCOORD2FPROC greg_glTexCoord2f;
PFNGLTEXCOORD2FVPROC greg_glTexCoord2fv;
PFNGLTEXCOORD2IPROC greg_glTexCoord2i;
PFNGLTEXCOORD2IVPROC greg_glTexCoord2iv;
PFNGLTEXCOORD2SPROC greg_glTexCoord2s;
PFNGLTEXCOORD2SVPROC greg_glTexCoord2sv;
PFNGLTEXCOORD3DPROC greg_glTexCoord3d;
PFNGLTEXCOORD3DVPROC greg_glTexCoord3dv;
PFNGLTEXCOORD3FPROC greg_glTexCoord3f;
PFNGLTEXCOORD3FVPROC greg_glTexCoord3fv;
PFNGLTEXCOORD3IPROC greg_glTexCoord3i;
PFNGLTEXCOORD3IVPROC greg_glTexCoord3iv;
PFNGLTEXCOORD3SPROC greg_glTexCoord3s;
PFNGLTEXCOORD3SVPROC greg_glTexCoord3sv;
PFNGLTEXCOORD4DPROC greg_glTexCoord4d;
PFNGLTEXCOORD4DVPROC greg_glTexCoord4dv;
PFNGLTEXCOORD4FPROC greg_glTexCoord4f;
PFNGLTEXCOORD4FVPROC greg_glTexCoord4fv;
PFNGLTEXCOORD4IPROC greg_glTexCoord4i;
PFNGLTEXCOORD4IVPROC greg_glTexCoord4iv;
PFNGLTEXCOORD4SPROC greg_glTexCoord4s;
PFNGLTEXCOORD4SVPROC greg_glTexCoord4sv;
PFNGLTEXCOORDPOINTERPROC greg_glTexCoordPointer;
PFNGLTEXENVFPROC greg_glTexEnvf;
PFNGLTEXENVFVPROC greg_glTexEnvfv;
PFNGLTEXENVIPROC greg_glTexEnvi;
PFNGLTEXENVIVPROC greg_glTexEnviv;
PFNGLTEXGENDPROC greg_glTexGend;
PFNGLTEXGENDVPROC greg_glTexGendv;
PFNGLTEXGENFPROC greg_glTexGenf;
PFNGLTEXGENFVPROC greg_glTexGenfv;
PFNGLTEXGENIPROC greg_glTexGeni;
PFNGLTEXGENIVPROC greg_glTexGeniv;
PFNGLTEXIMAGE1DPROC greg_glTexImage1D;
PFNGLTEXIMAGE2DPROC greg_glTexImage2D;
PFNGLTEXIMAGE2DMULTISAMPLEPROC greg_glTexImage2DMultisample;
PFNGLTEXIMAGE3DPROC greg_glTexImage3D;
PFNGLTEXIMAGE3DMULTISAMPLEPROC greg_glTexImage3DMultisample;
PFNGLTEXPARAMETERIIVPROC greg_glTexParameterIiv;
PFNGLTEXPARAMETERIUIVPROC greg_glTexParameterIuiv;
PFNGLTEXPARAMETERFPROC greg_glTexParameterf;
PFNGLTEXPARAMETERFVPROC greg_glTexParameterfv;
PFNGLTEXPARAMETERIPROC greg_glTexParameteri;
PFNGLTEXPARAMETERIVPROC greg_glTexParameteriv;
PFNGLTEXSUBIMAGE1DPROC greg_glTexSubImage1D;
PFNGLTEXSUBIMAGE2DPROC greg_glTexSubImage2D;
PFNGLTEXSUBIMAGE3DPROC greg_glTexSubImage3D;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC greg_glTransformFeedbackVaryings;
PFNGLTRANSLATEDPROC greg_glTranslated;
PFNGLTRANSLATEFPROC greg_glTranslatef;
PFNGLUNIFORM1FPROC greg_glUniform1f;
PFNGLUNIFORM1FVPROC greg_glUniform1fv;
PFNGLUNIFORM1IPROC greg_glUniform1i;
PFNGLUNIFORM1IVPROC greg_glUniform1iv;
PFNGLUNIFORM1UIPROC greg_glUniform1ui;
PFNGLUNIFORM1UIVPROC greg_glUniform1uiv;
PFNGLUNIFORM2FPROC greg_glUniform2f;
PFNGLUNIFORM2FVPROC greg_glUniform2fv;
PFNGLUNIFORM2IPROC greg_glUniform2i;
PFNGLUNIFORM2IVPROC greg_glUniform2iv;
PFNGLUNIFORM2UIPROC greg_glUniform2ui;
PFNGLUNIFORM2UIVPROC greg_glUniform2uiv;
PFNGLUNIFORM3FPROC greg_glUniform3f;
PFNGLUNIFORM3FVPROC greg_glUniform3fv;
PFNGLUNIFORM3IPROC greg_glUniform3i;
PFNGLUNIFORM3IVPROC greg_glUniform3iv;
PFNGLUNIFORM3UIPROC greg_glUniform3ui;
PFNGLUNIFORM3UIVPROC greg_glUniform3uiv;
PFNGLUNIFORM4FPROC greg_glUniform4f;
PFNGLUNIFORM4FVPROC greg_glUniform4fv;
PFNGLUNIFORM4IPROC greg_glUniform4i;
PFNGLUNIFORM4IVPROC greg_glUniform4iv;
PFNGLUNIFORM4UIPROC greg_glUniform4ui;
PFNGLUNIFORM4UIVPROC greg_glUniform4uiv;
PFNGLUNIFORMBLOCKBINDINGPROC greg_glUniformBlockBinding;
PFNGLUNIFORMMATRIX2FVPROC greg_glUniformMatrix2fv;
PFNGLUNIFORMMATRIX2X3FVPROC greg_glUniformMatrix2x3fv;
PFNGLUNIFORMMATRIX2X4FVPROC greg_glUniformMatrix2x4fv;
PFNGLUNIFORMMATRIX3FVPROC greg_glUniformMatrix3fv;
PFNGLUNIFORMMATRIX3X2FVPROC greg_glUniformMatrix3x2fv;
PFNGLUNIFORMMATRIX3X4FVPROC greg_glUniformMatrix3x4fv;
PFNGLUNIFORMMATRIX4FVPROC greg_glUniformMatrix4fv;
PFNGLUNIFORMMATRIX4X2FVPROC greg_glUniformMatrix4x2fv;
PFNGLUNIFORMMATRIX4X3FVPROC greg_glUniformMatrix4x3fv;
PFNGLUNMAPBUFFERPROC greg_glUnmapBuffer;
PFNGLUSEPROGRAMPROC greg_glUseProgram;
PFNGLVALIDATEPROGRAMPROC greg_glValidateProgram;
PFNGLVERTEX2DPROC greg_glVertex2d;
PFNGLVERTEX2DVPROC greg_glVertex2dv;
PFNGLVERTEX2FPROC greg_glVertex2f;
PFNGLVERTEX2FVPROC greg_glVertex2fv;
PFNGLVERTEX2IPROC greg_glVertex2i;
PFNGLVERTEX2IVPROC greg_glVertex2iv;
PFNGLVERTEX2SPROC greg_glVertex2s;
PFNGLVERTEX2SVPROC greg_glVertex2sv;
PFNGLVERTEX3DPROC greg_glVertex3d;
PFNGLVERTEX3DVPROC greg_glVertex3dv;
PFNGLVERTEX3FPROC greg_glVertex3f;
PFNGLVERTEX3FVPROC greg_glVertex3fv;
PFNGLVERTEX3IPROC greg_glVertex3i;
PFNGLVERTEX3IVPROC greg_glVertex3iv;
PFNGLVERTEX3SPROC greg_glVertex3s;
PFNGLVERTEX3SVPROC greg_glVertex3sv;
PFNGLVERTEX4DPROC greg_glVertex4d;
PFNGLVERTEX4DVPROC greg_glVertex4dv;
PFNGLVERTEX4FPROC greg_glVertex4f;
PFNGLVERTEX4FVPROC greg_glVertex4fv;
PFNGLVERTEX4IPROC greg_glVertex4i;
PFNGLVERTEX4IVPROC greg_glVertex4iv;
PFNGLVERTEX4SPROC greg_glVertex4s;
PFNGLVERTEX4SVPROC greg_glVertex4sv;
PFNGLVERTEXATTRIB1DPROC greg_glVertexAttrib1d;
PFNGLVERTEXATTRIB1DVPROC greg_glVertexAttrib1dv;
PFNGLVERTEXATTRIB1FPROC greg_glVertexAttrib1f;
PFNGLVERTEXATTRIB1FVPROC greg_glVertexAttrib1fv;
PFNGLVERTEXATTRIB1SPROC greg_glVertexAttrib1s;
PFNGLVERTEXATTRIB1SVPROC greg_glVertexAttrib1sv;
PFNGLVERTEXATTRIB2DPROC greg_glVertexAttrib2d;
PFNGLVERTEXATTRIB2DVPROC greg_glVertexAttrib2dv;
PFNGLVERTEXATTRIB2FPROC greg_glVertexAttrib2f;
PFNGLVERTEXATTRIB2FVPROC greg_glVertexAttrib2fv;
PFNGLVERTEXATTRIB2SPROC greg_glVertexAttrib2s;
PFNGLVERTEXATTRIB2SVPROC greg_glVertexAttrib2sv;
PFNGLVERTEXATTRIB3DPROC greg_glVertexAttrib3d;
PFNGLVERTEXATTRIB3DVPROC greg_glVertexAttrib3dv;
PFNGLVERTEXATTRIB3FPROC greg_glVertexAttrib3f;
PFNGLVERTEXATTRIB3FVPROC greg_glVertexAttrib3fv;
PFNGLVERTEXATTRIB3SPROC greg_glVertexAttrib3s;
PFNGLVERTEXATTRIB3SVPROC greg_glVertexAttrib3sv;
PFNGLVERTEXATTRIB4NBVPROC greg_glVertexAttrib4Nbv;
PFNGLVERTEXATTRIB4NIVPROC greg_glVertexAttrib4Niv;
PFNGLVERTEXATTRIB4NSVPROC greg_glVertexAttrib4Nsv;
PFNGLVERTEXATTRIB4NUBPROC greg_glVertexAttrib4Nub;
PFNGLVERTEXATTRIB4NUBVPROC greg_glVertexAttrib4Nubv;
PFNGLVERTEXATTRIB4NUIVPROC greg_glVertexAttrib4Nuiv;
PFNGLVERTEXATTRIB4NUSVPROC greg_glVertexAttrib4Nusv;
PFNGLVERTEXATTRIB4BVPROC greg_glVertexAttrib4bv;
PFNGLVERTEXATTRIB4DPROC greg_glVertexAttrib4d;
PFNGLVERTEXATTRIB4DVPROC greg_glVertexAttrib4dv;
PFNGLVERTEXATTRIB4FPROC greg_glVertexAttrib4f;
PFNGLVERTEXATTRIB4FVPROC greg_glVertexAttrib4fv;
PFNGLVERTEXATTRIB4IVPROC greg_glVertexAttrib4iv;
PFNGLVERTEXATTRIB4SPROC greg_glVertexAttrib4s;
PFNGLVERTEXATTRIB4SVPROC greg_glVertexAttrib4sv;
PFNGLVERTEXATTRIB4UBVPROC greg_glVertexAttrib4ubv;
PFNGLVERTEXATTRIB4UIVPROC greg_glVertexAttrib4uiv;
PFNGLVERTEXATTRIB4USVPROC greg_glVertexAttrib4usv;
PFNGLVERTEXATTRIBI1IPROC greg_glVertexAttribI1i;
PFNGLVERTEXATTRIBI1IVPROC greg_glVertexAttribI1iv;
PFNGLVERTEXATTRIBI1UIPROC greg_glVertexAttribI1ui;
PFNGLVERTEXATTRIBI1UIVPROC greg_glVertexAttribI1uiv;
PFNGLVERTEXATTRIBI2IPROC greg_glVertexAttribI2i;
PFNGLVERTEXATTRIBI2IVPROC greg_glVertexAttribI2iv;
PFNGLVERTEXATTRIBI2UIPROC greg_glVertexAttribI2ui;
PFNGLVERTEXATTRIBI2UIVPROC greg_glVertexAttribI2uiv;
PFNGLVERTEXATTRIBI3IPROC greg_glVertexAttribI3i;
PFNGLVERTEXATTRIBI3IVPROC greg_glVertexAttribI3iv;
PFNGLVERTEXATTRIBI3UIPROC greg_glVertexAttribI3ui;
PFNGLVERTEXATTRIBI3UIVPROC greg_glVertexAttribI3uiv;
PFNGLVERTEXATTRIBI4BVPROC greg_glVertexAttribI4bv;
PFNGLVERTEXATTRIBI4IPROC greg_glVertexAttribI4i;
PFNGLVERTEXATTRIBI4IVPROC greg_glVertexAttribI4iv;
PFNGLVERTEXATTRIBI4SVPROC greg_glVertexAttribI4sv;
PFNGLVERTEXATTRIBI4UBVPROC greg_glVertexAttribI4ubv;
PFNGLVERTEXATTRIBI4UIPROC greg_glVertexAttribI4ui;
PFNGLVERTEXATTRIBI4UIVPROC greg_glVertexAttribI4uiv;
PFNGLVERTEXATTRIBI4USVPROC greg_glVertexAttribI4usv;
PFNGLVERTEXATTRIBIPOINTERPROC greg_glVertexAttribIPointer;
PFNGLVERTEXATTRIBPOINTERPROC greg_glVertexAttribPointer;
PFNGLVERTEXPOINTERPROC greg_glVertexPointer;
PFNGLVIEWPORTPROC greg_glViewport;
PFNGLWAITSYNCPROC greg_glWaitSync;
PFNGLWINDOWPOS2DPROC greg_glWindowPos2d;
PFNGLWINDOWPOS2DVPROC greg_glWindowPos2dv;
PFNGLWINDOWPOS2FPROC greg_glWindowPos2f;
PFNGLWINDOWPOS2FVPROC greg_glWindowPos2fv;
PFNGLWINDOWPOS2IPROC greg_glWindowPos2i;
PFNGLWINDOWPOS2IVPROC greg_glWindowPos2iv;
PFNGLWINDOWPOS2SPROC greg_glWindowPos2s;
PFNGLWINDOWPOS2SVPROC greg_glWindowPos2sv;
PFNGLWINDOWPOS3DPROC greg_glWindowPos3d;
PFNGLWINDOWPOS3DVPROC greg_glWindowPos3dv;
PFNGLWINDOWPOS3FPROC greg_glWindowPos3f;
PFNGLWINDOWPOS3FVPROC greg_glWindowPos3fv;
PFNGLWINDOWPOS3IPROC greg_glWindowPos3i;
PFNGLWINDOWPOS3IVPROC greg_glWindowPos3iv;
PFNGLWINDOWPOS3SPROC greg_glWindowPos3s;
PFNGLWINDOWPOS3SVPROC greg_glWindowPos3sv;


static GLboolean gregHasContext(void)
{
#if defined(_GREG_USE_EGL)
  return eglGetCurrentContext() != EGL_NO_CONTEXT;
#elif defined(_GREG_USE_GLFW3)
  return glfwGetCurrentContext() != NULL;
#elif defined(_GREG_USE_SDL2)
  return SDL_GL_GetCurrentContext() != NULL;
#elif defined(_WIN32)
  return wglGetCurrentContext() != NULL;
#elif defined(__linux__)
  return glXGetCurrentContext() != NULL;
#elif defined(__APPLE__)
  return CGLGetCurrentContext() != NULL;
#endif
}

static GLboolean gregLoadLibrary(void)
{
#if defined(_GREG_USE_EGL)
#elif defined(_GREG_USE_GLFW3)
#elif defined(_GREG_USE_SDL2)
  if (SDL_GL_LoadLibrary(NULL) != 0)
    return GL_FALSE;
#elif defined(_WIN32)
  _greg.wgl.instance = LoadLibraryA("opengl32.dll");
  if (!_greg.wgl.instance)
    return GL_FALSE;
#elif defined(__linux__)
#elif defined(__APPLE__)
  _greg.nsgl.framework = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
  if (!_greg.nsgl.framework)
    return GL_FALSE;
#endif

  return GL_TRUE;
}

static void gregFreeLibrary(void)
{
#if defined(_GREG_USE_EGL)
#elif defined(_GREG_USE_GLFW3)
#elif defined(_GREG_USE_SDL2)
  SDL_GL_UnloadLibrary();
#elif defined(_WIN32)
  if (_greg.wgl.instance)
    FreeLibrary(_greg.wgl.instance);
#elif defined(__linux__)
#elif defined(__APPLE__)
  if (_greg.nsgl.framework)
    CFRelease(_greg.nsgl.framework);
#endif
}

static GREGglproc gregGetProcAddress(const char* name)
{
  GREGglproc proc;

#if defined(_GREG_USE_EGL)
  proc = (GREGglproc) eglGetProcAddress(name);
#elif defined(_GREG_USE_GLFW3)
  proc = (GREGglproc) glfwGetProcAddress(name);
#elif defined(_GREG_USE_SDL2)
  proc = (GREGglproc) SDL_GL_GetProcAddress(name);
#elif defined(_WIN32)
  proc = (GREGglproc) wglGetProcAddress(name);
  if (!proc)
    proc = (GREGglproc) GetProcAddress(_greg.wgl.instance, name);
#elif defined(__linux__)
  proc = (GREGglproc) glXGetProcAddress((const GLubyte*) name);
#elif defined(__APPLE__)
  CFStringRef native = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingASCII);
  proc = (GREGglproc) CFBundleGetFunctionPointerForName(_greg.nsgl.framework, native);
  CFRelease(native);
#endif

  return proc;
}

static GLboolean gregStringInExtensionString(const char* string,
                                             const char* extensions)
{
  const char* start = extensions;

  for (;;)
  {
    const char* end;
    const char* where = strstr(start, string);
    if (!where)
      return GL_FALSE;

    end = where + strlen(string);
    if (where == start || *(where - 1) == ' ')
    {
      if (*end == ' ' || *end == '\0')
        return GL_TRUE;
    }

    start = end;
  }
}

static GLboolean gregValidContext(void)
{
  int i;
  const char* version;
  const char* prefixes[] =
  {
    "OpenGL ES-CM ",
    "OpenGL ES-CL ",
    "OpenGL ES ",
    NULL
  };

  if (!glGetString)
    return GL_FALSE;

  version = (const char*) glGetString(GL_VERSION);
  if (!version)
    return GL_FALSE;

  for (i = 0;  prefixes[i];  i++)
  {
    const size_t length = strlen(prefixes[i]);
    if (strncmp(version, prefixes[i], length) == 0)
    {
      version += length;
      break;
    }
  }

  if (!sscanf(version, "%d.%d", &_greg.major, &_greg.minor))
    return GL_FALSE;

  return GL_TRUE;
}

static GLboolean gregVersionSupported(int major, int minor)
{
  return major > _greg.major || (major == _greg.major && minor >= _greg.minor);
}

static GLboolean gregExtensionSupported(const char* name)
{
  const char* e;

#if defined(GL_VERSION_3_0)
  if (_greg.major >= 3)
  {
    GLint i, count;

    if (!glGetIntegerv || !glGetStringi)
      return GL_FALSE;

    glGetIntegerv(GL_NUM_EXTENSIONS, &count);

    for (i = 0;  i < count;  i++)
    {
      e = (const char*) glGetStringi(GL_EXTENSIONS, i);
      if (!e)
        return GL_FALSE;

      if (strcmp(e, name) == 0)
        return GL_TRUE;
    }

    return GL_FALSE;
  }
#endif /*GL_VERSION_3_0*/

  if (!glGetString)
    return GL_FALSE;

  e = (const char*) glGetString(GL_EXTENSIONS);
  if (!e)
    return GL_FALSE;

  return gregStringInExtensionString(name, e);
}

int gregInit(void)
{
  memset(&_greg, 0, sizeof(_greg));

  if (!gregHasContext() || !gregLoadLibrary())
  {
    gregFreeLibrary();
    return GL_FALSE;
  }

  greg_glAccum = (PFNGLACCUMPROC) gregGetProcAddress("glAccum");
  greg_glActiveTexture = (PFNGLACTIVETEXTUREPROC) gregGetProcAddress("glActiveTexture");
  greg_glAlphaFunc = (PFNGLALPHAFUNCPROC) gregGetProcAddress("glAlphaFunc");
  greg_glAreTexturesResident = (PFNGLARETEXTURESRESIDENTPROC) gregGetProcAddress("glAreTexturesResident");
  greg_glArrayElement = (PFNGLARRAYELEMENTPROC) gregGetProcAddress("glArrayElement");
  greg_glAttachShader = (PFNGLATTACHSHADERPROC) gregGetProcAddress("glAttachShader");
  greg_glBegin = (PFNGLBEGINPROC) gregGetProcAddress("glBegin");
  greg_glBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDERPROC) gregGetProcAddress("glBeginConditionalRender");
  greg_glBeginQuery = (PFNGLBEGINQUERYPROC) gregGetProcAddress("glBeginQuery");
  greg_glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC) gregGetProcAddress("glBeginTransformFeedback");
  greg_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) gregGetProcAddress("glBindAttribLocation");
  greg_glBindBuffer = (PFNGLBINDBUFFERPROC) gregGetProcAddress("glBindBuffer");
  greg_glBindBufferBase = (PFNGLBINDBUFFERBASEPROC) gregGetProcAddress("glBindBufferBase");
  greg_glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC) gregGetProcAddress("glBindBufferRange");
  greg_glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) gregGetProcAddress("glBindFragDataLocation");
  greg_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) gregGetProcAddress("glBindFramebuffer");
  greg_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) gregGetProcAddress("glBindRenderbuffer");
  greg_glBindTexture = (PFNGLBINDTEXTUREPROC) gregGetProcAddress("glBindTexture");
  greg_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) gregGetProcAddress("glBindVertexArray");
  greg_glBitmap = (PFNGLBITMAPPROC) gregGetProcAddress("glBitmap");
  greg_glBlendColor = (PFNGLBLENDCOLORPROC) gregGetProcAddress("glBlendColor");
  greg_glBlendEquation = (PFNGLBLENDEQUATIONPROC) gregGetProcAddress("glBlendEquation");
  greg_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) gregGetProcAddress("glBlendEquationSeparate");
  greg_glBlendFunc = (PFNGLBLENDFUNCPROC) gregGetProcAddress("glBlendFunc");
  greg_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) gregGetProcAddress("glBlendFuncSeparate");
  greg_glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) gregGetProcAddress("glBlitFramebuffer");
  greg_glBufferData = (PFNGLBUFFERDATAPROC) gregGetProcAddress("glBufferData");
  greg_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) gregGetProcAddress("glBufferSubData");
  greg_glCallList = (PFNGLCALLLISTPROC) gregGetProcAddress("glCallList");
  greg_glCallLists = (PFNGLCALLLISTSPROC) gregGetProcAddress("glCallLists");
  greg_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) gregGetProcAddress("glCheckFramebufferStatus");
  greg_glClampColor = (PFNGLCLAMPCOLORPROC) gregGetProcAddress("glClampColor");
  greg_glClear = (PFNGLCLEARPROC) gregGetProcAddress("glClear");
  greg_glClearAccum = (PFNGLCLEARACCUMPROC) gregGetProcAddress("glClearAccum");
  greg_glClearBufferfi = (PFNGLCLEARBUFFERFIPROC) gregGetProcAddress("glClearBufferfi");
  greg_glClearBufferfv = (PFNGLCLEARBUFFERFVPROC) gregGetProcAddress("glClearBufferfv");
  greg_glClearBufferiv = (PFNGLCLEARBUFFERIVPROC) gregGetProcAddress("glClearBufferiv");
  greg_glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC) gregGetProcAddress("glClearBufferuiv");
  greg_glClearColor = (PFNGLCLEARCOLORPROC) gregGetProcAddress("glClearColor");
  greg_glClearDepth = (PFNGLCLEARDEPTHPROC) gregGetProcAddress("glClearDepth");
  greg_glClearIndex = (PFNGLCLEARINDEXPROC) gregGetProcAddress("glClearIndex");
  greg_glClearStencil = (PFNGLCLEARSTENCILPROC) gregGetProcAddress("glClearStencil");
  greg_glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC) gregGetProcAddress("glClientActiveTexture");
  greg_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) gregGetProcAddress("glClientWaitSync");
  greg_glClipPlane = (PFNGLCLIPPLANEPROC) gregGetProcAddress("glClipPlane");
  greg_glColor3b = (PFNGLCOLOR3BPROC) gregGetProcAddress("glColor3b");
  greg_glColor3bv = (PFNGLCOLOR3BVPROC) gregGetProcAddress("glColor3bv");
  greg_glColor3d = (PFNGLCOLOR3DPROC) gregGetProcAddress("glColor3d");
  greg_glColor3dv = (PFNGLCOLOR3DVPROC) gregGetProcAddress("glColor3dv");
  greg_glColor3f = (PFNGLCOLOR3FPROC) gregGetProcAddress("glColor3f");
  greg_glColor3fv = (PFNGLCOLOR3FVPROC) gregGetProcAddress("glColor3fv");
  greg_glColor3i = (PFNGLCOLOR3IPROC) gregGetProcAddress("glColor3i");
  greg_glColor3iv = (PFNGLCOLOR3IVPROC) gregGetProcAddress("glColor3iv");
  greg_glColor3s = (PFNGLCOLOR3SPROC) gregGetProcAddress("glColor3s");
  greg_glColor3sv = (PFNGLCOLOR3SVPROC) gregGetProcAddress("glColor3sv");
  greg_glColor3ub = (PFNGLCOLOR3UBPROC) gregGetProcAddress("glColor3ub");
  greg_glColor3ubv = (PFNGLCOLOR3UBVPROC) gregGetProcAddress("glColor3ubv");
  greg_glColor3ui = (PFNGLCOLOR3UIPROC) gregGetProcAddress("glColor3ui");
  greg_glColor3uiv = (PFNGLCOLOR3UIVPROC) gregGetProcAddress("glColor3uiv");
  greg_glColor3us = (PFNGLCOLOR3USPROC) gregGetProcAddress("glColor3us");
  greg_glColor3usv = (PFNGLCOLOR3USVPROC) gregGetProcAddress("glColor3usv");
  greg_glColor4b = (PFNGLCOLOR4BPROC) gregGetProcAddress("glColor4b");
  greg_glColor4bv = (PFNGLCOLOR4BVPROC) gregGetProcAddress("glColor4bv");
  greg_glColor4d = (PFNGLCOLOR4DPROC) gregGetProcAddress("glColor4d");
  greg_glColor4dv = (PFNGLCOLOR4DVPROC) gregGetProcAddress("glColor4dv");
  greg_glColor4f = (PFNGLCOLOR4FPROC) gregGetProcAddress("glColor4f");
  greg_glColor4fv = (PFNGLCOLOR4FVPROC) gregGetProcAddress("glColor4fv");
  greg_glColor4i = (PFNGLCOLOR4IPROC) gregGetProcAddress("glColor4i");
  greg_glColor4iv = (PFNGLCOLOR4IVPROC) gregGetProcAddress("glColor4iv");
  greg_glColor4s = (PFNGLCOLOR4SPROC) gregGetProcAddress("glColor4s");
  greg_glColor4sv = (PFNGLCOLOR4SVPROC) gregGetProcAddress("glColor4sv");
  greg_glColor4ub = (PFNGLCOLOR4UBPROC) gregGetProcAddress("glColor4ub");
  greg_glColor4ubv = (PFNGLCOLOR4UBVPROC) gregGetProcAddress("glColor4ubv");
  greg_glColor4ui = (PFNGLCOLOR4UIPROC) gregGetProcAddress("glColor4ui");
  greg_glColor4uiv = (PFNGLCOLOR4UIVPROC) gregGetProcAddress("glColor4uiv");
  greg_glColor4us = (PFNGLCOLOR4USPROC) gregGetProcAddress("glColor4us");
  greg_glColor4usv = (PFNGLCOLOR4USVPROC) gregGetProcAddress("glColor4usv");
  greg_glColorMask = (PFNGLCOLORMASKPROC) gregGetProcAddress("glColorMask");
  greg_glColorMaski = (PFNGLCOLORMASKIPROC) gregGetProcAddress("glColorMaski");
  greg_glColorMaterial = (PFNGLCOLORMATERIALPROC) gregGetProcAddress("glColorMaterial");
  greg_glColorPointer = (PFNGLCOLORPOINTERPROC) gregGetProcAddress("glColorPointer");
  greg_glCompileShader = (PFNGLCOMPILESHADERPROC) gregGetProcAddress("glCompileShader");
  greg_glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC) gregGetProcAddress("glCompressedTexImage1D");
  greg_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) gregGetProcAddress("glCompressedTexImage2D");
  greg_glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC) gregGetProcAddress("glCompressedTexImage3D");
  greg_glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) gregGetProcAddress("glCompressedTexSubImage1D");
  greg_glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) gregGetProcAddress("glCompressedTexSubImage2D");
  greg_glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) gregGetProcAddress("glCompressedTexSubImage3D");
  greg_glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC) gregGetProcAddress("glCopyBufferSubData");
  greg_glCopyPixels = (PFNGLCOPYPIXELSPROC) gregGetProcAddress("glCopyPixels");
  greg_glCopyTexImage1D = (PFNGLCOPYTEXIMAGE1DPROC) gregGetProcAddress("glCopyTexImage1D");
  greg_glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC) gregGetProcAddress("glCopyTexImage2D");
  greg_glCopyTexSubImage1D = (PFNGLCOPYTEXSUBIMAGE1DPROC) gregGetProcAddress("glCopyTexSubImage1D");
  greg_glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC) gregGetProcAddress("glCopyTexSubImage2D");
  greg_glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC) gregGetProcAddress("glCopyTexSubImage3D");
  greg_glCreateProgram = (PFNGLCREATEPROGRAMPROC) gregGetProcAddress("glCreateProgram");
  greg_glCreateShader = (PFNGLCREATESHADERPROC) gregGetProcAddress("glCreateShader");
  greg_glCullFace = (PFNGLCULLFACEPROC) gregGetProcAddress("glCullFace");
  greg_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) gregGetProcAddress("glDebugMessageCallback");
  greg_glDebugMessageCallbackKHR = (PFNGLDEBUGMESSAGECALLBACKKHRPROC) gregGetProcAddress("glDebugMessageCallbackKHR");
  greg_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) gregGetProcAddress("glDebugMessageControl");
  greg_glDebugMessageControlKHR = (PFNGLDEBUGMESSAGECONTROLKHRPROC) gregGetProcAddress("glDebugMessageControlKHR");
  greg_glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC) gregGetProcAddress("glDebugMessageInsert");
  greg_glDebugMessageInsertKHR = (PFNGLDEBUGMESSAGEINSERTKHRPROC) gregGetProcAddress("glDebugMessageInsertKHR");
  greg_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) gregGetProcAddress("glDeleteBuffers");
  greg_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) gregGetProcAddress("glDeleteFramebuffers");
  greg_glDeleteLists = (PFNGLDELETELISTSPROC) gregGetProcAddress("glDeleteLists");
  greg_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) gregGetProcAddress("glDeleteProgram");
  greg_glDeleteQueries = (PFNGLDELETEQUERIESPROC) gregGetProcAddress("glDeleteQueries");
  greg_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) gregGetProcAddress("glDeleteRenderbuffers");
  greg_glDeleteShader = (PFNGLDELETESHADERPROC) gregGetProcAddress("glDeleteShader");
  greg_glDeleteSync = (PFNGLDELETESYNCPROC) gregGetProcAddress("glDeleteSync");
  greg_glDeleteTextures = (PFNGLDELETETEXTURESPROC) gregGetProcAddress("glDeleteTextures");
  greg_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) gregGetProcAddress("glDeleteVertexArrays");
  greg_glDepthFunc = (PFNGLDEPTHFUNCPROC) gregGetProcAddress("glDepthFunc");
  greg_glDepthMask = (PFNGLDEPTHMASKPROC) gregGetProcAddress("glDepthMask");
  greg_glDepthRange = (PFNGLDEPTHRANGEPROC) gregGetProcAddress("glDepthRange");
  greg_glDetachShader = (PFNGLDETACHSHADERPROC) gregGetProcAddress("glDetachShader");
  greg_glDisable = (PFNGLDISABLEPROC) gregGetProcAddress("glDisable");
  greg_glDisableClientState = (PFNGLDISABLECLIENTSTATEPROC) gregGetProcAddress("glDisableClientState");
  greg_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) gregGetProcAddress("glDisableVertexAttribArray");
  greg_glDisablei = (PFNGLDISABLEIPROC) gregGetProcAddress("glDisablei");
  greg_glDrawArrays = (PFNGLDRAWARRAYSPROC) gregGetProcAddress("glDrawArrays");
  greg_glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) gregGetProcAddress("glDrawArraysInstanced");
  greg_glDrawBuffer = (PFNGLDRAWBUFFERPROC) gregGetProcAddress("glDrawBuffer");
  greg_glDrawBuffers = (PFNGLDRAWBUFFERSPROC) gregGetProcAddress("glDrawBuffers");
  greg_glDrawElements = (PFNGLDRAWELEMENTSPROC) gregGetProcAddress("glDrawElements");
  greg_glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC) gregGetProcAddress("glDrawElementsBaseVertex");
  greg_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) gregGetProcAddress("glDrawElementsInstanced");
  greg_glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC) gregGetProcAddress("glDrawElementsInstancedBaseVertex");
  greg_glDrawPixels = (PFNGLDRAWPIXELSPROC) gregGetProcAddress("glDrawPixels");
  greg_glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC) gregGetProcAddress("glDrawRangeElements");
  greg_glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) gregGetProcAddress("glDrawRangeElementsBaseVertex");
  greg_glEdgeFlag = (PFNGLEDGEFLAGPROC) gregGetProcAddress("glEdgeFlag");
  greg_glEdgeFlagPointer = (PFNGLEDGEFLAGPOINTERPROC) gregGetProcAddress("glEdgeFlagPointer");
  greg_glEdgeFlagv = (PFNGLEDGEFLAGVPROC) gregGetProcAddress("glEdgeFlagv");
  greg_glEnable = (PFNGLENABLEPROC) gregGetProcAddress("glEnable");
  greg_glEnableClientState = (PFNGLENABLECLIENTSTATEPROC) gregGetProcAddress("glEnableClientState");
  greg_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) gregGetProcAddress("glEnableVertexAttribArray");
  greg_glEnablei = (PFNGLENABLEIPROC) gregGetProcAddress("glEnablei");
  greg_glEnd = (PFNGLENDPROC) gregGetProcAddress("glEnd");
  greg_glEndConditionalRender = (PFNGLENDCONDITIONALRENDERPROC) gregGetProcAddress("glEndConditionalRender");
  greg_glEndList = (PFNGLENDLISTPROC) gregGetProcAddress("glEndList");
  greg_glEndQuery = (PFNGLENDQUERYPROC) gregGetProcAddress("glEndQuery");
  greg_glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC) gregGetProcAddress("glEndTransformFeedback");
  greg_glEvalCoord1d = (PFNGLEVALCOORD1DPROC) gregGetProcAddress("glEvalCoord1d");
  greg_glEvalCoord1dv = (PFNGLEVALCOORD1DVPROC) gregGetProcAddress("glEvalCoord1dv");
  greg_glEvalCoord1f = (PFNGLEVALCOORD1FPROC) gregGetProcAddress("glEvalCoord1f");
  greg_glEvalCoord1fv = (PFNGLEVALCOORD1FVPROC) gregGetProcAddress("glEvalCoord1fv");
  greg_glEvalCoord2d = (PFNGLEVALCOORD2DPROC) gregGetProcAddress("glEvalCoord2d");
  greg_glEvalCoord2dv = (PFNGLEVALCOORD2DVPROC) gregGetProcAddress("glEvalCoord2dv");
  greg_glEvalCoord2f = (PFNGLEVALCOORD2FPROC) gregGetProcAddress("glEvalCoord2f");
  greg_glEvalCoord2fv = (PFNGLEVALCOORD2FVPROC) gregGetProcAddress("glEvalCoord2fv");
  greg_glEvalMesh1 = (PFNGLEVALMESH1PROC) gregGetProcAddress("glEvalMesh1");
  greg_glEvalMesh2 = (PFNGLEVALMESH2PROC) gregGetProcAddress("glEvalMesh2");
  greg_glEvalPoint1 = (PFNGLEVALPOINT1PROC) gregGetProcAddress("glEvalPoint1");
  greg_glEvalPoint2 = (PFNGLEVALPOINT2PROC) gregGetProcAddress("glEvalPoint2");
  greg_glFeedbackBuffer = (PFNGLFEEDBACKBUFFERPROC) gregGetProcAddress("glFeedbackBuffer");
  greg_glFenceSync = (PFNGLFENCESYNCPROC) gregGetProcAddress("glFenceSync");
  greg_glFinish = (PFNGLFINISHPROC) gregGetProcAddress("glFinish");
  greg_glFlush = (PFNGLFLUSHPROC) gregGetProcAddress("glFlush");
  greg_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) gregGetProcAddress("glFlushMappedBufferRange");
  greg_glFogCoordPointer = (PFNGLFOGCOORDPOINTERPROC) gregGetProcAddress("glFogCoordPointer");
  greg_glFogCoordd = (PFNGLFOGCOORDDPROC) gregGetProcAddress("glFogCoordd");
  greg_glFogCoorddv = (PFNGLFOGCOORDDVPROC) gregGetProcAddress("glFogCoorddv");
  greg_glFogCoordf = (PFNGLFOGCOORDFPROC) gregGetProcAddress("glFogCoordf");
  greg_glFogCoordfv = (PFNGLFOGCOORDFVPROC) gregGetProcAddress("glFogCoordfv");
  greg_glFogf = (PFNGLFOGFPROC) gregGetProcAddress("glFogf");
  greg_glFogfv = (PFNGLFOGFVPROC) gregGetProcAddress("glFogfv");
  greg_glFogi = (PFNGLFOGIPROC) gregGetProcAddress("glFogi");
  greg_glFogiv = (PFNGLFOGIVPROC) gregGetProcAddress("glFogiv");
  greg_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) gregGetProcAddress("glFramebufferRenderbuffer");
  greg_glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC) gregGetProcAddress("glFramebufferTexture");
  greg_glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC) gregGetProcAddress("glFramebufferTexture1D");
  greg_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) gregGetProcAddress("glFramebufferTexture2D");
  greg_glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC) gregGetProcAddress("glFramebufferTexture3D");
  greg_glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) gregGetProcAddress("glFramebufferTextureLayer");
  greg_glFrontFace = (PFNGLFRONTFACEPROC) gregGetProcAddress("glFrontFace");
  greg_glFrustum = (PFNGLFRUSTUMPROC) gregGetProcAddress("glFrustum");
  greg_glGenBuffers = (PFNGLGENBUFFERSPROC) gregGetProcAddress("glGenBuffers");
  greg_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) gregGetProcAddress("glGenFramebuffers");
  greg_glGenLists = (PFNGLGENLISTSPROC) gregGetProcAddress("glGenLists");
  greg_glGenQueries = (PFNGLGENQUERIESPROC) gregGetProcAddress("glGenQueries");
  greg_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) gregGetProcAddress("glGenRenderbuffers");
  greg_glGenTextures = (PFNGLGENTEXTURESPROC) gregGetProcAddress("glGenTextures");
  greg_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) gregGetProcAddress("glGenVertexArrays");
  greg_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) gregGetProcAddress("glGenerateMipmap");
  greg_glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) gregGetProcAddress("glGetActiveAttrib");
  greg_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) gregGetProcAddress("glGetActiveUniform");
  greg_glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) gregGetProcAddress("glGetActiveUniformBlockName");
  greg_glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC) gregGetProcAddress("glGetActiveUniformBlockiv");
  greg_glGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAMEPROC) gregGetProcAddress("glGetActiveUniformName");
  greg_glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC) gregGetProcAddress("glGetActiveUniformsiv");
  greg_glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) gregGetProcAddress("glGetAttachedShaders");
  greg_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) gregGetProcAddress("glGetAttribLocation");
  greg_glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC) gregGetProcAddress("glGetBooleani_v");
  greg_glGetBooleanv = (PFNGLGETBOOLEANVPROC) gregGetProcAddress("glGetBooleanv");
  greg_glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC) gregGetProcAddress("glGetBufferParameteri64v");
  greg_glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) gregGetProcAddress("glGetBufferParameteriv");
  greg_glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC) gregGetProcAddress("glGetBufferPointerv");
  greg_glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC) gregGetProcAddress("glGetBufferSubData");
  greg_glGetClipPlane = (PFNGLGETCLIPPLANEPROC) gregGetProcAddress("glGetClipPlane");
  greg_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC) gregGetProcAddress("glGetCompressedTexImage");
  greg_glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC) gregGetProcAddress("glGetDebugMessageLog");
  greg_glGetDebugMessageLogKHR = (PFNGLGETDEBUGMESSAGELOGKHRPROC) gregGetProcAddress("glGetDebugMessageLogKHR");
  greg_glGetDoublev = (PFNGLGETDOUBLEVPROC) gregGetProcAddress("glGetDoublev");
  greg_glGetError = (PFNGLGETERRORPROC) gregGetProcAddress("glGetError");
  greg_glGetFloatv = (PFNGLGETFLOATVPROC) gregGetProcAddress("glGetFloatv");
  greg_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) gregGetProcAddress("glGetFragDataLocation");
  greg_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) gregGetProcAddress("glGetFramebufferAttachmentParameteriv");
  greg_glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC) gregGetProcAddress("glGetInteger64i_v");
  greg_glGetInteger64v = (PFNGLGETINTEGER64VPROC) gregGetProcAddress("glGetInteger64v");
  greg_glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC) gregGetProcAddress("glGetIntegeri_v");
  greg_glGetIntegerv = (PFNGLGETINTEGERVPROC) gregGetProcAddress("glGetIntegerv");
  greg_glGetLightfv = (PFNGLGETLIGHTFVPROC) gregGetProcAddress("glGetLightfv");
  greg_glGetLightiv = (PFNGLGETLIGHTIVPROC) gregGetProcAddress("glGetLightiv");
  greg_glGetMapdv = (PFNGLGETMAPDVPROC) gregGetProcAddress("glGetMapdv");
  greg_glGetMapfv = (PFNGLGETMAPFVPROC) gregGetProcAddress("glGetMapfv");
  greg_glGetMapiv = (PFNGLGETMAPIVPROC) gregGetProcAddress("glGetMapiv");
  greg_glGetMaterialfv = (PFNGLGETMATERIALFVPROC) gregGetProcAddress("glGetMaterialfv");
  greg_glGetMaterialiv = (PFNGLGETMATERIALIVPROC) gregGetProcAddress("glGetMaterialiv");
  greg_glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC) gregGetProcAddress("glGetMultisamplefv");
  greg_glGetObjectLabel = (PFNGLGETOBJECTLABELPROC) gregGetProcAddress("glGetObjectLabel");
  greg_glGetObjectLabelKHR = (PFNGLGETOBJECTLABELKHRPROC) gregGetProcAddress("glGetObjectLabelKHR");
  greg_glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC) gregGetProcAddress("glGetObjectPtrLabel");
  greg_glGetObjectPtrLabelKHR = (PFNGLGETOBJECTPTRLABELKHRPROC) gregGetProcAddress("glGetObjectPtrLabelKHR");
  greg_glGetPixelMapfv = (PFNGLGETPIXELMAPFVPROC) gregGetProcAddress("glGetPixelMapfv");
  greg_glGetPixelMapuiv = (PFNGLGETPIXELMAPUIVPROC) gregGetProcAddress("glGetPixelMapuiv");
  greg_glGetPixelMapusv = (PFNGLGETPIXELMAPUSVPROC) gregGetProcAddress("glGetPixelMapusv");
  greg_glGetPointerv = (PFNGLGETPOINTERVPROC) gregGetProcAddress("glGetPointerv");
  greg_glGetPointervKHR = (PFNGLGETPOINTERVKHRPROC) gregGetProcAddress("glGetPointervKHR");
  greg_glGetPolygonStipple = (PFNGLGETPOLYGONSTIPPLEPROC) gregGetProcAddress("glGetPolygonStipple");
  greg_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) gregGetProcAddress("glGetProgramInfoLog");
  greg_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) gregGetProcAddress("glGetProgramiv");
  greg_glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC) gregGetProcAddress("glGetQueryObjectiv");
  greg_glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC) gregGetProcAddress("glGetQueryObjectuiv");
  greg_glGetQueryiv = (PFNGLGETQUERYIVPROC) gregGetProcAddress("glGetQueryiv");
  greg_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) gregGetProcAddress("glGetRenderbufferParameteriv");
  greg_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) gregGetProcAddress("glGetShaderInfoLog");
  greg_glGetShaderSource = (PFNGLGETSHADERSOURCEPROC) gregGetProcAddress("glGetShaderSource");
  greg_glGetShaderiv = (PFNGLGETSHADERIVPROC) gregGetProcAddress("glGetShaderiv");
  greg_glGetString = (PFNGLGETSTRINGPROC) gregGetProcAddress("glGetString");
  greg_glGetStringi = (PFNGLGETSTRINGIPROC) gregGetProcAddress("glGetStringi");
  greg_glGetSynciv = (PFNGLGETSYNCIVPROC) gregGetProcAddress("glGetSynciv");
  greg_glGetTexEnvfv = (PFNGLGETTEXENVFVPROC) gregGetProcAddress("glGetTexEnvfv");
  greg_glGetTexEnviv = (PFNGLGETTEXENVIVPROC) gregGetProcAddress("glGetTexEnviv");
  greg_glGetTexGendv = (PFNGLGETTEXGENDVPROC) gregGetProcAddress("glGetTexGendv");
  greg_glGetTexGenfv = (PFNGLGETTEXGENFVPROC) gregGetProcAddress("glGetTexGenfv");
  greg_glGetTexGeniv = (PFNGLGETTEXGENIVPROC) gregGetProcAddress("glGetTexGeniv");
  greg_glGetTexImage = (PFNGLGETTEXIMAGEPROC) gregGetProcAddress("glGetTexImage");
  greg_glGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC) gregGetProcAddress("glGetTexLevelParameterfv");
  greg_glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC) gregGetProcAddress("glGetTexLevelParameteriv");
  greg_glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC) gregGetProcAddress("glGetTexParameterIiv");
  greg_glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC) gregGetProcAddress("glGetTexParameterIuiv");
  greg_glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC) gregGetProcAddress("glGetTexParameterfv");
  greg_glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC) gregGetProcAddress("glGetTexParameteriv");
  greg_glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) gregGetProcAddress("glGetTransformFeedbackVarying");
  greg_glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC) gregGetProcAddress("glGetUniformBlockIndex");
  greg_glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC) gregGetProcAddress("glGetUniformIndices");
  greg_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) gregGetProcAddress("glGetUniformLocation");
  greg_glGetUniformfv = (PFNGLGETUNIFORMFVPROC) gregGetProcAddress("glGetUniformfv");
  greg_glGetUniformiv = (PFNGLGETUNIFORMIVPROC) gregGetProcAddress("glGetUniformiv");
  greg_glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC) gregGetProcAddress("glGetUniformuiv");
  greg_glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC) gregGetProcAddress("glGetVertexAttribIiv");
  greg_glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC) gregGetProcAddress("glGetVertexAttribIuiv");
  greg_glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC) gregGetProcAddress("glGetVertexAttribPointerv");
  greg_glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC) gregGetProcAddress("glGetVertexAttribdv");
  greg_glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) gregGetProcAddress("glGetVertexAttribfv");
  greg_glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC) gregGetProcAddress("glGetVertexAttribiv");
  greg_glHint = (PFNGLHINTPROC) gregGetProcAddress("glHint");
  greg_glIndexMask = (PFNGLINDEXMASKPROC) gregGetProcAddress("glIndexMask");
  greg_glIndexPointer = (PFNGLINDEXPOINTERPROC) gregGetProcAddress("glIndexPointer");
  greg_glIndexd = (PFNGLINDEXDPROC) gregGetProcAddress("glIndexd");
  greg_glIndexdv = (PFNGLINDEXDVPROC) gregGetProcAddress("glIndexdv");
  greg_glIndexf = (PFNGLINDEXFPROC) gregGetProcAddress("glIndexf");
  greg_glIndexfv = (PFNGLINDEXFVPROC) gregGetProcAddress("glIndexfv");
  greg_glIndexi = (PFNGLINDEXIPROC) gregGetProcAddress("glIndexi");
  greg_glIndexiv = (PFNGLINDEXIVPROC) gregGetProcAddress("glIndexiv");
  greg_glIndexs = (PFNGLINDEXSPROC) gregGetProcAddress("glIndexs");
  greg_glIndexsv = (PFNGLINDEXSVPROC) gregGetProcAddress("glIndexsv");
  greg_glIndexub = (PFNGLINDEXUBPROC) gregGetProcAddress("glIndexub");
  greg_glIndexubv = (PFNGLINDEXUBVPROC) gregGetProcAddress("glIndexubv");
  greg_glInitNames = (PFNGLINITNAMESPROC) gregGetProcAddress("glInitNames");
  greg_glInterleavedArrays = (PFNGLINTERLEAVEDARRAYSPROC) gregGetProcAddress("glInterleavedArrays");
  greg_glIsBuffer = (PFNGLISBUFFERPROC) gregGetProcAddress("glIsBuffer");
  greg_glIsEnabled = (PFNGLISENABLEDPROC) gregGetProcAddress("glIsEnabled");
  greg_glIsEnabledi = (PFNGLISENABLEDIPROC) gregGetProcAddress("glIsEnabledi");
  greg_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) gregGetProcAddress("glIsFramebuffer");
  greg_glIsList = (PFNGLISLISTPROC) gregGetProcAddress("glIsList");
  greg_glIsProgram = (PFNGLISPROGRAMPROC) gregGetProcAddress("glIsProgram");
  greg_glIsQuery = (PFNGLISQUERYPROC) gregGetProcAddress("glIsQuery");
  greg_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) gregGetProcAddress("glIsRenderbuffer");
  greg_glIsShader = (PFNGLISSHADERPROC) gregGetProcAddress("glIsShader");
  greg_glIsSync = (PFNGLISSYNCPROC) gregGetProcAddress("glIsSync");
  greg_glIsTexture = (PFNGLISTEXTUREPROC) gregGetProcAddress("glIsTexture");
  greg_glIsVertexArray = (PFNGLISVERTEXARRAYPROC) gregGetProcAddress("glIsVertexArray");
  greg_glLightModelf = (PFNGLLIGHTMODELFPROC) gregGetProcAddress("glLightModelf");
  greg_glLightModelfv = (PFNGLLIGHTMODELFVPROC) gregGetProcAddress("glLightModelfv");
  greg_glLightModeli = (PFNGLLIGHTMODELIPROC) gregGetProcAddress("glLightModeli");
  greg_glLightModeliv = (PFNGLLIGHTMODELIVPROC) gregGetProcAddress("glLightModeliv");
  greg_glLightf = (PFNGLLIGHTFPROC) gregGetProcAddress("glLightf");
  greg_glLightfv = (PFNGLLIGHTFVPROC) gregGetProcAddress("glLightfv");
  greg_glLighti = (PFNGLLIGHTIPROC) gregGetProcAddress("glLighti");
  greg_glLightiv = (PFNGLLIGHTIVPROC) gregGetProcAddress("glLightiv");
  greg_glLineStipple = (PFNGLLINESTIPPLEPROC) gregGetProcAddress("glLineStipple");
  greg_glLineWidth = (PFNGLLINEWIDTHPROC) gregGetProcAddress("glLineWidth");
  greg_glLinkProgram = (PFNGLLINKPROGRAMPROC) gregGetProcAddress("glLinkProgram");
  greg_glListBase = (PFNGLLISTBASEPROC) gregGetProcAddress("glListBase");
  greg_glLoadIdentity = (PFNGLLOADIDENTITYPROC) gregGetProcAddress("glLoadIdentity");
  greg_glLoadMatrixd = (PFNGLLOADMATRIXDPROC) gregGetProcAddress("glLoadMatrixd");
  greg_glLoadMatrixf = (PFNGLLOADMATRIXFPROC) gregGetProcAddress("glLoadMatrixf");
  greg_glLoadName = (PFNGLLOADNAMEPROC) gregGetProcAddress("glLoadName");
  greg_glLoadTransposeMatrixd = (PFNGLLOADTRANSPOSEMATRIXDPROC) gregGetProcAddress("glLoadTransposeMatrixd");
  greg_glLoadTransposeMatrixf = (PFNGLLOADTRANSPOSEMATRIXFPROC) gregGetProcAddress("glLoadTransposeMatrixf");
  greg_glLogicOp = (PFNGLLOGICOPPROC) gregGetProcAddress("glLogicOp");
  greg_glMap1d = (PFNGLMAP1DPROC) gregGetProcAddress("glMap1d");
  greg_glMap1f = (PFNGLMAP1FPROC) gregGetProcAddress("glMap1f");
  greg_glMap2d = (PFNGLMAP2DPROC) gregGetProcAddress("glMap2d");
  greg_glMap2f = (PFNGLMAP2FPROC) gregGetProcAddress("glMap2f");
  greg_glMapBuffer = (PFNGLMAPBUFFERPROC) gregGetProcAddress("glMapBuffer");
  greg_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) gregGetProcAddress("glMapBufferRange");
  greg_glMapGrid1d = (PFNGLMAPGRID1DPROC) gregGetProcAddress("glMapGrid1d");
  greg_glMapGrid1f = (PFNGLMAPGRID1FPROC) gregGetProcAddress("glMapGrid1f");
  greg_glMapGrid2d = (PFNGLMAPGRID2DPROC) gregGetProcAddress("glMapGrid2d");
  greg_glMapGrid2f = (PFNGLMAPGRID2FPROC) gregGetProcAddress("glMapGrid2f");
  greg_glMaterialf = (PFNGLMATERIALFPROC) gregGetProcAddress("glMaterialf");
  greg_glMaterialfv = (PFNGLMATERIALFVPROC) gregGetProcAddress("glMaterialfv");
  greg_glMateriali = (PFNGLMATERIALIPROC) gregGetProcAddress("glMateriali");
  greg_glMaterialiv = (PFNGLMATERIALIVPROC) gregGetProcAddress("glMaterialiv");
  greg_glMatrixMode = (PFNGLMATRIXMODEPROC) gregGetProcAddress("glMatrixMode");
  greg_glMultMatrixd = (PFNGLMULTMATRIXDPROC) gregGetProcAddress("glMultMatrixd");
  greg_glMultMatrixf = (PFNGLMULTMATRIXFPROC) gregGetProcAddress("glMultMatrixf");
  greg_glMultTransposeMatrixd = (PFNGLMULTTRANSPOSEMATRIXDPROC) gregGetProcAddress("glMultTransposeMatrixd");
  greg_glMultTransposeMatrixf = (PFNGLMULTTRANSPOSEMATRIXFPROC) gregGetProcAddress("glMultTransposeMatrixf");
  greg_glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC) gregGetProcAddress("glMultiDrawArrays");
  greg_glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC) gregGetProcAddress("glMultiDrawElements");
  greg_glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC) gregGetProcAddress("glMultiDrawElementsBaseVertex");
  greg_glMultiTexCoord1d = (PFNGLMULTITEXCOORD1DPROC) gregGetProcAddress("glMultiTexCoord1d");
  greg_glMultiTexCoord1dv = (PFNGLMULTITEXCOORD1DVPROC) gregGetProcAddress("glMultiTexCoord1dv");
  greg_glMultiTexCoord1f = (PFNGLMULTITEXCOORD1FPROC) gregGetProcAddress("glMultiTexCoord1f");
  greg_glMultiTexCoord1fv = (PFNGLMULTITEXCOORD1FVPROC) gregGetProcAddress("glMultiTexCoord1fv");
  greg_glMultiTexCoord1i = (PFNGLMULTITEXCOORD1IPROC) gregGetProcAddress("glMultiTexCoord1i");
  greg_glMultiTexCoord1iv = (PFNGLMULTITEXCOORD1IVPROC) gregGetProcAddress("glMultiTexCoord1iv");
  greg_glMultiTexCoord1s = (PFNGLMULTITEXCOORD1SPROC) gregGetProcAddress("glMultiTexCoord1s");
  greg_glMultiTexCoord1sv = (PFNGLMULTITEXCOORD1SVPROC) gregGetProcAddress("glMultiTexCoord1sv");
  greg_glMultiTexCoord2d = (PFNGLMULTITEXCOORD2DPROC) gregGetProcAddress("glMultiTexCoord2d");
  greg_glMultiTexCoord2dv = (PFNGLMULTITEXCOORD2DVPROC) gregGetProcAddress("glMultiTexCoord2dv");
  greg_glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC) gregGetProcAddress("glMultiTexCoord2f");
  greg_glMultiTexCoord2fv = (PFNGLMULTITEXCOORD2FVPROC) gregGetProcAddress("glMultiTexCoord2fv");
  greg_glMultiTexCoord2i = (PFNGLMULTITEXCOORD2IPROC) gregGetProcAddress("glMultiTexCoord2i");
  greg_glMultiTexCoord2iv = (PFNGLMULTITEXCOORD2IVPROC) gregGetProcAddress("glMultiTexCoord2iv");
  greg_glMultiTexCoord2s = (PFNGLMULTITEXCOORD2SPROC) gregGetProcAddress("glMultiTexCoord2s");
  greg_glMultiTexCoord2sv = (PFNGLMULTITEXCOORD2SVPROC) gregGetProcAddress("glMultiTexCoord2sv");
  greg_glMultiTexCoord3d = (PFNGLMULTITEXCOORD3DPROC) gregGetProcAddress("glMultiTexCoord3d");
  greg_glMultiTexCoord3dv = (PFNGLMULTITEXCOORD3DVPROC) gregGetProcAddress("glMultiTexCoord3dv");
  greg_glMultiTexCoord3f = (PFNGLMULTITEXCOORD3FPROC) gregGetProcAddress("glMultiTexCoord3f");
  greg_glMultiTexCoord3fv = (PFNGLMULTITEXCOORD3FVPROC) gregGetProcAddress("glMultiTexCoord3fv");
  greg_glMultiTexCoord3i = (PFNGLMULTITEXCOORD3IPROC) gregGetProcAddress("glMultiTexCoord3i");
  greg_glMultiTexCoord3iv = (PFNGLMULTITEXCOORD3IVPROC) gregGetProcAddress("glMultiTexCoord3iv");
  greg_glMultiTexCoord3s = (PFNGLMULTITEXCOORD3SPROC) gregGetProcAddress("glMultiTexCoord3s");
  greg_glMultiTexCoord3sv = (PFNGLMULTITEXCOORD3SVPROC) gregGetProcAddress("glMultiTexCoord3sv");
  greg_glMultiTexCoord4d = (PFNGLMULTITEXCOORD4DPROC) gregGetProcAddress("glMultiTexCoord4d");
  greg_glMultiTexCoord4dv = (PFNGLMULTITEXCOORD4DVPROC) gregGetProcAddress("glMultiTexCoord4dv");
  greg_glMultiTexCoord4f = (PFNGLMULTITEXCOORD4FPROC) gregGetProcAddress("glMultiTexCoord4f");
  greg_glMultiTexCoord4fv = (PFNGLMULTITEXCOORD4FVPROC) gregGetProcAddress("glMultiTexCoord4fv");
  greg_glMultiTexCoord4i = (PFNGLMULTITEXCOORD4IPROC) gregGetProcAddress("glMultiTexCoord4i");
  greg_glMultiTexCoord4iv = (PFNGLMULTITEXCOORD4IVPROC) gregGetProcAddress("glMultiTexCoord4iv");
  greg_glMultiTexCoord4s = (PFNGLMULTITEXCOORD4SPROC) gregGetProcAddress("glMultiTexCoord4s");
  greg_glMultiTexCoord4sv = (PFNGLMULTITEXCOORD4SVPROC) gregGetProcAddress("glMultiTexCoord4sv");
  greg_glNewList = (PFNGLNEWLISTPROC) gregGetProcAddress("glNewList");
  greg_glNormal3b = (PFNGLNORMAL3BPROC) gregGetProcAddress("glNormal3b");
  greg_glNormal3bv = (PFNGLNORMAL3BVPROC) gregGetProcAddress("glNormal3bv");
  greg_glNormal3d = (PFNGLNORMAL3DPROC) gregGetProcAddress("glNormal3d");
  greg_glNormal3dv = (PFNGLNORMAL3DVPROC) gregGetProcAddress("glNormal3dv");
  greg_glNormal3f = (PFNGLNORMAL3FPROC) gregGetProcAddress("glNormal3f");
  greg_glNormal3fv = (PFNGLNORMAL3FVPROC) gregGetProcAddress("glNormal3fv");
  greg_glNormal3i = (PFNGLNORMAL3IPROC) gregGetProcAddress("glNormal3i");
  greg_glNormal3iv = (PFNGLNORMAL3IVPROC) gregGetProcAddress("glNormal3iv");
  greg_glNormal3s = (PFNGLNORMAL3SPROC) gregGetProcAddress("glNormal3s");
  greg_glNormal3sv = (PFNGLNORMAL3SVPROC) gregGetProcAddress("glNormal3sv");
  greg_glNormalPointer = (PFNGLNORMALPOINTERPROC) gregGetProcAddress("glNormalPointer");
  greg_glObjectLabel = (PFNGLOBJECTLABELPROC) gregGetProcAddress("glObjectLabel");
  greg_glObjectLabelKHR = (PFNGLOBJECTLABELKHRPROC) gregGetProcAddress("glObjectLabelKHR");
  greg_glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC) gregGetProcAddress("glObjectPtrLabel");
  greg_glObjectPtrLabelKHR = (PFNGLOBJECTPTRLABELKHRPROC) gregGetProcAddress("glObjectPtrLabelKHR");
  greg_glOrtho = (PFNGLORTHOPROC) gregGetProcAddress("glOrtho");
  greg_glPassThrough = (PFNGLPASSTHROUGHPROC) gregGetProcAddress("glPassThrough");
  greg_glPixelMapfv = (PFNGLPIXELMAPFVPROC) gregGetProcAddress("glPixelMapfv");
  greg_glPixelMapuiv = (PFNGLPIXELMAPUIVPROC) gregGetProcAddress("glPixelMapuiv");
  greg_glPixelMapusv = (PFNGLPIXELMAPUSVPROC) gregGetProcAddress("glPixelMapusv");
  greg_glPixelStoref = (PFNGLPIXELSTOREFPROC) gregGetProcAddress("glPixelStoref");
  greg_glPixelStorei = (PFNGLPIXELSTOREIPROC) gregGetProcAddress("glPixelStorei");
  greg_glPixelTransferf = (PFNGLPIXELTRANSFERFPROC) gregGetProcAddress("glPixelTransferf");
  greg_glPixelTransferi = (PFNGLPIXELTRANSFERIPROC) gregGetProcAddress("glPixelTransferi");
  greg_glPixelZoom = (PFNGLPIXELZOOMPROC) gregGetProcAddress("glPixelZoom");
  greg_glPointParameterf = (PFNGLPOINTPARAMETERFPROC) gregGetProcAddress("glPointParameterf");
  greg_glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC) gregGetProcAddress("glPointParameterfv");
  greg_glPointParameteri = (PFNGLPOINTPARAMETERIPROC) gregGetProcAddress("glPointParameteri");
  greg_glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC) gregGetProcAddress("glPointParameteriv");
  greg_glPointSize = (PFNGLPOINTSIZEPROC) gregGetProcAddress("glPointSize");
  greg_glPolygonMode = (PFNGLPOLYGONMODEPROC) gregGetProcAddress("glPolygonMode");
  greg_glPolygonOffset = (PFNGLPOLYGONOFFSETPROC) gregGetProcAddress("glPolygonOffset");
  greg_glPolygonStipple = (PFNGLPOLYGONSTIPPLEPROC) gregGetProcAddress("glPolygonStipple");
  greg_glPopAttrib = (PFNGLPOPATTRIBPROC) gregGetProcAddress("glPopAttrib");
  greg_glPopClientAttrib = (PFNGLPOPCLIENTATTRIBPROC) gregGetProcAddress("glPopClientAttrib");
  greg_glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC) gregGetProcAddress("glPopDebugGroup");
  greg_glPopDebugGroupKHR = (PFNGLPOPDEBUGGROUPKHRPROC) gregGetProcAddress("glPopDebugGroupKHR");
  greg_glPopMatrix = (PFNGLPOPMATRIXPROC) gregGetProcAddress("glPopMatrix");
  greg_glPopName = (PFNGLPOPNAMEPROC) gregGetProcAddress("glPopName");
  greg_glPrimitiveRestartIndex = (PFNGLPRIMITIVERESTARTINDEXPROC) gregGetProcAddress("glPrimitiveRestartIndex");
  greg_glPrioritizeTextures = (PFNGLPRIORITIZETEXTURESPROC) gregGetProcAddress("glPrioritizeTextures");
  greg_glProvokingVertex = (PFNGLPROVOKINGVERTEXPROC) gregGetProcAddress("glProvokingVertex");
  greg_glPushAttrib = (PFNGLPUSHATTRIBPROC) gregGetProcAddress("glPushAttrib");
  greg_glPushClientAttrib = (PFNGLPUSHCLIENTATTRIBPROC) gregGetProcAddress("glPushClientAttrib");
  greg_glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC) gregGetProcAddress("glPushDebugGroup");
  greg_glPushDebugGroupKHR = (PFNGLPUSHDEBUGGROUPKHRPROC) gregGetProcAddress("glPushDebugGroupKHR");
  greg_glPushMatrix = (PFNGLPUSHMATRIXPROC) gregGetProcAddress("glPushMatrix");
  greg_glPushName = (PFNGLPUSHNAMEPROC) gregGetProcAddress("glPushName");
  greg_glRasterPos2d = (PFNGLRASTERPOS2DPROC) gregGetProcAddress("glRasterPos2d");
  greg_glRasterPos2dv = (PFNGLRASTERPOS2DVPROC) gregGetProcAddress("glRasterPos2dv");
  greg_glRasterPos2f = (PFNGLRASTERPOS2FPROC) gregGetProcAddress("glRasterPos2f");
  greg_glRasterPos2fv = (PFNGLRASTERPOS2FVPROC) gregGetProcAddress("glRasterPos2fv");
  greg_glRasterPos2i = (PFNGLRASTERPOS2IPROC) gregGetProcAddress("glRasterPos2i");
  greg_glRasterPos2iv = (PFNGLRASTERPOS2IVPROC) gregGetProcAddress("glRasterPos2iv");
  greg_glRasterPos2s = (PFNGLRASTERPOS2SPROC) gregGetProcAddress("glRasterPos2s");
  greg_glRasterPos2sv = (PFNGLRASTERPOS2SVPROC) gregGetProcAddress("glRasterPos2sv");
  greg_glRasterPos3d = (PFNGLRASTERPOS3DPROC) gregGetProcAddress("glRasterPos3d");
  greg_glRasterPos3dv = (PFNGLRASTERPOS3DVPROC) gregGetProcAddress("glRasterPos3dv");
  greg_glRasterPos3f = (PFNGLRASTERPOS3FPROC) gregGetProcAddress("glRasterPos3f");
  greg_glRasterPos3fv = (PFNGLRASTERPOS3FVPROC) gregGetProcAddress("glRasterPos3fv");
  greg_glRasterPos3i = (PFNGLRASTERPOS3IPROC) gregGetProcAddress("glRasterPos3i");
  greg_glRasterPos3iv = (PFNGLRASTERPOS3IVPROC) gregGetProcAddress("glRasterPos3iv");
  greg_glRasterPos3s = (PFNGLRASTERPOS3SPROC) gregGetProcAddress("glRasterPos3s");
  greg_glRasterPos3sv = (PFNGLRASTERPOS3SVPROC) gregGetProcAddress("glRasterPos3sv");
  greg_glRasterPos4d = (PFNGLRASTERPOS4DPROC) gregGetProcAddress("glRasterPos4d");
  greg_glRasterPos4dv = (PFNGLRASTERPOS4DVPROC) gregGetProcAddress("glRasterPos4dv");
  greg_glRasterPos4f = (PFNGLRASTERPOS4FPROC) gregGetProcAddress("glRasterPos4f");
  greg_glRasterPos4fv = (PFNGLRASTERPOS4FVPROC) gregGetProcAddress("glRasterPos4fv");
  greg_glRasterPos4i = (PFNGLRASTERPOS4IPROC) gregGetProcAddress("glRasterPos4i");
  greg_glRasterPos4iv = (PFNGLRASTERPOS4IVPROC) gregGetProcAddress("glRasterPos4iv");
  greg_glRasterPos4s = (PFNGLRASTERPOS4SPROC) gregGetProcAddress("glRasterPos4s");
  greg_glRasterPos4sv = (PFNGLRASTERPOS4SVPROC) gregGetProcAddress("glRasterPos4sv");
  greg_glReadBuffer = (PFNGLREADBUFFERPROC) gregGetProcAddress("glReadBuffer");
  greg_glReadPixels = (PFNGLREADPIXELSPROC) gregGetProcAddress("glReadPixels");
  greg_glRectd = (PFNGLRECTDPROC) gregGetProcAddress("glRectd");
  greg_glRectdv = (PFNGLRECTDVPROC) gregGetProcAddress("glRectdv");
  greg_glRectf = (PFNGLRECTFPROC) gregGetProcAddress("glRectf");
  greg_glRectfv = (PFNGLRECTFVPROC) gregGetProcAddress("glRectfv");
  greg_glRecti = (PFNGLRECTIPROC) gregGetProcAddress("glRecti");
  greg_glRectiv = (PFNGLRECTIVPROC) gregGetProcAddress("glRectiv");
  greg_glRects = (PFNGLRECTSPROC) gregGetProcAddress("glRects");
  greg_glRectsv = (PFNGLRECTSVPROC) gregGetProcAddress("glRectsv");
  greg_glRenderMode = (PFNGLRENDERMODEPROC) gregGetProcAddress("glRenderMode");
  greg_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) gregGetProcAddress("glRenderbufferStorage");
  greg_glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) gregGetProcAddress("glRenderbufferStorageMultisample");
  greg_glRotated = (PFNGLROTATEDPROC) gregGetProcAddress("glRotated");
  greg_glRotatef = (PFNGLROTATEFPROC) gregGetProcAddress("glRotatef");
  greg_glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) gregGetProcAddress("glSampleCoverage");
  greg_glSampleMaski = (PFNGLSAMPLEMASKIPROC) gregGetProcAddress("glSampleMaski");
  greg_glScaled = (PFNGLSCALEDPROC) gregGetProcAddress("glScaled");
  greg_glScalef = (PFNGLSCALEFPROC) gregGetProcAddress("glScalef");
  greg_glScissor = (PFNGLSCISSORPROC) gregGetProcAddress("glScissor");
  greg_glSecondaryColor3b = (PFNGLSECONDARYCOLOR3BPROC) gregGetProcAddress("glSecondaryColor3b");
  greg_glSecondaryColor3bv = (PFNGLSECONDARYCOLOR3BVPROC) gregGetProcAddress("glSecondaryColor3bv");
  greg_glSecondaryColor3d = (PFNGLSECONDARYCOLOR3DPROC) gregGetProcAddress("glSecondaryColor3d");
  greg_glSecondaryColor3dv = (PFNGLSECONDARYCOLOR3DVPROC) gregGetProcAddress("glSecondaryColor3dv");
  greg_glSecondaryColor3f = (PFNGLSECONDARYCOLOR3FPROC) gregGetProcAddress("glSecondaryColor3f");
  greg_glSecondaryColor3fv = (PFNGLSECONDARYCOLOR3FVPROC) gregGetProcAddress("glSecondaryColor3fv");
  greg_glSecondaryColor3i = (PFNGLSECONDARYCOLOR3IPROC) gregGetProcAddress("glSecondaryColor3i");
  greg_glSecondaryColor3iv = (PFNGLSECONDARYCOLOR3IVPROC) gregGetProcAddress("glSecondaryColor3iv");
  greg_glSecondaryColor3s = (PFNGLSECONDARYCOLOR3SPROC) gregGetProcAddress("glSecondaryColor3s");
  greg_glSecondaryColor3sv = (PFNGLSECONDARYCOLOR3SVPROC) gregGetProcAddress("glSecondaryColor3sv");
  greg_glSecondaryColor3ub = (PFNGLSECONDARYCOLOR3UBPROC) gregGetProcAddress("glSecondaryColor3ub");
  greg_glSecondaryColor3ubv = (PFNGLSECONDARYCOLOR3UBVPROC) gregGetProcAddress("glSecondaryColor3ubv");
  greg_glSecondaryColor3ui = (PFNGLSECONDARYCOLOR3UIPROC) gregGetProcAddress("glSecondaryColor3ui");
  greg_glSecondaryColor3uiv = (PFNGLSECONDARYCOLOR3UIVPROC) gregGetProcAddress("glSecondaryColor3uiv");
  greg_glSecondaryColor3us = (PFNGLSECONDARYCOLOR3USPROC) gregGetProcAddress("glSecondaryColor3us");
  greg_glSecondaryColor3usv = (PFNGLSECONDARYCOLOR3USVPROC) gregGetProcAddress("glSecondaryColor3usv");
  greg_glSecondaryColorPointer = (PFNGLSECONDARYCOLORPOINTERPROC) gregGetProcAddress("glSecondaryColorPointer");
  greg_glSelectBuffer = (PFNGLSELECTBUFFERPROC) gregGetProcAddress("glSelectBuffer");
  greg_glShadeModel = (PFNGLSHADEMODELPROC) gregGetProcAddress("glShadeModel");
  greg_glShaderSource = (PFNGLSHADERSOURCEPROC) gregGetProcAddress("glShaderSource");
  greg_glStencilFunc = (PFNGLSTENCILFUNCPROC) gregGetProcAddress("glStencilFunc");
  greg_glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) gregGetProcAddress("glStencilFuncSeparate");
  greg_glStencilMask = (PFNGLSTENCILMASKPROC) gregGetProcAddress("glStencilMask");
  greg_glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC) gregGetProcAddress("glStencilMaskSeparate");
  greg_glStencilOp = (PFNGLSTENCILOPPROC) gregGetProcAddress("glStencilOp");
  greg_glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) gregGetProcAddress("glStencilOpSeparate");
  greg_glTexBuffer = (PFNGLTEXBUFFERPROC) gregGetProcAddress("glTexBuffer");
  greg_glTexCoord1d = (PFNGLTEXCOORD1DPROC) gregGetProcAddress("glTexCoord1d");
  greg_glTexCoord1dv = (PFNGLTEXCOORD1DVPROC) gregGetProcAddress("glTexCoord1dv");
  greg_glTexCoord1f = (PFNGLTEXCOORD1FPROC) gregGetProcAddress("glTexCoord1f");
  greg_glTexCoord1fv = (PFNGLTEXCOORD1FVPROC) gregGetProcAddress("glTexCoord1fv");
  greg_glTexCoord1i = (PFNGLTEXCOORD1IPROC) gregGetProcAddress("glTexCoord1i");
  greg_glTexCoord1iv = (PFNGLTEXCOORD1IVPROC) gregGetProcAddress("glTexCoord1iv");
  greg_glTexCoord1s = (PFNGLTEXCOORD1SPROC) gregGetProcAddress("glTexCoord1s");
  greg_glTexCoord1sv = (PFNGLTEXCOORD1SVPROC) gregGetProcAddress("glTexCoord1sv");
  greg_glTexCoord2d = (PFNGLTEXCOORD2DPROC) gregGetProcAddress("glTexCoord2d");
  greg_glTexCoord2dv = (PFNGLTEXCOORD2DVPROC) gregGetProcAddress("glTexCoord2dv");
  greg_glTexCoord2f = (PFNGLTEXCOORD2FPROC) gregGetProcAddress("glTexCoord2f");
  greg_glTexCoord2fv = (PFNGLTEXCOORD2FVPROC) gregGetProcAddress("glTexCoord2fv");
  greg_glTexCoord2i = (PFNGLTEXCOORD2IPROC) gregGetProcAddress("glTexCoord2i");
  greg_glTexCoord2iv = (PFNGLTEXCOORD2IVPROC) gregGetProcAddress("glTexCoord2iv");
  greg_glTexCoord2s = (PFNGLTEXCOORD2SPROC) gregGetProcAddress("glTexCoord2s");
  greg_glTexCoord2sv = (PFNGLTEXCOORD2SVPROC) gregGetProcAddress("glTexCoord2sv");
  greg_glTexCoord3d = (PFNGLTEXCOORD3DPROC) gregGetProcAddress("glTexCoord3d");
  greg_glTexCoord3dv = (PFNGLTEXCOORD3DVPROC) gregGetProcAddress("glTexCoord3dv");
  greg_glTexCoord3f = (PFNGLTEXCOORD3FPROC) gregGetProcAddress("glTexCoord3f");
  greg_glTexCoord3fv = (PFNGLTEXCOORD3FVPROC) gregGetProcAddress("glTexCoord3fv");
  greg_glTexCoord3i = (PFNGLTEXCOORD3IPROC) gregGetProcAddress("glTexCoord3i");
  greg_glTexCoord3iv = (PFNGLTEXCOORD3IVPROC) gregGetProcAddress("glTexCoord3iv");
  greg_glTexCoord3s = (PFNGLTEXCOORD3SPROC) gregGetProcAddress("glTexCoord3s");
  greg_glTexCoord3sv = (PFNGLTEXCOORD3SVPROC) gregGetProcAddress("glTexCoord3sv");
  greg_glTexCoord4d = (PFNGLTEXCOORD4DPROC) gregGetProcAddress("glTexCoord4d");
  greg_glTexCoord4dv = (PFNGLTEXCOORD4DVPROC) gregGetProcAddress("glTexCoord4dv");
  greg_glTexCoord4f = (PFNGLTEXCOORD4FPROC) gregGetProcAddress("glTexCoord4f");
  greg_glTexCoord4fv = (PFNGLTEXCOORD4FVPROC) gregGetProcAddress("glTexCoord4fv");
  greg_glTexCoord4i = (PFNGLTEXCOORD4IPROC) gregGetProcAddress("glTexCoord4i");
  greg_glTexCoord4iv = (PFNGLTEXCOORD4IVPROC) gregGetProcAddress("glTexCoord4iv");
  greg_glTexCoord4s = (PFNGLTEXCOORD4SPROC) gregGetProcAddress("glTexCoord4s");
  greg_glTexCoord4sv = (PFNGLTEXCOORD4SVPROC) gregGetProcAddress("glTexCoord4sv");
  greg_glTexCoordPointer = (PFNGLTEXCOORDPOINTERPROC) gregGetProcAddress("glTexCoordPointer");
  greg_glTexEnvf = (PFNGLTEXENVFPROC) gregGetProcAddress("glTexEnvf");
  greg_glTexEnvfv = (PFNGLTEXENVFVPROC) gregGetProcAddress("glTexEnvfv");
  greg_glTexEnvi = (PFNGLTEXENVIPROC) gregGetProcAddress("glTexEnvi");
  greg_glTexEnviv = (PFNGLTEXENVIVPROC) gregGetProcAddress("glTexEnviv");
  greg_glTexGend = (PFNGLTEXGENDPROC) gregGetProcAddress("glTexGend");
  greg_glTexGendv = (PFNGLTEXGENDVPROC) gregGetProcAddress("glTexGendv");
  greg_glTexGenf = (PFNGLTEXGENFPROC) gregGetProcAddress("glTexGenf");
  greg_glTexGenfv = (PFNGLTEXGENFVPROC) gregGetProcAddress("glTexGenfv");
  greg_glTexGeni = (PFNGLTEXGENIPROC) gregGetProcAddress("glTexGeni");
  greg_glTexGeniv = (PFNGLTEXGENIVPROC) gregGetProcAddress("glTexGeniv");
  greg_glTexImage1D = (PFNGLTEXIMAGE1DPROC) gregGetProcAddress("glTexImage1D");
  greg_glTexImage2D = (PFNGLTEXIMAGE2DPROC) gregGetProcAddress("glTexImage2D");
  greg_glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC) gregGetProcAddress("glTexImage2DMultisample");
  greg_glTexImage3D = (PFNGLTEXIMAGE3DPROC) gregGetProcAddress("glTexImage3D");
  greg_glTexImage3DMultisample = (PFNGLTEXIMAGE3DMULTISAMPLEPROC) gregGetProcAddress("glTexImage3DMultisample");
  greg_glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC) gregGetProcAddress("glTexParameterIiv");
  greg_glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC) gregGetProcAddress("glTexParameterIuiv");
  greg_glTexParameterf = (PFNGLTEXPARAMETERFPROC) gregGetProcAddress("glTexParameterf");
  greg_glTexParameterfv = (PFNGLTEXPARAMETERFVPROC) gregGetProcAddress("glTexParameterfv");
  greg_glTexParameteri = (PFNGLTEXPARAMETERIPROC) gregGetProcAddress("glTexParameteri");
  greg_glTexParameteriv = (PFNGLTEXPARAMETERIVPROC) gregGetProcAddress("glTexParameteriv");
  greg_glTexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC) gregGetProcAddress("glTexSubImage1D");
  greg_glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC) gregGetProcAddress("glTexSubImage2D");
  greg_glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) gregGetProcAddress("glTexSubImage3D");
  greg_glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC) gregGetProcAddress("glTransformFeedbackVaryings");
  greg_glTranslated = (PFNGLTRANSLATEDPROC) gregGetProcAddress("glTranslated");
  greg_glTranslatef = (PFNGLTRANSLATEFPROC) gregGetProcAddress("glTranslatef");
  greg_glUniform1f = (PFNGLUNIFORM1FPROC) gregGetProcAddress("glUniform1f");
  greg_glUniform1fv = (PFNGLUNIFORM1FVPROC) gregGetProcAddress("glUniform1fv");
  greg_glUniform1i = (PFNGLUNIFORM1IPROC) gregGetProcAddress("glUniform1i");
  greg_glUniform1iv = (PFNGLUNIFORM1IVPROC) gregGetProcAddress("glUniform1iv");
  greg_glUniform1ui = (PFNGLUNIFORM1UIPROC) gregGetProcAddress("glUniform1ui");
  greg_glUniform1uiv = (PFNGLUNIFORM1UIVPROC) gregGetProcAddress("glUniform1uiv");
  greg_glUniform2f = (PFNGLUNIFORM2FPROC) gregGetProcAddress("glUniform2f");
  greg_glUniform2fv = (PFNGLUNIFORM2FVPROC) gregGetProcAddress("glUniform2fv");
  greg_glUniform2i = (PFNGLUNIFORM2IPROC) gregGetProcAddress("glUniform2i");
  greg_glUniform2iv = (PFNGLUNIFORM2IVPROC) gregGetProcAddress("glUniform2iv");
  greg_glUniform2ui = (PFNGLUNIFORM2UIPROC) gregGetProcAddress("glUniform2ui");
  greg_glUniform2uiv = (PFNGLUNIFORM2UIVPROC) gregGetProcAddress("glUniform2uiv");
  greg_glUniform3f = (PFNGLUNIFORM3FPROC) gregGetProcAddress("glUniform3f");
  greg_glUniform3fv = (PFNGLUNIFORM3FVPROC) gregGetProcAddress("glUniform3fv");
  greg_glUniform3i = (PFNGLUNIFORM3IPROC) gregGetProcAddress("glUniform3i");
  greg_glUniform3iv = (PFNGLUNIFORM3IVPROC) gregGetProcAddress("glUniform3iv");
  greg_glUniform3ui = (PFNGLUNIFORM3UIPROC) gregGetProcAddress("glUniform3ui");
  greg_glUniform3uiv = (PFNGLUNIFORM3UIVPROC) gregGetProcAddress("glUniform3uiv");
  greg_glUniform4f = (PFNGLUNIFORM4FPROC) gregGetProcAddress("glUniform4f");
  greg_glUniform4fv = (PFNGLUNIFORM4FVPROC) gregGetProcAddress("glUniform4fv");
  greg_glUniform4i = (PFNGLUNIFORM4IPROC) gregGetProcAddress("glUniform4i");
  greg_glUniform4iv = (PFNGLUNIFORM4IVPROC) gregGetProcAddress("glUniform4iv");
  greg_glUniform4ui = (PFNGLUNIFORM4UIPROC) gregGetProcAddress("glUniform4ui");
  greg_glUniform4uiv = (PFNGLUNIFORM4UIVPROC) gregGetProcAddress("glUniform4uiv");
  greg_glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC) gregGetProcAddress("glUniformBlockBinding");
  greg_glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) gregGetProcAddress("glUniformMatrix2fv");
  greg_glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) gregGetProcAddress("glUniformMatrix2x3fv");
  greg_glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) gregGetProcAddress("glUniformMatrix2x4fv");
  greg_glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) gregGetProcAddress("glUniformMatrix3fv");
  greg_glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) gregGetProcAddress("glUniformMatrix3x2fv");
  greg_glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) gregGetProcAddress("glUniformMatrix3x4fv");
  greg_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) gregGetProcAddress("glUniformMatrix4fv");
  greg_glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) gregGetProcAddress("glUniformMatrix4x2fv");
  greg_glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) gregGetProcAddress("glUniformMatrix4x3fv");
  greg_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) gregGetProcAddress("glUnmapBuffer");
  greg_glUseProgram = (PFNGLUSEPROGRAMPROC) gregGetProcAddress("glUseProgram");
  greg_glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) gregGetProcAddress("glValidateProgram");
  greg_glVertex2d = (PFNGLVERTEX2DPROC) gregGetProcAddress("glVertex2d");
  greg_glVertex2dv = (PFNGLVERTEX2DVPROC) gregGetProcAddress("glVertex2dv");
  greg_glVertex2f = (PFNGLVERTEX2FPROC) gregGetProcAddress("glVertex2f");
  greg_glVertex2fv = (PFNGLVERTEX2FVPROC) gregGetProcAddress("glVertex2fv");
  greg_glVertex2i = (PFNGLVERTEX2IPROC) gregGetProcAddress("glVertex2i");
  greg_glVertex2iv = (PFNGLVERTEX2IVPROC) gregGetProcAddress("glVertex2iv");
  greg_glVertex2s = (PFNGLVERTEX2SPROC) gregGetProcAddress("glVertex2s");
  greg_glVertex2sv = (PFNGLVERTEX2SVPROC) gregGetProcAddress("glVertex2sv");
  greg_glVertex3d = (PFNGLVERTEX3DPROC) gregGetProcAddress("glVertex3d");
  greg_glVertex3dv = (PFNGLVERTEX3DVPROC) gregGetProcAddress("glVertex3dv");
  greg_glVertex3f = (PFNGLVERTEX3FPROC) gregGetProcAddress("glVertex3f");
  greg_glVertex3fv = (PFNGLVERTEX3FVPROC) gregGetProcAddress("glVertex3fv");
  greg_glVertex3i = (PFNGLVERTEX3IPROC) gregGetProcAddress("glVertex3i");
  greg_glVertex3iv = (PFNGLVERTEX3IVPROC) gregGetProcAddress("glVertex3iv");
  greg_glVertex3s = (PFNGLVERTEX3SPROC) gregGetProcAddress("glVertex3s");
  greg_glVertex3sv = (PFNGLVERTEX3SVPROC) gregGetProcAddress("glVertex3sv");
  greg_glVertex4d = (PFNGLVERTEX4DPROC) gregGetProcAddress("glVertex4d");
  greg_glVertex4dv = (PFNGLVERTEX4DVPROC) gregGetProcAddress("glVertex4dv");
  greg_glVertex4f = (PFNGLVERTEX4FPROC) gregGetProcAddress("glVertex4f");
  greg_glVertex4fv = (PFNGLVERTEX4FVPROC) gregGetProcAddress("glVertex4fv");
  greg_glVertex4i = (PFNGLVERTEX4IPROC) gregGetProcAddress("glVertex4i");
  greg_glVertex4iv = (PFNGLVERTEX4IVPROC) gregGetProcAddress("glVertex4iv");
  greg_glVertex4s = (PFNGLVERTEX4SPROC) gregGetProcAddress("glVertex4s");
  greg_glVertex4sv = (PFNGLVERTEX4SVPROC) gregGetProcAddress("glVertex4sv");
  greg_glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC) gregGetProcAddress("glVertexAttrib1d");
  greg_glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC) gregGetProcAddress("glVertexAttrib1dv");
  greg_glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC) gregGetProcAddress("glVertexAttrib1f");
  greg_glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC) gregGetProcAddress("glVertexAttrib1fv");
  greg_glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC) gregGetProcAddress("glVertexAttrib1s");
  greg_glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC) gregGetProcAddress("glVertexAttrib1sv");
  greg_glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC) gregGetProcAddress("glVertexAttrib2d");
  greg_glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC) gregGetProcAddress("glVertexAttrib2dv");
  greg_glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC) gregGetProcAddress("glVertexAttrib2f");
  greg_glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC) gregGetProcAddress("glVertexAttrib2fv");
  greg_glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC) gregGetProcAddress("glVertexAttrib2s");
  greg_glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC) gregGetProcAddress("glVertexAttrib2sv");
  greg_glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC) gregGetProcAddress("glVertexAttrib3d");
  greg_glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC) gregGetProcAddress("glVertexAttrib3dv");
  greg_glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC) gregGetProcAddress("glVertexAttrib3f");
  greg_glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC) gregGetProcAddress("glVertexAttrib3fv");
  greg_glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC) gregGetProcAddress("glVertexAttrib3s");
  greg_glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC) gregGetProcAddress("glVertexAttrib3sv");
  greg_glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC) gregGetProcAddress("glVertexAttrib4Nbv");
  greg_glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC) gregGetProcAddress("glVertexAttrib4Niv");
  greg_glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC) gregGetProcAddress("glVertexAttrib4Nsv");
  greg_glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC) gregGetProcAddress("glVertexAttrib4Nub");
  greg_glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC) gregGetProcAddress("glVertexAttrib4Nubv");
  greg_glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC) gregGetProcAddress("glVertexAttrib4Nuiv");
  greg_glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC) gregGetProcAddress("glVertexAttrib4Nusv");
  greg_glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC) gregGetProcAddress("glVertexAttrib4bv");
  greg_glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC) gregGetProcAddress("glVertexAttrib4d");
  greg_glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC) gregGetProcAddress("glVertexAttrib4dv");
  greg_glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC) gregGetProcAddress("glVertexAttrib4f");
  greg_glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC) gregGetProcAddress("glVertexAttrib4fv");
  greg_glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC) gregGetProcAddress("glVertexAttrib4iv");
  greg_glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC) gregGetProcAddress("glVertexAttrib4s");
  greg_glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC) gregGetProcAddress("glVertexAttrib4sv");
  greg_glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC) gregGetProcAddress("glVertexAttrib4ubv");
  greg_glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC) gregGetProcAddress("glVertexAttrib4uiv");
  greg_glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC) gregGetProcAddress("glVertexAttrib4usv");
  greg_glVertexAttribI1i = (PFNGLVERTEXATTRIBI1IPROC) gregGetProcAddress("glVertexAttribI1i");
  greg_glVertexAttribI1iv = (PFNGLVERTEXATTRIBI1IVPROC) gregGetProcAddress("glVertexAttribI1iv");
  greg_glVertexAttribI1ui = (PFNGLVERTEXATTRIBI1UIPROC) gregGetProcAddress("glVertexAttribI1ui");
  greg_glVertexAttribI1uiv = (PFNGLVERTEXATTRIBI1UIVPROC) gregGetProcAddress("glVertexAttribI1uiv");
  greg_glVertexAttribI2i = (PFNGLVERTEXATTRIBI2IPROC) gregGetProcAddress("glVertexAttribI2i");
  greg_glVertexAttribI2iv = (PFNGLVERTEXATTRIBI2IVPROC) gregGetProcAddress("glVertexAttribI2iv");
  greg_glVertexAttribI2ui = (PFNGLVERTEXATTRIBI2UIPROC) gregGetProcAddress("glVertexAttribI2ui");
  greg_glVertexAttribI2uiv = (PFNGLVERTEXATTRIBI2UIVPROC) gregGetProcAddress("glVertexAttribI2uiv");
  greg_glVertexAttribI3i = (PFNGLVERTEXATTRIBI3IPROC) gregGetProcAddress("glVertexAttribI3i");
  greg_glVertexAttribI3iv = (PFNGLVERTEXATTRIBI3IVPROC) gregGetProcAddress("glVertexAttribI3iv");
  greg_glVertexAttribI3ui = (PFNGLVERTEXATTRIBI3UIPROC) gregGetProcAddress("glVertexAttribI3ui");
  greg_glVertexAttribI3uiv = (PFNGLVERTEXATTRIBI3UIVPROC) gregGetProcAddress("glVertexAttribI3uiv");
  greg_glVertexAttribI4bv = (PFNGLVERTEXATTRIBI4BVPROC) gregGetProcAddress("glVertexAttribI4bv");
  greg_glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC) gregGetProcAddress("glVertexAttribI4i");
  greg_glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC) gregGetProcAddress("glVertexAttribI4iv");
  greg_glVertexAttribI4sv = (PFNGLVERTEXATTRIBI4SVPROC) gregGetProcAddress("glVertexAttribI4sv");
  greg_glVertexAttribI4ubv = (PFNGLVERTEXATTRIBI4UBVPROC) gregGetProcAddress("glVertexAttribI4ubv");
  greg_glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC) gregGetProcAddress("glVertexAttribI4ui");
  greg_glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC) gregGetProcAddress("glVertexAttribI4uiv");
  greg_glVertexAttribI4usv = (PFNGLVERTEXATTRIBI4USVPROC) gregGetProcAddress("glVertexAttribI4usv");
  greg_glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC) gregGetProcAddress("glVertexAttribIPointer");
  greg_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) gregGetProcAddress("glVertexAttribPointer");
  greg_glVertexPointer = (PFNGLVERTEXPOINTERPROC) gregGetProcAddress("glVertexPointer");
  greg_glViewport = (PFNGLVIEWPORTPROC) gregGetProcAddress("glViewport");
  greg_glWaitSync = (PFNGLWAITSYNCPROC) gregGetProcAddress("glWaitSync");
  greg_glWindowPos2d = (PFNGLWINDOWPOS2DPROC) gregGetProcAddress("glWindowPos2d");
  greg_glWindowPos2dv = (PFNGLWINDOWPOS2DVPROC) gregGetProcAddress("glWindowPos2dv");
  greg_glWindowPos2f = (PFNGLWINDOWPOS2FPROC) gregGetProcAddress("glWindowPos2f");
  greg_glWindowPos2fv = (PFNGLWINDOWPOS2FVPROC) gregGetProcAddress("glWindowPos2fv");
  greg_glWindowPos2i = (PFNGLWINDOWPOS2IPROC) gregGetProcAddress("glWindowPos2i");
  greg_glWindowPos2iv = (PFNGLWINDOWPOS2IVPROC) gregGetProcAddress("glWindowPos2iv");
  greg_glWindowPos2s = (PFNGLWINDOWPOS2SPROC) gregGetProcAddress("glWindowPos2s");
  greg_glWindowPos2sv = (PFNGLWINDOWPOS2SVPROC) gregGetProcAddress("glWindowPos2sv");
  greg_glWindowPos3d = (PFNGLWINDOWPOS3DPROC) gregGetProcAddress("glWindowPos3d");
  greg_glWindowPos3dv = (PFNGLWINDOWPOS3DVPROC) gregGetProcAddress("glWindowPos3dv");
  greg_glWindowPos3f = (PFNGLWINDOWPOS3FPROC) gregGetProcAddress("glWindowPos3f");
  greg_glWindowPos3fv = (PFNGLWINDOWPOS3FVPROC) gregGetProcAddress("glWindowPos3fv");
  greg_glWindowPos3i = (PFNGLWINDOWPOS3IPROC) gregGetProcAddress("glWindowPos3i");
  greg_glWindowPos3iv = (PFNGLWINDOWPOS3IVPROC) gregGetProcAddress("glWindowPos3iv");
  greg_glWindowPos3s = (PFNGLWINDOWPOS3SPROC) gregGetProcAddress("glWindowPos3s");
  greg_glWindowPos3sv = (PFNGLWINDOWPOS3SVPROC) gregGetProcAddress("glWindowPos3sv");


  if (!gregValidContext())
  {
    gregFreeLibrary();
    return GL_FALSE;
  }

  GREG_VERSION_1_0 = gregVersionSupported(1, 0);
  GREG_VERSION_1_1 = gregVersionSupported(1, 1);
  GREG_VERSION_1_2 = gregVersionSupported(1, 2);
  GREG_VERSION_1_3 = gregVersionSupported(1, 3);
  GREG_VERSION_1_4 = gregVersionSupported(1, 4);
  GREG_VERSION_1_5 = gregVersionSupported(1, 5);
  GREG_VERSION_2_0 = gregVersionSupported(2, 0);
  GREG_VERSION_2_1 = gregVersionSupported(2, 1);
  GREG_VERSION_3_0 = gregVersionSupported(3, 0);
  GREG_VERSION_3_1 = gregVersionSupported(3, 1);
  GREG_VERSION_3_2 = gregVersionSupported(3, 2);


  GREG_ARB_texture_float = gregExtensionSupported("GL_ARB_texture_float");
  GREG_EXT_texture_filter_anisotropic = gregExtensionSupported("GL_EXT_texture_filter_anisotropic");
  GREG_KHR_debug = gregExtensionSupported("GL_KHR_debug");


  gregFreeLibrary();
  return GL_TRUE;
}

