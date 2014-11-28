#include <stdint.h>
#include <string.h>
#ifndef __DES_H__
#define __DES_H__

#define ID_OK   0
#define RE_LEN  1

//typedef unsigned short int uint16_t;
//typedef unsigned long int uint32_t;

typedef struct
{
    uint32_t subkeys[32];                                             /* subkeys */
    uint32_t iv[2];                                       /* initializing vector */
    uint32_t originalIV[2];                        /* for restarting the context */
    int encrypt;                                               /* encrypt flag */
} DES_CBC_CTX;

typedef struct
{
    uint32_t subkeys[32];                                             /* subkeys */
    uint32_t iv[2];                                       /* initializing vector */
    uint32_t inputWhitener[2];                                 /* input whitener */
    uint32_t outputWhitener[2];                               /* output whitener */
    uint32_t originalIV[2];                        /* for restarting the context */
    int encrypt;                                              /* encrypt flag */
} DESX_CBC_CTX;

typedef struct
{
    uint32_t subkeys[3][32];                     /* subkeys for three operations */
    uint32_t iv[2];                                       /* initializing vector */
    uint32_t originalIV[2];                        /* for restarting the context */
    int encrypt;                                              /* encrypt flag */
} DES3_CBC_CTX;


void DES_CBCInit(DES_CBC_CTX *context, unsigned char *key, unsigned char *iv, int encrypt);
int DES_CBCUpdate(DES_CBC_CTX *context, unsigned char *output, unsigned char *input, unsigned int len);
void DES_CBCRestart(DES_CBC_CTX *context);

void DESX_CBCInit(DESX_CBC_CTX *context, unsigned char *key, unsigned char *iv, int encrypt);
int DESX_CBCUpdate (DESX_CBC_CTX *context, unsigned char *output, unsigned char *input, unsigned int len);
void DESX_CBCRestart(DESX_CBC_CTX *context);

void DES3_CBCInit(DES3_CBC_CTX *context, unsigned char *key, unsigned char *iv, int encrypt);
int DES3_CBCUpdate(DES3_CBC_CTX *context, unsigned char *output, unsigned char *input, unsigned int len);
int DES3_ECBUpdate(DES3_CBC_CTX *context, unsigned char *output, unsigned char *input, unsigned int len);
void DES3_CBCRestart(DES3_CBC_CTX *context);

#endif // __DES_H__
