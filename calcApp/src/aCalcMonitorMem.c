/*
FILENAME...     aCalcMonitorMem.cc
USAGE...        Monitor memory allocated by array calcs.

*/


/*
*  Original Author: Kevin Peterson (kmpeters@anl.gov) as motorUtil.cc
*  Date: December 11, 2002
*  Current Author: Tim Mooney
*
*/

#include <stdio.h>
#include <string.h>
#include <dbStaticLib.h>
#include <dbAccess.h>
#include <dbAddr.h>
#include <dbDefs.h>
#include <epicsString.h>
#include <cantProceed.h>
#include <iocsh.h>
#include <epicsExport.h>
#include <errlog.h>
#include <epicsThread.h>
#include <aCalcPostfix.h>
#include <aCalcoutRecord.h>

#define TIMEOUT 60 /* seconds */

/* ----- External Declarations ----- */
extern char **getRecordList();
/* ----- --------------------- ----- */

/* ----- Function Declarations ----- */
static int aCalcMonitorMemInit(char *);
static int aCalcMonitorMem_task(void *);
/* ----- --------------------- ----- */


typedef struct acalcoutRecord_info
{
	char name[PVNAME_SZ];      /* pv names limited to 60 chars + term. in dbDefs.h */
	struct dbAddr Addr;
	long AMEM;
} AcalcoutRecord_info;



/* ----- Global Variables ----- */
int aCalcMonitorMem_debug = 0;
int numRecords = 0;
/* ----- ---------------- ----- */

/* ----- Local Variables  ----- */
static AcalcoutRecord_info *recordArray;
static char **recordlist = 0;
static char *prefix;
struct dbAddr acalcRecMem_dbAddr, acalcCalcMem_dbAddr, acalcTotMem_dbAddr, freeMem_dbAddr;
struct dbAddr *acalcRecMem_pAddr=0, *acalcCalcMem_pAddr=0, *acalcTotMem_pAddr=0, *freeMem_pAddr=0;
/* ----- ---------------- ----- */

char **getRecordList()
{
	DBENTRY dbentry, *pdbentry = &dbentry;
	long    status, a_status;
	char    **paprecords = 0, temp[PVNAME_STRINGSZ];
	int     num_entries = 0, length = 0, index = 0;

	dbInitEntry(pdbbase,pdbentry);
	status = dbFindRecordType(pdbentry,"acalcout");
	if (status)
		errlogPrintf("getRecordList(): No record description\n");

	while (!status) {
		num_entries = dbGetNRecords(pdbentry);
		paprecords = (char **) callocMustSucceed(num_entries, sizeof(char *),
								"getRecordList(1st)");
		status = dbFirstRecord(pdbentry);
		while (!status) {
			a_status = dbIsAlias(pdbentry);
			if (a_status == 0) {
				length = sprintf(temp, "%s", dbGetRecordName(pdbentry));
				paprecords[index] = (char *) callocMustSucceed(length+1,
				sizeof(char), "getRecordList(2nd)");
				strcpy(paprecords[index], temp);
				if (aCalcMonitorMem_debug) {
					printf("aCalcMonitorMem:getRecordList: acalcout record name %s\n", temp);
				}
				index++;
			}
			status = dbNextRecord(pdbentry);
		}
		numRecords = index;
	}

	dbFinishEntry(pdbentry);
	return(paprecords);
}

int aCalcMonitorMemInit(char *ioc_prefix)
{
	int status = 0;
	static int initialized = 0;	/* aCalcMonitorMem initialized indicator. */
    
	if (initialized)
	{
		printf( "aCalcMonitorMem already initialized. Exiting\n");
		return(-1);
	}

	initialized = 1;
	prefix = epicsStrDup(ioc_prefix);

	epicsThreadCreate((char *) "aCalcMonitorMem", epicsThreadPriorityMedium,
					epicsThreadGetStackSize(epicsThreadStackMedium),
					(EPICSTHREADFUNC) aCalcMonitorMem_task, (void *) NULL);
	return(status);
}


