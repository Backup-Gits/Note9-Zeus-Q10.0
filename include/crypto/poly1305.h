/*
 * Common values for the Poly1305 algorithm
 */

#ifndef _CRYPTO_POLY1305_H
#define _CRYPTO_POLY1305_H

#include <linux/types.h>
#include <linux/crypto.h>

#define POLY1305_BLOCK_SIZE	16
#define POLY1305_KEY_SIZE	32
#define POLY1305_DIGEST_SIZE	16

struct poly1305_key {
	u32 r[5];	/* key, base 2^26 */
};

struct poly1305_state {
	u32 h[5];	/* accumulator, base 2^26 */
};

struct poly1305_desc_ctx {
	/* key */
	struct poly1305_key r;
	/* finalize key */
	u32 s[4];
	/* accumulator */
	struct poly1305_state h;
	/* partial buffer */
	u8 buf[POLY1305_BLOCK_SIZE];
	/* bytes used in partial buffer */
	unsigned int buflen;
	/* r key has been set */
	bool rset;
	/* s key has been set */
	bool sset;
};

int crypto_poly1305_init(struct shash_desc *desc);
unsigned int crypto_poly1305_setdesckey(struct poly1305_desc_ctx *dctx,
					const u8 *src, unsigned int srclen);
int crypto_poly1305_update(struct shash_desc *desc,
			   const u8 *src, unsigned int srclen);
int crypto_poly1305_final(struct shash_desc *desc, u8 *dst);

#endif
