#include <stddef.h>
#include <stdlib.h>
/* #include <ctype.h> */
#include <math.h>
#include <stdio.h>

/* #include <dbEvent.h> */
#include <dbDefs.h>
#include <dbCommon.h>
#include <recSup.h>
#include <genSubRecord.h>

#define MAXORDER 10
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define NINT(f)  (int)((f)>0 ? (f)+0.5 : (f)-0.5)
static double polyInterp(double *x, double *ya, int n, double xx);

volatile int interpDebug=0;

static long interp_init(genSubRecord *pgsub)
{
	double	*a, *b, *c, *lo_lim, *hi_lim;
	int i, j, n=100, *order;
	long *e;

	e = (long *)pgsub->e;
	if (*e == 0) {
		a = (double *)pgsub->a;
		b = (double *)pgsub->b;
		c = (double *)pgsub->c;
		lo_lim = (double *)pgsub->valf;
		hi_lim = (double *)pgsub->valg;
		order = (int *)pgsub->f;
		*e = n;
		for (i=0; i<n/2; i++) {
			a[i] = 4*3.141592654*i/(n-1);
			b[i] = sin(a[i]);
			c[i] = cos(a[i]);
		}
		for (i=n/2, j=0; i<n; i++, j++) {
			a[i] = 4*3.141592654*i/(n-1);
			b[i] = j<5 ? 0 : 1;
			c[i] = j>5 ? 0 : 1;
			if (j > 10) j = 0;
		}
		*lo_lim = 0;
		*hi_lim = 4*3.141592654;
	}
	return(0);
}

static long interp_do(genSubRecord *pgsub)
{
	double	*a, *b, *c, *d, *lo_lim, *hi_lim, ix;
	double	*valb, *valc;
	int    	hi, lo, n, mid, i, s=0, first, *order;

	a = (double *)pgsub->a;
	b = (double *)pgsub->b;
	c = (double *)pgsub->c;
	d = (double *)pgsub->d;
	n = *(int *)pgsub->e;
	valb = (double *)pgsub->valb;
	valc = (double *)pgsub->valc;
	lo_lim = (double *)pgsub->valf;
	hi_lim = (double *)pgsub->valg;
	order = (int *)pgsub->f;

	if (interpDebug) printf("interp: x=%f, ", *d);
	/* if arrays haven't been set up yet, output is same as input */
	if (n <= 1) {
		*valb = *valc = *d;
		return(0);
	}
	if (*d < a[0]) {*valb = b[0]; *valc = c[0]; return(-1);}
	if (*d > a[n-1]) {*valb = b[n-1]; *valc = c[n-1]; return(-1);}

	/* find limits of independent variable */
	*lo_lim = *hi_lim  = a[0];
	for (i=1; i<n; i++) {
		if (a[i] < *lo_lim) *lo_lim = a[i];
		if (a[i] > *hi_lim) *hi_lim = a[i];
	}
	/* binary search for indexes of a[] bracketing *d */
	for (lo=0, hi=n-1, mid = (hi+lo)/2; abs(hi-lo)>1;) {
		if (*d > a[mid]) {
			lo = mid;
		} else {
			hi = mid;
		}
		mid = (hi+lo)/2;
	}
	/* index at which *d would occur in a[] */
	ix = lo + (*d-a[lo])/(a[hi]-a[lo]);
	if (interpDebug) printf(" ix=%f, ", ix);

	if (*order > 1) {
		/* polynomial */
		i = MIN(*order+1, n);	/* number of points required */
		/* arrange that we switch from one set of points to the next */
		/* when *d crosses a point, and not halfway between */
		first = (int)(ix - i/2) + ((i%2)?0:1);
		first = MAX(0, MIN(n-i, first));
		*valb = polyInterp(&a[first], &b[first], i, *d);
		*valc = polyInterp(&a[first], &c[first], i, *d);
		if (interpDebug >= 10) {
			printf("poly (O%d, first=%d) valb = %f\n",
				*order, first, *valb);
		}
	} else {
		/* linear interpolation */
		*valb = b[lo] + (ix-lo)*(b[hi]-b[lo]);
		*valc = c[lo] + (ix-lo)*(c[hi]-c[lo]);
		if (interpDebug) printf("linear (%d:%f:%d) valb = %f\n",
			lo, ix, hi, *valb);
	}
	if (interpDebug) {
		printf("\n");
		*valc = b[lo] + (ix-lo)*(b[hi]-b[lo]);
	}
	return(s);
}

/* Lagrange interpolation */
static double polyInterp(double *x, double *y, int n, double xx)
{
	int i, j;
	double yy, p[MAXORDER+2];

	if (interpDebug >= 20) {
		printf("interp:polyInterp: x[0..%d] = [ ", n-1);
		for (i=0; i<=n; i++) printf("%f ", x[i]);
		printf("]\n");
	}
	for (i=0; i<n; i++) {
		p[i] = y[i];
		for (j=0; j<n; j++) {
			if (i != j) {
				if (x[i] == x[j]) {
					printf("Error in polynomial interpolation");
					return(0.0);
				}
				p[i] *= (xx-x[j])/(x[i]-x[j]);
			}
		}
	}
	yy = 0;
	for (i=0; i<n; i++) {
		if (interpDebug >= 20) printf("interp:polyInterp: p[i]=%f\n", p[i]);
		yy += p[i];
	}
	return(yy);
}

#include <registryFunction.h>
#include <epicsExport.h>

epicsExportAddress(int, interpDebug);

static registryFunctionRef interpRef[] = {
	{"interp_init", (REGISTRYFUNCTION)interp_init},
	{"interp_do", (REGISTRYFUNCTION)interp_do}
};

static void interpRegister(void) {
	registryFunctionRefAdd(interpRef, NELEMENTS(interpRef));
}

epicsExportRegistrar(interpRegister);
