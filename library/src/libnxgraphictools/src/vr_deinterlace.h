#ifndef __VR_DEINTERACE__
#define __VR_DEINTERACE__

#include <nx_alloc_mem.h>

extern const char deinterace_vertex_shader[];
extern const char deinterace_frag_shader[];
extern const char scaler_vertex_shader[];
extern const char scaler_frag_shader[];
extern const char cvt2y_vertex_shader[];
extern const char cvt2y_frag_shader[];
extern const char cvt2uv_vertex_shader[];
extern const char cvt2uv_frag_shader[];
extern const char cvt2rgba_vertex_shader[];
extern const char cvt2rgba_frag_shader[];

typedef struct vrSurfaceTarget* HSURFTARGET;
typedef struct vrSurfaceSource* HSURFSOURCE;

int                vrInitializeGLSurface    ( void );

HSURFTARGET vrCreateDeinterlaceTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault );
HSURFSOURCE vrCreateDeinterlaceSource  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data);
void               vrRunDeinterlace           ( HSURFTARGET target, HSURFSOURCE source);
void               vrWaitDeinterlaceDone      ( void );
void               vrDestroyDeinterlaceTarget ( HSURFTARGET target, int iIsDefault );
void               vrDestroyDeinterlaceSource ( HSURFSOURCE source );

HSURFTARGET vrCreateScaleTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault);
HSURFSOURCE vrCreateScaleSource  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data);
void               vrRunScale           	      ( HSURFTARGET target, HSURFSOURCE source);
void               vrWaitScaleDone      ( void );
void               vrDestroyScaleTarget ( HSURFTARGET target, int iIsDefault );
void               vrDestroyScaleSource ( HSURFSOURCE source );

HSURFTARGET vrCreateCvt2YTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault);
HSURFSOURCE vrCreateCvt2YSource  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data);
void               vrRunCvt2Y           	      ( HSURFTARGET target, HSURFSOURCE source);
void               vrWaitCvt2YDone      ( void );
void               vrDestroyCvt2YTarget ( HSURFTARGET target, int iIsDefault );
void               vrDestroyCvt2YSource ( HSURFSOURCE source );

HSURFTARGET vrCreateCvt2UVTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault);
HSURFSOURCE vrCreateCvt2UVSource  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data);
void               vrRunCvt2UV           	      ( HSURFTARGET target, HSURFSOURCE source);
void               vrWaitCvt2UVDone      ( void );
void               vrDestroyCvt2UVTarget ( HSURFTARGET target, int iIsDefault );
void               vrDestroyCvt2UVSource ( HSURFSOURCE source );

HSURFTARGET vrCreateCvt2RgbaTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault);
HSURFSOURCE vrCreateCvt2RgbaSource  (unsigned int uiWidth, unsigned int uiHeight, 
												NX_MEMORY_HANDLE DataY, NX_MEMORY_HANDLE DataU, NX_MEMORY_HANDLE DataV);
void               vrRunCvt2Rgba           	( HSURFTARGET target, HSURFSOURCE source);
void               vrWaitCvt2RgbaDone      ( void );
void               vrDestroyCvt2RgbaTarget ( HSURFTARGET target, int iIsDefault );
void               vrDestroyCvt2RgbaSource ( HSURFSOURCE source );

void               vrTerminateGLSurface     ( void );

#endif  /* __VR_DEINTERACE__ */

