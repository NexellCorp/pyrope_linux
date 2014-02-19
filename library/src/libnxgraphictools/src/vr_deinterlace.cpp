#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "vr_common_def.h"
#include "vr_deinterlace_private.h"
#include "vr_egl_runtime.h"

#include "dbgmsg.h"

static bool    s_Initialized = false;
static Statics s_Statics;
Statics* vrGetStatics( void )
{
	return &s_Statics;
}

namespace 
{
	class AutoBackupCurrentEGL 
	{
	public:
		EGLContext eglCurrentContext;
		EGLSurface eglCurrentSurface[2];
		EGLDisplay eglCurrentDisplay;
		AutoBackupCurrentEGL(void)
		{
			eglCurrentContext    = eglGetCurrentContext();
			eglCurrentSurface[0] = eglGetCurrentSurface(EGL_DRAW);
			eglCurrentSurface[1] = eglGetCurrentSurface(EGL_READ);
			eglCurrentDisplay    = eglGetCurrentDisplay();	
		}
		~AutoBackupCurrentEGL()
		{
			//????
			//eglMakeCurrent(eglCurrentDisplay, eglCurrentSurface[0], eglCurrentSurface[1], eglCurrentContext);
		}
	};
	#define _AUTO_BACKUP_CURRENT_EGL_ AutoBackupCurrentEGL instanceAutoBackupCurrentEGL
};

static int vrInitializeDeinterlace( HSURFTARGET target );
static int vrInitializeScaler( HSURFTARGET target );
static int vrInitializeCvt2Y( HSURFTARGET target );
static int vrInitializeCvt2UV( HSURFTARGET target );
static int vrInitializeCvt2Rgba( HSURFTARGET target );
static int vrDeinitializeDeinterlace( HSURFTARGET target );
static int vrDeinitializeScaler( HSURFTARGET target );
static int vrDeinitializeCvt2Y( HSURFTARGET target );
static int vrDeinitializeCvt2UV( HSURFTARGET target );
static int vrDeinitializeCvt2Rgba( HSURFTARGET target );
static char *loadShader( const char *sFilename);
static int processShader(GLuint *pShader, const char *sFilename, GLint iShaderType);


int VR_GRAPHIC_DBG_OPEN_CLOSE = 0;
int VR_GRAPHIC_DBG_TARGET = 0;
int VR_GRAPHIC_DBG_SOURCE = 0;
int VR_GRAPHIC_DBG_CTX = 0;

static int vrInitializeEGLSurface( void )
{
	int ret;
	Statics* pStatics = &s_Statics;
	if( !pStatics ){ return -1; }

	MEMSET(pStatics, 0x0, sizeof(s_Statics));

	VR_INFO("\n", VR_GRAPHIC_DBG_OPEN_CLOSE, "vrInitializeEGLSurface start <===\n"); 	
	ret = vrInitializeEGLConfig();
	if( ret ){ ErrMsg("Error: vrInitializeEGLSurface at %s:%i\n", __FILE__, __LINE__); return ret; }

	pStatics->default_target_memory[VR_PROGRAM_DEINTERLACE] = NX_AllocateMemory(64*64, 4);		
	pStatics->default_target_memory[VR_PROGRAM_SCALE] = NX_AllocateMemory(64*64, 4);		
	pStatics->default_target_memory[VR_PROGRAM_CVT2Y] = NX_AllocateMemory(64*64*4, 4);		
	pStatics->default_target_memory[VR_PROGRAM_CVT2UV] = NX_AllocateMemory(64*64*4, 4);		
	pStatics->default_target_memory[VR_PROGRAM_CVT2RGBA] = NX_AllocateMemory(64*64*4, 4);		

	VR_INFO("", VR_GRAPHIC_DBG_OPEN_CLOSE, "vrInitializeEGLSurface end\n"); 	

	return 0;
}

int vrInitializeGLSurface( void )
{
	int ret;
	if( s_Initialized )
	{ 
		//ErrMsg("Error: vrInitializeEGLSurface at %s:%i\n", __FILE__, __LINE__); 
		return 0; 
	}
	
	ret = vrInitializeEGLSurface();
	if( ret ){ ErrMsg("Error: vrInitializeGLSurface at %s:%i\n", __FILE__, __LINE__); return ret; }

	Statics* pStatics = &s_Statics;
	if( !pStatics ){ return -1; }
		
	VR_INFO("", VR_GRAPHIC_DBG_OPEN_CLOSE, "vrInitializeGLSurface start <=\n"); 	
	ret = vrInitEglExtensions();
	if( ret ){ ErrMsg("Error: vrInitializeGLSurface at %s:%i\n", __FILE__, __LINE__); return ret; }	

	if(!pStatics->default_target[VR_PROGRAM_DEINTERLACE])
		pStatics->default_target[VR_PROGRAM_DEINTERLACE] = vrCreateDeinterlaceTarget(64, 64, pStatics->default_target_memory[VR_PROGRAM_DEINTERLACE], VR_TRUE);
	if(!pStatics->default_target[VR_PROGRAM_SCALE])
		pStatics->default_target[VR_PROGRAM_SCALE] = vrCreateScaleTarget(64, 64, pStatics->default_target_memory[VR_PROGRAM_SCALE], VR_TRUE);
	if(!pStatics->default_target[VR_PROGRAM_CVT2Y])
		pStatics->default_target[VR_PROGRAM_CVT2Y] = vrCreateCvt2YTarget(64, 64, pStatics->default_target_memory[VR_PROGRAM_CVT2Y], VR_TRUE);
	if(!pStatics->default_target[VR_PROGRAM_CVT2UV])
		pStatics->default_target[VR_PROGRAM_CVT2UV] = vrCreateCvt2UVTarget(64, 64, pStatics->default_target_memory[VR_PROGRAM_CVT2UV], VR_TRUE);
	if(!pStatics->default_target[VR_PROGRAM_CVT2RGBA])
		pStatics->default_target[VR_PROGRAM_CVT2RGBA] = vrCreateCvt2RgbaTarget(64, 64, pStatics->default_target_memory[VR_PROGRAM_CVT2RGBA], VR_TRUE);
	
	for(unsigned int program = 0 ; program < VR_PROGRAM_MAX ; program++)
	{
		pStatics->shader[program].iVertName = 0;
		pStatics->shader[program].iFragName = 0;
		pStatics->shader[program].iProgName = 0;
		pStatics->shader[program].iLocPosition = -1;
		pStatics->shader[program].iLocTexCoord = -1;
		pStatics->shader[program].iLocInputHeight = -1;
		pStatics->shader[program].iLocOutputHeight = -1;
		pStatics->shader[program].iLocMainTex[0] = -1;
		pStatics->shader[program].iLocRefTex = -1;
	}	
 	pStatics->tex_deinterlace_ref_id = 0;

	if(vrInitializeDeinterlace( pStatics->default_target[VR_PROGRAM_DEINTERLACE] ) != 0)
		return -1;
	if(vrInitializeScaler(pStatics->default_target[VR_PROGRAM_SCALE] ) != 0)
		return -1;
	if(vrInitializeCvt2Y(pStatics->default_target[VR_PROGRAM_CVT2Y] ) != 0)
		return -1;
	if(vrInitializeCvt2UV(pStatics->default_target[VR_PROGRAM_CVT2UV] ) != 0)
		return -1;
	if(vrInitializeCvt2Rgba(pStatics->default_target[VR_PROGRAM_CVT2RGBA] ) != 0)
		return -1;
		
	s_Initialized = true;

	VR_INFO("", VR_GRAPHIC_DBG_OPEN_CLOSE, "vrInitializeGLSurface end\n"); 	
	return 0;
}

static void  vrTerminateEGLSurface( void )
{
	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return; }
	NX_FreeMemory( pStatics->default_target_memory[VR_PROGRAM_DEINTERLACE] );
	NX_FreeMemory( pStatics->default_target_memory[VR_PROGRAM_SCALE] );
	NX_FreeMemory( pStatics->default_target_memory[VR_PROGRAM_CVT2Y] );
	NX_FreeMemory( pStatics->default_target_memory[VR_PROGRAM_CVT2UV] );
	NX_FreeMemory( pStatics->default_target_memory[VR_PROGRAM_CVT2RGBA] );
	
	vrTerminateEGL();

	VR_INFO("", VR_GRAPHIC_DBG_OPEN_CLOSE, "vrTerminateEGLSurface done ===>\n"); 
}

