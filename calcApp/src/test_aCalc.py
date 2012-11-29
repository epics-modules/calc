#!/usr/bin/env python

#from ca_util import *
import epics
from math import *
from string import *
import time
import copy
import os
#os.environ['EPICS_CA_ADDR_LIST'] = "164.54.53.99"

aCalcRecord = "xxx:userArrayCalc10"
calc = aCalcRecord + ".CALC"
result = aCalcRecord + ".VAL"
aresult = aCalcRecord + ".AVAL"

small = 1.e-9

def nint(a):
	if (a>0):
		return(int(a+.5))
	else:
		return(int(a-.5))

def arraySum(a1, a2):
	minlen = min(len(a1), len(a2))
	result = []
	for i in range(minlen):
		result.append(a1[i]+a2[i])
	print a1[:minlen], " + ", a2[:minlen], " = ", result
	return result

def same(a1, a2, l):
	result = True
	for i in range(l):
		result = result and (a1[i]==a2[i])
	return result

# Initialize an aCalcout record's fields for testing
A2L = "ABCDEFGHIJKL"

A=1.
B=2.
C=3.
D=4.
E=5.
F=6.
G=7.
H=8.
I=9.
J=10.
K=11.
L=12.
AA = [1,2,3]
BB = [4,5,6]
CC = [7,8,9]
DD = [10,11,12]
EE = [13,14,15]
FF = [16,17,18]
GG = [19,20,21]
HH = [1,2,3]
II = [1,2,3]
JJ = [1,2,3]
KK = [1,2,3]
LL = [1,2,3]

epics.caput(calc, "0")
for i in range(12):
	#print "connecting to", aCalcRecord + "." + A2L[i]
	epics.caput(aCalcRecord + "." + A2L[i], eval(A2L[i]))
	#print "connecting to", aCalcRecord + "." + A2L[i] + A2L[i]
	epics.caput(aCalcRecord + "." + A2L[i] + A2L[i], eval(A2L[i]+A2L[i]) )

# List of expressions for testing
# exp = [(aCalc_expression, equivalent_python_expression), ...]
# If equivalent_python_expression == None, then the aCalc_expression can
# be evaluated as-is by python.
exp = [
	("tan(A)", None),
	("sin(B)", None),
	("max(A,B,C)", None),
	("min(D,E,F)", None),
	("A<<2", "int(A)<<2"),
	("L>>1", "int(L)>>1"),
	("A?B:C", "(B,C)[A==0]"),
	("A&&B", "(A and B) != 0"),
	("A||B", "(A or B) != 0"),
	("LL[0,1]", "LL[0:2]"),
	("AA{1,2}", "[0,2,3]"),
	("A>B", None),
	("A>B?BB:AA[A,A]", "(AA[nint(A):nint(A+1)],BB)[A>B]"),
	("A<B?BB:AA[A,A]", "(AA[nint(A):nint(A+1)],BB)[A<B]"),
	("A>=4", None),
	("A=0?1:0", "(0,1)[A==0]"),
	("A+B", None),
	("(B=0)?(A+16384):A+B", "(A+B,A+16384)[B==0]"),
	("1.e7/A", None),
	("A>9?1:0", "(0,1)[A>9]"),
	("A%10+1", None),
	("!A", "not A"),
	("C+((A-E)/(D-E))*(B-C)", None),
	("A#B", "A!=B"),
	("E+nint(D*((A-C)/(B-C)))", None),
	("(A+1)*1000", None),
	("B?0:!A", "(0,not A)[B==0]"),
	("1", None),
	("A?0:B", "(0,B)[A==0]"),
	("A&&B&&!I", "(((A and B) != 0) and (not I)) != 0"),
	("(A&B&C&D)=1", "(nint(A)&nint(B)&nint(C)&nint(D))==1"),
	("(A&B&C&D&E&F&G&H)=1", "(nint(A)&nint(B)&nint(C)&nint(D)&nint(E)&nint(F)&nint(G)&nint(H))==1"),
	("(A>15)&&B", "((A>15) and B) != 0"),
	("(ABS(A)>1)&&B", "((abs(A)>1) and B) != 0"),
	("(A)=1", "(A)==1"),
	("A*4095", None),
	("(A||B||C||D||E||F)?1:0", "(1,0)[(A or B or C or D or E or F)==0]"),
	("(A<B)?C:D", "(C,D)[(A<B)==0]"),
	("A/(10**(B-2))", None),
	("(F<0.01)?0:(((A+C-B-D)/F)*E)", "(0,(((A+C-B-D)/F)*E))[(F<0.01)==0]"),
	("((A+B)<0.01)?0:(((A-B)/(A+B))*E)", "(0,(((A-B)/(A+B))*E))[((A+B)<0.01)==0]"),
	("((A+B)<0.01)?0:(((A-B)/(A+B))*E)", "(0,(((A-B)/(A+B))*E))[((A+B)<0.01)==0]"),
	("(A - ((B*C)/D))", None),
	("A-B*C/D", None),
	("A/(10**(B-2))", None),
	("min(max(C,A/B),D)", None),
	("(A=B)?C:D", "(C,D)[(A==B)==0]"),
	("A?A+(B|C|D|E):F", "(A+(int(round(B))|int(round(C))|int(round(D))|int(round(E))),F)[A==0]"),
	("D?4:C?3:B?2:A?1:0", "(4,((3,(2,(1,0)[A==0])[B==0])[C==0]))[D==0]"),
	("a>0?1:0", "(1,0)[(A>0)==0]"),
	("AA + BB", "arraySum(AA,BB)"),
	("A=1", "A==1"),
	("A&(I||!J)&(K||!L)", "nint(A)&nint(I or not J)&nint(K or not L)"),
	("(A||!B)&(C||!D)&(E||!F)&(G||!H)", "nint(A or not B)&nint(C or  not D)&nint(E or not F)&nint(G or not H)"),
	("(A-1.0)", None),
	("A&&C?A-1:B", "(A-1,B)[(A and C)==0]"),
	("C+(A/D)*(B-C)", None),
	("nint(4095*((A-C)/(B-C)))", None),
	(".005*A/8", None),
	("A=0||a=2", "A==0 or A==2"),
	("A?2:1", "(2,1)[A==0]"),
	("AA+(BB)+CC", "arraySum(arraySum(AA,BB),CC)"),
	("A*1.0", None),
	("DD+AA+EE+BB", "arraySum(arraySum(DD,AA),arraySum(EE,BB))"),
	("log(A)", None),
	("-(-2)**2", "--2**2"),
	("--2**2", None),
	("A--B",None),
	("A--B*C",None),
	("A+B*C",None),
	("D*((A-B)/C)", None),
	# the following two must be in order, because of the store to c
	("A?0;C:=3.3:B", "(0,B)[A==0]"),
	("C;C:=3", "3.3"),
	# the following two must be in order, because of the store to c
	("A?(0;C:=3.3):B", "(0,B)[A==0]"),
	("C;C:=3", "3.3")
]

