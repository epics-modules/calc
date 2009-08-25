/* @(#)subAve.c	1.2 4/27/95     */
/* subAve.c -  */
/*
 * Author:      Frank Lenkszus
 * Date:        9/29/93
 *
 *      Experimental Physics and Industrial Control System (EPICS)
*/
/*
*****************************************************************
                          COPYRIGHT NOTIFICATION
*****************************************************************

THE FOLLOWING IS A NOTICE OF COPYRIGHT, AVAILABILITY OF THE CODE,
AND DISCLAIMER WHICH MUST BE INCLUDED IN THE PROLOGUE OF THE CODE
AND IN ALL SOURCE LISTINGS OF THE CODE.
 
(C)  COPYRIGHT 1993 UNIVERSITY OF CHICAGO
 
Argonne National Laboratory (ANL), with facilities in the States of 
Illinois and Idaho, is owned by the United States Government, and
operated by the University of Chicago under provision of a contract
with the Department of Energy.

Portions of this material resulted from work developed under a U.S.
Government contract and are subject to the following license:  For
a period of five years from March 30, 1993, the Government is
granted for itself and others acting on its behalf a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, and perform
publicly and display publicly.  With the approval of DOE, this
period may be renewed for two additional five year periods. 
Following the expiration of this period or periods, the Government
is granted for itself and others acting on its behalf, a paid-up,
nonexclusive, irrevocable worldwide license in this computer
software to reproduce, prepare derivative works, distribute copies
to the public, perform publicly and display publicly, and to permit
others to do so.

*****************************************************************
                                DISCLAIMER
*****************************************************************

NEITHER THE UNITED STATES GOVERNMENT NOR ANY AGENCY THEREOF, NOR
THE UNIVERSITY OF CHICAGO, NOR ANY OF THEIR EMPLOYEES OR OFFICERS,
MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY
OWNED RIGHTS.  

*****************************************************************
LICENSING INQUIRIES MAY BE DIRECTED TO THE INDUSTRIAL TECHNOLOGY
DEVELOPMENT CENTER AT ARGONNE NATIONAL LABORATORY (708-252-2000).
*/
/*
* Modification Log:
* -----------------
* .01  9-29-93  frl  initial
* .02  4-27-95  frl  added RESTART and MODE
*  03  9-05-02  tmm  If NUM_2_AVE (.A) is greater than allowed, set it
*                    to the maximum allowed number, so user can see it.
*                    Set restart field (.C) to zero after we use it.
*                    Report current sample via .E field.
*/

/*  subroutine to average data */
/*  F. Lenkszus */

#ifdef vxWorks
#include <vxWorks.h>
#else
#define OK 0
#define ERROR -1
#endif

#include <stdio.h>
#include <stdlib.h>

#include <alarm.h>
#include <cvtTable.h>
#include <dbDefs.h>
#include <dbAccess.h>
#include <recGbl.h>
#include <recSup.h>
#include <devSup.h>
#include <link.h>
#include <devLib.h>
#include <errlog.h>
#include <dbEvent.h>
#include <subRecord.h>
#include <registryFunction.h>
#include <epicsExport.h>

#define	CIRBUFSIZE	1000
#define NO_ERR_RPT	-1

#define NUM_2_AVE	psub->a
#define INPUTVAL	psub->b
#define RESTART		psub->c
#define MODE		psub->d
#define FILL		psub->e
#define ALGORITHM	psub->f

#define MODE_CONTINUOUS	0
#define MODE_STOPONNUM  1

#define ALGORITHM_AVERAGE	0
#define ALGORITHM_LINFIT	1

static double calculate_yFit(struct subRecord *psub);

int	debugSubAve = 0;
/* The following statement serves to make this debugging symbol available, 
 * but more importantly to force the linker to include this entire module
 */
epicsExportAddress(int, debugSubAve);

struct	fcirBuf {
	short	num;	/* Number of values to average */
	short	fill;	/* Number of values acquired thus far */
	short	algorithm;	/* Average or fit line */
	double	*yp;	/* Pointer to next y value */
	double	sum;	/* running sum of y values */
	double	ave;	/* running average of y values */
	double	ybuf[CIRBUFSIZE];
	epicsTimeStamp	*xp;	/* Pointer to next x value */
	epicsTimeStamp	xbuf[CIRBUFSIZE];
};

long	initSubAve(struct subRecord *psub)
{
	char	*xname="initSubAve";
	struct	fcirBuf	*p;
	short	i;

	if ((psub->dpvt = malloc(sizeof(struct fcirBuf))) == NULL) {
		errPrintf(S_dev_noMemory, __FILE__, __LINE__,
			"%s: couldn't allocate memory for %s", xname, psub->name);
		return(S_dev_noMemory);
	}
	p = (struct fcirBuf *)psub->dpvt;
	if (debugSubAve)
		printf("%s: Init completed for Subroutine Record %s\n", xname, psub->name);
	for (i=0 ; i<CIRBUFSIZE; i++) {
		p->ybuf[i] = 0;
	}
	p->num = 1;
	p->fill = 0;
	p->yp = p->ybuf;
	p->xp = p->xbuf;
	p->ave = p->sum = 0;
	p->algorithm = (short)ALGORITHM;
	return(OK);
}


