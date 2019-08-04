// defs.h ... global definitions
// part of Multi-attribute Linear-hashed Files
// Defines types and constants used throughout code
// Last modified by John Shepherd, July 2019

#ifndef DEFS_H
#define DEFS_H 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "util.h"

#define PAGESIZE    1024
#define NO_PAGE     0xffffffff
#define MAXERRMSG   200
#define MAXTUPLEN   200
#define MAXRELNAME  200
#define MAXFILENAME MAXRELNAME+8
#define MAXBITS     32
#define OK          0
#define TRUE        1
#define FALSE       0
#define ZERO        0x0
#define ONE         0x1

// extracts i'th bit from hash value
#define getBit(position,hash) (((hash) & (1 << (position))) >> (position))

typedef char Bool;
typedef unsigned char Byte;
typedef int Status;
typedef unsigned int Offset;
typedef unsigned int Count;
typedef Offset PageID;

#endif