void  vrTerminateGLSurface( void )
{	
	if( ! s_Initialized ){ return; }

	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return; }

	VR_INFO("", VR_GRAPHIC_DBG_OPEN_CLOSE, "vrTerminateGLSurface start\n"); 	

	if(vrDeinitializeDeinterlace(pStatics->default_target[VR_PROGRAM_DEINTERLACE]) != 0)
		ErrMsg("Error: vrDeinitializeDeinterlace() %s:%i\n", __FILE__, __LINE__);
	if(vrDeinitializeScaler(pStatics->default_target[VR_PROGRAM_SCALE]) != 0)
		ErrMsg("Error: vrDeinitializeScaler() %s:%i\n", __FILE__, __LINE__);
	if(vrDeinitializeCvt2Y(pStatics->default_target[VR_PROGRAM_CVT2Y]) != 0)
		ErrMsg("Error: vrDeinitializeCvt2Y() %s:%i\n", __FILE__, __LINE__);
	if(vrDeinitializeCvt2UV(pStatics->default_target[VR_PROGRAM_CVT2UV]) != 0)
		ErrMsg("Error: vrDeinitializeCvt2UV() %s:%i\n", __FILE__, __LINE__);
	if(vrDeinitializeCvt2Rgba(pStatics->default_target[VR_PROGRAM_CVT2RGBA]) != 0)
		ErrMsg("Error: vrDeinitializeCvt2Rgba() %s:%i\n", __FILE__, __LINE__);

	for(int i = 0 ; i < VR_PROGRAM_MAX ; i++)
	{
		if(pStatics->egl_info.sEGLContext[i])
			ErrMsg("ERROR: vrTerminateGLSurface(0x%x), idx(%d), ref(%d)\n", (int)pStatics->egl_info.sEGLContext[i], i, pStatics->egl_info.sEGLContextRef[i]);
		VR_ASSERT("ctx_ref must be zero", !pStatics->egl_info.sEGLContextRef[i]);
	}
	
	if(pStatics->default_target[VR_PROGRAM_DEINTERLACE])	
		vrDestroyDeinterlaceTarget( pStatics->default_target[VR_PROGRAM_DEINTERLACE], VR_TRUE );
	if(pStatics->default_target[VR_PROGRAM_SCALE])	
		vrDestroyScaleTarget( pStatics->default_target[VR_PROGRAM_SCALE], VR_TRUE );
	if(pStatics->default_target[VR_PROGRAM_CVT2Y])	
		vrDestroyCvt2YTarget( pStatics->default_target[VR_PROGRAM_CVT2Y], VR_TRUE );
	if(pStatics->default_target[VR_PROGRAM_CVT2UV])	
		vrDestroyCvt2UVTarget( pStatics->default_target[VR_PROGRAM_CVT2UV], VR_TRUE );
	if(pStatics->default_target[VR_PROGRAM_CVT2RGBA])	
		vrDestroyCvt2RgbaTarget( pStatics->default_target[VR_PROGRAM_CVT2RGBA], VR_TRUE );

	pStatics->default_target[VR_PROGRAM_DEINTERLACE] = NULL;
	pStatics->default_target[VR_PROGRAM_SCALE] = NULL;
	pStatics->default_target[VR_PROGRAM_CVT2Y] = NULL;
	pStatics->default_target[VR_PROGRAM_CVT2UV] = NULL;
	pStatics->default_target[VR_PROGRAM_CVT2RGBA] = NULL;	
	VR_INFO("", VR_GRAPHIC_DBG_OPEN_CLOSE, "vrTerminateGLSurface end =>\n"); 

	vrTerminateEGLSurface();

	s_Initialized = false;	
}

struct vrSurfaceTarget
{
	unsigned int        width         ;
	unsigned int        height        ;
	EGLNativePixmapType native_pixmap ;
	EGLSurface          pixmap_surface;
} ;

static void vrDestroySurfaceTarget ( HSURFTARGET target)
{
	Statics *pStatics = vrGetStatics();
	if( target->pixmap_surface ) { EGL_CHECK(eglDestroySurface(pStatics->egl_info.sEGLDisplay,target->pixmap_surface)); }
	if( target->native_pixmap  ) { vrDestroyPixmap(target->native_pixmap); }
	FREE(target);
}

static void vrResetShaderInfo(Shader* pShader)
{
	pShader->iVertName = -1;
	pShader->iFragName = -1;
	pShader->iProgName = -1;
}

HSURFTARGET vrCreateDeinterlaceTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault)
{
	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFTARGET)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFTARGET result = (HSURFTARGET)MALLOC(sizeof(vrSurfaceTarget));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFTARGET)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceTarget) );

	/* Create a EGLNativePixmapType. */
	EGLNativePixmapType pixmap_output = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_TRUE, 32, VR_TRUE);
	if(pixmap_output == NULL || ((fbdev_pixmap*)pixmap_output)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		FREE( result );
		return (HSURFTARGET)0;
	}

	/* Create a EGLSurface. */
	EGLSurface surface = eglCreatePixmapSurface(pStatics->egl_info.sEGLDisplay, pStatics->egl_info.sEGLConfig[VR_PROGRAM_DEINTERLACE], (EGLNativePixmapType)pixmap_output, NULL);
	if(surface == EGL_NO_SURFACE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to create EGL surface at %s:%i\n", __FILE__, __LINE__);
		vrDestroyPixmap(pixmap_output);
		FREE( result );
		return (HSURFTARGET)0;
	}

	result->width          = uiWidth ;
	result->height         = uiHeight;
	result->native_pixmap  = pixmap_output;
	result->pixmap_surface = surface;	
	
	/* Increase Ctx ref. */
	if(!iIsDefault) ++pStatics->egl_info.sEGLContextRef[VR_PROGRAM_DEINTERLACE];
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrCreateDeinterlaceTarget, 32ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_DEINTERLACE]); 	
	
	return result;
}

void vrDestroyDeinterlaceTarget ( HSURFTARGET target, int iIsDefault )
{
	if( !target ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;

	vrDestroySurfaceTarget(target);
	
	/* Decrease Ctx ref. */
	if(!iIsDefault) 
	{
		VR_ASSERT("Ref must be greater than 0", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_DEINTERLACE] > 0);
		--pStatics->egl_info.sEGLContextRef[VR_PROGRAM_DEINTERLACE];
	}
	
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrDestroyDeinterlaceTarget done, 32ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_DEINTERLACE]); 			
}

HSURFTARGET vrCreateScaleTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault)
{
	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFTARGET)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFTARGET result = (HSURFTARGET)MALLOC(sizeof(vrSurfaceTarget));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFTARGET)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceTarget) );

	/* Create a EGLNativePixmapType. */
	EGLNativePixmapType pixmap_output = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_TRUE, 8, VR_TRUE);
	if(pixmap_output == NULL || ((fbdev_pixmap*)pixmap_output)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		FREE( result );
		return (HSURFTARGET)0;
	}
	
	/* Create a EGLSurface. */
	EGLSurface surface = eglCreatePixmapSurface(pStatics->egl_info.sEGLDisplay, pStatics->egl_info.sEGLConfig[VR_PROGRAM_SCALE], (EGLNativePixmapType)pixmap_output, NULL);
	if(surface == EGL_NO_SURFACE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to create EGL surface at %s:%i\n", __FILE__, __LINE__);
		vrDestroyPixmap(pixmap_output);
		FREE( result );
		return (HSURFTARGET)0;
	}

	result->width          = uiWidth ;
	result->height         = uiHeight;
	result->native_pixmap  = pixmap_output;
	result->pixmap_surface = surface;	

	/* Increase Ctx ref. */
	if(!iIsDefault) ++pStatics->egl_info.sEGLContextRef[VR_PROGRAM_SCALE];
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrCreateScaleTarget, 8ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_SCALE]); 	
	
	return result;
}

void vrDestroyScaleTarget ( HSURFTARGET target, int iIsDefault )
{
	if( !target ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
			
	vrDestroySurfaceTarget(target);

	/* Decrease Ctx ref. */
	if(!iIsDefault)
	{
		VR_ASSERT("Ref must be greater than 0", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_SCALE] > 0);
		--pStatics->egl_info.sEGLContextRef[VR_PROGRAM_SCALE];
	}
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrDestroyScaleTarget done, 8ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_SCALE]); 		
}

HSURFTARGET vrCreateCvt2YTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault)
{
	//ErrMsg("vrCreateCvt2YTarget start(%dx%d)\n", uiWidth, uiHeight);

	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFTARGET)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFTARGET result = (HSURFTARGET)MALLOC(sizeof(vrSurfaceTarget));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFTARGET)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceTarget) );

	/* Create a EGLNativePixmapType. */
	EGLNativePixmapType pixmap_output = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_TRUE, 8, VR_TRUE);
	if(pixmap_output == NULL || ((fbdev_pixmap*)pixmap_output)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		FREE( result );
		return (HSURFTARGET)0;
	}
	
	/* Create a EGLSurface. */
	EGLSurface surface = eglCreatePixmapSurface(pStatics->egl_info.sEGLDisplay, pStatics->egl_info.sEGLConfig[VR_PROGRAM_CVT2Y], (EGLNativePixmapType)pixmap_output, NULL);
	if(surface == EGL_NO_SURFACE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to create EGL surface at %s:%i\n", __FILE__, __LINE__);
		vrDestroyPixmap(pixmap_output);
		FREE( result );
		return (HSURFTARGET)0;
	}

	result->width          = uiWidth ;
	result->height         = uiHeight;
	result->native_pixmap  = pixmap_output;
	result->pixmap_surface = surface;	

	/* Increase Ctx ref. */
	if(!iIsDefault) ++pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2Y];
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrCreateCvt2YTarget, 8ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2Y]); 	
	
	return result;
}

