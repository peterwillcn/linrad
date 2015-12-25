//#if Linux == FreeBSD
// Under Linux Linux becomes Linux and generates an error since Linux is
// undefined. (FreeBSD is also undefined.)
//  #define RUSAGE_OLD 1
//#else
  #if OSNUM == OSNUM_LINUX
    #define RUSAGE_OLD 0
  #else
    #define RUSAGE_OLD 0
  #endif
//#endif