long SubAve(struct subRecord *psub)
{
	char	*xname="SubAve";
	long	num;
	short	i, restart, algorithm;
	unsigned short monitor_mask;
	struct	fcirBuf	*p;

	if ((p = (struct fcirBuf *)psub->dpvt) == NULL) {
		if (debugSubAve)
			errPrintf(S_dev_noMemory, __FILE__, __LINE__,
				"%s: dpvt in NULL for %s", xname, psub->name);
		return(ERROR);
	}
	num = (long)NUM_2_AVE;
	algorithm = (short)ALGORITHM;
	if (num > CIRBUFSIZE ) {
		if (debugSubAve)
			errPrintf(NO_ERR_RPT, __FILE__, __LINE__,
				"%s: Num to ave (%ld) exceeds limit (%d) for PV %s",
				 xname, num, CIRBUFSIZE, psub->name);
		num = CIRBUFSIZE;
		NUM_2_AVE = num;
		db_post_events(psub, &psub->a, DBE_VALUE);
	}
	
	restart = RESTART;
	if (RESTART) {
		RESTART=0;
		db_post_events(psub, &psub->c, DBE_VALUE);
	}

	if (algorithm != p->algorithm) {
		restart = 1;
		p->algorithm = algorithm;
	}

	if ( ((num != p->num) && (MODE == MODE_CONTINUOUS)) || restart ) {
		/*
		 * Number of values to average changed, or user said 'restart',
		 * or user changed algorithm.
		 */
		for (i=0; i < p->num; i++)
			p->ybuf[i]=0;
		p->yp = p->ybuf;
		*p->yp = 0;
		p->xp = p->xbuf;
		p->fill = 0;
		p->num = num;
		p->ave = p->sum = 0;
	}
	if (p->fill == p->num) {
		if (MODE == MODE_CONTINUOUS) {
			p->sum += INPUTVAL - *p->yp;
			p->ave = p->sum/(double)num;
			*p->yp = INPUTVAL;
			if (++(p->yp) >= p->ybuf + p->num) {
				p->yp = p->ybuf;
				if (debugSubAve > 10) {
					for (i=0; i < p->num; i++)
						printf("ybuf[%d] = %f\n", i, p->ybuf[i]);
				}
			}
			epicsTimeGetCurrent(p->xp);
			if (++(p->xp) >= p->xbuf + p->num) {
				p->xp = p->xbuf;
			}
		}
		monitor_mask = recGblResetAlarms(psub);
		db_post_events(psub, &psub->val, monitor_mask);
	} else {
		recGblSetSevr(psub, SOFT_ALARM, MAJOR_ALARM);
		p->sum += INPUTVAL;
		p->ave = p->sum/(double)(++p->fill);
		*p->yp = INPUTVAL;
		if (++(p->yp) >= p->ybuf + p->num)
			p->yp = p->ybuf;
		epicsTimeGetCurrent(p->xp);
		if (++(p->xp) >= p->xbuf + p->num)
			p->xp = p->xbuf;
		if (debugSubAve > 10) {
			printf("y val = %f, x sec = %d\n", *(p->yp), (*(p->xp)).secPastEpoch);
		}
	}
	if (algorithm == ALGORITHM_AVERAGE) {
		psub->val = p->ave;
	} else {
		if (p->fill > 1) {
			psub->val = calculate_yFit(psub);
		} else {
			psub->val = INPUTVAL;
		}
	}

	if (debugSubAve) {
		printf("%s: result = %.3f, sum = %.3f, num = %d\n", xname, p->ave, p->sum, p->fill);
	}
	psub->e = p->fill;
	db_post_events(psub, &psub->e, DBE_VALUE);

	return(OK);
}

static double calculate_yFit(struct subRecord *psub) {
	struct	fcirBuf	*p = (struct fcirBuf *)psub->dpvt;
	double sumx=0, sumy=0, sumxy=0, sumx2=0;
	double x, y, m=0, b=0;
	int i, n;
	double	*yp = p->yp;	/* Pointer to y value */
	epicsTimeStamp	*xp = p->xp;	/* Pointer to x value */
	epicsTimeStamp	*currTime = NULL;	/* Pointer to x value */

	if (p->num < 2)
		return(0.0);
	n = p->fill;
	for (i=0; i<n; i++) {
		if (--yp < p->ybuf) yp = p->ybuf + (p->num-1);
		if (--xp < p->xbuf) xp = p->xbuf + (p->num-1);
		if (i == 0) currTime = xp;
		x = epicsTimeDiffInSeconds(currTime, xp);
		y = *yp;
		if (debugSubAve > 10) {
			printf("%s: x[%d] = %.3f, y[%d] = %.3f\n", psub->name, i, x, i, y);
		}
		sumx += x;
		sumy += y;
		sumxy += x*y;
		sumx2 += x*x;
	}
	m = (sumxy - sumx*sumy/n)/(sumx2 - sumx*sumx/n);
	b = (sumy - m * sumx)/n;
	if (debugSubAve > 10) {
		printf("%s: m = %.3f, b = %.3f\n", psub->name, m, b);
	}
	return(b);
}

static registryFunctionRef subAveRef[] = {
	{"initSubAve", (REGISTRYFUNCTION)initSubAve},
	{"SubAve", (REGISTRYFUNCTION)SubAve}
};

static void subAveRegister(void) {
	registryFunctionRefAdd(subAveRef, NELEMENTS(subAveRef));
}

epicsExportRegistrar(subAveRegister);
