#include <cmath>
#include <algorithm>
#include <vector>

#include <epicsTypes.h>
#include <epicsMath.h>
#include <epicsUnitTest.h>
#include <testMain.h>

#include "aCalcPostfix.h"

static void testValExpr(const char* expr, double* args, double** aargs, double expected)
{	
	unsigned char rpn[255];
	short err;
	
	double val;
	double aval[1];
	
	epicsUInt32 amask;
	
	if (aCalcPostfix(expr, rpn, &err))
	{
		testDiag("postfix: %s in expression '%s'", aCalcErrorStr(err), expr);
		return;
	}
	
	if (aCalcPerform(args, 12, aargs, 12, 3, &val, aval, rpn, 1, &amask))
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

static void testAValExpr(const char* expr, double* args, double** aargs, double* expected, int length)
{	
	unsigned char rpn[255];
	short err;
	
	double val;
	double aval[12];
	
	epicsUInt32 amask;
	
	if (aCalcPostfix(expr, rpn, &err))
	{
		testDiag("postfix: %s in expression '%s'", aCalcErrorStr(err), expr);
		return;
	}
	
	if (aCalcPerform(args, 12, aargs, 12, 12, &val, aval, rpn, 12, &amask))
	{
		testDiag("calcPerform: error evaluating '%s'", expr);
		return;
	}
	
	bool pass = true;
	int i = 0;
	
	for (i = 0; i < length; i += 1)
	{
		if (finite(expected[i]) && finite(aval[i]))
		{
			if (fabs(expected[i] - aval[i]) > 1e-8)
			{
				pass = false;
				break;
			}
		}
		else if (isnan(expected[i]) && !isnan(aval[i]))
		{
			pass = false;
			break;
		}
		else if (aval[i] != expected[i])
		{
			pass = false;
			break;
		}
	}
	
	if(!testOk(pass, "%s", expr))
	{	
		testDiag("Expected aval[%d]: %f, Got: %f", i, expected[i], aval[i]);
	}	
}


MAIN(acalcTest)
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
	
	double AA[3] = {1.0, 2.0, 3.0};
	double BB[3] = {4.0, 5.0, 6.0};
	double CC[3] = {7.0, 8.0, 9.0};
	double DD[3] = {-1.0, 0.0, 1.0};
	double EE[3] = {0.0, 1.0, 2.0};
	double FF[3] = {16.0, 17.0, 18.0};
	double GG[3] = {19.0, 20.0, 21.0};
	double HH[3] = {1.0, 2.0, 3.0};
	double II[3] = {1.0, 2.0, 3.0};
	double JJ[3] = {1.0, 2.0, 3.0};
	double KK[3] = {1.0, 2.0, 3.0};
	double LL[3] = {1.0, 2.0, 3.0};
	
	double args[12] = {A, B, C, D, E, F, G, H, I, J, K, L};
	double* aargs[12] = {AA, BB, CC, DD, EE, FF, GG, HH, II, JJ, KK, LL};
	
	testPlan(122);

	testValExpr("finite(1)", args, aargs, 1);
	testValExpr("finite(AA)", args, aargs, 1);
	testValExpr("isnan(1)", args, aargs, 0);
	testValExpr("isnan(AA)", args, aargs, 0);
	testValExpr("isinf(1)", args, aargs, 0);
	testValExpr("isinf(AA)", args, aargs, 0);
	testValExpr("PI/S2R", args, aargs, 180.0*3600.0);
	testValExpr("R2S*PI", args, aargs, 180.0*3600.0);
	testValExpr("1.234", args, aargs, 1.234);
	testValExpr("tan(A)", args, aargs, tan(A));
	testValExpr("sin(B)", args, aargs, sin(B));
	testValExpr("max(A,B,C)", args, aargs, std::max(A, std::max(B, C)));
	testValExpr("min(D,E,F)", args, aargs, std::min(D, std::min(E, F)));
	testValExpr("A<<2", args, aargs, (int) A << 2);
	testValExpr("L>>1", args, aargs, (int) args[11] >> 1);
	testValExpr("A?B:C", args, aargs, A ? B : C);
	testValExpr("A&&B", args, aargs, (int) (A && B));
	testValExpr("A||B", args, aargs, (int) (A || B));
	testValExpr("A>B", args, aargs, (int) (A > B));
	testValExpr("A>=4", args, aargs, (int) (A >= 4));
	testValExpr("A=0?1:0", args, aargs, 0);
	testValExpr("A+B", args, aargs, A + B);
	testValExpr("(B=0)?(A+16384):A+B", args, aargs, (B == 0) ? (A + 16384) : (A + B));
	testValExpr("1.e7/A", args, aargs, (1e7 / A));
	testValExpr("A>9?1:0", args, aargs, (A > 9) ? 1 : 0);
	testValExpr("A%10+1", args, aargs, (int) A % 10 + 1);
	testValExpr("!A", args, aargs, !A);
	testValExpr("C+((A-E)/(D-E))*(B-C)", args, aargs, C+((A-E)/(D-E))*(B-C));
	testValExpr("A#B", args, aargs, A != B);
	testValExpr("E+nint(D*((A-C)/(B-C)))", args, aargs, E + (int)(D*(A - C)/(B-C)));
	testValExpr("(A+1)*1000", args, aargs, (A + 1) * 1000);
	testValExpr("B?0:!A", args, aargs, B ? 0 : (!A));
	testValExpr("1", args, aargs, 1);
	testValExpr("A?0:B", args, aargs, A ? 0 : B);
	testValExpr("A&&B&&!I", args, aargs, A && B && (!I));
	testValExpr("(A&B&C&D)=1", args, aargs, ((int) A & (int) B & (int) C & (int) D) == 1);
	testValExpr("(A&B&C&D&E&F&G&H)=1", args, aargs, ((int) A & (int) B & (int) C & (int) D & (int) E & (int) F & (int) G & (int) H) == 1);
	testValExpr("(A>15)&&B", args, aargs, (A > 15) && B);
	testValExpr("(ABS(A)>1)&&B", args, aargs, 0);
	testValExpr("(A)=1", args, aargs, A == 1);
	testValExpr("A*4095", args, aargs, A * 4095);
	testValExpr("(A||B||C||D||E||F)?1:0", args, aargs, (A || B || C || D || E || F) ? 1 : 0);
	testValExpr("(A<B)?C:D", args, aargs, (A < B) ? C : D);
	testValExpr("A/(10**(B-2))", args, aargs, A / (pow(10, (B - 2))));
	testValExpr("(F<0.01)?0:(((A+C-B-D)/F)*E)", args, aargs, (F < 0.01) ? 0 : (((A + C - B - D) / F) * E));
	testValExpr("((A+B)<0.01)?0:(((A-B)/(A+B))*E)", args, aargs, ((A + B) < 0.01) ? 0 : (((A - B) / (A + B)) * E));
	testValExpr("(A - ((B*C)/D))", args, aargs, A - ((B * C) / D));
	testValExpr("A-B*C/D", args, aargs, A - B * C / D);
	testValExpr("min(max(C,A/B),D)", args, aargs, std::min(std::max(C, A / B), D));
	testValExpr("(A=B)?C:D", args, aargs, (A == B) ? C : D);
	testValExpr("A?A+(B|C|D|E):F", args, aargs, A ? A + ((int) B | (int) C | (int) D | (int) E) : F);
	testValExpr("D?4:C?3:B?2:A?1:0", args, aargs, D ? 4 : C ? 3 : B ? 2 : A ? 1 : 0);
	testValExpr("a>0?1:0", args, aargs, A > 0 ? 1 : 0);
	testValExpr("A=1", args, aargs, A == 1);
	testValExpr("A&(I||!J)&(K||!L)", args, aargs, (int) A & (int) (I || !J) & (int) (K || !L));
	testValExpr("(A||!B)&(C||!D)&(E||!F)&(G||!H)", args, aargs, (int) (A || !B) & (int) (C || !D) & (int) (E || !F) & (int) (G || !H));
	testValExpr("(A-1.0)", args, aargs, (A - 1.0));
	testValExpr("A&&C?A-1:B", args, aargs, A && C ? A - 1 : B);
	testValExpr("C+(A/D)*(B-C)", args, aargs, C + (A / D) * (B - C));
	testValExpr("nint(4095*((A-C)/(B-C)))", args, aargs, (int) (4095 * ((A - C) / (B - C))));
	testValExpr(".005*A/8", args, aargs, .005 * A / 8);
	testValExpr("A=0||a=2", args, aargs, A == 0 || A == 2);
	testValExpr("A?2:1", args, aargs, A ? 2 : 1);
	testValExpr("A*1.0", args, aargs, A * 1.0);
	testValExpr("log(A)", args, aargs, std::log(A));
	testValExpr("-(-2)**2", args, aargs, pow(-1 * -2., 2));
	testValExpr("--2**2", args, aargs, pow(-1 * -2., 2));
	testValExpr("A--B", args, aargs, A - -B);
	testValExpr("A--B*C", args, aargs, A - -B * C);
	testValExpr("A+B*C", args, aargs, A + B * C);
	testValExpr("D*((A-B)/C)", args, aargs, D * ((A - B) / C));
	testValExpr("cos(pi)", args, aargs, cos(3.14159265358979323846));
	testValExpr("ceil(1.5)", args, aargs, ceil(1.5));
	testValExpr("ceil(-1.5)", args, aargs, ceil(-1.5));
	testValExpr("floor(1.5)", args, aargs, floor(1.5));
	testValExpr("floor(-1.5)", args, aargs, floor(-1.5));
	testValExpr("1!=2", args, aargs, 1 != 2);
	testValExpr("D2R", args, aargs, 3.14159265358979323846 / 180);
	testValExpr("R2D", args, aargs, 180 / 3.14159265358979323846);
	testValExpr("atan(pi)", args, aargs, atan(3.14159265358979323846));
	testValExpr("1<=2", args, aargs, 1 <= 2);
	testValExpr("cosh(pi)", args, aargs, cosh(3.14159265358979323846));
	testValExpr("sinh(pi)", args, aargs, sinh(3.14159265358979323846));
	testValExpr("tanh(pi)", args, aargs, tanh(3.14159265358979323846));
	testValExpr("atan2(10,5)", args, aargs, atan2(5., 10));
	testValExpr("sqr(10)", args, aargs, sqrt(10.));
	testValExpr("sqrt(10)", args, aargs, sqrt(10.));
	testValExpr("2^3", args, aargs, pow(2.,3));
	testValExpr("2**3", args, aargs, pow(2.,3));
	testValExpr("a<?b", args, aargs, std::min(A,B));
	testValExpr("a>?b", args, aargs, std::max(A,B));
	testValExpr("5xor3", args, aargs, 5 ^ 3);
	testValExpr("asin(.3)", args, aargs, asin(.3));
	testValExpr("acos(.3)", args, aargs, acos(.3));
	testValExpr("A?0;C:=3.3:B", args, aargs, A==0 ? B : 0);
	testValExpr("C;C:=3", args, aargs, 3.3);
	testValExpr("A?(0;C:=3.3):B", args, aargs, A==0 ? B : 0);
	testValExpr("C;C:=3", args, aargs, 3.3);
	testValExpr("sum(BB)", args, aargs, BB[0] + BB[1] + BB[2]);
	testValExpr("avg(BB[0,2])", args, aargs, (BB[0] + BB[1] + BB[2]) / 3.);
	testValExpr("amax(aa[0,2])", args, aargs, std::max(std::max(AA[0], AA[1]), AA[2]));
	testValExpr("amin(aa[0,2])", args, aargs, std::min(std::min(AA[0], AA[1]), AA[2]));
	testValExpr("ixmax(aa[0,2])", args, aargs, 2);
	testValExpr("ixmin(aa[0,2])", args, aargs, 0);
	testValExpr("ixz(dd)", args, aargs, 1);
	testValExpr("ixnz(ee)", args, aargs, 1);
	
	std::vector<double> temp;
	
	temp.push_back(LL[0]);
	temp.push_back(LL[1]);
	testAValExpr("LL[0,1]", args, aargs, &temp[0], 2);
	
	temp.clear();
	
	temp.push_back(0.);
	temp.push_back(AA[1]);
	temp.push_back(AA[2]);	
	testAValExpr("AA{1,2}", args, aargs, &temp[0], 3);
	
	temp.clear();
	
	temp.push_back(AA[0] + BB[0]);
	temp.push_back(AA[1] + BB[1]);
	temp.push_back(AA[2] + BB[2]);	
	testAValExpr("AA+BB", args, aargs, &temp[0], 3);

	temp.clear();
	
	temp.push_back(AA[0] + BB[0] + CC[0]);
	temp.push_back(AA[1] + BB[1] + CC[1]);
	temp.push_back(AA[2] + BB[2] + CC[2]);	
	testAValExpr("AA+(BB)+CC", args, aargs, &temp[0], 3);
	
	temp.clear();
	
	temp.push_back(DD[0] + AA[0] + BB[0] + EE[0]);
	temp.push_back(DD[1] + AA[1] + BB[1] + EE[1]);
	temp.push_back(DD[2] + AA[2] + BB[2] + EE[2]);	
	testAValExpr("DD+AA+EE+BB", args, aargs, &temp[0], 3);
		
	temp.clear();

	temp.push_back(1.);
	temp.push_back(2.);
	temp.push_back(3.);	
	temp.push_back(4.);	
	temp.push_back(5.);	
	testAValExpr("cat(aa[0,2],bb[0,2])", args, aargs, &temp[0], 5);	
	
	temp.clear();
	
	temp.push_back(1.);
	temp.push_back(2.);
	temp.push_back(3.);	
	temp.push_back(7.);
	testAValExpr("cat(aa[0,2],7)", args, aargs, &temp[0], 4);
	
	temp.clear();
	
	temp.push_back(4.);
	temp.push_back(9.);
	temp.push_back(15.);	
	testAValExpr("CUM(BB)", args, aargs, &temp[0], 3);
	
	temp.clear();
	
	temp.push_back(std::max(std::max(AA[0], B), C));
	temp.push_back(std::max(std::max(AA[1], B), C));
	temp.push_back(std::max(std::max(AA[2], B), C));	
	testAValExpr("max(AA,B,C)", args, aargs, &temp[0], 3);
	
	temp.clear();
	
	temp.push_back(std::max(std::max(CC[0], B), A));
	temp.push_back(std::max(std::max(CC[1], B), A));
	temp.push_back(std::max(std::max(CC[2], B), A));	
	testAValExpr("max(A,B,CC)", args, aargs, &temp[0], 3);
	
	temp.clear();
	
	temp.push_back(std::max(std::max(BB[0], A), C));
	temp.push_back(std::max(std::max(BB[1], A), C));
	temp.push_back(std::max(std::max(BB[2], A), C));	
	testAValExpr("max(A,BB,C)", args, aargs, &temp[0], 3);
	
	temp.clear();
	
	temp.push_back(AA[(int) A]);
	testAValExpr("A>B?BB:AA[A,A]", args, aargs, A>B ? BB : &temp[0], A>B ? 3 : 1);
	
	temp.clear();
	
	temp.push_back(AA[(int) A]);
	testAValExpr("A<B?BB:AA[A,A]", args, aargs, A<B ? BB : &temp[0], A<B ? 3 : 1);
	
	testAValExpr("ix[1,3]", args, aargs, AA, 3);
	
	double BB_check[3] = {4.0, 5.0, 6.0};
	testAValExpr("@@0:=BB;AA;aa:=aa-3[0,2]", args, aargs, BB_check, 3);
	testAValExpr("a:=-7;@@-a:=BB;HH;a:=1;hh:=ii", args, aargs, BB, 3);

	return testDone();
}
