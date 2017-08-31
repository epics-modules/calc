#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include <epicsTypes.h>
#include <epicsMath.h>
#include <epicsUnitTest.h>
#include <testMain.h>

#include "sCalcPostfix.h"


static void testValExpr(const char* expr, double* args, const char** sargs, double expected)
{	
	unsigned char rpn[255];
	short err;
	
	double val;
	char sval[256];
	
	if (sCalcPostfix(expr, rpn, &err))
	{
		testDiag("postfix: %s in expression '%s'", sCalcErrorStr(err), expr);
		return;
	}
	
	if (sCalcPerform(args, 12, (char**) sargs, 12, &val, sval, 256, rpn, 3))
	{
		testDiag("calcPerform: error evaluating '%s'", expr);
		return;
	}
	
	
	bool pass;
	
	if (finite(expected) && finite(val))
	{
		pass = fabs(expected - val) < 1e-8;
	}
	else if (isnan(expected))
	{
		pass = (bool) isnan(val);
	}
	else
	{
		pass = (val == expected);
	}
	
	if(!testOk(pass, "%s", expr))
	{
		testDiag("Expected: %f, Got: %f", expected, val);
	}	
}

static void testSValExpr(const char* expr, double* args, const char** sargs, const char* expected)
{	
	unsigned char rpn[255];
	short err;
	
	double val;
	char sval[256];
	
	if (sCalcPostfix(expr, rpn, &err))
	{
		testDiag("postfix: %s in expression '%s'", sCalcErrorStr(err), expr);
		return;
	}
	
	if (sCalcPerform(args, 12, (char**) sargs, 12, &val, sval, 256, rpn, 3))
	{
		testDiag("calcPerform: error evaluating '%s'", expr);
		return;
	}	
	
	bool pass;
	
	if (expected)
	{
		pass = (strncmp(expected, sval, 256) == 0);
	}
	else
	{
		pass = false;
	}
	
	if(!testOk(pass, "%s", expr))
	{
		testDiag("Expected: %s, Got: %s", expected, sval);
	}	
}


