// This file will be used in case Linrad is complied on
// a non-INTEL architecture
// Entries should never be called, but just in case...

#include "globdef.h"
#include "uidef.h"

void split_one(void)
{
lirerr(999999);
}

void split_two(void)
{
lirerr(999998);
}

void expand_rawdat(void)
{
lirerr(999997);
}

void compress_rawdat_disk(void)
{
lirerr(999996);
}

void compress_rawdat_net(void)
{
lirerr(999995);
}

// The function check_mmx is not really needed.
// We need it only in an x86 build environment without support
// of assembly language. Such platforms are rare, but it happens
// when CMake cannot find NASM.
int check_mmx(void)
{
  return 0;
}

