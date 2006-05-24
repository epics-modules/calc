/* acalcout.c - Record Support Routines for array calc with output records */
/*
 *   Author : Tim Mooney
 *   Based on acalcoutRecord
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 2006, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 * Modification Log:
 * -----------------
 * 03-21-06    tmm    v1.0: created from scalcout record
 * 05-24-06    tmm    v1.1  call DSET->init_record()
 */

#define VERSION 1.1


#include	<stdlib.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#include	<epicsVersion.h>
#include	<alarm.h>
#include	<dbDefs.h>
#include	<dbAccess.h>
#include	<dbEvent.h>
#include	<dbScan.h>
#include	<errMdef.h>
#include	<recSup.h>
#include	<devSup.h>
#include	<recGbl.h>
#include	<special.h>
#include	<callback.h>
#include	<taskwd.h>
#include	"aCalcPostfix.h"

#define GEN_SIZE_OFFSET
#include	"aCalcoutRecord.h"
#undef  GEN_SIZE_OFFSET
#include	<menuIvoa.h>
#include	<epicsExport.h>

/* Create RSET - Record Support Entry Table*/
#define report NULL
#define initialize NULL
static long init_record();
static long process();
static long special();
#define get_value NULL
static long cvt_dbaddr();
static long get_array_info();
static long put_array_info();
static long get_units();
static long get_precision();
#define get_enum_str NULL
#define get_enum_strs NULL
#define put_enum_str NULL
static long get_graphic_double();
static long get_control_double();
static long get_alarm_double();

rset acalcoutRSET={
	RSETNUMBER,
	report,
	initialize,
	init_record,
	process,
	special,
	get_value,
	cvt_dbaddr,
	get_array_info,
	put_array_info,
	get_units,
	get_precision,
	get_enum_str,
	get_enum_strs,
	put_enum_str,
	get_graphic_double,
	get_control_double,
	get_alarm_double
};

typedef struct acalcoutDSET {
    long       number;
    DEVSUPFUN  dev_report;
    DEVSUPFUN  init;
    DEVSUPFUN  init_record;
    DEVSUPFUN  get_ioint_info;
    DEVSUPFUN  write;
} acalcoutDSET;

epicsExportAddress(rset, acalcoutRSET);

/* To provide feedback to the user as to the connection status of the 
 * links (.INxV and .OUTV), the following algorithm has been implemented ...
 *
 * A new PV_LINK [either in init() or special()] is searched for using
 * dbNameToAddr. If local, it is so indicated. If not, a checkLinkCb
 * callback is scheduled to check the connectivity later using 
 * dbCaIsLinkConnected(). Anytime there are unconnected CA_LINKs, another
 * callback is scheduled. Once all connections are established, the CA_LINKs
 * are checked whenever the record processes. 
 *
 */

#define NO_CA_LINKS     0
#define CA_LINKS_ALL_OK 1
#define CA_LINKS_NOT_OK 2

typedef struct rpvtStruct {
	CALLBACK	doOutCb;
	CALLBACK	checkLinkCb;
	short		wd_id_1_LOCK;
	short		caLinkStat; /* NO_CA_LINKS,CA_LINKS_ALL_OK,CA_LINKS_NOT_OK */
	short		outlink_field_type;
} rpvtStruct;

static void checkAlarms();
static void monitor();
static int fetch_values();
static void execOutput();
static void checkLinks();
static void checkLinksCallback();
static long writeValue(acalcoutRecord *pcalc);

volatile int    aCalcoutRecordDebug = 0;
epicsExportAddress(int, aCalcoutRecordDebug);

#define ARG_MAX 12
#define ARRAY_ARG_MAX 12


