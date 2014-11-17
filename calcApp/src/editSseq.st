program editSseq  ("P=xxx:, Q=ES:")

/* The purpose of this program is to help edit a running copy of a sseq record.
 * To use:
 * 1) load the associated database:
 *        dbLoadRecords("$(TOP)/xxxApp/Db/editSseq.db", "P=xxxL:,Q=ES:")
 *     where P is the ioc prefix, and Q identifies an instance of the database.
 * 2) run this program:
 *        seq &editSseq, "P=xxx:, Q=ES:"
 * 3) set the pv $(P)$(Q)recordName to the name of a sseq record (with or without a trailing
 *    field name, so you can use DragAndDrop).
 * 4) enter one of the following commands:
 *        "c+" where 'c' is one of [0,1,2,3,4,5,6,7,8,9,a,A]
 *             This moves line c down.
 *        "c-" where 'c' is one of [0,1,2,3,4,5,6,7,8,9,a,A]
 *             This moves line c up.
 *        "c/d" where 'c' and 'd' are one of [0,1,2,3,4,5,6,7,8,9,a,A]
 *              This swaps line c with line d.
 *
 * Tim Mooney 8/20/2014
 */

/* SNL compiler options */
option +r;

/* arguments on the command line  :
 * 
 * ex.  seq &editSseq, "P=xxx:, Q=ES:"
 *
 *      P - prefix     
 *      Q - instance of editSseq
 */

#include <seq_release.h> /* definition of MAGIC */
#define VERSION_INT(MAJ,MIN) ((MAJ)*1000000+(MIN)*1000)
#define LT_SEQ_VERSION(MAJ,MIN) ((MAGIC) < VERSION_INT(MAJ,MIN))

/* Define escaped C functions at end of file */
%% static int lineNum(char c);
%% static char fieldChar(int n);
%% static char fieldCharCheck(char c);
%% static int reconcilePVs(SS_ID ssId, struct UserVar *pVar,
%%	char *pattern, string *PV, int *PVIndex, char c_dest);
/* General Purpose PV's used by sequence */
short Debug;
assign  Debug to "{P}{Q}Debug"; 
monitor Debug;

string  message;
assign  message to "{P}{Q}message"; 

short   opAlert;
assign  opAlert to "{P}{Q}Alert";

short   opAck;
assign  opAck to "{P}{Q}OperAck";
monitor opAck;

string recordName;
assign recordName to "{P}{Q}recordName";
monitor recordName;
evflag recordNameMon; sync recordName recordNameMon;

#define RECTYPE_NONE 0
#define RECTYPE_SSEQ 1
#define RECTYPE_SEQ 2
int recType;

string recordType;
assign recordType to "";

string command;
assign command to "{P}{Q}command";
monitor command;
evflag commandMon; sync command commandMon;

#define NLINES 10
#define NLINESP1 11
/* extra place for swapping */
string DOL[NLINESP1];
assign DOL to {""};
double DLY[NLINESP1];
assign DLY to {""};
string STR[NLINESP1];
assign STR to {""};
double DO[NLINESP1];
assign DO to {""};
string LNK[NLINESP1];
assign LNK to {""};
string WAIT[NLINESP1];
assign WAIT to {""};

int DOLIndex[NLINES];
int STRIndex[NLINES];
int LNKIndex[NLINES];
int WAITIndex[NLINES];

int i, j, src, dest, inc, n, permit;
string fieldName, pattern;
int rewroteRecordName;
char *s, c, c_src, c_dest;
int ic;

%%#include <string.h>
%%#include <math.h>
%%#include <stdlib.h>
%%#include <ctype.h>

#define ALERT(format, arg) \
	sprintf(message, (format), (arg)); pvPut(message); \
	opAlert = 1; pvPut(opAlert)

