#ifndef DEBUGUTIL_H
#define DEBUGUTIL_H

#define DEBUG

#ifdef DEBUG
#    include <cstdio>
#    define dprintf(fmt...) { printf("%s : %d\n\t",__FILE__,__LINE__);printf(fmt); fflush(stdout);}
#else
#    define dprintf(fmt...)
#endif


#endif //DEBUGUTIL_H