static long init_record(acalcoutRecord *pcalc, int pass)
{
	DBLINK *plink;
	int i;
	double *pvalue;
	unsigned short *plinkValid;
	short error_number;
	double **pa;
	char *s;
    acalcoutDSET *pacalcoutDSET;

	dbAddr       Addr;
	dbAddr       *pAddr = &Addr;
	rpvtStruct   *prpvt;

	if (pass==0) {
		pcalc->vers = VERSION;
		pcalc->rpvt = (void *)calloc(1, sizeof(struct rpvtStruct));
		/* allocate space for value arrays */
		pcalc->aa = (double *)calloc(ARRAY_ARG_MAX*pcalc->nelm, sizeof(double));
		pa=(double **)&(pcalc->aa);
		for (i=0; i<ARRAY_ARG_MAX; i++) {
			pa[i] = pcalc->aa + i*pcalc->nelm;
		}
		if (aCalcoutRecordDebug >= 10) printf("acalcoutRecord(%s):init: aa=%p, bb=%p\n",
				pcalc->name, (void *)pcalc->aa, (void *)pcalc->bb);
		/* allocate space for previous-value arrays */
		pcalc->paa = (double *)calloc(ARRAY_ARG_MAX*pcalc->nelm, sizeof(double));
		pa=(double **)&(pcalc->paa);
		for (i=0; i<ARRAY_ARG_MAX; i++)
			pa[i] = pcalc->paa + i*pcalc->nelm;
		pcalc->aval = (double *)calloc(pcalc->nelm, sizeof(double));
		pcalc->pavl = (double *)calloc(pcalc->nelm, sizeof(double));
		pcalc->oav = (double *)calloc(pcalc->nelm, sizeof(double));
		pcalc->poav = (double *)calloc(pcalc->nelm, sizeof(double));
		if (pcalc->nuse > pcalc->nelm) {
			pcalc->nuse = pcalc->nelm;
			db_post_events(pcalc,&pcalc->nuse,DBE_VALUE|DBE_LOG);
		}
		return(0);
	}
    
	if (!(pacalcoutDSET = (acalcoutDSET *)pcalc->dset)) {
		recGblRecordError(S_dev_noDSET,(void *)pcalc,"acalcout:init_record");
		return(S_dev_noDSET);
	}
	/* must have write defined */
	if ((pacalcoutDSET->number < 5) || (pacalcoutDSET->write == NULL)) {
		recGblRecordError(S_dev_missingSup,(void *)pcalc,"acalcout:init_record");
		return(S_dev_missingSup);
	}

	prpvt = (rpvtStruct *)pcalc->rpvt;
	plink = &pcalc->inpa;
	pvalue = &pcalc->a;
	plinkValid = &pcalc->inav;
	for (i=0; i<(ARG_MAX+ARRAY_ARG_MAX+1); i++, plink++, pvalue++, plinkValid++) {
		if (plink->type == CONSTANT) {
			/* Don't InitConstantLink the array links or the output link. */
			if (i < ARG_MAX) { 
				recGblInitConstantLink(plink,DBF_DOUBLE,pvalue);
				db_post_events(pcalc,pvalue,DBE_VALUE);
			}
			*plinkValid = acalcoutINAV_CON;
			if (plink == &pcalc->out)
				prpvt->outlink_field_type = DBF_NOACCESS;
        } else if (!dbNameToAddr(plink->value.pv_link.pvname, pAddr)) {
			/* see if the PV resides on this ioc */
			*plinkValid = acalcoutINAV_LOC;
			if (plink == &pcalc->out)
				prpvt->outlink_field_type = pAddr->field_type;
			if (aCalcoutRecordDebug && (pAddr->field_type >= DBF_INLINK) &&
					(pAddr->field_type <= DBF_FWDLINK)) {
				s = strchr(plink->value.pv_link.pvname, (int)' ') + 1;
				if (strncmp(s,"CA",2)) {
					printf("acalcoutRecord(%s):init_record:dblink to link field\n", pcalc->name);
				}
			}
		} else {
			/* pv is not on this ioc. Callback later for connection stat */
			*plinkValid = acalcoutINAV_EXT_NC;
			prpvt->caLinkStat = CA_LINKS_NOT_OK;
			if (plink == &pcalc->out)
				prpvt->outlink_field_type = DBF_NOACCESS; /* don't know field type */
		}
		db_post_events(pcalc,plinkValid,DBE_VALUE);
	}

	pcalc->clcv = aCalcPostfix(pcalc->calc,pcalc->rpcl,&error_number);
	if (pcalc->clcv) {
		recGblRecordError(S_db_badField,(void *)pcalc,
			"acalcout: init_record: Illegal CALC field");
		if (aCalcoutRecordDebug >= 10)
			printf("acalcPostfix returns: %d\n", error_number);
	}
	db_post_events(pcalc,&pcalc->clcv,DBE_VALUE);

	pcalc->oclv = aCalcPostfix(pcalc->ocal,(char *)pcalc->orpc,&error_number);
	if (pcalc->oclv) {
		recGblRecordError(S_db_badField,(void *)pcalc,
			"acalcout: init_record: Illegal OCAL field");
		if (aCalcoutRecordDebug >= 10)
			printf("acalcPostfix returns: %d\n", error_number);
	}
	db_post_events(pcalc,&pcalc->oclv,DBE_VALUE);

	callbackSetCallback(checkLinksCallback, &prpvt->checkLinkCb);
	callbackSetPriority(0, &prpvt->checkLinkCb);
	callbackSetUser(pcalc, &prpvt->checkLinkCb);
	prpvt->wd_id_1_LOCK = 0;

	if (prpvt->caLinkStat == CA_LINKS_NOT_OK) {
		callbackRequestDelayed(&prpvt->checkLinkCb,1.0);
		prpvt->wd_id_1_LOCK = 1;
	}

	if (pacalcoutDSET->init_record ) {
		return (*pacalcoutDSET->init_record)(pcalc);
	}
    return(0);
}