void vrDestroyCvt2YTarget ( HSURFTARGET target, int iIsDefault )
{
	if( !target ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
	
	vrDestroySurfaceTarget(target);

	/* Decrease Ctx ref. */
	if(!iIsDefault)
	{
		VR_ASSERT("Ref must be greater than 0", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2Y] > 0);
		--pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2Y];
	}	
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrDestroyCvt2YTarget done, 8ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2Y]); 		
}

HSURFTARGET vrCreateCvt2UVTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault)
{
	//ErrMsg("vrCreateCvt2UVTarget start(%dx%d)\n", uiWidth, uiHeight);

	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFTARGET)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFTARGET result = (HSURFTARGET)MALLOC(sizeof(vrSurfaceTarget));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFTARGET)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceTarget) );

	//4pixel마다 UV존재
	uiWidth /= 2;
	uiHeight /= 2;
	
	/* Create a EGLNativePixmapType. */
	EGLNativePixmapType pixmap_output = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_TRUE, 16, VR_TRUE);
	if(pixmap_output == NULL || ((fbdev_pixmap*)pixmap_output)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		FREE( result );
		return (HSURFTARGET)0;
	}
	
	/* Create a EGLSurface. */
	EGLSurface surface = eglCreatePixmapSurface(pStatics->egl_info.sEGLDisplay, pStatics->egl_info.sEGLConfig[VR_PROGRAM_CVT2UV], (EGLNativePixmapType)pixmap_output, NULL);
	if(surface == EGL_NO_SURFACE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to create EGL surface at %s:%i\n", __FILE__, __LINE__);
		vrDestroyPixmap(pixmap_output);
		FREE( result );
		return (HSURFTARGET)0;
	}

	result->width          = uiWidth ;
	result->height         = uiHeight;
	result->native_pixmap  = pixmap_output;
	result->pixmap_surface = surface;	

	/* Increase Ctx ref. */
	if(!iIsDefault) ++pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2UV];
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrCreateCvt2UVTarget, 16ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2UV]); 		
	
	return result;
}

void vrDestroyCvt2UVTarget ( HSURFTARGET target, int iIsDefault )
{
	if( !target ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
	
	vrDestroySurfaceTarget(target);

	/* Decrease Ctx ref. */
	if(!iIsDefault)
	{
		VR_ASSERT("Ref must be greater than 0", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2UV] > 0);
		--pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2UV];		
	}	
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrDestroyCvt2UVTarget done, 16ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2UV]); 		
}

HSURFTARGET vrCreateCvt2RgbaTarget  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data, int iIsDefault)
{
	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFTARGET)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFTARGET result = (HSURFTARGET)MALLOC(sizeof(vrSurfaceTarget));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFTARGET)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceTarget) );

	/* Create a EGLNativePixmapType. */
	EGLNativePixmapType pixmap_output = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_TRUE, 32, VR_FALSE);
	if(pixmap_output == NULL || ((fbdev_pixmap*)pixmap_output)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		FREE( result );
		return (HSURFTARGET)0;
	}
	
	/* Create a EGLSurface. */
	EGLSurface surface = eglCreatePixmapSurface(pStatics->egl_info.sEGLDisplay, pStatics->egl_info.sEGLConfig[VR_PROGRAM_CVT2RGBA], (EGLNativePixmapType)pixmap_output, NULL);
	if(surface == EGL_NO_SURFACE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to create EGL surface at %s:%i\n", __FILE__, __LINE__);
		vrDestroyPixmap(pixmap_output);
		FREE( result );
		return (HSURFTARGET)0;
	}

	result->width          = uiWidth ;
	result->height         = uiHeight;
	result->native_pixmap  = pixmap_output;
	result->pixmap_surface = surface;	

	/* Increase Ctx ref. */
	if(!iIsDefault) ++pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2RGBA];
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrCreateCvt2RgbaTarget, 32ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2RGBA]); 		
	
	return result;
}

void vrDestroyCvt2RgbaTarget ( HSURFTARGET target, int iIsDefault )
{
	if( !target ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
		
	vrDestroySurfaceTarget(target);

	/* Decrease Ctx ref. */
	if(!iIsDefault)
	{
		VR_ASSERT("Ref must be greater than 0", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2RGBA] > 0);
		--pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2RGBA];
	}
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "vrDestroyCvt2RgbaTarget done, 32ctx ref(%d)\n", pStatics->egl_info.sEGLContextRef[VR_PROGRAM_CVT2RGBA]); 			
}

struct vrSurfaceSource
{
	unsigned int        width        ;
	unsigned int        height       ;
	GLuint              texture_name[VR_INPUT_MODE_YUV_MAX] ;
	EGLNativePixmapType native_pixmap[VR_INPUT_MODE_YUV_MAX];
	EGLImageKHR         pixmap_image[VR_INPUT_MODE_YUV_MAX];
};

HSURFSOURCE vrCreateDeinterlaceSource  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data)
{
	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFSOURCE)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFSOURCE result = (HSURFSOURCE)MALLOC(sizeof(struct vrSurfaceSource));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFSOURCE)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceSource) );
	
	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, pStatics->default_target[VR_PROGRAM_DEINTERLACE]->pixmap_surface, 
								pStatics->default_target[VR_PROGRAM_DEINTERLACE]->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_DEINTERLACE]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return 0;	
	}

	EGLNativePixmapType pixmapInput = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_TRUE, 32, VR_TRUE);
	if(pixmapInput == NULL || ((fbdev_pixmap*)pixmapInput)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);\
		FREE(result);
		return (HSURFSOURCE)0;
	}

	//RGB is not supported	
	EGLint imageAttributes[] = {
		EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, 
		EGL_NONE
	};	
	EGLImageKHR eglImage = EGL_CHECK(_eglCreateImageKHR( pStatics->egl_info.sEGLDisplay, EGL_NO_CONTEXT, 
							           EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmapInput, imageAttributes));	

	GLuint textureName;
	GL_CHECK(glGenTextures(1, &textureName));
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_DEINTERLACE));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureName));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(_glEGLImageTargetTexture2DOES( GL_TEXTURE_2D, (GLeglImageOES)eglImage));		

	result->width        = uiWidth ;
	result->height       = uiHeight;
	result->texture_name[0] = textureName;
	result->native_pixmap[0]= pixmapInput;
	result->pixmap_image[0] = eglImage   ;	

	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrCreateDeinterlaceSource done\n"); 		
	return result;
}

void vrDestroyDeinterlaceSource ( HSURFSOURCE source )
{
	if( !source ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
	GL_CHECK(glDeleteTextures(1,&source->texture_name[0]));
	EGL_CHECK(_eglDestroyImageKHR(pStatics->egl_info.sEGLDisplay, source->pixmap_image[0]));
	vrDestroyPixmap(source->native_pixmap[0]);
	FREE(source);
	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrDestroyDeinterlaceSource done\n"); 		
}

HSURFSOURCE vrCreateScaleSource  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data)
{
	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFSOURCE)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFSOURCE result = (HSURFSOURCE)MALLOC(sizeof(struct vrSurfaceSource));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFSOURCE)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceSource) );

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, pStatics->default_target[VR_PROGRAM_SCALE]->pixmap_surface, 
									pStatics->default_target[VR_PROGRAM_SCALE]->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_SCALE]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return 0;	
	}

	EGLNativePixmapType pixmapInput = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_TRUE, 8, VR_TRUE);
	if(pixmapInput == NULL || ((fbdev_pixmap*)pixmapInput)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);\
		FREE(result);
		return (HSURFSOURCE)0;
	}

	//RGB is not supported	
	EGLint imageAttributes[] = {
		EGL_IMAGE_PRESERVED_KHR, /*EGL_TRUE*/EGL_FALSE, 
		EGL_NONE
	};	
	EGLImageKHR eglImage = EGL_CHECK(_eglCreateImageKHR( pStatics->egl_info.sEGLDisplay, EGL_NO_CONTEXT, 
							           EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmapInput, imageAttributes));	

	GLuint textureName;
	GL_CHECK(glGenTextures(1, &textureName));
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_NEAREST));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureName));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(_glEGLImageTargetTexture2DOES( GL_TEXTURE_2D, (GLeglImageOES)eglImage));		

	result->width        = uiWidth ;
	result->height       = uiHeight;
	result->texture_name[0] = textureName;
	result->native_pixmap[0]= pixmapInput;
	result->pixmap_image[0] = eglImage   ;				 

	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrCreateScaleSource done\n"); 		
	return result;
}

