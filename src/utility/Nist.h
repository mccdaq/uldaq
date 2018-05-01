#ifndef _NIST_H_
#define _NIST_H_

namespace ul
{

#define NIST_TYPE_J	0
#define NIST_TYPE_K	1
#define NIST_TYPE_T	2
#define NIST_TYPE_E	3
#define NIST_TYPE_R	4
#define NIST_TYPE_S	5
#define NIST_TYPE_B	6
#define NIST_TYPE_N	7

typedef unsigned char byte;

double NISTCalcVoltage(byte, double);
double NISTCalcTemp(byte, double);
}

#endif