static long process(acalcoutRecord *pcalc)
{
	rpvtStruct   *prpvt = (rpvtStruct *)pcalc->rpvt;
	short		doOutput = 0;
	long		i, stat;

	if (aCalcoutRecordDebug) printf("acalcoutRecord(%s):process: pact=%d\n",
		pcalc->name, pcalc->pact);

	if (!pcalc->pact) {
		pcalc->pact = TRUE;

		/* if some links are CA, check connections */
		if (prpvt->caLinkStat != NO_CA_LINKS) checkLinks(pcalc);
		if (fetch_values(pcalc)==0) {
			if (aCalcoutRecordDebug >= 5) printf("acalcoutRecord(%s):process: calling aCalcPerform\n", pcalc->name);
			/* Note that we want to permit nuse == 0 as a way of saying "use nelm". */
			i = (pcalc->nuse > 0) ? pcalc->nuse : pcalc->nelm;
			stat = aCalcPerform(&pcalc->a, ARG_MAX, &pcalc->aa,
					ARRAY_ARG_MAX, i, &pcalc->val, pcalc->aval, pcalc->rpcl);
			if (i < pcalc->nelm) {
				for (; i<pcalc->nelm; i++) {
					pcalc->aval[i] = 0;
				}
			}
			if (aCalcoutRecordDebug >= 5) {
				printf("acalcoutRecord(%s):aCalcPerform returns val=%f, aval=[%f %f...]\n",
					pcalc->name, pcalc->val, pcalc->aval[0], pcalc->aval[1]);
			}
			if (stat)
				recGblSetSevr(pcalc,CALC_ALARM,INVALID_ALARM);
			else
				pcalc->udf = FALSE;
		}
		/* check for output link execution */
		switch (pcalc->oopt) {
		case acalcoutOOPT_Every_Time:
			doOutput = 1;
			break;
		case acalcoutOOPT_On_Change:
			if (fabs(pcalc->pval - pcalc->val) > pcalc->mdel) doOutput = 1;
			break;
		case acalcoutOOPT_Transition_To_Zero:
			if ((pcalc->pval != 0) && (pcalc->val == 0)) doOutput = 1;
			break;         
		case acalcoutOOPT_Transition_To_Non_zero:
			if ((pcalc->pval == 0) && (pcalc->val != 0)) doOutput = 1;
			break;
		case acalcoutOOPT_When_Zero:
			if (!pcalc->val) doOutput = 1;
			break;
		case acalcoutOOPT_When_Non_zero:
			if (pcalc->val) doOutput = 1;
			break;
		case acalcoutOOPT_Never:
			doOutput = 0;
			break;
		}
		pcalc->pval = pcalc->val;

		if (doOutput) {
			if (pcalc->odly > 0.0) {
				pcalc->dlya = 1;
				db_post_events(pcalc,&pcalc->dlya,DBE_VALUE);
                callbackRequestProcessCallbackDelayed(&prpvt->doOutCb,
                    pcalc->prio, pcalc, (double)pcalc->odly);
				if (aCalcoutRecordDebug >= 5)
					printf("acalcoutRecord(%s):process: exit, wait for delay\n",
						pcalc->name);
				return(0);
			} else {
				if (aCalcoutRecordDebug >= 5)
					printf("acalcoutRecord(%s):calling execOutput\n", pcalc->name);
                pcalc->pact = FALSE;
				execOutput(pcalc);
                if (pcalc->pact) {
					if (aCalcoutRecordDebug >= 5)
						printf("acalcoutRecord(%s):process: exit, pact==1\n",
							pcalc->name);
					return(0);
				}
				pcalc->pact = TRUE;
			}
		}
	} else { /* pact == TRUE */
		/* Who invoked us ? */
		if (pcalc->dlya) {
			/* callbackRequestProcessCallbackDelayed() called us */
			pcalc->dlya = 0;
			db_post_events(pcalc,&pcalc->dlya,DBE_VALUE);

            /* Must set pact 0 so that asynchronous device support works */
            pcalc->pact = 0;
			execOutput(pcalc);
            if (pcalc->pact) return(0);
            pcalc->pact = TRUE;
		} else {
			/* We must have been called by asynchronous device support */
            writeValue(pcalc);
		}
	}
    checkAlarms(pcalc);
    recGblGetTimeStamp(pcalc);

	if (aCalcoutRecordDebug >= 5) {
		printf("acalcoutRecord(%s):process:calling monitor aval=[%f %f...]\n",
			pcalc->name, pcalc->aval[0], pcalc->aval[1]);
	}

    monitor(pcalc);
    recGblFwdLink(pcalc);
    pcalc->pact = FALSE;

	if (aCalcoutRecordDebug >= 5) {
		printf("acalcoutRecord(%s):process-done aval=[%f %f...]\n",
			pcalc->name, pcalc->aval[0], pcalc->aval[1]);
	}
	if (aCalcoutRecordDebug) printf("acalcoutRecord(%s):process: exit, pact==0\n",
		pcalc->name);
	return(0);
}