void vrDestroyScaleSource ( HSURFSOURCE source )
{
	if( !source ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
	GL_CHECK(glDeleteTextures(1,&source->texture_name[0]));
	EGL_CHECK(_eglDestroyImageKHR(pStatics->egl_info.sEGLDisplay, source->pixmap_image[0]));
	vrDestroyPixmap(source->native_pixmap[0]);
	FREE(source);
	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrDestroyScaleSource done\n"); 		
}

HSURFSOURCE vrCreateCvt2YSource  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data)
{
	//ErrMsg("vrCreateCvt2YSource start(%dx%d)\n", uiWidth, uiHeight);

	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFSOURCE)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFSOURCE result = (HSURFSOURCE)MALLOC(sizeof(struct vrSurfaceSource));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFSOURCE)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceSource) );
	
	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, pStatics->default_target[VR_PROGRAM_CVT2Y]->pixmap_surface, 
								pStatics->default_target[VR_PROGRAM_CVT2Y]->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2Y]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return 0;	
	}

	EGLNativePixmapType pixmapInput = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_FALSE, 32, VR_FALSE);
	if(pixmapInput == NULL || ((fbdev_pixmap*)pixmapInput)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);\
		FREE(result);
		return (HSURFSOURCE)0;
	}

	//RGB is not supported	
	EGLint imageAttributes[] = {
		EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, 
		EGL_NONE
	};	
	EGLImageKHR eglImage = EGL_CHECK(_eglCreateImageKHR( pStatics->egl_info.sEGLDisplay, EGL_NO_CONTEXT, 
							           EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmapInput, imageAttributes));	

	GLuint textureName;
	GL_CHECK(glGenTextures(1, &textureName));
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_NEAREST));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureName));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(_glEGLImageTargetTexture2DOES( GL_TEXTURE_2D, (GLeglImageOES)eglImage));		

	result->width        = uiWidth ;
	result->height       = uiHeight;
	result->texture_name[0] = textureName;
	result->native_pixmap[0]= pixmapInput;
	result->pixmap_image[0] = eglImage   ;	

	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrCreateCvt2YSource done\n"); 		
	return result;
}

void vrDestroyCvt2YSource ( HSURFSOURCE source )
{
	if( !source ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
	GL_CHECK(glDeleteTextures(1,&source->texture_name[0]));
	EGL_CHECK(_eglDestroyImageKHR(pStatics->egl_info.sEGLDisplay, source->pixmap_image[0]));
	vrDestroyPixmap(source->native_pixmap[0]);
	FREE(source);
	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrDestroyCvt2YSource done\n"); 		
}

HSURFSOURCE vrCreateCvt2UVSource  (unsigned int uiWidth, unsigned int uiHeight, NX_MEMORY_HANDLE Data)
{
	//ErrMsg("vrCreateCvt2UVSource start(%dx%d)\n", uiWidth, uiHeight);

	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFSOURCE)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFSOURCE result = (HSURFSOURCE)MALLOC(sizeof(struct vrSurfaceSource));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFSOURCE)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceSource) );
	
	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, pStatics->default_target[VR_PROGRAM_CVT2UV]->pixmap_surface, 
								pStatics->default_target[VR_PROGRAM_CVT2UV]->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2UV]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return 0;	
	}

	EGLNativePixmapType pixmapInput = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, Data, VR_FALSE, 32, VR_FALSE);
	if(pixmapInput == NULL || ((fbdev_pixmap*)pixmapInput)->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);\
		FREE(result);
		return (HSURFSOURCE)0;
	}

	//RGB is not supported	
	EGLint imageAttributes[] = {
		EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, 
		EGL_NONE
	};	
	EGLImageKHR eglImage = EGL_CHECK(_eglCreateImageKHR( pStatics->egl_info.sEGLDisplay, EGL_NO_CONTEXT, 
							           EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmapInput, imageAttributes));	

	GLuint textureName;
	GL_CHECK(glGenTextures(1, &textureName));
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_LINEAR));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureName));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(_glEGLImageTargetTexture2DOES( GL_TEXTURE_2D, (GLeglImageOES)eglImage));		

	result->width        = uiWidth ;
	result->height       = uiHeight;
	result->texture_name[0] = textureName;
	result->native_pixmap[0]= pixmapInput;
	result->pixmap_image[0] = eglImage   ;	

	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrCreateCvt2UVSource done\n"); 		
	return result;
}

void vrDestroyCvt2UVSource ( HSURFSOURCE source )
{
	if( !source ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
	GL_CHECK(glDeleteTextures(1,&source->texture_name[0]));
	EGL_CHECK(_eglDestroyImageKHR(pStatics->egl_info.sEGLDisplay, source->pixmap_image[0]));
	vrDestroyPixmap(source->native_pixmap[0]);
	FREE(source);
	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrDestroyCvt2UVSource done\n"); 		
}

HSURFSOURCE vrCreateCvt2RgbaSource  (unsigned int uiWidth, unsigned int uiHeight, 
								NX_MEMORY_HANDLE DataY, NX_MEMORY_HANDLE DataU, NX_MEMORY_HANDLE DataV)
{
	Statics *pStatics = vrGetStatics();
	if( !pStatics ){ return (HSURFSOURCE)0; }
	_AUTO_BACKUP_CURRENT_EGL_;
	HSURFSOURCE result = (HSURFSOURCE)MALLOC(sizeof(struct vrSurfaceSource));
	if( !result )
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return (HSURFSOURCE)0;
	}
	MEMSET( result, 0, sizeof(struct vrSurfaceSource) );
	
	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, pStatics->default_target[VR_PROGRAM_CVT2RGBA]->pixmap_surface, 
								pStatics->default_target[VR_PROGRAM_CVT2RGBA]->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2RGBA]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return 0;	
	}

	EGLNativePixmapType pixmapInput[VR_INPUT_MODE_YUV_MAX] = {NULL,};
	pixmapInput[VR_INPUT_MODE_Y] = (fbdev_pixmap*)vrCreatePixmap(uiWidth, uiHeight, DataY, VR_TRUE, 8, VR_FALSE);
	if(pixmapInput[VR_INPUT_MODE_Y] == NULL || ((fbdev_pixmap*)pixmapInput[VR_INPUT_MODE_Y])->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);\
		FREE(result);
		return (HSURFSOURCE)0;
	}
	pixmapInput[VR_INPUT_MODE_U] = (fbdev_pixmap*)vrCreatePixmap(uiWidth/2, uiHeight/2, DataU, VR_TRUE, 8, VR_FALSE);
	if(pixmapInput[VR_INPUT_MODE_U] == NULL || ((fbdev_pixmap*)pixmapInput[VR_INPUT_MODE_U])->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);\
		FREE(result);
		return (HSURFSOURCE)0;
	}
	pixmapInput[VR_INPUT_MODE_V] = (fbdev_pixmap*)vrCreatePixmap(uiWidth/2, uiHeight/2, DataV, VR_TRUE, 8, VR_FALSE);
	if(pixmapInput[VR_INPUT_MODE_V] == NULL || ((fbdev_pixmap*)pixmapInput[VR_INPUT_MODE_V])->data == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);\
		FREE(result);
		return (HSURFSOURCE)0;
	}

	//RGB is not supported	
	EGLint imageAttributes[] = {
		EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, 
		EGL_NONE
	};	
	EGLImageKHR eglImage[VR_INPUT_MODE_YUV_MAX] = {NULL,};
	eglImage[VR_INPUT_MODE_Y] = EGL_CHECK(_eglCreateImageKHR( pStatics->egl_info.sEGLDisplay, EGL_NO_CONTEXT, 
							           EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmapInput[VR_INPUT_MODE_Y], imageAttributes));	
	eglImage[VR_INPUT_MODE_U] = EGL_CHECK(_eglCreateImageKHR( pStatics->egl_info.sEGLDisplay, EGL_NO_CONTEXT, 
							           EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmapInput[VR_INPUT_MODE_U], imageAttributes));	
	eglImage[VR_INPUT_MODE_V] = EGL_CHECK(_eglCreateImageKHR( pStatics->egl_info.sEGLDisplay, EGL_NO_CONTEXT, 
							           EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmapInput[VR_INPUT_MODE_V], imageAttributes));	

	GLuint textureName[VR_INPUT_MODE_YUV_MAX];
	GL_CHECK(glGenTextures(VR_INPUT_MODE_YUV_MAX, textureName));
	
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_Y));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureName[VR_INPUT_MODE_Y]));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(_glEGLImageTargetTexture2DOES( GL_TEXTURE_2D, (GLeglImageOES)eglImage[VR_INPUT_MODE_Y]));		
	result->texture_name[VR_INPUT_MODE_Y] = textureName[VR_INPUT_MODE_Y];
	result->native_pixmap[VR_INPUT_MODE_Y]= pixmapInput[VR_INPUT_MODE_Y];
	result->pixmap_image[VR_INPUT_MODE_Y] = eglImage[VR_INPUT_MODE_Y]   ;				 

	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_U));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureName[VR_INPUT_MODE_U]));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(_glEGLImageTargetTexture2DOES( GL_TEXTURE_2D, (GLeglImageOES)eglImage[VR_INPUT_MODE_U]));		
	result->texture_name[VR_INPUT_MODE_U] = textureName[VR_INPUT_MODE_U];
	result->native_pixmap[VR_INPUT_MODE_U]= pixmapInput[VR_INPUT_MODE_U];
	result->pixmap_image[VR_INPUT_MODE_U] = eglImage[VR_INPUT_MODE_U]   ;				 

	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_V));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureName[VR_INPUT_MODE_V]));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(_glEGLImageTargetTexture2DOES( GL_TEXTURE_2D, (GLeglImageOES)eglImage[VR_INPUT_MODE_V]));		
	result->texture_name[VR_INPUT_MODE_V] = textureName[VR_INPUT_MODE_V];
	result->native_pixmap[VR_INPUT_MODE_V]= pixmapInput[VR_INPUT_MODE_V];
	result->pixmap_image[VR_INPUT_MODE_V] = eglImage[VR_INPUT_MODE_V]   ;				 
	
	result->width        = uiWidth ;
	result->height       = uiHeight;

	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrCreateCvt2RgbaSource done\n"); 		
	return result;
}

