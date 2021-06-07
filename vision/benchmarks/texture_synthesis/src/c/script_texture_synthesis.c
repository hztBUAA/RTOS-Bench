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
 * @bug The `SIGNATURES` symbol is defined in multiple locations.
 *
 * @author Sravanthi Kota Venkata, for the original version.
 */

#include "texture.h"

int WIDTHin, HEIGHTin;
extern F2D *target, *result;
int WIDTH, HEIGHT;
int localx, localy, targetin;
extern int *atlas;
extern int *xloopin, *yloopin;
extern int *xloopout, *yloopout;

static params *data;
static F2D *image;

/**
 * @brief Will load the images that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * To initialize the benchmark `parse_flags()` and `init_params()` will be called.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	I2D *im;
	unsigned int *start, *end, *elapsed;

	data = malloc(sizeof(params));
	im = parse_flags(parameters_num, parameters);
	image = fiDeepCopy(im);
	init_params(data);
	iFreeHandle(im);
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
	create_texture(image, data);

#ifdef CHECK
	{
		if (parameters_num < 1) {
			elogf(LOG_LEVEL_ERR, "Missing output folder path");
			exit(-1);
		}
		int ret = 0;
#ifdef GENERATE_OUTPUT
		fWriteMatrix(result, parameters[0]);
#endif
		ret = fSelfCheck(result, parameters[0], 1.0);
		if (ret < 0)
			printf("Error in Texture Synthesis\n");
	}
#endif
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will free `::image`,`::data`,`::target`,`::result` and `::atlas`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	fFreeHandle(image);
	free(data);

	fFreeHandle(target);
	fFreeHandle(result);
	free(atlas);
	//    free(xloopout);
	//    free(yloopout);
}

/** @brief Will parse the input parameters and initialize data accordingly.
 * @param[in] argc The number of parameters passed (should be 1).
 * @param[in] argv The parameters list. Should contain only the path to the input folder.
 * @returns The read `::image` from the input folder.
 * @details The followin shared variables will also be initialized: `::atlas`, `::target`, `::result`, `::WIDTHin`, `::HEIGHTin`, `::localx`,
 * `::localy`, `::WIDTH`, `::HEIGTH`, `::xloopout`, `::yloopout`.
 */
I2D *parse_flags(int argc, char **argv)
{
	int i, tsx, tsy;
	I2D *image;
	char fileNm[256];

	sprintf(fileNm, "%s/1.bmp", argv[0]);
	image = readImage(fileNm);
	WIDTHin = image->width;
	HEIGHTin = image->height;

	localx = 3;
	localy = 3;

#ifdef test
	WIDTH = WIDTHin * 2;
	HEIGHT = HEIGHTin * 2;
	localx = 2;
	localy = 2;
#endif
#ifdef sim_fast
	WIDTH = WIDTHin * 2;
	HEIGHT = HEIGHTin * 2;
	localx = 3;
	localy = 3;
#endif
#ifdef sim
	WIDTH = WIDTHin * 3;
	HEIGHT = HEIGHTin * 3;
	localx = 2;
	localy = 2;
#endif
#ifdef sqcif
	WIDTH = WIDTHin * 6;
	HEIGHT = HEIGHTin * 6;
	localx = 2;
	localy = 2;
#endif
#ifdef qcif
	WIDTH = WIDTHin * 10;
	HEIGHT = HEIGHTin * 10;
	localx = 2;
	localy = 2;
#endif
#ifdef cif
	WIDTH = WIDTHin * 10;
	HEIGHT = HEIGHTin * 10;
	localx = 3;
	localy = 3;
#endif
#ifdef vga
	WIDTH = WIDTHin * 20;
	HEIGHT = HEIGHTin * 20;
	localx = 3;
	localy = 3;
#endif
#ifdef fullhd
	WIDTH = WIDTHin * 20;
	HEIGHT = HEIGHTin * 20;
	localx = 15;
	localy = 15;
#endif
#ifdef wuxga
	WIDTH = WIDTHin * 20;
	HEIGHT = HEIGHTin * 20;
	localx = 5;
	localy = 5;
#endif
	printf("Input size\t\t- (%dx%d)\n", HEIGHTin, WIDTHin);

	//    xloopin = malloc(2*WIDTHin*sizeof(int));
	//    yloopin = malloc(2*HEIGHTin*sizeof(int));
	//
	//    for(i=-WIDTHin/2;i<WIDTHin+WIDTHin/2;i++)
	//    {
	//        arrayref(xloopin,i+WIDTHin/2) = (WIDTHin+i)%WIDTHin;
	//    }
	//
	//    for(i=-HEIGHTin/2;i<HEIGHTin+HEIGHTin/2;i++)
	//    {
	//        arrayref(yloopin,i+HEIGHTin/2) = (HEIGHTin+i)%HEIGHTin;
	//    }
	//    xloopin += WIDTHin/2; yloopin += HEIGHTin/2;

	result = fMallocHandle(1, HEIGHT * WIDTH);
	target = fMallocHandle(1, WIDTH * HEIGHT);

	atlas = malloc(2 * WIDTH * HEIGHT * sizeof(int));
	xloopout = malloc(2 * WIDTH * sizeof(int));
	yloopout = malloc(2 * HEIGHT * sizeof(int));

	for (i = -WIDTH / 2; i < WIDTH + WIDTH / 2; i++) {
		arrayref(xloopout, i + WIDTH / 2) = (WIDTH + i) % WIDTH;
	}
	for (i = -HEIGHT / 2; i < HEIGHT + HEIGHT / 2; i++) {
		arrayref(yloopout, i + HEIGHT / 2) = (HEIGHT + i) % HEIGHT;
	}
	xloopout += WIDTH / 2;
	yloopout += HEIGHT / 2;

	if (result == NULL) {
		printf("Can't allocate %dx%d image. Exiting.\n", WIDTH, HEIGHT);
		exit(1);
	}

	return image;
}

void init_params(params *data)
{
	int i, j;
	data->localx = localx;
	data->localy = localy;
	data->widthin = WIDTHin;
	data->widthout = WIDTH;
	data->heightin = HEIGHTin;
	data->heightout = HEIGHT;

	if (!targetin) {
		for (i = 0; i < data->heightout; i++) {
			for (j = 0; j < data->widthout; j++) {
				asubsref(target, a(j, i, data->widthout) + R) =
					1.0;
				//                asubsref(target,a(j,i,data->widthout)+G) = 1.0;
				//                asubsref(target,a(j,i,data->widthout)+B) = 1.0;
			}
		}
	}

	for (i = 0; i < data->heightout; i++) {
		for (j = 0; j < data->widthout; j++) {
			asubsref(result, a(j, i, data->widthout) + R) = 1.0;
			//            asubsref(result,a(j,i,data->widthout)+G)  = 1.0;
			//            asubsref(result,a(j,i,data->widthout)+B)  = 1.0;
		}
	}
}