static long special(dbAddr	*paddr, int after)
{
	acalcoutRecord	*pcalc = (acalcoutRecord *)(paddr->precord);
	rpvtStruct		*prpvt = (struct rpvtStruct *)pcalc->rpvt;
	dbAddr			Addr;
	dbAddr			*pAddr = &Addr;
	short			error_number;
	int				fieldIndex = dbGetFieldIndex(paddr);
	int				lnkIndex;
	DBLINK			*plink;
	double			*pvalue;
	unsigned short	*plinkValid;
	char			*s;

	if (!after) return(0);
	switch (fieldIndex) {
	case acalcoutRecordCALC:
		pcalc->clcv = aCalcPostfix(pcalc->calc, pcalc->rpcl, &error_number);
		if (pcalc->clcv) {
			recGblRecordError(S_db_badField,(void *)pcalc,
				"acalcout: special(): Illegal CALC field");
			if (aCalcoutRecordDebug >= 10)
				printf("acalcPostfix returns: %d\n", error_number);
		}
		db_post_events(pcalc,&pcalc->clcv,DBE_VALUE);
		return(0);
		break;

	case acalcoutRecordOCAL:
		pcalc->oclv = aCalcPostfix(pcalc->ocal, (char *)pcalc->orpc, &error_number);
		if (pcalc->oclv) {
			recGblRecordError(S_db_badField,(void *)pcalc,
				"acalcout: special(): Illegal OCAL field");
			if (aCalcoutRecordDebug >= 10)
				printf("acalcPostfix returns: %d\n", error_number);
		}
		db_post_events(pcalc,&pcalc->oclv,DBE_VALUE);
		return(0);
		break;

	case acalcoutRecordNUSE:
		if (pcalc->nuse > pcalc->nelm) {
			pcalc->nuse = pcalc->nelm;
			db_post_events(pcalc,&pcalc->nuse,DBE_VALUE);
			return(-1);
		}
		return(0);
		break;

	case(acalcoutRecordINPA):
	case(acalcoutRecordINPB):
	case(acalcoutRecordINPC):
	case(acalcoutRecordINPD):
	case(acalcoutRecordINPE):
	case(acalcoutRecordINPF):
	case(acalcoutRecordINPG):
	case(acalcoutRecordINPH):
	case(acalcoutRecordINPI):
	case(acalcoutRecordINPJ):
	case(acalcoutRecordINPK):
	case(acalcoutRecordINPL):
	case(acalcoutRecordINAA):
	case(acalcoutRecordINBB):
	case(acalcoutRecordINCC):
	case(acalcoutRecordINDD):
	case(acalcoutRecordINEE):
	case(acalcoutRecordINFF):
	case(acalcoutRecordINGG):
	case(acalcoutRecordINHH):
	case(acalcoutRecordINII):
	case(acalcoutRecordINJJ):
	case(acalcoutRecordINKK):
	case(acalcoutRecordINLL):
	case(acalcoutRecordOUT):
		lnkIndex = fieldIndex - acalcoutRecordINPA;
		plink   = &pcalc->inpa + lnkIndex;
		pvalue  = &pcalc->a    + lnkIndex;
		plinkValid = &pcalc->inav + lnkIndex;

		if (plink->type == CONSTANT) {
			if (fieldIndex <= acalcoutRecordINPL) {
				recGblInitConstantLink(plink,DBF_DOUBLE,pvalue);
				db_post_events(pcalc,pvalue,DBE_VALUE);
			}
			*plinkValid = acalcoutINAV_CON;
			if (fieldIndex == acalcoutRecordOUT)
				prpvt->outlink_field_type = DBF_NOACCESS;
		} else if (!dbNameToAddr(plink->value.pv_link.pvname, pAddr)) {
			/* PV resides on this ioc */
			*plinkValid = acalcoutINAV_LOC;
			if (fieldIndex == acalcoutRecordOUT) {
				prpvt->outlink_field_type = pAddr->field_type;
				if (aCalcoutRecordDebug && (pAddr->field_type >= DBF_INLINK) &&
					(pAddr->field_type <= DBF_FWDLINK)) {
					s = strchr(plink->value.pv_link.pvname, (int)' ') + 1;
					if (strncmp(s,"CA",2)) {
						printf("acalcoutRecord:special:dblink to link field\n");
					}
				}
			}
		} else {
			/* pv is not on this ioc. Callback later for connection stat */
			*plinkValid = acalcoutINAV_EXT_NC;
			/* DO_CALLBACK, if not already scheduled */
			if (!prpvt->wd_id_1_LOCK) {
				callbackRequestDelayed(&prpvt->checkLinkCb,.5);
				prpvt->wd_id_1_LOCK = 1;
				prpvt->caLinkStat = CA_LINKS_NOT_OK;
			}
			if (fieldIndex == acalcoutRecordOUT)
				prpvt->outlink_field_type = DBF_NOACCESS; /* don't know */
		}
        db_post_events(pcalc,plinkValid,DBE_VALUE);
		return(0);
		break;

	default:
		recGblDbaddrError(S_db_badChoice,paddr,"calc: special");
		return(S_db_badChoice);
	}
	return(0);
}