void vrDestroyCvt2RgbaSource ( HSURFSOURCE source )
{
	if( !source ){ return; }
	Statics *pStatics = vrGetStatics();
	_AUTO_BACKUP_CURRENT_EGL_;
	GL_CHECK(glDeleteTextures(VR_INPUT_MODE_YUV_MAX, source->texture_name));
	EGL_CHECK(_eglDestroyImageKHR(pStatics->egl_info.sEGLDisplay, source->pixmap_image[VR_INPUT_MODE_Y]));
	EGL_CHECK(_eglDestroyImageKHR(pStatics->egl_info.sEGLDisplay, source->pixmap_image[VR_INPUT_MODE_U]));
	EGL_CHECK(_eglDestroyImageKHR(pStatics->egl_info.sEGLDisplay, source->pixmap_image[VR_INPUT_MODE_V]));
	vrDestroyPixmap(source->native_pixmap[VR_INPUT_MODE_Y]);
	vrDestroyPixmap(source->native_pixmap[VR_INPUT_MODE_U]);
	vrDestroyPixmap(source->native_pixmap[VR_INPUT_MODE_V]);
	FREE(source);
	VR_INFO("", VR_GRAPHIC_DBG_SOURCE, "vrDestroyCvt2RgbaSource done\n"); 		
}


void  vrRunDeinterlace( HSURFTARGET target, HSURFSOURCE source)
{
	Statics* pStatics = vrGetStatics();
	Shader* pshader = &(vrGetStatics()->shader[VR_PROGRAM_DEINTERLACE]);	
	const float aSquareVertex[] =
	{
		-1.0f,	-1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0f,	-1.0f, 0.0f,
	};	
	const float aSquareTexCoord[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	if( NULL == pStatics || NULL == pshader || NULL == target || NULL == source )
	{
		ErrMsg("Error: NULL output surface at %s:%i\n", __FILE__, __LINE__);
		return;
	}
	_AUTO_BACKUP_CURRENT_EGL_;

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_DEINTERLACE]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return;	
	}
	GL_CHECK(glUseProgram(pshader->iProgName));
	GL_CHECK(glViewport(0,0,((fbdev_pixmap *)target->native_pixmap)->width,((fbdev_pixmap *)target->native_pixmap)->height));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // To optimize for tile-based renderer
	GL_CHECK(glVertexAttribPointer(pshader->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, aSquareVertex));
	GL_CHECK(glVertexAttribPointer(pshader->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aSquareTexCoord));

	GL_CHECK(glEnableVertexAttribArray(pshader->iLocPosition));
	GL_CHECK(glEnableVertexAttribArray(pshader->iLocTexCoord));

    GL_CHECK(glUniform1f(pshader->iLocInputHeight, source->height));
    //GL_CHECK(glUniform1f(pStatics->shader[program].iLocOutputHeight, output_height));
	GL_CHECK(glUniform1i(pshader->iLocMainTex[0], VR_INPUT_MODE_DEINTERLACE));
	GL_CHECK(glUniform1i(pshader->iLocRefTex, VR_INPUT_MODE_DEINTERLACE_REF));

	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_DEINTERLACE));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, source->texture_name[0]));

	//DbgMsg( "draw Deinterlace\n" ); 
	GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
	EGL_CHECK(eglSwapBuffers(pStatics->egl_info.sEGLDisplay, target->pixmap_surface));
}

void vrRunScale( HSURFTARGET target, HSURFSOURCE source)
{
	Statics* pStatics = vrGetStatics();
	Shader* pshader = &(vrGetStatics()->shader[VR_PROGRAM_SCALE]);	
	const float aSquareVertex[] =
	{
		-1.0f,	-1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0f,	-1.0f, 0.0f,
	};	
	const float aSquareTexCoord[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	if( NULL == pStatics || NULL == pshader || NULL == target || NULL == source )
	{
		ErrMsg("Error: NULL output surface at %s:%i\n", __FILE__, __LINE__);
		return;
	}
	_AUTO_BACKUP_CURRENT_EGL_;

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_SCALE]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return; 
	}

	GL_CHECK(glUseProgram(pshader->iProgName));
	GL_CHECK(glViewport(0,0,((fbdev_pixmap *)target->native_pixmap)->width,((fbdev_pixmap *)target->native_pixmap)->height));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // To optimize for tile-based renderer
	GL_CHECK(glVertexAttribPointer(pshader->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, aSquareVertex));
	GL_CHECK(glVertexAttribPointer(pshader->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aSquareTexCoord));

	GL_CHECK(glEnableVertexAttribArray(pshader->iLocPosition));
	GL_CHECK(glEnableVertexAttribArray(pshader->iLocTexCoord));

	GL_CHECK(glUniform1i(pshader->iLocMainTex[0], VR_INPUT_MODE_NEAREST));

	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_NEAREST));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, source->texture_name[0]));

	//DbgMsg( "draw scaler\n" );
	GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
	//EGL_CHECK(eglSwapBuffers(pStatics->egl_info.sEGLDisplay, target->pixmap_surface));
}

void  vrRunCvt2Y( HSURFTARGET target, HSURFSOURCE source)
{
	Statics* pStatics = vrGetStatics();
	Shader* pshader = &(vrGetStatics()->shader[VR_PROGRAM_CVT2Y]);	
	const float aSquareVertex[] =
	{
		-1.0f,	-1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0f,	-1.0f, 0.0f,
	};	
	const float aSquareTexCoord[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	if( NULL == pStatics || NULL == pshader || NULL == target || NULL == source )
	{
		ErrMsg("Error: NULL output surface at %s:%i\n", __FILE__, __LINE__);
		return;
	}
	_AUTO_BACKUP_CURRENT_EGL_;

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2Y]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return;	
	}
	GL_CHECK(glUseProgram(pshader->iProgName));
	GL_CHECK(glViewport(0,0,((fbdev_pixmap *)target->native_pixmap)->width,((fbdev_pixmap *)target->native_pixmap)->height));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // To optimize for tile-based renderer
	GL_CHECK(glVertexAttribPointer(pshader->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, aSquareVertex));
	GL_CHECK(glVertexAttribPointer(pshader->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aSquareTexCoord));

	GL_CHECK(glEnableVertexAttribArray(pshader->iLocPosition));
	GL_CHECK(glEnableVertexAttribArray(pshader->iLocTexCoord));

	GL_CHECK(glUniform1i(pshader->iLocMainTex[0], VR_INPUT_MODE_NEAREST));

	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_NEAREST));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, source->texture_name[0]));

	//DbgMsg( "draw\n" ); 
	GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
	//EGL_CHECK(eglSwapBuffers(pStatics->egl_info.sEGLDisplay, target->pixmap_surface));
}

