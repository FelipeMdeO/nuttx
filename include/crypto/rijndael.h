/****************************************************************************
 * include/crypto/rijndael.h
 *
 * SPDX-License-Identifier: NuttX-PublicDomain
 *
 * SPDX-FileContributor: Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 * SPDX-FileContributor: Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
 * SPDX-FileContributor: Paulo Barreto <paulo.barreto@terra.com.br>
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/

#ifndef __INCLUDE_CRYPTO_RIJNDAEL_H
#define __INCLUDE_CRYPTO_RIJNDAEL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>

#define AES_MAXKEYBITS  (256)
#define AES_MAXKEYBYTES (AES_MAXKEYBITS / 8)

/* for 256-bit keys, fewer for less */

#define AES_MAXROUNDS 14

/*  The structure for key information */

typedef struct
{
  int enc_only;                         /* context contains only encrypt schedule */
  int nr;                               /* key-length-dependent number of rounds */
  uint32_t ek[4 * (AES_MAXROUNDS + 1)]; /* encrypt key schedule */
  uint32_t dk[4 * (AES_MAXROUNDS + 1)]; /* decrypt key schedule */
} rijndael_ctx;

int rijndael_set_key(FAR rijndael_ctx *, FAR const u_char *, int);
int rijndael_set_key_enc_only(FAR rijndael_ctx *, FAR const u_char *, int);
void rijndael_decrypt(FAR rijndael_ctx *, FAR const u_char *, FAR u_char *);
void rijndael_encrypt(FAR rijndael_ctx *, FAR const u_char *, FAR u_char *);

int rijndael_keysetupenc(unsigned int [],
                         const unsigned char [],
                         int);
int rijndael_keysetupdec(unsigned int [],
                         const unsigned char [],
                         int);
void rijndael_encrypt1(const unsigned int [],
                       int,
                       const unsigned char [],
                       unsigned char []);

#endif /* __INCLUDE_CRYPTO_RIJNDAEL_H */