static long cvt_dbaddr(dbAddr *paddr)
{
	acalcoutRecord	*pcalc = (acalcoutRecord *) paddr->precord;
	double			**pfield = (double **)paddr->pfield;
	double			**ppd;
	short			i;
    int fieldIndex = dbGetFieldIndex(paddr);

	if (aCalcoutRecordDebug >= 20) printf("acalcout: cvt_dbaddr: paddr->pfield = %p\n",
		(void *)paddr->pfield);
	if ((fieldIndex>=acalcoutRecordAA) && (fieldIndex<=acalcoutRecordLL)) {
		ppd = &(pcalc->aa);
		i = pfield - ppd;
		paddr->pfield = ppd[i];
	} else if ((fieldIndex>=acalcoutRecordPAA) && (fieldIndex<=acalcoutRecordPLL)) {
		ppd = &(pcalc->paa);
		i = pfield - ppd;
		paddr->pfield = ppd[i];
	} else if (fieldIndex==acalcoutRecordAVAL) {
		paddr->pfield = pcalc->aval;
	} else if (fieldIndex==acalcoutRecordPAVL) {
		paddr->pfield = pcalc->pavl;
	} else if (fieldIndex==acalcoutRecordOAV) {
		paddr->pfield = pcalc->oav;
	} else if (fieldIndex==acalcoutRecordPOAV) {
		paddr->pfield = pcalc->poav;
	}
	paddr->no_elements = (pcalc->nuse > 0) ? pcalc->nuse : pcalc->nelm;
	paddr->field_type = DBF_DOUBLE;
	paddr->field_size = sizeof(double);
	paddr->dbr_field_type = DBF_DOUBLE;
	return(0);
}

static long get_array_info(struct dbAddr *paddr, long *no_elements, long *offset)
{
	acalcoutRecord	*pcalc = (acalcoutRecord *) paddr->precord;

	if (aCalcoutRecordDebug >= 20) printf("acalcout: get_array_info: paddr->pfield = %p\n",
		(void *)paddr->pfield);
    *no_elements =  (pcalc->nuse > 0) ? pcalc->nuse : pcalc->nelm;
    *offset = 0;
    return(0);
}

static long put_array_info(struct dbAddr *paddr, long nNew)
{
	acalcoutRecord	*pcalc = (acalcoutRecord *) paddr->precord;
	double			*pfield = (double *)paddr->pfield;
	double			**ppd, *pd = NULL;
	long			i;
    int				fieldIndex = dbGetFieldIndex(paddr);

	if (aCalcoutRecordDebug >= 20) {
		printf("acalcoutRecord(%s):put_array_info: paddr->pfield = %p, pcalc->aa=%p, nNew=%ld\n",
			pcalc->name, (void *)paddr->pfield, (void *)pcalc->aa, nNew);
	}

	if ((fieldIndex>=acalcoutRecordAA) && (fieldIndex<=acalcoutRecordLL)) {
		ppd = &(pcalc->aa);
		pd = ppd[pfield - pcalc->aa];
	} else if ((fieldIndex>=acalcoutRecordPAA) && (fieldIndex<=acalcoutRecordPLL)) {
		ppd = &(pcalc->paa);
		pd = ppd[pfield - pcalc->paa];
	} else if (fieldIndex==acalcoutRecordAVAL) {
		pd = pcalc->aval;
	} else if (fieldIndex==acalcoutRecordPAVL) {
		pd = pcalc->pavl;
	} else if (fieldIndex==acalcoutRecordOAV) {
		pd = pcalc->oav;
	} else if (fieldIndex==acalcoutRecordPOAV) {
		pd = pcalc->poav;
	}

	if (aCalcoutRecordDebug >= 20) {
		printf("acalcoutRecord(%s):put_array_info: pd=%p\n", pcalc->name, (void *)pd);
	}
	if (pd && (nNew < pcalc->nelm))
		for (i=nNew; i<pcalc->nelm; i++) pd[i] = 0.;
	
	/* We could set nuse to the number of elements just written, but that would also
	 * affect the other arrays.  For now, with all arrays sharing a single value of nuse,
	 * it seems better to require that nuse be set explicitly.  Currently, I'm leaving
	 * unanswered the question of whether each array should have its own 'nuse'.  The
	 * array-calc engine currently doesn't support per-array 'nuse'.
	 */

    return(0);
}

static long get_units(dbAddr *paddr, char *units)
{
	acalcoutRecord	*pcalc=(acalcoutRecord *)paddr->precord;

	strncpy(units,pcalc->egu,DB_UNITS_SIZE);
	return(0);
}

static long get_precision(dbAddr *paddr, long *precision)
{
	acalcoutRecord	*pcalc=(acalcoutRecord *)paddr->precord;
	int fieldIndex = dbGetFieldIndex(paddr);

	*precision = pcalc->prec;
	if (fieldIndex == acalcoutRecordVAL) return(0);
	recGblGetPrec(paddr,precision);
	return(0);
}

static long get_graphic_double(dbAddr *paddr, struct dbr_grDouble *pgd)
{
    acalcoutRecord	*pcalc=(acalcoutRecord *)paddr->precord;
    int fieldIndex = dbGetFieldIndex(paddr);

	switch (fieldIndex) {
	case acalcoutRecordVAL:
	case acalcoutRecordHIHI:
	case acalcoutRecordHIGH:
	case acalcoutRecordLOW:
	case acalcoutRecordLOLO:
		pgd->upper_disp_limit = pcalc->hopr;
		pgd->lower_disp_limit = pcalc->lopr;
		return(0);
	default:
		break;
	} 

	if (fieldIndex >= acalcoutRecordA && fieldIndex <= acalcoutRecordL) {
		pgd->upper_disp_limit = pcalc->hopr;
		pgd->lower_disp_limit = pcalc->lopr;
		return(0);
	}
	if (fieldIndex >= acalcoutRecordPA && fieldIndex <= acalcoutRecordPL) {
		pgd->upper_disp_limit = pcalc->hopr;
		pgd->lower_disp_limit = pcalc->lopr;
		return(0);
	}
	return(0);
}