void  vrRunCvt2UV( HSURFTARGET target, HSURFSOURCE source)
{
	//DbgMsg( "vrRunCvt2UV start\n" ); 

	Statics* pStatics = vrGetStatics();
	Shader* pshader = &(vrGetStatics()->shader[VR_PROGRAM_CVT2UV]);	
	const float aSquareVertex[] =
	{
		-1.0f,	-1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0f,	-1.0f, 0.0f,
	};	
	const float aSquareTexCoord[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	if( NULL == pStatics || NULL == pshader || NULL == target || NULL == source )
	{
		ErrMsg("Error: NULL output surface at %s:%i\n", __FILE__, __LINE__);
		return;
	}
	_AUTO_BACKUP_CURRENT_EGL_;

	/* Make context current. */
	GL_CHECK();
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2UV]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return;	
	}
	EGL_CHECK();
	GL_CHECK();
	GL_CHECK(glUseProgram(pshader->iProgName));
	GL_CHECK(glViewport(0,0,((fbdev_pixmap *)target->native_pixmap)->width,((fbdev_pixmap *)target->native_pixmap)->height));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // To optimize for tile-based renderer
	GL_CHECK(glVertexAttribPointer(pshader->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, aSquareVertex));
	GL_CHECK(glVertexAttribPointer(pshader->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aSquareTexCoord));

	GL_CHECK(glEnableVertexAttribArray(pshader->iLocPosition));
	GL_CHECK(glEnableVertexAttribArray(pshader->iLocTexCoord));

	GL_CHECK(glUniform1i(pshader->iLocMainTex[0], VR_INPUT_MODE_LINEAR));

	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_LINEAR));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, source->texture_name[0]));

	GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
	//EGL_CHECK(eglSwapBuffers(pStatics->egl_info.sEGLDisplay, target->pixmap_surface));
}

void  vrRunCvt2Rgba( HSURFTARGET target, HSURFSOURCE source)
{
	Statics* pStatics = vrGetStatics();
	Shader* pshader = &(vrGetStatics()->shader[VR_PROGRAM_CVT2RGBA]);	
	const float aSquareVertex[] =
	{
		-1.0f,	-1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f,
		 1.0f,	-1.0f, 0.0f,
	};	
	const float aSquareTexCoord[] =
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	if( NULL == pStatics || NULL == pshader || NULL == target || NULL == source )
	{
		ErrMsg("Error: NULL output surface at %s:%i\n", __FILE__, __LINE__);
		return;
	}
	_AUTO_BACKUP_CURRENT_EGL_;

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2RGBA]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return;	
	}
	GL_CHECK(glUseProgram(pshader->iProgName));
	GL_CHECK(glViewport(0,0,((fbdev_pixmap *)target->native_pixmap)->width,((fbdev_pixmap *)target->native_pixmap)->height));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // To optimize for tile-based renderer
	GL_CHECK(glVertexAttribPointer(pshader->iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, aSquareVertex));
	GL_CHECK(glVertexAttribPointer(pshader->iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, aSquareTexCoord));

	GL_CHECK(glEnableVertexAttribArray(pshader->iLocPosition));
	GL_CHECK(glEnableVertexAttribArray(pshader->iLocTexCoord));

    //GL_CHECK(glUniform1f(pStatics->shader[program].iLocOutputHeight, output_height));
	GL_CHECK(glUniform1i(pshader->iLocMainTex[VR_INPUT_MODE_Y], VR_INPUT_MODE_Y));
	GL_CHECK(glUniform1i(pshader->iLocMainTex[VR_INPUT_MODE_U], VR_INPUT_MODE_U));
	GL_CHECK(glUniform1i(pshader->iLocMainTex[VR_INPUT_MODE_V], VR_INPUT_MODE_V));

	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_Y));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, source->texture_name[VR_INPUT_MODE_Y]));
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_U));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, source->texture_name[VR_INPUT_MODE_U]));
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_V));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, source->texture_name[VR_INPUT_MODE_V]));

	//DbgMsg( "draw\n" ); 
	GL_CHECK(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
	//EGL_CHECK(eglSwapBuffers(pStatics->egl_info.sEGLDisplay, target->pixmap_surface));
}

void  vrWaitDeinterlaceDone ( void )
{
	EGL_CHECK(eglWaitGL());
}

void  vrWaitScaleDone ( void )
{
	EGL_CHECK(eglWaitGL());
}

void  vrWaitCvt2YDone ( void )
{
	EGL_CHECK(eglWaitGL());
}

void  vrWaitCvt2UVDone ( void )
{
	EGL_CHECK(eglWaitGL());
}

void  vrWaitCvt2RgbaDone ( void )
{
	EGL_CHECK(eglWaitGL());
}

#ifdef VR_FEATURE_SHADER_FILE_USE
/* loadShader():	Load the shader source into memory.
 *
 * sFilename: String holding filename to load.
 */
static char *loadShader(const char *sFilename)
{
	char *pResult = NULL;
	FILE *pFile = NULL;
	long iLen = 0;

	pFile = fopen(sFilename, "r");
	if(pFile == NULL) {
		ErrMsg("Error: Cannot read file '%s'\n", sFilename);
		return NULL;
	}
	fseek(pFile, 0, SEEK_END); /* Seek end of file. */
	iLen = ftell(pFile);
	fseek(pFile, 0, SEEK_SET); /* Seek start of file again. */
	pResult = (char*)CALLOC(iLen+1, sizeof(char));
	if(pResult == NULL)
	{
		ErrMsg("Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		return NULL;
	}
	fread(pResult, sizeof(char), iLen, pFile);
	pResult[iLen] = '\0';
	fclose(pFile);

	return pResult;
}

/* processShader(): Create shader, load in source, compile, dump debug as necessary.
 *
 * pShader: Pointer to return created shader ID.
 * sFilename: Passed-in filename from which to load shader source.
 * iShaderType: Passed to GL, e.g. GL_VERTEX_SHADER.
 */
static int processShader(GLuint *pShader, const char *sFilename, GLint iShaderType)
{
	GLint iStatus;
	const char *aStrings[1] = { NULL };

	/* Create shader and load into GL. */
	*pShader = GL_CHECK(glCreateShader(iShaderType));
	aStrings[0] = loadShader(sFilename);
	if(aStrings[0] == NULL)
	{
		ErrMsg("Error: wrong shader code %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	GL_CHECK(glShaderSource(*pShader, 1, aStrings, NULL));

	/* Clean up shader source. */
	FREE((void *)(aStrings[0]));
	aStrings[0] = NULL;

	/* Try compiling the shader. */
	GL_CHECK(glCompileShader(*pShader));
	GL_CHECK(glGetShaderiv(*pShader, GL_COMPILE_STATUS, &iStatus));

	/* Dump debug info (source and log) if compilation failed. */
	if(iStatus != GL_TRUE) {
		GLint iLen;
		char *sDebugSource = NULL;
		char *sErrorLog = NULL;

		/* Get shader source. */
		GL_CHECK(glGetShaderiv(*pShader, GL_SHADER_SOURCE_LENGTH, &iLen));
		sDebugSource = (char*)MALLOC(iLen);
		GL_CHECK(glGetShaderSource(*pShader, iLen, NULL, sDebugSource));
		DbgMsg("Debug source START:\n%s\nDebug source END\n\n", sDebugSource);
		FREE(sDebugSource);

		/* Now get the info log. */
		GL_CHECK(glGetShaderiv(*pShader, GL_INFO_LOG_LENGTH, &iLen));
		sErrorLog = (char*)MALLOC(iLen);
		GL_CHECK(glGetShaderInfoLog(*pShader, iLen, NULL, sErrorLog));
		DbgMsg("Log START:\n%s\nLog END\n\n", sErrorLog);
		FREE(sErrorLog);

		DbgMsg("Compilation FAILED!\n\n");
		return -1;
	}
	return 0;
}
#else
/* processShader(): Create shader, load in source, compile, dump debug as necessary.
 *
 * pShader: Pointer to return created shader ID.
 * sFilename: Passed-in filename from which to load shader source.
 * iShaderType: Passed to GL, e.g. GL_VERTEX_SHADER.
 */
static int processShader(GLuint *pShader, const char *pString, GLint iShaderType)
{
	GLint iStatus;
	const char *aStrings[1] = { NULL };

	if(pString == NULL)
	{
		ErrMsg("Error: wrong shader code %s:%i\n", __FILE__, __LINE__);
		return -1;
	}

	/* Create shader and load into GL. */
	*pShader = GL_CHECK(glCreateShader(iShaderType));
	if(pShader == NULL)
	{
		ErrMsg("Error: wrong shader code %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	aStrings[0] = pString;
	GL_CHECK(glShaderSource(*pShader, 1, aStrings, NULL));

	/* Clean up shader source. */
	aStrings[0] = NULL;

	/* Try compiling the shader. */
	GL_CHECK(glCompileShader(*pShader));
	GL_CHECK(glGetShaderiv(*pShader, GL_COMPILE_STATUS, &iStatus));

	/* Dump debug info (source and log) if compilation failed. */
	if(iStatus != GL_TRUE) {
		GLint iLen;
		char *sDebugSource = NULL;
		char *sErrorLog = NULL;

		/* Get shader source. */
		GL_CHECK(glGetShaderiv(*pShader, GL_SHADER_SOURCE_LENGTH, &iLen));
		sDebugSource = (char*)MALLOC(iLen);
		GL_CHECK(glGetShaderSource(*pShader, iLen, NULL, sDebugSource));
		DbgMsg("Debug source START:\n%s\nDebug source END\n\n", sDebugSource);
		FREE(sDebugSource);

		/* Now get the info log. */
		GL_CHECK(glGetShaderiv(*pShader, GL_INFO_LOG_LENGTH, &iLen));
		sErrorLog = (char*)MALLOC(iLen);
		GL_CHECK(glGetShaderInfoLog(*pShader, iLen, NULL, sErrorLog));
		DbgMsg("Log START:\n%s\nLog END\n\n", sErrorLog);
		FREE(sErrorLog);

		DbgMsg("Compilation FAILED!\n\n");
		return -1;
	}
	return 0;
}
#endif

/* For Deinterlace. */
//For 32bit context
static int vrInitializeDeinterlace( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	unsigned int program = VR_PROGRAM_DEINTERLACE;
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitializeDeinterlace start\n");	

	if(vrCreateEGLContext(program) != 0)
	{
		ErrMsg("Error: Fail to create context %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}
	
	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[program]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}	
	
	/* Load shaders. */
	if(processShader(&pStatics->shader[program].iVertName, VERTEX_SHADER_SOURCE_DEINTERLACE, GL_VERTEX_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	if(processShader(&pStatics->shader[program].iFragName, FRAGMENT_SHADER_SOURCE_DEINTERLACE, GL_FRAGMENT_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}	

	/* Set up shaders. */
	pStatics->shader[program].iProgName = GL_CHECK(glCreateProgram());
	//DbgMsg("Deinterlace iProgName(%d)\n", pStatics->shader[program].iProgName);		
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iVertName));
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iFragName));
	GL_CHECK(glLinkProgram(pStatics->shader[program].iProgName));
	GL_CHECK(glUseProgram(pStatics->shader[program].iProgName));
	
	/* Vertex positions. */
	pStatics->shader[program].iLocPosition = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v4Position"));
	if(pStatics->shader[program].iLocPosition == -1)
	{
		ErrMsg("Error: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocPosition));

	/* Fill texture. */
	pStatics->shader[program].iLocTexCoord = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v2TexCoord"));
	if(pStatics->shader[program].iLocTexCoord == -1)
	{
		ErrMsg("Warning: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocTexCoord));

    /* Texture Height. */
    pStatics->shader[program].iLocInputHeight = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "u_fTexHeight"));
    if(pStatics->shader[program].iLocInputHeight == -1)
    {
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
    }

    /* diffuse texture. */
    pStatics->shader[program].iLocMainTex[0] = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "diffuse"));
    if(pStatics->shader[program].iLocMainTex[0] == -1)
    {
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
    }
    else 
    {
        //GL_CHECK(glUniform1i(pStatics->shader[program].iLocMainTex[0], VR_INPUT_MODE_DEINTERLACE));
    }	

	/* ref texture. */
    pStatics->shader[program].iLocRefTex = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "ref_tex"));
    if(pStatics->shader[program].iLocRefTex == -1)
    {
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
    }
    else 
    {
        //GL_CHECK(glUniform1i(pStatics->shader[program].iLocRefTex, VR_INPUT_MODE_DEINTERLACE_REF));
    }
	
	//set texture
	GL_CHECK(glGenTextures(1, &pStatics->tex_deinterlace_ref_id));
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + VR_INPUT_MODE_DEINTERLACE_REF));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, pStatics->tex_deinterlace_ref_id));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)); 
	{
		unsigned int temp_imgbuf[2] = {0x00000000, 0xFFFFFFFF};
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,
				 1,2,0,
				 GL_RGBA,GL_UNSIGNED_BYTE,temp_imgbuf));	
	}	

	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitializeDeinterlace end\n"); 	
	return 0;			
}				

