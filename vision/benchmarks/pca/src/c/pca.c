/**
 * @file pca.c
 * @brief Functions used to run the pca benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the execution portion.
 * @author F. Murtagh, for the original version.
 */
/***********************  Contents  ****************************************
 * Principal Components Analysis: C, 638 lines. ****************************
 * Sample input data set (final 36 lines). *********************************
 ***************************************************************************
 */

/*********************************/
/* Principal Components Analysis */
/*********************************/

/*********************************************************************/
/* Principal Components Analysis or the Karhunen-Loeve expansion is a
   classical method for dimensionality reduction or exploratory data
   analysis.  One reference among many is: F. Murtagh and A. Heck,
   Multivariate Data Analysis, Kluwer Academic, Dordrecht, 1987.

Author:
F. Murtagh
Phone:        + 49 89 32006298 (work)
+ 49 89 965307 (home)
Earn/Bitnet:  fionn@dgaeso51,  fim@dgaipp1s,  murtagh@stsci
Span:         esomc1::fionn
Internet:     murtagh@scivax.stsci.edu

F. Murtagh, Munich, 6 June 1989                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "logging.h"

#define SIGN(a, b) ((b) < 0 ? -fabs(a) : fabs(a))

/// Mean vector
static float *mean;
/// Standard deviation vector
static float *stddev;
/// Correlation matrix
static float **symmat;
/// Copy of correlation matrix
static float **symmat2;
/// Input data matrix
static float **data;
/// Vector of eigenvalues
static float *evals;
/// Intermediate dummy vector
static float *interm;
static char option;
/// Number of input rows
static int n;
/// Number of input columns
static int m;

/**  Correlation matrix: creation  ***********************************/

void corcol(float **data, int n, int m, float **symmat, float *mean,
	    float *stddev)
/* Create m * m correlation matrix from given n * m data matrix. */
{
	float eps = 0.005;
	float x;
	int i, j, j1, j2;

	/* Determine mean of column vectors of input data matrix */

	for (j = 1; j <= m; j++) {
		mean[j] = 0.0;
		for (i = 1; i <= n; i++) {
			mean[j] += data[i][j];
		}
		mean[j] /= (float)n;
	}

	//elogf(LOG_LEVEL_TRACE,"\nMeans of column vectors:\n");
	for (j = 1; j <= m; j++) {
		//elogf(LOG_LEVEL_TRACE,"%7.1f", mean[j]);
	}
	//elogf(LOG_LEVEL_TRACE,"\n");

	/* Determine standard deviations of column vectors of data matrix. */

	for (j = 1; j <= m; j++) {
		stddev[j] = 0.0;
		for (i = 1; i <= n; i++) {
			stddev[j] += ((data[i][j] - mean[j]) *
				      (data[i][j] - mean[j]));
		}
		stddev[j] /= (float)n;
		stddev[j] = sqrt(stddev[j]);
		/* The following in an inelegant but usual way to handle
       near-zero std. dev. values, which below would cause a zero-
       divide. */
		if (stddev[j] <= eps)
			stddev[j] = 1.0;
	}

	//elogf(LOG_LEVEL_TRACE,"\nStandard deviations of columns:\n");
	for (j = 1; j <= m; j++) {
		//elogf(LOG_LEVEL_TRACE,"%7.1f", stddev[j]);
	}
	//elogf(LOG_LEVEL_TRACE,"\n");

	/* Center and reduce the column vectors. */

	for (i = 1; i <= n; i++) {
		for (j = 1; j <= m; j++) {
			data[i][j] -= mean[j];
			x = sqrt((float)n);
			x *= stddev[j];
			data[i][j] /= x;
			//elogf(LOG_LEVEL_TRACE,"value is %lf\n", data[i][j]);
		}
	}

	/* Calculate the m * m correlation matrix. */
	for (j1 = 1; j1 <= m - 1; j1++) {
		symmat[j1][j1] = 1.0;
		for (j2 = j1 + 1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += (data[i][j1] * data[i][j2]);
				/*elogf(LOG_LEVEL_TRACE,"multiplying values [%d][%d] * [%d][%d]\n",
				       i, j1, i, j2);*/
				/*elogf(LOG_LEVEL_TRACE,"Multiplying %lf and %lf\n", data[i][j1],
				       data[i][j2]);*/

				/*elogf(LOG_LEVEL_TRACE,"Value at %d %d = %lf\n", j1, j2,
				       symmat[j1][j2]);*/
			}
			//elogf(LOG_LEVEL_TRACE,"**SPLIT**\n");
			/*elogf(LOG_LEVEL_TRACE,"swapping [%d] [%d] = [%d] [%d]\n", j2, j1, j1,
			       j2);*/

			symmat[j2][j1] = symmat[j1][j2];
		}
	}
	symmat[m][m] = 1.0;

	return;
}