static long get_control_double(dbAddr *paddr, struct dbr_ctrlDouble *pcd)
{
	acalcoutRecord	*pcalc=(acalcoutRecord *)paddr->precord;
	int fieldIndex = dbGetFieldIndex(paddr);

	switch (fieldIndex) {
	case acalcoutRecordVAL:
	case acalcoutRecordHIHI:
	case acalcoutRecordHIGH:
	case acalcoutRecordLOW:
	case acalcoutRecordLOLO:
		pcd->upper_ctrl_limit = pcalc->hopr;
		pcd->lower_ctrl_limit = pcalc->lopr;
		return(0);
	default:
		break;
    } 

	if (fieldIndex >= acalcoutRecordA && fieldIndex <= acalcoutRecordL) {
		pcd->upper_ctrl_limit = pcalc->hopr;
		pcd->lower_ctrl_limit = pcalc->lopr;
		return(0);
	}
	if (fieldIndex >= acalcoutRecordPA && fieldIndex <= acalcoutRecordPL) {
		pcd->upper_ctrl_limit = pcalc->hopr;
		pcd->lower_ctrl_limit = pcalc->lopr;
		return(0);
	}
	return(0);
}
static long get_alarm_double(dbAddr *paddr, struct dbr_alDouble *pad)
{
	acalcoutRecord	*pcalc=(acalcoutRecord *)paddr->precord;
	int fieldIndex = dbGetFieldIndex(paddr);

	if (fieldIndex == acalcoutRecordVAL) {
		pad->upper_alarm_limit = pcalc->hihi;
		pad->upper_warning_limit = pcalc->high;
		pad->lower_warning_limit = pcalc->low;
		pad->lower_alarm_limit = pcalc->lolo;
	} else
		 recGblGetAlarmDouble(paddr,pad);
	return(0);
}


static void checkAlarms(acalcoutRecord *pcalc)
{
	double			val;
	double			hyst, lalm, hihi, high, low, lolo;
	unsigned short	hhsv, llsv, hsv, lsv;

	if (pcalc->udf == TRUE) {
		recGblSetSevr(pcalc,UDF_ALARM,INVALID_ALARM);
		return;
	}
	hihi = pcalc->hihi; 
	lolo = pcalc->lolo; 
	high = pcalc->high;  
	low = pcalc->low;
	hhsv = pcalc->hhsv; 
	llsv = pcalc->llsv; 
	hsv = pcalc->hsv; 
	lsv = pcalc->lsv;
	val = pcalc->val; 
	hyst = pcalc->hyst; 
	lalm = pcalc->lalm;

	/* alarm condition hihi */
	if (hhsv && (val >= hihi || ((lalm==hihi) && (val >= hihi-hyst)))) {
		if (recGblSetSevr(pcalc,HIHI_ALARM,pcalc->hhsv)) pcalc->lalm = hihi;
		return;
	}

	/* alarm condition lolo */
	if (llsv && (val <= lolo || ((lalm==lolo) && (val <= lolo+hyst)))) {
		if (recGblSetSevr(pcalc,LOLO_ALARM,pcalc->llsv)) pcalc->lalm = lolo;
		return;
	}

	/* alarm condition high */
	if (hsv && (val >= high || ((lalm==high) && (val >= high-hyst)))) {
		if (recGblSetSevr(pcalc,HIGH_ALARM,pcalc->hsv)) pcalc->lalm = high;
		return;
	}

	/* alarm condition low */
	if (lsv && (val <= low || ((lalm==low) && (val <= low+hyst)))) {
		if (recGblSetSevr(pcalc,LOW_ALARM,pcalc->lsv)) pcalc->lalm = low;
		return;
	}

	/* we get here only if val is out of alarm by at least hyst */
	pcalc->lalm = val;
	return;
}


static void execOutput(acalcoutRecord *pcalc)
{
	long	i, status;

	/* Determine output data */
	if (aCalcoutRecordDebug >= 10)
		printf("acalcoutRecord(%s):execOutput:entry\n", pcalc->name);
	switch (pcalc->dopt) {
	case acalcoutDOPT_Use_VAL:
		pcalc->oval = pcalc->val;
		for (i=0; i<pcalc->nelm; i++) pcalc->oav[i] = pcalc->aval[i];
		break;

	case acalcoutDOPT_Use_OVAL:
		i = (pcalc->nuse > 0) ? pcalc->nuse : pcalc->nelm;
		if (aCalcPerform(&pcalc->a, ARG_MAX, &pcalc->aa,
				ARRAY_ARG_MAX, i, &pcalc->oval, pcalc->oav, pcalc->rpcl)) {
			recGblSetSevr(pcalc,CALC_ALARM,INVALID_ALARM);
		}
		if (i < pcalc->nelm) {
			for (; i<pcalc->nelm; i++) {
				pcalc->oav[i] = 0;
			}
		}
		break;
	}

	/* Check to see what to do if INVALID */
	if (pcalc->sevr < INVALID_ALARM) {
		/* Output the value */
		if (aCalcoutRecordDebug >= 10)
			printf("acalcoutRecord(%s):execOutput:calling writeValue\n", pcalc->name);
		status = writeValue(pcalc);
		/* post event if output event != 0 */
		if (pcalc->oevt > 0) post_event((int)pcalc->oevt);
	} else {
		switch (pcalc->ivoa) {
		case menuIvoaContinue_normally:
			/* write the new value */
			status = writeValue(pcalc);
			/* post event if output event != 0 */
			if (pcalc->oevt > 0) post_event((int)pcalc->oevt);
			break;

		case menuIvoaDon_t_drive_outputs:
			break;

		case menuIvoaSet_output_to_IVOV:
			pcalc->oval=pcalc->ivov;
			status = writeValue(pcalc);
			/* post event if output event != 0 */
			if (pcalc->oevt > 0) post_event((int)pcalc->oevt);
			break;

		default:
			status=-1;
			recGblRecordError(S_db_badField,(void *)pcalc,
				"acalcout:process Illegal IVOA field");
		}
	} 
}