/* For Scaler. */
//For 8bit context
static int vrInitializeScaler( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	unsigned int program = VR_PROGRAM_SCALE;
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitialize_scale start\n");		

	if(vrCreateEGLContext(program) != 0)
	{
		ErrMsg("Error: Fail to create context %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[program]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}
	
	/* Load shaders. */
	if(processShader(&pStatics->shader[program].iVertName, VERTEX_SHADER_SOURCE_SCALE, GL_VERTEX_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	if(processShader(&pStatics->shader[program].iFragName, FRAGMENT_SHADER_SOURCE_SCALE, GL_FRAGMENT_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	
	/* Set up shaders. */
	pStatics->shader[program].iProgName = GL_CHECK(glCreateProgram());
	//DbgMsg("Scaler iProgName(%d)\n", pStatics->shader[program].iProgName);
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iVertName));
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iFragName));
	GL_CHECK(glLinkProgram(pStatics->shader[program].iProgName));
	GL_CHECK(glUseProgram(pStatics->shader[program].iProgName));

	/* Vertex positions. */
	pStatics->shader[program].iLocPosition = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v4Position"));
	if(pStatics->shader[program].iLocPosition == -1)
	{
		ErrMsg("Error: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocPosition));

	/* Fill texture. */
	pStatics->shader[program].iLocTexCoord = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v2TexCoord"));
	if(pStatics->shader[program].iLocTexCoord == -1)
	{
		ErrMsg("Warning: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocTexCoord));

	/* diffuse texture. */
	pStatics->shader[program].iLocMainTex[0] = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "diffuse"));
	if(pStatics->shader[program].iLocMainTex[0] == -1)
	{
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
	}
	else 
	{
		//GL_CHECK(glUniform1i(pStatics->shader[program].iLocMainTex[0], VR_INPUT_MODE_NEAREST));
	}
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitialize_scale end\n"); 		
	return 0;			
}

/* For Cvt2Y. */
//For 8bit context		
static int vrInitializeCvt2Y( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	unsigned int program = VR_PROGRAM_CVT2Y;
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitializeCvt2Y start\n");			

	if(vrCreateEGLContext(program) != 0)
	{
		ErrMsg("Error: Fail to create context %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[program]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}
	
	/* Load shaders. */
	if(processShader(&pStatics->shader[program].iVertName, VERTEX_SHADER_SOURCE_CVT2Y, GL_VERTEX_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	if(processShader(&pStatics->shader[program].iFragName, FRAGMENT_SHADER_SOURCE_CVT2Y, GL_FRAGMENT_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	
	/* Set up shaders. */
	pStatics->shader[program].iProgName = GL_CHECK(glCreateProgram());
	//DbgMsg("Scaler iProgName(%d)\n", pStatics->shader[program].iProgName);
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iVertName));
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iFragName));
	GL_CHECK(glLinkProgram(pStatics->shader[program].iProgName));
	GL_CHECK(glUseProgram(pStatics->shader[program].iProgName));

	/* Vertex positions. */
	pStatics->shader[program].iLocPosition = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v4Position"));
	if(pStatics->shader[program].iLocPosition == -1)
	{
		ErrMsg("Error: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocPosition));

	/* Fill texture. */
	pStatics->shader[program].iLocTexCoord = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v2TexCoord"));
	if(pStatics->shader[program].iLocTexCoord == -1)
	{
		ErrMsg("Warning: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocTexCoord));

	/* diffuse texture. */
	pStatics->shader[program].iLocMainTex[0] = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "diffuse"));
	if(pStatics->shader[program].iLocMainTex[0] == -1)
	{
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
	}
	else 
	{
		//GL_CHECK(glUniform1i(pStatics->shader[program].iLocMainTex[0], VR_INPUT_MODE_NEAREST));
	}
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitializeCvt2Y end\n"); 			
	return 0;
}			


/* For Cvt2UV. */
//For 16bit context
static int vrInitializeCvt2UV( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	unsigned int program = VR_PROGRAM_CVT2UV;
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitializeCvt2UV start\n");	

	if(vrCreateEGLContext(program) != 0)
	{
		ErrMsg("Error: Fail to create context %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[program]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}
	
	/* Load shaders. */
	if(processShader(&pStatics->shader[program].iVertName, VERTEX_SHADER_SOURCE_CVT2UV, GL_VERTEX_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	if(processShader(&pStatics->shader[program].iFragName, FRAGMENT_SHADER_SOURCE_CVT2UV, GL_FRAGMENT_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	
	/* Set up shaders. */
	pStatics->shader[program].iProgName = GL_CHECK(glCreateProgram());
	//DbgMsg("Scaler iProgName(%d)\n", pStatics->shader[program].iProgName);
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iVertName));
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iFragName));
	GL_CHECK(glLinkProgram(pStatics->shader[program].iProgName));
	GL_CHECK(glUseProgram(pStatics->shader[program].iProgName));

	/* Vertex positions. */
	pStatics->shader[program].iLocPosition = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v4Position"));
	if(pStatics->shader[program].iLocPosition == -1)
	{
		ErrMsg("Error: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocPosition));

	/* Fill texture. */
	pStatics->shader[program].iLocTexCoord = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v2TexCoord"));
	if(pStatics->shader[program].iLocTexCoord == -1)
	{
		ErrMsg("Warning: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocTexCoord));

	/* diffuse texture. */
	pStatics->shader[program].iLocMainTex[0] = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "diffuse"));
	if(pStatics->shader[program].iLocMainTex[0] == -1)
	{
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
	}
	else 
	{
		//GL_CHECK(glUniform1i(pStatics->shader[program].iLocMainTex[0], VR_INPUT_MODE_NEAREST));
	}
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitializeCvt2UV end\n"); 		
	return 0;
}

/* For Cvt2Rgba. */		
//For 32bit context
static int vrInitializeCvt2Rgba( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	unsigned int program = VR_PROGRAM_CVT2RGBA;
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitialize_cvt2rgb start\n");		

	if(vrCreateEGLContext(program) != 0)
	{
		ErrMsg("Error: Fail to create context %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}

	/* Make context current. */
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, target->pixmap_surface, target->pixmap_surface, pStatics->egl_info.sEGLContext[program]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1;	
	}
	
	/* Load shaders. */
	if(processShader(&pStatics->shader[program].iVertName, VERTEX_SHADER_SOURCE_CVT2RGBA, GL_VERTEX_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	if(processShader(&pStatics->shader[program].iFragName, FRAGMENT_SHADER_SOURCE_CVT2RGBA, GL_FRAGMENT_SHADER) < 0)
	{
		ErrMsg("Error: wrong shader %s:%i\n", __FILE__, __LINE__);
		return -1;
	}

	/* Set up shaders. */
	pStatics->shader[program].iProgName = GL_CHECK(glCreateProgram());
	//DbgMsg("Deinterlace iProgName(%d)\n", pStatics->shader[program].iProgName);		
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iVertName));
	GL_CHECK(glAttachShader(pStatics->shader[program].iProgName, pStatics->shader[program].iFragName));
	GL_CHECK(glLinkProgram(pStatics->shader[program].iProgName));
	GL_CHECK(glUseProgram(pStatics->shader[program].iProgName));

	/* Vertex positions. */
	pStatics->shader[program].iLocPosition = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v4Position"));
	if(pStatics->shader[program].iLocPosition == -1)
	{
		ErrMsg("Error: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocPosition));

	/* Fill texture. */
	pStatics->shader[program].iLocTexCoord = GL_CHECK(glGetAttribLocation(pStatics->shader[program].iProgName, "a_v2TexCoord"));
	if(pStatics->shader[program].iLocTexCoord == -1)
	{
		ErrMsg("Warning: Attribute not found at %s:%i\n", __FILE__, __LINE__);
		return -1;
	}
	//else GL_CHECK(glEnableVertexAttribArray(pStatics->shader[program].iLocTexCoord));
		
	/* Y texture. */
	pStatics->shader[program].iLocMainTex[VR_INPUT_MODE_Y] = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "diffuseY"));
	if(pStatics->shader[program].iLocMainTex[VR_INPUT_MODE_Y] == -1)
	{
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
	}
	else 
	{
		//GL_CHECK(glUniform1i(pStatics->shader[program].iLocMainTex[0], VR_INPUT_MODE_DEINTERLACE));
	}
		
	/* U texture. */
	pStatics->shader[program].iLocMainTex[VR_INPUT_MODE_U] = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "diffuseU"));
	if(pStatics->shader[program].iLocMainTex[VR_INPUT_MODE_U] == -1)
	{
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
	}
	else 
	{
		//GL_CHECK(glUniform1i(pStatics->shader[program].iLocMainTex[0], VR_INPUT_MODE_DEINTERLACE));
	}
		
	/* V texture. */
	pStatics->shader[program].iLocMainTex[VR_INPUT_MODE_V] = GL_CHECK(glGetUniformLocation(pStatics->shader[program].iProgName, "diffuseV"));
	if(pStatics->shader[program].iLocMainTex[VR_INPUT_MODE_V] == -1)
	{
		ErrMsg("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
		//return -1;
	}
	else 
	{
		//GL_CHECK(glUniform1i(pStatics->shader[program].iLocMainTex[0], VR_INPUT_MODE_DEINTERLACE));
	}	
	VR_INFO("", VR_GRAPHIC_DBG_CTX, "vrInitialize_cvt2rgb end\n"); 		
	return 0;
}

static int vrDeinitializeDeinterlace( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	int ret = 0;
	
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, 
				target->pixmap_surface, target->pixmap_surface, 
				pStatics->egl_info.sEGLContext[VR_PROGRAM_DEINTERLACE]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1; 
	}

	GL_CHECK(glDeleteTextures(1,&pStatics->tex_deinterlace_ref_id));
	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_DEINTERLACE].iVertName	));
	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_DEINTERLACE].iFragName	));
	GL_CHECK(glDeleteProgram(pStatics->shader[VR_PROGRAM_DEINTERLACE].iProgName ));
	
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "Deinterlace eglDestroyContext start, 32ctx\n");		
	vrResetShaderInfo(&pStatics->shader[VR_PROGRAM_DEINTERLACE]);

	ret = vrDestroyEGLContext(VR_PROGRAM_DEINTERLACE);	
	return ret;
}

static int vrDeinitializeScaler( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	int ret = 0;
	
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, 
				target->pixmap_surface, target->pixmap_surface, 
				pStatics->egl_info.sEGLContext[VR_PROGRAM_SCALE]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1; 
	}

	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_SCALE].iVertName ));
	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_SCALE].iFragName ));
	GL_CHECK(glDeleteProgram(pStatics->shader[VR_PROGRAM_SCALE].iProgName ));
	
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "Scale eglDestroyContext start, 8ctx\n");		
	vrResetShaderInfo(&pStatics->shader[VR_PROGRAM_SCALE]);
	
	ret = vrDestroyEGLContext(VR_PROGRAM_SCALE);	
	return ret;
}

