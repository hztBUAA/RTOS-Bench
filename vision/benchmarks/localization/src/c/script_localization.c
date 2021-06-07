/**
 * @file script_localization.c
 * @ingroup localization
 * @brief Functions used to run the localization benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the execution portion.
 * @author Sravanthi Kota Venkata, for the original version.
 */

#include <stdio.h>
#include <stdlib.h>
#include "localization.h"
#include "logging.h"

///Data from the input file.
static F2D *fid;
///The array where each row of `::fid` will be stored.
static F2D **sData_cached;
///Where additional data for each `::fid` row will be stored.
static I2D **sType_cached;
///Number of rows of `::fid` stored as an array.
static int cached_data_len = 0;

/**
 * @brief Will load the data that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: input folder path;
 *
 * The initialization will load the input file and store its content, row by row, in memory.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	char im1[100];
	int i;
	F2D *sData_tmp;
	I2D *sType;
	I2D *index, *isEOF;

	if (parameters_num < 1) {
		printf("We need input image path\n");
		return -1;
	}

	sprintf(im1, "%s/1.txt", parameters[0]);
	fid = readFile(im1);

	index = iSetArray(1, 1, -1);
	sType = iSetArray(1, 1, -1);
	isEOF = iSetArray(1, 1, -1);

	//discover the number of elements to cache in memory
	while (1) {
		sData_tmp = readSensorData(index, fid, sType, isEOF);
		cached_data_len++;
		fFreeHandle(sData_tmp);
		if (asubsref(isEOF, 0) == 1)
			break;
	}

	sData_cached = malloc(sizeof(F2D *) * cached_data_len);
	sType_cached = malloc(sizeof(I2D *) * cached_data_len);

	iFreeHandle(index);
	iFreeHandle(sType);
	iFreeHandle(isEOF);

	index = iSetArray(1, 1, -1);
	isEOF = iSetArray(1, 1, -1);

	for (i = 0; i < cached_data_len; i++) {
		sType_cached[i] = iSetArray(1, 1, -1);
		sData_cached[i] =
			readSensorData(index, fid, sType_cached[i], isEOF);
	}

	iFreeHandle(index);
	iFreeHandle(isEOF);
	return 0;
}

/**
 * @brief This handler is where the  robot localization is performed.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path (which is generally the same as the input folder path) as only element of the parameters array.
 * This is needed only is the macro `CHECK` or `GENERATE_OUTPUT` are defined.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	int i, j, k, icount = -1;
	int sData_index;
	float STDDEV_GPSVel = 0.5;
	float STDDEV_ACCL = 1;
	float M_STDDEV_GYRO = 0.1;
	float M_STDDEV_POS = 0.1;
	float pi = 3.1416;
	int rows, cols;

	F2D *eul1, *eul2, *quat;
	F2D *sData, *gyro, *norm_gyro, *angleAlpha;
	F2D *quatDelta, *Opos, *temp_STDDEV_GPSPos, *w;
	F2D *qConj, *orgWorld, *accl, *gtemp;
	F2D *gravity, *t1;
	I2D *tStamp, *sType;
	F2D *randn;

	F2D *randW, *eulAngle;

	F2D *pos, *vel, *ones;
	F2D *resultMat;
	F2D *STDDEV_GPSPos;
	float STDDEV_ODOVel = 0.1;
	int n = 1000;
	float gyroTimeInterval = 0.01;
	float acclTimeInterval = 0.01;
	float M_STDDEV_VEL = 0.02;

#ifdef test
	n = 3;
	gyroTimeInterval = 0.1;
	acclTimeInterval = 0.1;
	M_STDDEV_VEL = 0.2;
#endif
#ifdef sim_fast
	n = 3;
#endif
#ifdef sim
	n = 10;
#endif
#ifdef sqcif
	n = 800;
#endif
#ifdef qcif
	n = 500;
#endif
#ifdef vga
	n = 2000;
#endif
#ifdef wuxga
	n = 3000;
#endif

#ifdef CHECK
	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR,
		      "We need the path to the output dir, with the correct result file\n");
		exit(-1);
	}
#endif

	resultMat = fSetArray(3, fid->height, 0);

	pos = fSetArray(n, 3, 0);
	vel = fSetArray(n, 3, 0);
	ones = fSetArray(n, 1, 1);

	randn = randWrapper(n, 3);

	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
			subsref(vel, i, j) +=
				subsref(randn, i, j) * STDDEV_ODOVel;

	fFreeHandle(randn);
	STDDEV_GPSPos = fSetArray(3, 3, 0);

	{
		eulAngle = fSetArray(n, 3, 0);
		F2D *randn;
		randn = randWrapper(n, 1);

		for (i = 0; i < n; i++) {
			subsref(eulAngle, i, 2) = subsref(randn, i, 0) * 2 * pi;
		}

		eul1 = eul2quat(eulAngle);
		fFreeHandle(eulAngle);

		eulAngle = fSetArray(1, 3, 0);
		subsref(eulAngle, 0, 0) = pi;
		eul2 = eul2quat(eulAngle);

		fFreeHandle(randn);
		fFreeHandle(eulAngle);
	}

	quat = quatMul(eul1, eul2);
	fFreeHandle(eul1);
	fFreeHandle(eul2);

	i = 0;

	rows = 0;
	cols = 5;
	randW = randnWrapper(n, 3);

	for (sData_index = 0; sData_index < cached_data_len; sData_index++) {
		sData = sData_cached[sData_index];
		sType = sType_cached[sData_index];

		icount = icount + 1;

		/*
        S = 4
        A = 3
        G = 2
        V = 1
        */

		rows++;

		if (asubsref(sType, 0) == 2) {
			//Motion model

			{
				int i;
				F2D *t, *t1;
				F2D *abc, *abcd;
				int qD_r = 0, qD_c = 0;
				F2D *cosA, *sinA;

				t = fDeepCopyRange(sData, 0, 1, 0, 3);
				gyro = fMtimes(ones, t);
				abc = fMallocHandle(gyro->height, gyro->width);
				t1 = fDeepCopy(randW);

				for (i = 0; i < (n * 3); i++) {
					asubsref(t1, i) = asubsref(randW, i) *
							  M_STDDEV_GYRO;
					asubsref(gyro, i) += asubsref(t1, i);
					asubsref(abc, i) =
						pow(asubsref(gyro, i), 2);
				}
				fFreeHandle(t1);
				abcd = fSum2(abc, 2);

				norm_gyro = fMallocHandle(abcd->height,
							  abcd->width);
				angleAlpha = fMallocHandle(abcd->height,
							   abcd->width);

				for (i = 0; i < (abcd->height * abcd->width);
				     i++) {
					asubsref(norm_gyro, i) =
						sqrt(asubsref(abcd, i));
					asubsref(angleAlpha, i) =
						asubsref(norm_gyro, i) *
						gyroTimeInterval;
				}

				qD_r += angleAlpha->height + gyro->height;
				qD_c += angleAlpha->width + 3;

				fFreeHandle(t);
				fFreeHandle(abcd);

				cosA = fSetArray(angleAlpha->height,
						 angleAlpha->width, 0);
				sinA = fSetArray(angleAlpha->height,
						 angleAlpha->width, 0);

				for (i = 0; i < (cosA->height * cosA->width);
				     i++)
					asubsref(cosA, i) = cos(
						asubsref(angleAlpha, i) / 2);

				for (i = 0; i < (sinA->height * sinA->width);
				     i++)
					asubsref(sinA, i) = sin(
						asubsref(angleAlpha, i) / 2);

				fFreeHandle(abc);
				abc = fSetArray(1, 3, 1);
				t1 = fMtimes(norm_gyro, abc);
				t = ffDivide(gyro, t1);
				fFreeHandle(t1);

				abcd = fMtimes(sinA, abc);
				t1 = fTimes(t, abcd);
				quatDelta = fHorzcat(cosA, t1);

				fFreeHandle(abcd);
				fFreeHandle(t);
				fFreeHandle(t1);
				fFreeHandle(abc);

				t = quatMul(quat, quatDelta);
				fFreeHandle(quat);
				fFreeHandle(quatDelta);
				quat = fDeepCopy(t);

				fFreeHandle(t);
				fFreeHandle(norm_gyro);
				fFreeHandle(gyro);
				fFreeHandle(angleAlpha);
				fFreeHandle(cosA);
				fFreeHandle(sinA);
			}
		}

		if (asubsref(sType, 0) == 4) {
			//Observation

			float tempSum = 0;
			F2D *Ovel;
			float OvelNorm;
			int i;

			asubsref(STDDEV_GPSPos, 0) = asubsref(sData, 6);
			asubsref(STDDEV_GPSPos, 4) = asubsref(sData, 7);
			asubsref(STDDEV_GPSPos, 8) = 15;

			Opos = fDeepCopyRange(sData, 0, 1, 0, 3);

			//Initialize

			for (i = 0; i < (pos->height * pos->width); i++)
				tempSum += asubsref(pos, i);

			if (tempSum == 0) {
				F2D *t, *t1;
				t = fMtimes(randW, STDDEV_GPSPos);
				t1 = fMtimes(ones, Opos);

				for (i = 0; i < (pos->height * pos->width); i++)
					asubsref(pos, i) = asubsref(t, i) +
							   asubsref(t1, i);

				fFreeHandle(t);
				fFreeHandle(t1);
			} else {
				int rows, cols;
				int mnrows, mncols;

				rows = STDDEV_GPSPos->height;
				cols = STDDEV_GPSPos->width;

				temp_STDDEV_GPSPos = fSetArray(rows, cols, 1);
				for (mnrows = 0; mnrows < rows; mnrows++) {
					for (mncols = 0; mncols < cols;
					     mncols++) {
						subsref(temp_STDDEV_GPSPos,
							mnrows, mncols) =
							pow(subsref(STDDEV_GPSPos,
								    mnrows,
								    mncols),
							    -1);
					}
				}

				w = mcl(pos, Opos, temp_STDDEV_GPSPos);
				fFreeHandle(temp_STDDEV_GPSPos);
				generateSample(w, quat, vel, pos);
			}
			fFreeHandle(Opos);

			//compare direction
			Ovel = fDeepCopyRange(sData, 0, 1, 3, 3);
			OvelNorm = 2; //1.1169e+09;

			if (OvelNorm > 0.5) {
				F2D *t;
				t = fDeepCopy(Ovel);
				fFreeHandle(Ovel);
				/* This is a double precision division */
				Ovel = fDivide(t, OvelNorm);
				qConj = quatConj(quat);
				fFreeHandle(t);

				{
					t = fSetArray(1, 3, 0);
					subsref(t, 0, 0) = 1;
					orgWorld = quatRot(t, qConj);
					fFreeHandle(t);
					fFreeHandle(qConj);
					t = fSetArray(3, 3, 0);
					asubsref(t, 0) = 1;
					asubsref(t, 4) = 1;
					asubsref(t, 8) = 1;

					{
						int i;
						for (i = 0;
						     i < (t->height * t->width);
						     i++)
							asubsref(t, i) =
								asubsref(t, i) /
								STDDEV_GPSVel;
						w = mcl(orgWorld, Ovel, t);
						generateSample(w, quat, vel,
							       pos);
					}

					fFreeHandle(t);
					fFreeHandle(w);
					fFreeHandle(orgWorld);
				}
			}
			fFreeHandle(Ovel);
		}

		if (asubsref(sType, 0) == 1) {
			//Observation
			F2D *Ovel;
			F2D *t, *t1, *t2;
			float valVel;

			t = fSetArray(vel->height, 1, 0);

			for (i = 0; i < vel->height; i++) {
				subsref(t, i, 0) =
					sqrt(pow(subsref(vel, i, 0), 2) +
					     pow(subsref(vel, i, 1), 2) +
					     pow(subsref(vel, i, 2), 2));
			}

			Ovel = fSetArray(1, 1, asubsref(sData, 0));
			valVel = 1.0 / STDDEV_ODOVel;

			t1 = fSetArray(1, 1, (1.0 / STDDEV_ODOVel));
			w = mcl(t, Ovel, t1);
			generateSample(w, quat, vel, pos);

			fFreeHandle(w);
			fFreeHandle(t);
			fFreeHandle(t1);
			fFreeHandle(Ovel);
		}

		if (asubsref(sType, 0) == 3) {
			//Observation
			F2D *t;
			t = fSetArray(1, 3, 0);
			asubsref(t, 2) = -9.8;

			accl = fDeepCopyRange(sData, 0, 1, 0, 3);
			gtemp = fMtimes(ones, t);
			gravity = quatRot(gtemp, quat);

			fFreeHandle(gtemp);
			fFreeHandle(t);
			t = fSetArray(3, 3, 0);
			asubsref(t, 0) = 1;
			asubsref(t, 4) = 1;
			asubsref(t, 8) = 1;

			{
				int i;
				for (i = 0; i < (t->height * t->width); i++)
					asubsref(t, i) =
						asubsref(t, i) / STDDEV_ACCL;
				w = mcl(gravity, accl, t);
			}

			generateSample(w, quat, vel, pos);
			fFreeHandle(t);
			//Motion model
			t = fMtimes(ones, accl);
			fFreeHandle(accl);

			accl = fMinus(t, gravity);

			fFreeHandle(w);
			fFreeHandle(gravity);
			fFreeHandle(t);

			{
				//pos=pos+quatRot(vel,quatConj(quat))*acclTimeInterval+1/2*quatRot(accl,quatConj(quat))*acclTimeInterval^2+randn(n,3)*M_STDDEV_POS;

				F2D *s, *is;
				int i;
				is = quatConj(quat);
				s = quatRot(vel, is);
				fFreeHandle(is);

				for (i = 0; i < (s->height * s->width); i++) {
					asubsref(s, i) =
						asubsref(s, i) *
						acclTimeInterval; //+(1/2);
				}
				is = fPlus(pos, s);
				fFreeHandle(pos);
				pos = fDeepCopy(is);
				fFreeHandle(is);
				fFreeHandle(s);

				/* pos_ above stores: pos+quatRot(vel,quatConj(quat))*acclTimeInterval */

				is = quatConj(quat);
				s = quatRot(accl, is);
				t = fDeepCopy(s);
				for (i = 0; i < (s->height * s->width); i++) {
					asubsref(t, i) = 1 / 2 *
							 asubsref(s, i) *
							 acclTimeInterval *
							 acclTimeInterval;
				}

				/* t_ above stores: 1/2*quatRot(accl,quatCong(quat))*acclTimeInterval^2 */

				fFreeHandle(s);
				fFreeHandle(is);
				s = randnWrapper(n, 3);

				for (i = 0; i < (s->height * s->width); i++) {
					asubsref(s, i) =
						asubsref(s, i) * M_STDDEV_POS;
				}

				/* s_ above stores: randn(n,3)*M_STDDEV_POS */

				is = fPlus(pos, t);
				fFreeHandle(pos);
				pos = fPlus(is, s);

				fFreeHandle(s);
				fFreeHandle(t);
				fFreeHandle(is);

				//            vel=vel+accl*acclTimeInterval+randn(n,3)*M_STDDEV_VEL; %??

				t = fDeepCopy(accl);
				for (i = 0; i < (accl->height * accl->width);
				     i++)
					asubsref(t, i) = asubsref(accl, i) *
							 acclTimeInterval;

				is = fPlus(vel, t);

				fFreeHandle(accl);
				fFreeHandle(t);
				s = randnWrapper(n, 3);
				for (i = 0; i < (s->height * s->width); i++) {
					asubsref(s, i) =
						asubsref(s, i) * M_STDDEV_VEL;
				}

				fFreeHandle(vel);
				vel = fPlus(is, s);
				fFreeHandle(is);
				fFreeHandle(s);
			}
		}

		// Self check
		{
			F2D *temp;
			float quatOut = 0, velOut = 0, posOut = 0;
			int i;

			for (i = 0; i < (quat->height * quat->width); i++)
				quatOut += asubsref(quat, i);

			for (i = 0; i < (vel->height * vel->width); i++)
				velOut += asubsref(vel, i);

			for (i = 0; i < (pos->height * pos->width); i++)
				posOut += asubsref(pos, i);

			subsref(resultMat, 0, icount) = quatOut;
			subsref(resultMat, 1, icount) = velOut;
			subsref(resultMat, 2, icount) = posOut;
		}
	}

	elogf(LOG_LEVEL_TRACE, "Input size\t\t- (%dx%dx%d)\n", rows, cols, n);
#ifdef CHECK

	// Self checking - use expected.txt from data directory
	{
		int ret = 0;
		float tol = 2.0;
#ifdef GENERATE_OUTPUT
		fWriteMatrix(resultMat, parameters[0]);
#endif
		ret = fSelfCheck(resultMat, parameters[0], tol);
		if (ret == -1)
			elogf(LOG_LEVEL_TRACE, "Error in Localization\n");
	}
	// Self checking done
#endif

	fFreeHandle(quat);
	fFreeHandle(STDDEV_GPSPos);
	fFreeHandle(resultMat);
	fFreeHandle(pos);
	fFreeHandle(vel);
	fFreeHandle(ones);
	fFreeHandle(randW);
}

/**
 * @brief Will revert what benchmark_init() has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will free the data cached in `::sData_cached`, `::sType_cached` and `::fid`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	int i;
	for (i = 0; i < cached_data_len; i++) {
		iFreeHandle(sType_cached[i]);
		fFreeHandle(sData_cached[i]);
	}
	free(sType_cached);
	free(sData_cached);
	fFreeHandle(fid);
}
