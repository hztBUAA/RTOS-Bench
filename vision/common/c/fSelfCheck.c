/********************************
Author: Sravanthi Kota Venkata
********************************/

#include "sdvbs_common.h"
#include "logging.h"

int fSelfCheck(F2D *in1, char *path, float tol)
{
	int r1, c1, ret = 1;
	float *buffer;
	FILE *fd;
	int count = 0, i, j;
	char file[256];

	r1 = in1->height;
	c1 = in1->width;

	buffer = (float *)malloc(sizeof(float) * r1 * c1);

	sprintf(file, "%s/expected_C.txt", path);
	fd = fopen(file, "r");

	if (fd == NULL) {
		free(buffer);
		elogf(LOG_LEVEL_ERR, "Error: Expected file not opened %s\n",
		      file);
		return -1;
	}

	while (!feof(fd)) {
		fscanf(fd, "%f", &buffer[count]);
		count++;
	}
	count--;

	if (count != (r1 * c1)) {
		elogf(LOG_LEVEL_ERR,
		      "Checking error: dimensions mismatch. Expected = %d, Observed = %d \n",
		      count, (r1 * c1));
		fclose(fd);
		free(buffer);
		return -1;
	}

	for (i = 0; i < r1 * c1; i++) {
		float inVal = asubsref(in1, i);

		if ((inVal - buffer[i]) > tol || (buffer[i] - inVal) > tol) {
			elogf(LOG_LEVEL_ERR, "Mismatch %d: (%f, %f)\n", i,
			      buffer[i], inVal);
			fclose(fd);
			free(buffer);
			return -1;
		}
	}

	fclose(fd);
	free(buffer);
	elogf(LOG_LEVEL_TRACE, "Verification\t\t- Successful\n");
	return ret;
}