static int vrDeinitializeCvt2Y( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	int ret = 0;
	
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, 
				target->pixmap_surface, target->pixmap_surface, 
				pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2Y]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1; 
	}

	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_CVT2Y].iVertName	));
	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_CVT2Y].iFragName	));
	GL_CHECK(glDeleteProgram(pStatics->shader[VR_PROGRAM_CVT2Y].iProgName ));
	
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "Cvt2Y eglDestroyContext start, 8ctx\n");	
	vrResetShaderInfo(&pStatics->shader[VR_PROGRAM_CVT2Y]);
	
	ret = vrDestroyEGLContext(VR_PROGRAM_CVT2Y);	
	return ret;
}

static int vrDeinitializeCvt2UV( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	int ret = 0;
	
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, 
				target->pixmap_surface, target->pixmap_surface, 
				pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2UV]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1; 
	}
	
	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_CVT2UV].iVertName	));
	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_CVT2UV].iFragName	));
	GL_CHECK(glDeleteProgram(pStatics->shader[VR_PROGRAM_CVT2UV].iProgName ));
	
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "Cvt2UV eglDestroyContext start, 16ctx\n");	
	vrResetShaderInfo(&pStatics->shader[VR_PROGRAM_CVT2UV]);

	ret = vrDestroyEGLContext(VR_PROGRAM_CVT2UV);	
	return ret;
}

static int vrDeinitializeCvt2Rgba( HSURFTARGET target)
{
	Statics* pStatics = vrGetStatics();
	int ret = 0;
	
	EGLBoolean bResult = eglMakeCurrent(pStatics->egl_info.sEGLDisplay, 
				target->pixmap_surface, target->pixmap_surface, 
				pStatics->egl_info.sEGLContext[VR_PROGRAM_CVT2RGBA]);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		ErrMsg("eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		ErrMsg("Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		return -1; 
	}
	
	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_CVT2RGBA].iVertName	));
	GL_CHECK(glDeleteShader(pStatics->shader[VR_PROGRAM_CVT2RGBA].iFragName	));
	GL_CHECK(glDeleteProgram(pStatics->shader[VR_PROGRAM_CVT2RGBA].iProgName ));
	
	VR_INFO("", VR_GRAPHIC_DBG_TARGET, "Cvt2Rgba eglDestroyContext start, 32ctx\n");
	vrResetShaderInfo(&pStatics->shader[VR_PROGRAM_CVT2RGBA]);

	ret = vrDestroyEGLContext(VR_PROGRAM_CVT2RGBA);	
	return ret;
}