static void monitor(acalcoutRecord *pcalc)
{
	unsigned short	monitor_mask;
	double			delta;
	double			*pnew, *pprev;
	double			**panew, **paprev;
	int				i, j, diff;

	if (aCalcoutRecordDebug >= 10)
		printf("acalcoutRecord(%s):monitor:entry\n", pcalc->name);
	monitor_mask = recGblResetAlarms(pcalc);
	/* check for value change */
	delta = pcalc->mlst - pcalc->val;
	if (delta < 0.0) delta = -delta;
	if (delta > pcalc->mdel) {
		/* post events for value change */
		monitor_mask |= DBE_VALUE;
		/* update last value monitored */
		pcalc->mlst = pcalc->val;
	}
	/* check for archive change */
	delta = pcalc->alst - pcalc->val;
	if (delta < 0.0) delta = -delta;
	if (delta > pcalc->adel) {
		/* post events on value field for archive change */
		monitor_mask |= DBE_LOG;
		/* update last archive value monitored */
		pcalc->alst = pcalc->val;
	}
	/* send out monitors connected to the value field */
	if (monitor_mask) db_post_events(pcalc,&pcalc->val,monitor_mask);

	for (i=0, diff=0; i<pcalc->nelm; i++) {
		if (pcalc->aval[i] != pcalc->pavl[i]) {diff = 1;break;}
	}
	if (diff) {
		if (aCalcoutRecordDebug >= 1)
			printf("acalcoutRecord(%s):posting .AVAL\n", pcalc->name);
		db_post_events(pcalc, pcalc->aval, monitor_mask|DBE_VALUE|DBE_LOG);
		for (i=0; i<pcalc->nelm; i++) pcalc->pavl[i] = pcalc->aval[i];
	}

	for (i=0, diff=0; i<pcalc->nelm; i++) {
		if (pcalc->oav[i] != pcalc->poav[i]) {diff = 1;break;}
	}
	if (diff) {
		db_post_events(pcalc, pcalc->oav, monitor_mask|DBE_VALUE|DBE_LOG);
		for (i=0; i<pcalc->nelm; i++) pcalc->poav[i] = pcalc->oav[i];
	}

	/* check all input fields for changes */
	for (i=0, pnew=&pcalc->a, pprev=&pcalc->pa; i<ARG_MAX;  i++, pnew++, pprev++) {
		if ((*pnew != *pprev) || (monitor_mask&DBE_ALARM)) {
			db_post_events(pcalc,pnew,monitor_mask|DBE_VALUE|DBE_LOG);
			*pprev = *pnew;
		}
	}

	for (i=0, panew=&pcalc->aa, paprev=&pcalc->paa; i<ARRAY_ARG_MAX; i++, panew++, paprev++) {
		for (j=0, diff=0; j<pcalc->nelm; j++) {
			if ((*panew)[j] != (*paprev)[j]) {diff = 1;break;}
		}
		if (diff) {
			db_post_events(pcalc, *panew, monitor_mask|DBE_VALUE|DBE_LOG);
			for (j=0; j<pcalc->nelm; j++) (*paprev)[j] = (*panew)[j];
		}

	}

	/* Check OVAL field */
	if (pcalc->povl != pcalc->oval) {
		db_post_events(pcalc, &pcalc->oval, monitor_mask|DBE_VALUE|DBE_LOG);
		pcalc->povl = pcalc->oval;
	}
	return;
}

static int fetch_values(acalcoutRecord *pcalc)
{
	DBLINK	*plink;	/* structure of the link field  */
	double	*pvalue;
	double	**pavalue;
	long	status = 0, nRequest = (pcalc->nuse > 0) ? pcalc->nuse : pcalc->nelm;
	int		i;

	if (aCalcoutRecordDebug >= 10)
		printf("acalcoutRecord(%s):fetch_values: entry\n", pcalc->name);
	for (i=0, plink=&pcalc->inpa, pvalue=&pcalc->a; i<ARG_MAX; 
			i++, plink++, pvalue++) {
		status = dbGetLink(plink, DBR_DOUBLE, pvalue, 0, 0);
		if (!RTN_SUCCESS(status)) return(status);
	}

	if (aCalcoutRecordDebug >= 10) printf("acalcoutRecord(%s):fetch_values: arrays\n", pcalc->name);
	for (i=0, plink=&pcalc->inaa, pavalue=(double **)(&pcalc->aa); i<ARRAY_ARG_MAX; 
			i++, plink++, pavalue++) {
		status = dbGetLink(plink, DBR_DOUBLE, *pavalue, 0, &nRequest);
		if (!RTN_SUCCESS(status)) {printf("acalcout:fetch:error\n");}
	}
	if (aCalcoutRecordDebug >= 10)
		printf("acalcoutRecord(%s):fetch_values: returning\n", pcalc->name);
	return(0);
}