def nint(x):
	return int(floor(x+.5))

def test1(i):
	e = exp[i]
	epics.caput(calc,e[0], wait=True)
	#time.sleep(5)
	rtry = epics.caget(result)
	atry = epics.caget(aresult)
	if (e[1]):
		r = eval(e[1])
		print "\n", e[0], "-->", e[1]
	else:
		r = eval(e[0])
		print "\n", e[0]
	if ((type(r) == type(1.0)) or (type(r) == type(1))):
		if (abs(r - rtry) < small):
			print "OK\t", "rtry=",rtry, ", r=",r
			return(0)
		else:
			print "ERROR\t", "rtry=",rtry, ", atry=",atry, ", r=",r
			return(1)
	elif (type(r) == type(True)):
		if ((abs(rtry) < small) == (r == False)):
			print "OK\t", "rtry=",rtry, ", r=",r
			return(0)
		else:
			print "ERROR\t", "rtry=",rtry, ", atry=",atry, ", r=",r
			return(1)
	else:
		print "ERROR\t", "rtry=",rtry, ", atry=",atry, ", r=",r
		return(1)

def test():
	numErrors = 0
	for e in exp:
		epics.caput(calc,e[0], wait=True)
		time.sleep(.1)
		rtry = epics.caget(result)
		atry = epics.caget(aresult)
		if (e[1]):
			r = eval(e[1])
			print "\n", e[0], "-->", e[1]
		else:
			r = eval(e[0])
			print "\n", e[0]
		print "type(r)=", type(r), "type(rtry)=", type(rtry)
		if ((type(r) == type(1.0)) or (type(r) == type(1))):
			if (abs(r - rtry) < small):
				print "OK(s)\t", "rtry=",rtry, ", r=",r
			else:
				print "ERROR(s)\t", "rtry=",rtry, ", atry=",atry, ", r=",r
				numErrors = numErrors+1
		elif (type(r) == type(True)):
			if ((abs(rtry) < small) == (r == False)):
				print "OK(b)\t", "rtry=",rtry, ", r=",r
			else:
				print "ERROR(b)\t", "rtry=",rtry, ", atry=",atry, ", r=",r
				numErrors = numErrors+1
		elif (type(r) == type([1,2])):
			minlen = min(len(r), len(atry))
			if same(r, atry, minlen):
			#if (r[:minlen] == atry[:minlen]):
				print "OK(a)\t", "atry=",atry[:minlen], ", r=",r
			else:
				print "ERROR(a)\t", "rtry=",rtry, ", atry=",atry[:minlen], ", r=",r[:minlen]
				numErrors = numErrors+1
		else:
			print "ERROR(?)\t", "rtry=",rtry, ", atry=",atry, ", r=",r
			numErrors = numErrors+1

	print "\n------------------------"
	print "  ", numErrors, " errors."
	print "------------------------"

if __name__ == "__main__":
	test()
