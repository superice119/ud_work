#ifndef __DEBUG_H__
#define __DEBUG_H__



#define SPI_SUCCESS     0
#define SPI_ERROR       -1
#define BURST_CHUNK     1024

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT   printf
#else
#define DEBUG_PRINT(...)
#endif

#ifdef DEBUG
    #define DEBUG_MSG(str)                fprintf(stderr, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stderr,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                if(a==NULL){return SPI_ERROR;}
#endif


#endif //__DEBUG_H_

