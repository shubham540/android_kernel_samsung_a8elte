
#include <linux/err.h>
#include <sdp/dek_common.h>
#include <sdp/dek_aes.h>

static struct crypto_blkcipher *dek_aes_key_setup(kek_t *kek)
{
	struct crypto_blkcipher *tfm = NULL;

	tfm = crypto_alloc_blkcipher("cbc(aes)", 0, CRYPTO_ALG_ASYNC);
	if (!IS_ERR(tfm)) {
		crypto_blkcipher_setkey(tfm, kek->buf, kek->len);
	} else {
		printk("dek: failed to alloc blkcipher\n");
	}
	return tfm;
}


static void dek_aes_key_free(struct crypto_blkcipher *tfm)
{
	crypto_free_blkcipher(tfm);
}

static int __dek_aes_encrypt(struct crypto_blkcipher *tfm, char *src, char *dst, int len) {
	struct blkcipher_desc desc;
	struct scatterlist src_sg, dst_sg;
	int bsize = crypto_blkcipher_ivsize(tfm);
	u8 iv[bsize];

	memset(&iv, 0, sizeof(iv));
	desc.tfm = tfm;
	desc.info = iv;
	desc.flags = 0;

	sg_init_one(&src_sg, src, len);
	sg_init_one(&dst_sg, dst, len);

	return crypto_blkcipher_encrypt_iv(&desc, &dst_sg, &src_sg, len);
}

static int __dek_aes_decrypt(struct crypto_blkcipher *tfm, char *src, char *dst, int len) {
	struct blkcipher_desc desc;
	struct scatterlist src_sg, dst_sg;
	int bsize = crypto_blkcipher_ivsize(tfm);
	u8 iv[bsize];

	memset(&iv, 0, sizeof(iv));
	desc.tfm = tfm;
	desc.info = iv;
	desc.flags = 0;

	sg_init_one(&src_sg, src, len);
	sg_init_one(&dst_sg, dst, len);

	return crypto_blkcipher_decrypt_iv(&desc, &dst_sg, &src_sg, len);
}

int dek_aes_encrypt(kek_t *kek, char *src, char *dst, int len) {
	int rc;
	struct crypto_blkcipher *tfm;

	if(kek == NULL) return -EINVAL;

	tfm = dek_aes_key_setup(kek);

	if(tfm) {
		rc = __dek_aes_encrypt(tfm, src, dst, len);
		dek_aes_key_free(tfm);
		return rc;
	} else
		return -ENOMEM;
}

int dek_aes_decrypt(kek_t *kek, char *src, char *dst, int len) {
	int rc;
	struct crypto_blkcipher *tfm;

	if(kek == NULL) return -EINVAL;

	tfm = dek_aes_key_setup(kek);

	if(tfm) {
		rc = __dek_aes_decrypt(tfm, src, dst, len);
		dek_aes_key_free(tfm);
		return rc;
	} else
		return -ENOMEM;
}
