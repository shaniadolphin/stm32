


const unsigned int EncryptKey[] =
{
	0xeb3c904d, 0x5357494e, 0x342e3100, 0x02404000,
	0x02000200, 0x00f8c000, 0x20004000, 0x01000000,
	0x00a62c00, 0x800029da, 0x102b3648, 0x35303020,
	0x56312e30, 0x30204641, 0x54313620, 0x202033c9,
	0x8ed1bcfc, 0x7b1607bd, 0x7800c576, 0x001e5616,
	0x55bf2205, 0x897e0089, 0x4e02b10b, 0xfcf3a406,
	0x1fbd007c, 0xc645fe0f, 0x384e247d, 0x208bc199,
	0xe87e0183, 0xeb3a66a1, 0x1c7c663b, 0x078a57fc
};


//加解密算法
//data 输入要加密的内容
//length 加密长度多少字节
//key 密匙，默认值0x00
void EncryptData(unsigned char *data, int length, unsigned char key)
{
	int i, j;
	unsigned char *tempbuf;

	tempbuf = (unsigned char *)data;

	if (!tempbuf || length <= 0)
		return;

	j = 0;
	for (i=0; i<length; i++)
	{
		tempbuf[i] ^= EncryptKey[j];
		tempbuf[i] ^= key;
		j = 2*i*j*j - 3*i*j + 3;
		if (j < 0 || j >= sizeof(EncryptKey))
			j = 2;
	}
}

unsigned int calc_sum(unsigned char* buf, unsigned int length)
{
	unsigned int i;
	unsigned int checksum = 0;
	unsigned char *p;

	p = buf;

	for (i=0; i<length; i++)
	{
		checksum = checksum + *p;
		p++;
	}

	return checksum;
}