ss editSseq
{
	state init {
		entry {
			if (Debug) printf("editSseq:init: entry\n");
		}
		when () {
			rewroteRecordName = 0;
			strcpy(recordType, "");
			recType = RECTYPE_NONE;
			strcpy(message, "");
			pvPut(message);
			opAlert = 0;
			pvPut(opAlert);
			efClear(commandMon);
		} state waitForCmnd
	}

	state waitForCmnd {
		entry {
			if (Debug>10) printf("editSseq:waitForCmnd: entry\n");
			if (rewroteRecordName) {
				efClear(recordNameMon);
				rewroteRecordName = 0;
			}
		}
		when (efTestAndClear(commandMon)) {
		} state newCommand
		when (efTestAndClear(recordNameMon)) {
		} state newRecordName
		when (opAck) {
			strcpy(message, "");
			pvPut(message);
			opAlert = 0;
			pvPut(opAlert);
			opAck = 0;
			pvPut(opAck);
		} state waitForCmnd
	}

	state newRecordName {
		entry {
			if (Debug) printf("editSseq:newRecordName: entry\n");
			if (Debug) printf("editSseq:newRecordName: recordName='%s'\n", recordName);
			/* strip trailing field */
			if (*recordName) {
				s = strchr(recordName, '.');
				if (s) {
					*s = '\0';
					pvPut(recordName);
					rewroteRecordName = 1;
				}
			}
			if (Debug) printf("editSseq:newRecordName: recordName='%s'\n", recordName);
			strcpy(recordType, "");
			recType = RECTYPE_NONE;
		}
		when () {
			sprintf(fieldName, "%s.RTYP", recordName);
			pvAssign(recordType, fieldName);
			pvGet(recordType);
			if (strcmp(recordType, "sseq")==0) {
				recType = RECTYPE_SSEQ;
			} else if (strcmp(recordType, "seq")==0) {
				recType = RECTYPE_SEQ;
			} else {
				recType = RECTYPE_NONE;
			}
			for (i=0; i<NLINES; i++) {
				sprintf(fieldName, "%s.DOL%c", recordName, fieldChar(i));
				pvAssign(DOL[i], fieldName);
				DOLIndex[i] = pvIndex(DOL[i]);

				sprintf(fieldName, "%s.DLY%c", recordName, fieldChar(i));
				pvAssign(DLY[i], fieldName);

				sprintf(fieldName, "%s.DO%c", recordName, fieldChar(i));
				pvAssign(DO[i], fieldName);

				sprintf(fieldName, "%s.LNK%c", recordName, fieldChar(i));
				pvAssign(LNK[i], fieldName);
				LNKIndex[i] = pvIndex(LNK[i]);

				if (recType == RECTYPE_SSEQ) {
					sprintf(fieldName, "%s.STR%c", recordName, fieldChar(i));
					pvAssign(STR[i], fieldName);
					STRIndex[i] = pvIndex(STR[i]);

					sprintf(fieldName, "%s.WAIT%c", recordName, fieldChar(i));
					pvAssign(WAIT[i], fieldName);
					WAITIndex[i] = pvIndex(WAIT[i]);
				}
			}
		} state waitForCmnd
	}


	state newCommand {
		entry {
			if (Debug) {
				printf("editSseq:newCommand: entry\n");
				printf("editSseq:newCommand: command: '%s'\n", command);
				printf("editSseq:newCommand: recordType=%s\n", recordType);
				printf("editSseq:newCommand: recType=%d\n", recType);
			}
			for (i=0; i<NLINES; i++) {
				if (recType == RECTYPE_SEQ || recType == RECTYPE_SSEQ) {
					pvGet(DOL[i]);
					pvGet(DLY[i]);
					pvGet(DO[i]);
					pvGet(LNK[i]);
					if (recType == RECTYPE_SSEQ) {
						pvGet(STR[i]);
						pvGet(WAIT[i]);
					}
				}
			}
			strcpy(message, "");
			pvPut(message);
			opAlert = 0;
			pvPut(opAlert);
		}

		when (strcmp(command, "")==0) {
		}  state waitForCmnd

		when (recType == RECTYPE_NONE) {
			sprintf(message, "unsupported recordType '%s'", recordType);
			pvPut(message);
			opAlert = 1;
			pvPut(opAlert);
		}  state waitForCmnd

		/* move a line up or down */
		when ((command[1] == '+') || (command[1] == '-'))  {
			if (Debug) printf("command '+/-'\n");
			c_src = fieldCharCheck(command[0]);
			src = lineNum(c_src);
			if (command[1]=='+') {
				dest = src+1;
			} else {
				dest = src-1;
			}
			c_dest = fieldChar(dest);
			if (Debug) printf("src='%c'(%d), dest='%c'(%d)\n", c_src, src, c_dest, dest);

			permit = 1;

			/* if we'd kill line <n>, and DOL<i>==DO<n>, don't permit */
			sprintf(pattern, "%s.DO%c", recordName, c_dest);
			for (i=0; i<NLINES; i++) {
				if (strstr(DOL[i], pattern) != 0) {
					permit = 0;
					ALERT("DOL%c would be orphaned", fieldChar(i));
				}
			}
			/* if we'd kill line <n>, and LNK<i>==DO<n>, don't permit */
			sprintf(pattern, "%s.DO%c", recordName, c_dest);
			for (i=0; i<NLINES; i++) {
				if (strstr(LNK[i], pattern) != 0) {
					permit = 0;
					ALERT("LNK%c would be orphaned", fieldChar(i));
				}
			}
			if (recType == RECTYPE_SSEQ) {
				/* if we'd kill line <n>, and WAIT<i>=After<n>, don't permit */
				sprintf(pattern, "After%c", c_dest);
				for (i=0; i<NLINES; i++) {
					if (strstr(WAIT[i], pattern) != 0) {
						permit = 0;
						ALERT("WAIT%c would be orphaned", fieldChar(i));
					}
				}

				/* if we'd kill line <n>, and DOL<i>==STR<n>, don't permit */
				sprintf(pattern, "%s.STR%c", recordName, c_dest);
				for (i=0; i<NLINES; i++) {
					if (strstr(DOL[i], pattern) != 0) {
						permit = 0;
						ALERT("DOL%c would be orphaned", fieldChar(i));
					}
				}
				/* if we'd kill line <n>, and LNK<i>==STR<n>, don't permit */
				sprintf(pattern, "%s.STR%c", recordName, c_dest);
				for (i=0; i<NLINES; i++) {
					if (strstr(LNK[i], pattern) != 0) {
						permit = 0;
						ALERT("LNK%c would be orphaned", fieldChar(i));
					}
				}
			}

			if (permit && src>=0 && src<=9 && dest>=0 && dest<=9) {
				strcpy(DOL[dest], DOL[src]);
				strcpy(DOL[src], "");
				DLY[dest] = DLY[src];
				DLY[src] = 0;
				DO[dest] = DO[src];
				DO[src] = 0;
				strcpy(LNK[dest], LNK[src]);
				strcpy(LNK[src], "");

				/* if moved line <n>, and LNK<i>==<recordName>.DO<n> */
				sprintf(pattern, "%s.DO%c", recordName, c_src);
				%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->LNK, pVar->LNKIndex, pVar->c_dest);

				/* if moved line <n>, and DOL<i>==<recordName>.DO<n> */
				sprintf(pattern, "%s.DO%c", recordName, c_src);
				%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->DOL, pVar->DOLIndex, pVar->c_dest);

				pvPut(DOL[src]);
				pvPut(DOL[dest]);
				pvPut(DLY[src]);
				pvPut(DLY[dest]);
				pvPut(DO[src]);
				pvPut(DO [dest]);
				pvPut(LNK[src]);
				pvPut(LNK[dest]);

				if (recType == RECTYPE_SSEQ) {
					strcpy(STR[dest], STR[src]);
					strcpy(STR[src], "");
					strcpy(WAIT[dest], WAIT[src]);
					strcpy(WAIT[src], "NoWait");

					/* if moved line <n>, and WAIT<k>==After<n> */
					sprintf(pattern, "After%c", c_src);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->WAIT, pVar->WAITIndex, pVar->c_dest);

					/* if moved line <n>, and LNK<k>==<recordName>.STR<n> */
					sprintf(pattern, "%s.STR%c", recordName, c_src);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->LNK, pVar->LNKIndex, pVar->c_dest);

					/* if moved line <n>, and DOL<k>==<recordName>.STR<n> */
					sprintf(pattern, "%s.STR%c", recordName, c_src);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->DOL, pVar->DOLIndex, pVar->c_dest);

					pvPut(STR[src]);
					pvPut(STR[dest]);
					pvPut(WAIT[src]);
					pvPut(WAIT[dest]);
				}	
			} else {
				if (permit) ALERT("src (%c) out of range", command[1]);
			}
		} state waitForCmnd

		/* swap lines */
		when (strchr(command, '/')!=0)  {
			if (Debug) printf("command '/'\n");
			c_src = fieldCharCheck(command[0]);
			src = lineNum(c_src);
			c_dest = fieldCharCheck(command[2]);
			dest = lineNum(c_dest);
			if (Debug) printf("swap src='%c'(%d), dest='%c'(%d)\n", c_src, src, c_dest, dest);

			if (src>=0 && src<=9 && dest>=0 && dest<=9) {
				strcpy(DOL[NLINES], DOL[dest]);
				strcpy(DOL[dest], DOL[src]);
				strcpy(DOL[src], DOL[NLINES]);

				DLY[NLINES] = DLY[dest];
				DLY[dest] = DLY[src];
				DLY[src] = DLY[NLINES];

				DO[NLINES] = DO[dest];
				DO[dest] = DO[src];
				DO[src] = DO[NLINES];

				strcpy(LNK[NLINES], LNK[dest]);
				strcpy(LNK[dest], LNK[src]);
				strcpy(LNK[src], LNK[NLINES]);

				/* if swapped lines <m> and <n>, and LNK<k>==<recordName>.DO<n>or<m> */
				sprintf(pattern, "%s.DO%c", recordName, c_src);
				%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->LNK, pVar->LNKIndex, '*');
				sprintf(pattern, "%s.DO%c", recordName, c_dest);
				%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->LNK, pVar->LNKIndex, pVar->c_src);
				sprintf(pattern, "%s.DO%c", recordName, '*');
				%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->LNK, pVar->LNKIndex, pVar->c_dest);

				/* if swapped lines <m> and <n>, and DOL<k>==<recordName>.DO<n>or<m> */
				sprintf(pattern, "%s.DO%c", recordName, c_src);
				%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->DOL, pVar->DOLIndex, '*');
				sprintf(pattern, "%s.DO%c", recordName, c_dest);
				%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->DOL, pVar->DOLIndex, pVar->c_src);
				sprintf(pattern, "%s.DO%c", recordName, '*');
				%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->DOL, pVar->DOLIndex, pVar->c_dest);

				pvPut(DOL[src]);
				pvPut(DOL[dest]);
				pvPut(DLY[src]);
				pvPut(DLY[dest]);
				pvPut(DO[src]);
				pvPut(DO [dest]);
				pvPut(LNK[src]);
				pvPut(LNK[dest]);

				if (recType == RECTYPE_SSEQ) {
					strcpy(STR[NLINES], STR[dest]);
					strcpy(STR[dest], STR[src]);
					strcpy(STR[src], STR[NLINES]);

					strcpy(WAIT[NLINES], WAIT[dest]);
					strcpy(WAIT[dest], WAIT[src]);
					strcpy(WAIT[src], WAIT[NLINES]);

					/* if swapped lines <m> and <n>, and WAIT<k>==After<n>or<m> */
					sprintf(pattern, "After%c", c_src);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->WAIT, pVar->WAITIndex, '*');
					sprintf(pattern, "After%c", c_dest);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->WAIT, pVar->WAITIndex, pVar->c_src);
					sprintf(pattern, "After%c", '*');
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->WAIT, pVar->WAITIndex, pVar->c_dest);

					/* if swapped lines <m> and <n>, and LNK<k>==<recordName>.STR<n>or<m> */
					sprintf(pattern, "%s.STR%c", recordName, c_src);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->LNK, pVar->LNKIndex, '*');
					sprintf(pattern, "%s.STR%c", recordName, c_dest);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->LNK, pVar->LNKIndex, pVar->c_src);
					sprintf(pattern, "%s.STR%c", recordName, '*');
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->LNK, pVar->LNKIndex, pVar->c_dest);

					/* if swapped lines <m> and <n>, and DOL<k>==<recordName>.STR<n>or<m> */
					sprintf(pattern, "%s.STR%c", recordName, c_src);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->DOL, pVar->DOLIndex, '*');
					sprintf(pattern, "%s.STR%c", recordName, c_dest);
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->DOL, pVar->DOLIndex, pVar->c_src);
					sprintf(pattern, "%s.STR%c", recordName, '*');
					%%reconcilePVs(ssId, pVar, pVar->pattern, pVar->DOL, pVar->DOLIndex, pVar->c_dest);

					pvPut(STR[src]);
					pvPut(STR[dest]);
					pvPut(WAIT[src]);
					pvPut(WAIT[dest]);
				}	
			} else {
				sprintf(message, "src (%c) out of range", command[1]);
				pvPut(message);
				opAlert = 1;
				pvPut(opAlert);
			}
		} state waitForCmnd

		when () {
			if (Debug) printf("unimplemented command '%s'\n", command);
		} state waitForCmnd
	}
}