static void checkLinksCallback(CALLBACK *pcallback)
{
    acalcoutRecord	*pcalc;
    rpvtStruct		*prpvt;

    callbackGetUser(pcalc, pcallback);
    prpvt = (rpvtStruct *)pcalc->rpvt;
    
	if (!interruptAccept) {
		/* Can't call dbScanLock yet.  Schedule another CALLBACK */
		prpvt->wd_id_1_LOCK = 1;  /* make sure */
		callbackRequestDelayed(&prpvt->checkLinkCb,.5);
	} else {
	    dbScanLock((struct dbCommon *)pcalc);
	    prpvt->wd_id_1_LOCK = 0;
	    checkLinks(pcalc);
	    dbScanUnlock((struct dbCommon *)pcalc);
	}
}


static void checkLinks(acalcoutRecord *pcalc)
{
	DBLINK			*plink;
	rpvtStruct		*prpvt = (rpvtStruct *)pcalc->rpvt;
	int i;
	int				isCaLink   = 0;
	int				isCaLinkNc = 0;
	unsigned short	*plinkValid;
	dbAddr			Addr;
	dbAddr			*pAddr = &Addr;
	char			*s;

	if (aCalcoutRecordDebug >= 10) printf("checkLinks() for %p\n", (void *)pcalc);

	plink   = &pcalc->inpa;
	plinkValid = &pcalc->inav;

	for (i=0; i<ARG_MAX+ARRAY_ARG_MAX+1; i++, plink++, plinkValid++) {
		if (plink->type == CA_LINK) {
			isCaLink = 1;
			if (dbCaIsLinkConnected(plink)) {
				if (*plinkValid == acalcoutINAV_EXT_NC) {
					*plinkValid = acalcoutINAV_EXT;
					db_post_events(pcalc,plinkValid,DBE_VALUE);
				}
				/* if outlink, get type of field we're connected to */
				if (plink == &pcalc->out) {
					prpvt->outlink_field_type = dbCaGetLinkDBFtype(plink);
					if (aCalcoutRecordDebug >= 10) {
						printf("acalcout:checkLinks: outlink type = %d\n",
							prpvt->outlink_field_type);
						if (!dbNameToAddr(plink->value.pv_link.pvname, pAddr)) {
							if ((pAddr->field_type >= DBF_INLINK) &&
									(pAddr->field_type <= DBF_FWDLINK)) {
								s = strchr(plink->value.pv_link.pvname, (int)' ') + 1;
								if (strncmp(s,"CA",2))
									printf("acalcoutRecord:checkLinks:dblink to link field\n");
							}
						}
					}
				}
			} else {
				if (*plinkValid == acalcoutINAV_EXT_NC) {
					isCaLinkNc = 1;
				}
				else if (*plinkValid == acalcoutINAV_EXT) {
					*plinkValid = acalcoutINAV_EXT_NC;
					db_post_events(pcalc,plinkValid,DBE_VALUE);
					isCaLinkNc = 1;
				}
				if (plink == &pcalc->out)
					prpvt->outlink_field_type = DBF_NOACCESS; /* don't know type */
			} 
		}
	}
	if (isCaLinkNc)
		prpvt->caLinkStat = CA_LINKS_NOT_OK;
	else if (isCaLink)
		prpvt->caLinkStat = CA_LINKS_ALL_OK;
	else
		prpvt->caLinkStat = NO_CA_LINKS;

	if (!prpvt->wd_id_1_LOCK && isCaLinkNc) {
		/* Schedule another CALLBACK */
		prpvt->wd_id_1_LOCK = 1;
		callbackRequestDelayed(&prpvt->checkLinkCb,.5);
	}
}


static long writeValue(acalcoutRecord *pcalc)
{
    acalcoutDSET	*pacalcoutDSET = (acalcoutDSET *)pcalc->dset;

	if (aCalcoutRecordDebug >= 10)
		printf("acalcoutRecord(%s):writeValue:entry\n", pcalc->name);

    if (!pacalcoutDSET || !pacalcoutDSET->write) {
        errlogPrintf("%s DSET write does not exist\n",pcalc->name);
        recGblSetSevr(pcalc,SOFT_ALARM,INVALID_ALARM);
        pcalc->pact = TRUE;
        return(-1);
    }
	if (aCalcoutRecordDebug >= 10)
		printf("acalcoutRecord(%s):writeValue:calling device support\n", pcalc->name);
	return pacalcoutDSET->write(pcalc);
}