static int aCalcMonitorMem_task(void *arg)
{
    char temp[PVNAME_STRINGSZ+5];
    int itera, status;
	long numReq=1;
	double recMem, calcMem, totalMem, prevTotalMem, freeMem;

    recordlist = getRecordList();
    if (aCalcMonitorMem_debug) {
		printf("aCalcMonitorMem_task: There are %i acalcout records\n", numRecords);
	}

	if (numRecords > 0) {
		recordArray = (AcalcoutRecord_info *) callocMustSucceed(numRecords,
			sizeof(AcalcoutRecord_info), "aCalcMonitorMem:init()");
	}

	/* Get dbAddr structures for all acalcout records' .AMEM fields */
	for (itera=0; itera < numRecords; itera++) {
		if (aCalcMonitorMem_debug) {
			printf("aCalcMonitorMem_task: acalcout record name %s\n", recordlist[itera]);
		}
		strcpy(recordArray[itera].name, recordlist[itera]);
		strcpy(temp, recordlist[itera]);
		strcat(temp, ".AMEM");
		status = dbNameToAddr(temp, &(recordArray[itera].Addr));
		status = dbGet(&(recordArray[itera].Addr), DBF_LONG, &(recordArray[itera].AMEM), NULL, &numReq, NULL);
	}

	/* Get dbAddr structures for $(P)acalcRecMem, $(P)acalcCalcMem, and $(P)acalcTotMem */
	strcpy(temp, prefix);
	strcat(temp, "acalcRecMem");
	status = dbNameToAddr(temp, &(acalcRecMem_dbAddr));
	if (status==0) acalcRecMem_pAddr = &acalcRecMem_dbAddr;

	strcpy(temp, prefix);
	strcat(temp, "acalcCalcMem");
	status = dbNameToAddr(temp, &(acalcCalcMem_dbAddr));
	if (status==0) acalcCalcMem_pAddr = &acalcCalcMem_dbAddr;

	strcpy(temp, prefix);
	strcat(temp, "acalcTotMem");
	status = dbNameToAddr(temp, &(acalcTotMem_dbAddr));
	if (status==0) acalcTotMem_pAddr = &acalcTotMem_dbAddr;

	strcpy(temp, prefix);
	strcat(temp, "MEM_MAX");
	status = dbNameToAddr(temp, &(freeMem_dbAddr));
	if (status==0) freeMem_pAddr = &freeMem_dbAddr;

	/* get allocated memory in acalcout records */
	prevTotalMem = 0.;
	freeMem = 0.;
	for (;1;) {
		recMem = 0.;
		if (numRecords > 0) {
			/* loop over records in recordlist and sum allocated memory */
			for (itera=0; itera < numRecords; itera++) {
				status = dbGet(&(recordArray[itera].Addr), DBF_LONG, &(recordArray[itera].AMEM), NULL, &numReq, NULL);
				recMem += recordArray[itera].AMEM / 1.e6;
			}
		}
		calcMem = acalcTotalAllocatedMemory() / 1.e6;
		totalMem = recMem + calcMem;
		if (acalcRecMem_pAddr) {
			dbPutField(acalcRecMem_pAddr, DBF_DOUBLE, &recMem, 1);
		}
		if (acalcCalcMem_pAddr) {
			dbPutField(acalcCalcMem_pAddr, DBF_DOUBLE, &calcMem, 1);
		}
		if (acalcTotMem_pAddr) {
			dbPutField(acalcTotMem_pAddr, DBF_DOUBLE, &totalMem, 1);
		}
		if (freeMem_pAddr) {
			dbGetField(freeMem_pAddr, DBR_DOUBLE, &freeMem, 0, NULL, 0);
			freeMem /= 1.e6;
		}
		if (totalMem !=prevTotalMem) {
			prevTotalMem = totalMem;
			if (aCalcMonitorMem_debug) {
				printf("aCalcMonitorMem_task: allocated memory=%f MB (max=%f)\n", totalMem, freeMem);
			}
			if (freeMem_pAddr && (totalMem > freeMem/2)) {
				printf("aCalcMonitorMem_task: allocated memory (%f MB) > half of total\n", totalMem);
			}
		}
		epicsThreadSleep(10);
	}
	return(0);
}



static const iocshArg Arg = {"IOC name", iocshArgString};
static const iocshArg * const aCalcMonitorMemArg[1]  = {&Arg};
static const iocshFuncDef aCalcMonitorMemDef  = {"aCalcMonitorMemInit", 1, aCalcMonitorMemArg};

static void aCalcMonitorMemCallFunc(const iocshArgBuf *args)
{
    aCalcMonitorMemInit(args[0].sval);
}

static const iocshArg ArgL = {"List moving motors", iocshArgString};
static const iocshArg * const listMovingMotorsArg[1]  = {&ArgL};
static const iocshFuncDef listMovingMotorsDef  = {"listMovingMotors", 1, listMovingMotorsArg};

static void aCalcMonitorMemRegister(void)
{
    iocshRegister(&aCalcMonitorMemDef,  aCalcMonitorMemCallFunc);
}

epicsExportRegistrar(aCalcMonitorMemRegister);
epicsExportAddress(int, aCalcMonitorMem_debug);
