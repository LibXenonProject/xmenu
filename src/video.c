#include <stdio.h>
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <stdint.h>
#include "video.h"

//utiliser pendant l'update
int max_vertices ;
int nr_primitives ;

//Initilisation de l'ecran
void initScreen(int width,int height)
{
	DEBUG("\t\t\t initScreen();\n");
	xe = &_xe;
		/* initialize the GPU */
	Xe_Init(xe);

	edram_init(xe);

		/* create a render target (the framebuffer) */
	fb = Xe_GetFramebufferSurface(xe);
	Xe_SetRenderTarget(xe, fb);//le buffer est xe 


	gfxplane = Xe_CreateTexture(xe, width,height, 1, XE_FMT_8888 | XE_FMT_ARGB , 0);//init de la texture 24bit argb

	/* let's define a vertex buffer format */
	
	/* un rectangle */
	static const struct XenosVBFFormat vbf =
	{
		2, {
		  {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT2},
		  {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
		}
	};
	
	float cube[] = {
		-1,  1, 0, 0,
		-1, -1, 0, 1,
		 1,  1, 1, 0,
		 1, -1, 1, 1
	};

	unsigned short cube_indices[] = {0, 2, 1, 1, 2, 3};

	printf("loading pixel shader...\n");
		/* load pixel shader */

	extern unsigned char content_datapspsu[];
	extern unsigned char content_datavsvsu[];

	sh_ps = Xe_LoadShaderFromMemory(xe,  content_datapspsu);
	Xe_InstantiateShader(xe, sh_ps, 0);

	printf("loading vertex shader...\n");
		/* load vertex shader */

	sh_vs = Xe_LoadShaderFromMemory(xe, content_datavsvsu);
	Xe_InstantiateShader(xe, sh_vs, 0);
	Xe_ShaderApplyVFetchPatches(xe, sh_vs, 0, &vbf);

	//M_BuildPersp(&g_proj, 45.0 / 180.0 * M_PI, 640.0/480.0, 1, 200.0);

	printf("create vb...\n");
	/* create and fill vertex buffer */
	vb = Xe_CreateVertexBuffer(xe, sizeof(cube));
	void *v = Xe_VB_Lock(xe, vb, 0, sizeof(cube), XE_LOCK_WRITE);
	memcpy(v, cube, sizeof(cube));
	Xe_VB_Unlock(xe, vb);

	printf("create ib...\n");
	/* create and fill index buffer */
	ib = Xe_CreateIndexBuffer(xe, sizeof(cube_indices), XE_FMT_INDEX16);
	unsigned short *i = Xe_IB_Lock(xe, ib, 0, sizeof(cube_indices), XE_LOCK_WRITE);
	memcpy(i, cube_indices, sizeof(cube_indices));
	Xe_IB_Unlock(xe, ib);

	/* stats */
	printf("render..\n");
	
	//do_edram_foo(xe, 1);
	//edram_init(xe);

	extern void edram_p4(int *res);

	max_vertices = sizeof(cube)/(sizeof(*cube)*12);
	nr_primitives = sizeof(cube_indices)/sizeof(*cube_indices) / 3;

}

void drawBg(int width,int height)
{

	DEBUG("\t\t\t drawBg();\n");
	//do_edram_foo(xe, 1);

	/** 
	copy bitmap in surface (from snes9x)
	**/
	/* flush cache */
	Xe_Surface_LockRect(xe, gfxplane, 0, 0, 0, 0, XE_LOCK_WRITE);
	Xe_Surface_Unlock(xe, gfxplane);

	/* begin a new frame, i.e. reset all renderstates to the default */
	

	/* set the light direction for the pixel shader */
	float lightDirection[] = {0, 0, -1, 0};
	Xe_SetPixelShaderConstantF(xe, 0, lightDirection, 1);



	/* draw cube */
	Xe_SetShader(xe, SHADER_TYPE_PIXEL, sh_ps, 0);
	Xe_SetShader(xe, SHADER_TYPE_VERTEX, sh_vs, 0);
	Xe_SetStreamSource(xe, 0, vb, 0, 4); /* using this vertex buffer */
	Xe_SetIndices(xe, ib); /* ... this index buffer... */
	Xe_SetTexture(xe, 0, gfxplane); /* ... and this texture */

}

//Mise à jours de l'ecran
void updateScreen(int width,int height)
{
	DEBUG("\t\t\t updateScreen();\n");
	
//#if 1
	Xe_InvalidateState(xe);
	
	//dessine l'ecran
	Xe_DrawIndexedPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 0, max_vertices, 0, nr_primitives);//dessine

	/* clear to white */
	//Xe_SetClearColor(xe, ~0);

	/* resolve (and clear) */
	Xe_Resolve(xe);

	/* wait for render finish */
	Xe_Sync(xe);

	//libere la matrice ??
	glPopMatrix();

//#endif
}