MAIN(scalcTest)
{
	double A = 1.0;
	double B = 2.0;
	double C = 3.0;
	double D = 4.0;
	double E = 5.0;
	double F = 6.0;
	double G = 7.0;
	double H = 8.0;
	double I = 9.0;
	double J = 10.0;
	double K = 11.0;
	double L = 12.0;
	
	char AA[60] = "string 1";
	char BB[60] = "string 2";
	char CC[60] = "string 3";
	char DD[60] = "string 4";
	char EE[60] = "string 5";
	char FF[60] = "string 6";
	char GG[60] = "string 7";
	char HH[60] = "string 8";
	char II[60] = "string 9";
	char JJ[60] = "string 10";
	char KK[60] = "string 11";
	char LL[60] = "xxx:scan1.EXSC";
	
	double args[12] = {A, B, C, D, E, F, G, H, I, J, K, L};
	const char* sargs[12] = {AA, BB, CC, DD, EE, FF, GG, HH, II, JJ, KK, LL};
	
	testPlan(113);

	testValExpr("finite(1)", args, sargs, 1);
	testValExpr("isnan(1)", args, sargs, 0);
	testValExpr("isinf(1)", args, sargs, 0);
	testValExpr("PI/S2R", args, sargs, 180.0*3600.0);
	testValExpr("R2S*PI", args, sargs, 180.0*3600.0);
	testValExpr("1.234", args, sargs, 1.234);
	testValExpr("tan(A)", args, sargs, tan(A));
	testValExpr("sin(B)", args, sargs, sin(B));
	testValExpr("max(A,B,C)", args, sargs, std::max(A, std::max(B, C)));
	testValExpr("min(D,E,F)", args, sargs, std::min(D, std::min(E, F)));
	testValExpr("A<<2", args, sargs, (int) A << 2);
	testValExpr("L>>1", args, sargs, (int) args[11] >> 1);
	testValExpr("A?B:C", args, sargs, A ? B : C);
	testValExpr("A&&B", args, sargs, (int) (A && B));
	testValExpr("A||B", args, sargs, (int) (A || B));
	testValExpr("A>B", args, sargs, (int) (A > B));
	testValExpr("A>=4", args, sargs, (int) (A >= 4));
	testValExpr("A=0?1:0", args, sargs, 0);
	testValExpr("A+B", args, sargs, A + B);
	testValExpr("(B=0)?(A+16384):A+B", args, sargs, (B == 0) ? (A + 16384) : (A + B));
	testValExpr("1.e7/A", args, sargs, (1e7 / A));
	testValExpr("A>9?1:0", args, sargs, (A > 9) ? 1 : 0);
	testValExpr("A%10+1", args, sargs, (int) A % 10 + 1);
	testValExpr("!A", args, sargs, !A);
	testValExpr("C+((A-E)/(D-E))*(B-C)", args, sargs, C+((A-E)/(D-E))*(B-C));
	testValExpr("A#B", args, sargs, A != B);
	testValExpr("E+nint(D*((A-C)/(B-C)))", args, sargs, E + (int)(D*(A - C)/(B-C)));
	testValExpr("(A+1)*1000", args, sargs, (A + 1) * 1000);
	testValExpr("B?0:!A", args, sargs, B ? 0 : (!A));
	testValExpr("1", args, sargs, 1);
	testValExpr("A?0:B", args, sargs, A ? 0 : B);
	testValExpr("A&&B&&!I", args, sargs, A && B && (!I));
	testValExpr("(A&B&C&D)=1", args, sargs, ((int) A & (int) B & (int) C & (int) D) == 1);
	testValExpr("(A&B&C&D&E&F&G&H)=1", args, sargs, ((int) A & (int) B & (int) C & (int) D & (int) E & (int) F & (int) G & (int) H) == 1);
	testValExpr("(A>15)&&B", args, sargs, (A > 15) && B);
	testValExpr("(ABS(A)>1)&&B", args, sargs, 0);
	testValExpr("(A)=1", args, sargs, A == 1);
	testValExpr("A*4095", args, sargs, A * 4095);
	testValExpr("(A||B||C||D||E||F)?1:0", args, sargs, (A || B || C || D || E || F) ? 1 : 0);
	testValExpr("(A<B)?C:D", args, sargs, (A < B) ? C : D);
	testValExpr("A/(10**(B-2))", args, sargs, A / (pow(10, (B - 2))));
	testValExpr("(F<0.01)?0:(((A+C-B-D)/F)*E)", args, sargs, (F < 0.01) ? 0 : (((A + C - B - D) / F) * E));
	testValExpr("((A+B)<0.01)?0:(((A-B)/(A+B))*E)", args, sargs, ((A + B) < 0.01) ? 0 : (((A - B) / (A + B)) * E));
	testValExpr("(A - ((B*C)/D))", args, sargs, A - ((B * C) / D));
	testValExpr("A-B*C/D", args, sargs, A - B * C / D);
	testValExpr("min(max(C,A/B),D)", args, sargs, std::min(std::max(C, A / B), D));
	testValExpr("(A=B)?C:D", args, sargs, (A == B) ? C : D);
	testValExpr("A?A+(B|C|D|E):F", args, sargs, A ? A + ((int) B | (int) C | (int) D | (int) E) : F);
	testValExpr("D?4:C?3:B?2:A?1:0", args, sargs, D ? 4 : C ? 3 : B ? 2 : A ? 1 : 0);
	testValExpr("a>0?1:0", args, sargs, A > 0 ? 1 : 0);
	testValExpr("A=1", args, sargs, A == 1);
	testValExpr("A&(I||!J)&(K||!L)", args, sargs, (int) A & (int) (I || !J) & (int) (K || !L));
	testValExpr("(A||!B)&(C||!D)&(E||!F)&(G||!H)", args, sargs, (int) (A || !B) & (int) (C || !D) & (int) (E || !F) & (int) (G || !H));
	testValExpr("(A-1.0)", args, sargs, (A - 1.0));
	testValExpr("A&&C?A-1:B", args, sargs, A && C ? A - 1 : B);
	testValExpr("C+(A/D)*(B-C)", args, sargs, C + (A / D) * (B - C));
	testValExpr("nint(4095*((A-C)/(B-C)))", args, sargs, (int) (4095 * ((A - C) / (B - C))));
	testValExpr(".005*A/8", args, sargs, .005 * A / 8);
	testValExpr("A=0||a=2", args, sargs, A == 0 || A == 2);
	testValExpr("A?2:1", args, sargs, A ? 2 : 1);
	testValExpr("A*1.0", args, sargs, A * 1.0);
	testValExpr("log(A)", args, sargs, std::log(A));
	testValExpr("-(-2)**2", args, sargs, pow(-1 * -2., 2));
	testValExpr("--2**2", args, sargs, pow(-1 * -2., 2));
	testValExpr("A--B", args, sargs, A - -B);
	testValExpr("A--B*C", args, sargs, A - -B * C);
	testValExpr("A+B*C", args, sargs, A + B * C);
	testValExpr("D*((A-B)/C)", args, sargs, D * ((A - B) / C));
	testValExpr("cos(pi)", args, sargs, cos(3.14159265358979323846));
	testValExpr("ceil(1.5)", args, sargs, ceil(1.5));
	testValExpr("ceil(-1.5)", args, sargs, ceil(-1.5));
	testValExpr("floor(1.5)", args, sargs, floor(1.5));
	testValExpr("floor(-1.5)", args, sargs, floor(-1.5));
	testValExpr("1!=2", args, sargs, 1 != 2);
	testValExpr("D2R", args, sargs, 3.14159265358979323846 / 180);
	testValExpr("R2D", args, sargs, 180 / 3.14159265358979323846);
	testValExpr("atan(pi)", args, sargs, atan(3.14159265358979323846));
	testValExpr("1<=2", args, sargs, 1 <= 2);
	testValExpr("cosh(pi)", args, sargs, cosh(3.14159265358979323846));
	testValExpr("sinh(pi)", args, sargs, sinh(3.14159265358979323846));
	testValExpr("tanh(pi)", args, sargs, tanh(3.14159265358979323846));
	testValExpr("atan2(10,5)", args, sargs, atan2(5., 10));
	testValExpr("sqr(10)", args, sargs, sqrt(10.));
	testValExpr("sqrt(10)", args, sargs, sqrt(10.));
	testValExpr("2^3", args, sargs, pow(2.,3));
	testValExpr("2**3", args, sargs, pow(2.,3));
	testValExpr("a<?b", args, sargs, std::min(A,B));
	testValExpr("a>?b", args, sargs, std::max(A,B));
	testValExpr("5xor3", args, sargs, 5 ^ 3);
	testValExpr("asin(.3)", args, sargs, asin(.3));
	testValExpr("acos(.3)", args, sargs, acos(.3));
	testValExpr("A?0;C:=3.3:B", args, sargs, A==0 ? B : 0);
	testValExpr("C;C:=3", args, sargs, 3.3);
	testValExpr("A?(0;C:=3.3):B", args, sargs, A==0 ? B : 0);
	testValExpr("C;C:=3", args, sargs, 3.3);
	testValExpr("SSCANF(AA,'%*6c%f')", args, sargs, 1);
	testValExpr("SSCANF('-1','%d')", args, sargs, -1);
	testValExpr("SSCANF('-1','%hd')", args, sargs, -1);
	testValExpr("SSCANF('-1','%ld')", args, sargs, -1);
	
	char temp[256];
	std::string sAA(AA);
	std::string sBB(BB);
	std::string sCC(CC);
	std::string sDD(DD);
	std::string sEE(EE);
	std::string sFF(FF);
	std::string sGG(GG);
	std::string sHH(HH);
	std::string sII(II);
	std::string sJJ(JJ);
	std::string sKK(KK);
	std::string sLL(LL);
	
	testSValExpr("LL[0,'.']", args, sargs, sLL.substr(0, sLL.find_first_of(".")).c_str());
	testSValExpr("A>B?BB:AA[A,A]", args, sargs, A>B ? BB : sAA.substr((int) A, 1).c_str());
	testSValExpr("'abcdef'{'bc','gh'}", args, sargs, "aghdef");
	testSValExpr("'yyy:'+'xxx:abc'-'xxx:'", args, sargs, "yyy:abc");
	testSValExpr("@@0:=BB;AA;aa:='string 1'", args, sargs, BB);
	testSValExpr("a:=-1;@@-a:=AA;BB;a:=1;bb:='string 2'", args, sargs, AA);
	
	sprintf(temp, "!PFCU%02d %s", (int) A, sAA.substr(0, 2).c_str());
	testSValExpr("printf('!PFCU%02d ', a)+aa[0,1]", args, sargs, temp);
	
	sprintf(temp, "!PFCU%02d E %d", (int) A, (int) (B*100));
	testSValExpr("$P('!PFCU%02d E ', a) + $P('%d',b*100)", args, sargs, temp);
	
	sprintf(temp, "RSET %d;RSET?", (int) A);
	testSValExpr("$P('RSET %d;RSET?',A)", args, sargs, temp);
	
	sprintf(temp, "SETP %5.2f;SETP?", A);
	testSValExpr("$P('SETP %5.2f;SETP?',A)", args, sargs, temp);
	
	sprintf(temp, "RAMP %d;RAMP?", (int) A);
	testSValExpr("$P('RAMP %d;RAMP?',A)", args, sargs, temp);
	
	sprintf(temp, "RAMPR %5.2f;RAMPR?", A);
	testSValExpr("$P('RAMPR %5.2f;RAMPR?',A)", args, sargs, temp);
	
	sprintf(temp, "%s%s%s", AA, BB, CC);
	testSValExpr("AA+(BB)+CC", args, sargs, temp);
	
	sprintf(temp, "%s%s%s%s", DD, AA, EE, BB);
	testSValExpr("DD+AA+EE+BB", args, sargs, temp);

	printf("%s\n", AA);
	
	return testDone();
}
