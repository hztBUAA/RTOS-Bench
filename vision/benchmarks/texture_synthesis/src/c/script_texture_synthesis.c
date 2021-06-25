/**
 * @file script_texture_synthesis.c
 * @ingroup texture_synthesis
 * @brief Functions used to run the disparity benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the execution portion.
 *
 * @bug The verification of the benchamrk fails after the first execution.
 *
 * @author Sravanthi Kota Venkata, for the original version.
 */

#include "texture.h"
#include <errno.h>

/// Synthesis parameters.
static params* data=NULL;
///Input image.
static I2D* im=NULL;

/**
 * @brief Will load the images that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * To initialize the benchmark `parse_flags()` will be called.
 * @returns `0` on success, `-1` on error, setting errno.
 */
int benchmark_init(int parameters_num, void **parameters)
{

	if(parameters_num<1){
		errno=EINVAL;
		elogf(LOG_LEVEL_ERR,"Missing input path\n");
		return -1;
	}

	data = malloc(sizeof(params));
	im = parse_flags(parameters_num,(char**) parameters,data);
	return 0;

}
/**
 * @brief This handler is where the texture synthesis is computed.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path (which is generally the same as the input folder path) as only element of the parameters array.
 * If self checking is enabled the paramters list must have the output folder path (where the check file is located) as first parameter.
 */