/**  Variance-covariance matrix: creation  *****************************/

void covcol(float **data, int n, int m, float **symmat, float *mean)
/* Create m * m covariance matrix from given n * m data matrix. */
{
	int i, j, j1, j2;

	/* Determine mean of column vectors of input data matrix */

	for (j = 1; j <= m; j++) {
		mean[j] = 0.0;
		for (i = 1; i <= n; i++) {
			mean[j] += data[i][j];
		}
		mean[j] /= (float)n;
	}

	//elogf(LOG_LEVEL_TRACE,"\nMeans of column vectors:\n");
	for (j = 1; j <= m; j++) {
		//elogf(LOG_LEVEL_TRACE,"%7.1f", mean[j]);
	}
	//elogf(LOG_LEVEL_TRACE,"\n");

	/* Center the column vectors. */

	for (i = 1; i <= n; i++) {
		for (j = 1; j <= m; j++) {
			data[i][j] -= mean[j];
		}
	}

	/* Calculate the m * m covariance matrix. */
	for (j1 = 1; j1 <= m; j1++) {
		for (j2 = j1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += data[i][j1] * data[i][j2];
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}

	return;
}

/**  Sums-of-squares-and-cross-products matrix: creation  **************/

void scpcol(float **data, int n, int m, float **symmat)
/* Create m * m sums-of-cross-products matrix from n * m data matrix. */
{
	int i, j1, j2;

	/* Calculate the m * m sums-of-squares-and-cross-products matrix. */

	for (j1 = 1; j1 <= m; j1++) {
		for (j2 = j1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += data[i][j1] * data[i][j2];
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}

	return;
}

/**  Error handler  **************************************************/

void erhand(char err_msg[])
/* Error handler */
{
	elogf(LOG_LEVEL_ERR, "Run-time error:\n");
	elogf(LOG_LEVEL_ERR, "%s\n", err_msg);
	elogf(LOG_LEVEL_ERR, "Exiting to system.\n");
	exit(1);
}

/**  Allocation of vector storage  ***********************************/

float *vector(int n)
/* Allocates a float vector with range [1..n]. */
{
	float *v;

	v = (float *)malloc((unsigned)n * sizeof(float));
	if (!v)
		erhand("Allocation failure in vector().");
	return v - 1;
}

/**  Allocation of float matrix storage  *****************************/

float **matrix(int n, int m)
/* Allocate a float matrix with range [1..n][1..m]. */
{
	int i;
	float **mat;

	/* Allocate pointers to rows. */
	mat = (float **)malloc((unsigned)(n) * sizeof(float *));
	if (!mat)
		erhand("Allocation failure 1 in matrix().");
	mat -= 1;

	/* Allocate rows and set pointers to them. */
	for (i = 1; i <= n; i++) {
		mat[i] = (float *)malloc((unsigned)(m) * sizeof(float));
		if (!mat[i])
			erhand("Allocation failure 2 in matrix().");
		mat[i] -= 1;
	}

	/* Return pointer to array of pointers to rows. */
	return mat;
}

/**  Deallocate vector storage  *********************************/

void free_vector(float *v, int n)
/* Free a float vector allocated by vector(). */
{
	free((char *)(v + 1));
}

/**  Deallocate float matrix storage  ***************************/

void free_matrix(float **mat, int n, int m)
/* Free a float matrix allocated by matrix(). */
{
	int i;

	for (i = n; i >= 1; i--) {
		free((char *)(mat[i] + 1));
	}
	free((char *)(mat + 1));
}

/**  Reduce a real, symmetric matrix to a symmetric, tridiag. matrix. */

void tred2(float **a, int n, float *d, float *e)
/* Householder reduction of matrix a to tridiagonal form.
Algorithm: Martin et al., Num. Math. 11, 181-195, 1968.
Ref: Smith et al., Matrix Eigensystem Routines -- EISPACK Guide
Springer-Verlag, 1976, pp. 489-494.
W H Press et al., Numerical Recipes in C, Cambridge U P,
1988, pp. 373-374.  */
{
	int l, k, j, i;
	float scale, hh, h, g, f;

	for (i = n; i >= 2; i--) {
		l = i - 1;
		h = scale = 0.0;
		if (l > 1) {
			for (k = 1; k <= l; k++)
				scale += fabs(a[i][k]);
			if (scale == 0.0)
				e[i] = a[i][l];
			else {
				for (k = 1; k <= l; k++) {
					a[i][k] /= scale;
					h += a[i][k] * a[i][k];
				}
				f = a[i][l];
				g = f > 0 ? -sqrt(h) : sqrt(h);
				e[i] = scale * g;
				h -= f * g;
				a[i][l] = f - g;
				f = 0.0;
				for (j = 1; j <= l; j++) {
					a[j][i] = a[i][j] / h;
					g = 0.0;
					for (k = 1; k <= j; k++)
						g += a[j][k] * a[i][k];
					for (k = j + 1; k <= l; k++)
						g += a[k][j] * a[i][k];
					e[j] = g / h;
					f += e[j] * a[i][j];
				}
				hh = f / (h + h);
				for (j = 1; j <= l; j++) {
					f = a[i][j];
					e[j] = g = e[j] - hh * f;
					for (k = 1; k <= j; k++)
						a[j][k] -= (f * e[k] +
							    g * a[i][k]);
				}
			}
		} else
			e[i] = a[i][l];
		d[i] = h;
	}
	d[1] = 0.0;
	e[1] = 0.0;
	for (i = 1; i <= n; i++) {
		l = i - 1;
		if (d[i]) {
			for (j = 1; j <= l; j++) {
				g = 0.0;
				for (k = 1; k <= l; k++)
					g += a[i][k] * a[k][j];
				for (k = 1; k <= l; k++)
					a[k][j] -= g * a[k][i];
			}
		}
		d[i] = a[i][i];
		a[i][i] = 1.0;
		for (j = 1; j <= l; j++)
			a[j][i] = a[i][j] = 0.0;
	}
}

/**  Tridiagonal QL algorithm -- Implicit  **********************/

void tqli(float d[], float e[], int n, float **z)
{
	int m, l, iter, i, k;
	float s, r, p, g, f, dd, c, b;
	void erhand();

	for (i = 2; i <= n; i++)
		e[i - 1] = e[i];
	e[n] = 0.0;
	for (l = 1; l <= n; l++) {
		iter = 0;
		do {
			for (m = l; m <= n - 1; m++) {
				dd = fabs(d[m]) + fabs(d[m + 1]);
				if (fabs(e[m]) + dd == dd)
					break;
			}
			if (m != l) {
				if (iter++ == 30)
					erhand("No convergence in TLQI.");
				g = (d[l + 1] - d[l]) / (2.0 * e[l]);
				r = sqrt((g * g) + 1.0);
				g = d[m] - d[l] + e[l] / (g + SIGN(r, g));
				s = c = 1.0;
				p = 0.0;
				for (i = m - 1; i >= l; i--) {
					f = s * e[i];
					b = c * e[i];
					if (fabs(f) >= fabs(g)) {
						c = g / f;
						r = sqrt((c * c) + 1.0);
						e[i + 1] = f * r;
						c *= (s = 1.0 / r);
					} else {
						s = f / g;
						r = sqrt((s * s) + 1.0);
						e[i + 1] = g * r;
						s *= (c = 1.0 / r);
					}
					g = d[i + 1] - p;
					r = (d[i] - g) * s + 2.0 * c * b;
					p = s * r;
					d[i + 1] = g + p;
					g = c * r - b;
					for (k = 1; k <= n; k++) {
						f = z[k][i + 1];
						z[k][i + 1] =
							s * z[k][i] + c * f;
						z[k][i] = c * z[k][i] - s * f;
					}
				}
				d[l] = d[l] - p;
				e[l] = g;
				e[m] = 0.0;
			}
		} while (m != l);
	}
}

/**
 * @brief Will load the file that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 4.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: input file full path;
 * - parameters[1]: number of rows of the input file;
 * - parameters[2]: number of columns of the input file;
 * - parameters[3]: type of analysis performend, can be:\n
 *   - `R` for correlation analysis;
 *   - `V` for variance*covariane analysis;
 *   - `S` for SSCP analysis;
 *
 * The function will read the input data and allocate memory for the chosen analysis option.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	FILE *stream;
	int i, j;
	float in_value;

	/*********************************************************************
    Get from command line:
    input data file name, #rows, #cols, option.

    Open input file: fopen opens the file whose name is stored in the
    pointer parameters[0]; if unsuccessful, error message is printed to
    stderr.
   *********************************************************************/

	if (parameters_num != 4) {
		elogf(LOG_LEVEL_ERR,"Syntax help: PCA filename #rows #cols option\n\n");
		elogf(LOG_LEVEL_ERR,"(filename -- give full path name,\n");
		elogf(LOG_LEVEL_ERR," #rows                          \n");
		elogf(LOG_LEVEL_ERR," #cols    -- integer values,\n");
		elogf(LOG_LEVEL_ERR," option   -- R (recommended) for correlation analysis,\n");
		elogf(LOG_LEVEL_ERR,"             V for variance/covariance analysis\n");
		elogf(LOG_LEVEL_ERR,"             S for SSCP analysis.)\n");
		exit(1);
	}

	n = atoi(parameters[1]); /* # rows */
	m = atoi(parameters[2]); /* # columns */
	strncpy(&option, parameters[3], 1); /* Analysis option */

	elogf(LOG_LEVEL_TRACE,"No. of rows: %d, no. of columns: %d.\n", n, m);
	elogf(LOG_LEVEL_TRACE,"Input file: %s.\n", parameters[0]);

	if ((stream = fopen(parameters[0], "r")) == NULL) {
		elogf(LOG_LEVEL_ERR,"Program cannot open file %s\n", parameters[0]);
		elogf(LOG_LEVEL_ERR, "Exiting to system.");
		return -1;
	}

	/* Now read in data. */

	data = matrix(n, m); /* Storage allocation for input data */

	for (i = 1; i <= n; i++) {
		for (j = 1; j <= m; j++) {
			fscanf(stream, "%f", &in_value);
			data[i][j] = in_value;
			//elogf(LOG_LEVEL_TRACE,"at row %d column %d is %lf\n", i, j,
			//       data[i][j]);
		}
	}

	/* Check on (part of) input data.
     for (i = 1; i <= 18; i++) {for (j = 1; j <= 8; j++)  {
     elogf(LOG_LEVEL_TRACE,"%7.1f", data[i][j]);  }  elogf(LOG_LEVEL_TRACE,"\n");  }
     */

	symmat = matrix(m, m); /* Allocation of correlation (etc.) matrix */

	/* Allocate storage for dummy and new vectors. */
	evals = vector(m); /* Storage alloc. for vector of eigenvalues */
	elogf(LOG_LEVEL_TRACE,"the vector storage size is %d\n", m);
	interm = vector(m); /* Storage alloc. for 'intermediate' vector */
	symmat2 = matrix(m, m); /* Duplicate of correlation (etc.) matrix */

	/* Allocate storage for mean and std. dev. vectors */

	mean = vector(m);
	stddev = vector(m);
	return 0;
}

/**
 * @brief This handler is where the PCA analysis is computed.
 * @param[in] parameters_num Number of passed parameters, ignored.
 * @param[in] parameters The list of passed parameters, ignored.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	int i, j, k, k2;
	/* Look at analysis option; branch in accordance with this. */

	switch (option) {
	case 'R':
	case 'r':
		elogf(LOG_LEVEL_TRACE,"Analysis of correlations chosen.\n");
		corcol(data, n, m, symmat, mean, stddev);

		/* Output correlation matrix.
         for (i = 1; i <= m; i++) {
         for (j = 1; j <= 8; j++)  {
         elogf(LOG_LEVEL_TRACE,"%7.4f", symmat[i][j]);  }
         elogf(LOG_LEVEL_TRACE,"\n");  }
         */
		break;
	case 'V':
	case 'v':
		elogf(LOG_LEVEL_TRACE,"Analysis of variances-covariances chosen.\n");
		covcol(data, n, m, symmat, mean);

		/* Output variance-covariance matrix.
         for (i = 1; i <= m; i++) {
         for (j = 1; j <= 8; j++)  {
         elogf(LOG_LEVEL_TRACE,"%7.1f", symmat[i][j]);  }
         elogf(LOG_LEVEL_TRACE,"\n");  }
         */
		break;
	case 'S':
	case 's':
		elogf(LOG_LEVEL_TRACE,"Analysis of sums-of-squares-cross-products");
		elogf(LOG_LEVEL_TRACE," matrix chosen.\n");
		scpcol(data, n, m, symmat);

		/* Output SSCP matrix.
         for (i = 1; i <= m; i++) {
         for (j = 1; j <= 8; j++)  {
         elogf(LOG_LEVEL_TRACE,"%7.1f", symmat[i][j]);  }
         elogf(LOG_LEVEL_TRACE,"\n");  }
         */
		break;
	default:
		elogf(LOG_LEVEL_ERR,"Option: %c\n", option);
		elogf(LOG_LEVEL_ERR,"For option, please type R, V, or S\n");
		elogf(LOG_LEVEL_ERR,"(upper or lower case).\n");
		elogf(LOG_LEVEL_ERR,"Exiting to system.\n");
		exit(1);
		break;
	}

	/*********************************************************************
    Eigen-reduction
   **********************************************************************/

	for (i = 1; i <= m; i++) {
		for (j = 1; j <= m; j++) {
			symmat2[i][j] =
				symmat[i]
				      [j]; /* Needed below for col. projections */
		}
	}
	tred2(symmat, m, evals, interm); /* Triangular decomposition */
	//elogf(LOG_LEVEL_TRACE,"eval value at 0 is %lf\n", evals[0]);
	//elogf(LOG_LEVEL_TRACE,"eval value at 1 is %lf\n", evals[1]);
	//elogf(LOG_LEVEL_TRACE,"eval value at 2 is %lf\n", evals[2]);

	//elogf(LOG_LEVEL_TRACE,"m/height is %lf \n", m);

	//elogf(LOG_LEVEL_TRACE,"m/height is %d \n", m);
	//elogf(LOG_LEVEL_TRACE,"m/height is %d \n", n);
	//elogf(LOG_LEVEL_TRACE,"m/height is %d \n", n);

	tqli(evals, interm, m, symmat); /* Reduction of sym. trid. matrix */
	/* evals now contains the eigenvalues,
     columns of symmat now contain the associated eigenvectors. */

	//elogf(LOG_LEVEL_TRACE,"\nEigenvalues:\n");
	for (j = m; j >= 1; j--) {
		//elogf(LOG_LEVEL_TRACE,"%18.5f\n", evals[j]);
	}
	//elogf(LOG_LEVEL_TRACE,"\n(Eigenvalues should be strictly positive; limited\n");
	//elogf(LOG_LEVEL_TRACE,"precision machine arithmetic may affect this.\n");
	//elogf(LOG_LEVEL_TRACE,"Eigenvalues are often expressed as cumulative\n");
	//elogf(LOG_LEVEL_TRACE,"percentages, representing the 'percentage variance\n");
	//elogf(LOG_LEVEL_TRACE,"explained' by the associated axis or principal component.)\n");

	//elogf(LOG_LEVEL_TRACE,"\nEigenvectors:\n");
	//elogf(LOG_LEVEL_TRACE,"(First three; their definition in terms of original vbes.)\n");
	for (j = 1; j <= m; j++) {
		for (i = 1; i <= 3; i++) {
			//elogf(LOG_LEVEL_TRACE,"%12.4f", symmat[j][m - i + 1]);
		}
		//elogf(LOG_LEVEL_TRACE,"\n");
	}

	/* Form projections of row-points on first three prin. components. */
	/* Store in 'data', overwriting original data. */
	for (i = 1; i <= n; i++) {
		for (j = 1; j <= m; j++) {
			interm[j] = data[i][j];
			/*elogf(LOG_LEVEL_TRACE,"Iteration i=%d j=%d data = %lf\n\n", i, j,
			       data[i][j]);*/
		}
		for (k = 1; k <= 3; k++) {
			data[i][k] = 0.0;
			for (k2 = 1; k2 <= m; k2++) {
				data[i][k] +=
					interm[k2] * symmat[k2][m - k + 1];
				/*elogf(LOG_LEVEL_TRACE,"Iteration i= %d, j = %d k=%d, k2=%d : data = %lf\n",
				       i, j, k, k2, data[i][k]);*/
			}
			//elogf(LOG_LEVEL_TRACE,"\n");
		}
	}

	//elogf(LOG_LEVEL_TRACE,"\nProjections of row-points on first 3 prin. comps.:\n");
	for (i = 1; i <= n; i++) {
		for (j = 1; j <= 3; j++) {
			//elogf(LOG_LEVEL_TRACE,"%12.4f", data[i][j]);
		}
		//elogf(LOG_LEVEL_TRACE,"\n");
	}
	return;

	/* Form projections of col.-points on first three prin. components. */
	/* Store in 'symmat2', overwriting what was stored in this. */
	for (j = 1; j <= m; j++) {
		for (k = 1; k <= m; k++) {
			interm[k] = symmat2[j][k];
		} /*symmat2[j][k] will be overwritten*/
		for (i = 1; i <= 3; i++) {
			symmat2[j][i] = 0.0;
			for (k2 = 1; k2 <= m; k2++) {
				symmat2[j][i] +=
					interm[k2] * symmat[k2][m - i + 1];
			}
			if (evals[m - i + 1] >
			    0.0005) /* Guard against zero eigenvalue */
				symmat2[j][i] /=
					sqrt(evals[m - i + 1]); /* Rescale */
			else
				symmat2[j][i] = 0.0; /* Standard kludge */
		}
	}

	//elogf(LOG_LEVEL_TRACE,"\nProjections of column-points on first 3 prin. comps.:\n");
	for (j = 1; j <= m; j++) {
		for (k = 1; k <= 3; k++) {
			//elogf(LOG_LEVEL_TRACE,"%12.4f", symmat2[j][k]);
		}
		//elogf(LOG_LEVEL_TRACE,"\n");
	}
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will deallocate memoory allocated in `benchmark_init()`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	if (data != NULL) {
		free_matrix(data, n, m);
	}
	if (symmat != NULL) {
		free_matrix(symmat, m, m);
	}
	if (symmat2 != NULL) {
		free_matrix(symmat2, m, m);
	}
	if (evals != NULL) {
		free_vector(evals, m);
	}
	if (interm != NULL) {
		free_vector(interm, m);
	}
	if (mean != NULL) {
		free_vector(mean, m);
	}
	if (stddev != NULL) {
		free_vector(stddev, m);
	}
}