/***********************************************************************************************************/
/* C functions */
%{
static int lineNum(char c) {
	int n;
	if (isdigit((int)c)) {
		if (c=='0') {
			n = 9; /* accept '0' as synonym for 'A' */
		} else {
			n = c-'1';
		}
	} else if (toupper((int)c) == 'A') {
		n = 9;
	} else {
		n = 10; /* illegal */
	}
	return(n);
}
static char fieldChars[NLINES] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A'};
static char fieldChar(int n) {
	char c;
	if (n>=0 && n<10) {
		c = fieldChars[n];
	} else {
		c = '?';
	}
	return(c);
}

static char fieldCharCheck(char c) {
	if (c=='0' || c=='a') c = 'A';
	return(c);
}

/* Find pattern, which ends with character indicating to which line it pertains, in array of PV.
 * If found, replace indicator character with c_dest and put.
 */
static int reconcilePVs(SS_ID ssId, struct UserVar *pVar,
	char *pattern, string *PV, int *PVIndex, char c_dest) {
	int i;

	for (i=0; i<NLINES; i++) {
		if (strstr(PV[i], pattern) != 0) {
			PV[i][strlen(pattern)-1] = c_dest;
			if (c_dest != '*') {
				seq_pvPut(ssId, PVIndex[i], 0);
			}
		}	
	}
	return(0);
}

}%