void benchmark_execution(int parameters_num, void **parameters)
{

	if(parameters_num<1){
		errno=EINVAL;
		elogf(LOG_LEVEL_ERR,"Missing input path\n");
		exit(EXIT_FAILURE);
	}

	F2D *target=NULL, *result=NULL;
	int *atlas=NULL;
	int *xloopin=NULL, *yloopin=NULL;
	int *xloopout=NULL, *yloopout=NULL;
	int *xloopin_base=NULL, *yloopin_base=NULL;
	int *xloopout_base=NULL, *yloopout_base=NULL;
	F2D* image=NULL;
	int HEIGHT=data->heightout;
	int WIDTH=data->widthout;
	int i=0,j=0;
	int targetin=0;

	/*    xloopin_base = malloc(2*WIDTHin*sizeof(int));
	    yloopin_base = malloc(2*HEIGHTin*sizeof(int));
	    if(xloopin_base==NULL || yloopin_base==NULL){
	          elogf(LOG_LEVEL_ERR,"Not enough memory\n");
	          errno=ENOMEM;
	          if(xloopin_base!=NULL){
	               free(xloopin_base);
	          }
	          if(yloopin_base!=NULL){
	               free(yloopin_base);
	          }
	          exit(EXIT_FAILURE);
	    }
	    xloopin=xloopin_base;
	    yloopin=yloopin_base;

	    for(i=-WIDTHin/2;i<WIDTHin+WIDTHin/2;i++)
	    {
	        arrayref(xloopin,i+WIDTHin/2) = (WIDTHin+i)%WIDTHin;
	    }

	    for(i=-HEIGHTin/2;i<HEIGHTin+HEIGHTin/2;i++)
	    {
	arrayref(yloopin,i+HEIGHTin/2) = (HEIGHTin+i)%HEIGHTin;
	    }
	    xloopin += WIDTHin/2; yloopin += HEIGHTin/2; */

	result = fMallocHandle(1,HEIGHT*WIDTH);
	target = fMallocHandle(1, WIDTH*HEIGHT);
	atlas = malloc(2*WIDTH*HEIGHT*sizeof(int));
	xloopout_base = malloc(2*WIDTH*sizeof(int));
	yloopout_base = malloc(2*HEIGHT*sizeof(int));

	    if(xloopout_base==NULL || yloopout_base==NULL || result==NULL || target==NULL || atlas == NULL){
	          elogf(LOG_LEVEL_ERR,"Not enough memory\n");
	          errno=ENOMEM;
		  if(result!=NULL){
			  free(result);
		  }
		  if(target!=NULL){
			  free(target);
		  }
		  if(atlas!=NULL){
			  free(atlas);
		  }
	          if(xloopout_base!=NULL){
	               free(xloopout_base);
	          }
	          if(yloopout_base!=NULL){
	               free(yloopout_base);
	          }
	          exit(EXIT_FAILURE);
	    }
	    xloopout=xloopout_base;
	    yloopout=yloopout_base;

	for(i=-WIDTH/2;i<WIDTH+WIDTH/2;i++)
	{
		arrayref(xloopout,i+WIDTH/2) = (WIDTH+i)%WIDTH;
	}
	for(i=-HEIGHT/2;i<HEIGHT+HEIGHT/2;i++)
	{
		arrayref(yloopout,i+HEIGHT/2) = (HEIGHT+i)%HEIGHT;
	}
	xloopout += WIDTH/2; yloopout += HEIGHT/2;

	if(!targetin)
	{
		for(i=0;i<data->heightout;i++)
		{
			for(j=0;j<data->widthout;j++)
			{
				asubsref(target,a(j,i,data->widthout)+R) = 1.0;
				//                asubsref(target,a(j,i,data->widthout)+G) = 1.0;
				//                asubsref(target,a(j,i,data->widthout)+B) = 1.0;
			}
		}
	}

	for(i=0;i<data->heightout;i++)
	{
		for(j=0;j<data->widthout;j++)
		{
			asubsref(result,a(j,i,data->widthout)+R)  = 1.0;
			//            asubsref(result,a(j,i,data->widthout)+G)  = 1.0;
			//            asubsref(result,a(j,i,data->widthout)+B)  = 1.0;
		}
	}

	image = fiDeepCopy(im);

	create_texture(image, data,result,target,atlas,xloopout,yloopout);

	#ifdef CHECK
	{
		int ret=0;
		#ifdef GENERATE_OUTPUT
		fWriteMatrix(result, parameters[0]);
		#endif
		ret = fSelfCheck(result, parameters[0], 1.0);
		if(ret < 0)
			elogf(LOG_LEVEL_ERR,"Error in Texture Synthesis\n");
	}
	#endif

	fFreeHandle(result);
	fFreeHandle(target);
	free(atlas);
	free(xloopout_base);
	free(yloopout_base);
	fFreeHandle(image);

	//    free(xloopin_base);
	//    free(yloopin_base);
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will free `::image`,`::data`,`::target`,`::result` and `::atlas`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{

if(data!=NULL){
	free(data);
}
if(im!=NULL){
	iFreeHandle(im);
}
}

/** @brief Will parse the input parameters and initialize data accordingly.
 * @param[in] argc The number of parameters passed (should be 1).
 * @param[in] argv The parameters list. Should contain only the path to the input folder.
 * @returns The read `::image` from the input folder.
 * @details The following shared variables will also be initialized: `::atlas`, `::target`, `::result`, `::WIDTHin`, `::HEIGHTin`, `::localx`,
 * `::localy`, `::WIDTH`, `::HEIGTH`, `::xloopout`, `::yloopout`.
 */
I2D *parse_flags(int argc, char **argv,params* data)
{

int WIDTHin=0,HEIGHTin=0;
int WIDTH=0,HEIGHT=0;
int localx=0, localy=0;
	int i=0, tsx=0,tsy=0;
	I2D* image=NULL;
	char fileNm[256];

	sprintf(fileNm, "%s/1.bmp", argv[0]);
	image = readImage(fileNm);
	WIDTHin = image->width;
	HEIGHTin = image->height;

	localx = 3;
	localy = 3;

	#ifdef test
	WIDTH = WIDTHin*2;
	HEIGHT = HEIGHTin*2;
	localx = 2;
	localy = 2;
	#endif
	#ifdef sim_fast
	WIDTH = WIDTHin*2;
	HEIGHT = HEIGHTin*2;
	localx = 3;
	localy = 3;
	#endif
	#ifdef sim
	WIDTH = WIDTHin*3;
	HEIGHT = HEIGHTin*3;
	localx = 2;
	localy = 2;
	#endif
	#ifdef sqcif
	WIDTH = WIDTHin*6;
	HEIGHT = HEIGHTin*6;
	localx = 2;
	localy = 2;
	#endif
	#ifdef qcif
	WIDTH = WIDTHin*10;
	HEIGHT = HEIGHTin*10;
	localx = 2;
	localy = 2;
	#endif
	#ifdef cif
	WIDTH = WIDTHin*10;
	HEIGHT = HEIGHTin*10;
	localx = 3;
	localy = 3;
	#endif
	#ifdef vga
	WIDTH = WIDTHin*20;
	HEIGHT = HEIGHTin*20;
	localx = 3;
	localy = 3;
	#endif
	#ifdef fullhd
	WIDTH = WIDTHin*20;
	HEIGHT = HEIGHTin*20;
	localx = 15;
	localy = 15;
	#endif
	#ifdef wuxga
	WIDTH = WIDTHin*20;
	HEIGHT = HEIGHTin*20;
	localx = 5;
	localy = 5;
	#endif
	elogf(LOG_LEVEL_TRACE,"Input size\t\t- (%dx%d)\n", HEIGHTin, WIDTHin);

	init_params(data,WIDTHin,HEIGHTin,WIDTH,HEIGHT,localx,localy);

	return image;
}

void init_params(params *data, int WIDTHin, int HEIGHTin, int WIDTH, int HEIGHT, int localx,int localy)
{
	data->localx = localx; data->localy = localy;
	data->widthin = WIDTHin; data->widthout = WIDTH;
	data->heightin = HEIGHTin; data->heightout = HEIGHT;
}
