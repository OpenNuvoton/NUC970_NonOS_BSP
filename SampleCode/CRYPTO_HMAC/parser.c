/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2012 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

extern UINT32  VectorDataBase, VectorDataLimit;


static UINT8   	*file_base;
static UINT32	file_idx, file_size;


static char  g_line[20*1024];

__align(32)   UINT8  g_hmac_msg_pool[1024];
UINT8   *g_hmac_msg;

__align(32)   UINT8	 g_hmac_mac_pool[1024];
UINT8	*g_hmac_mac;

int  	g_key_len, g_msg_len, g_mac_len;
UINT32	g_sha_mode;

int  open_test_file(void)
{
	file_base = (UINT8 *)&VectorDataBase;
	file_size = (UINT32)&VectorDataLimit - (UINT32)&VectorDataBase;
	file_idx = 0;
	return 1;
}


int  close_test_file()
{
	return 0;
}


static int  read_file(UINT8 *buffer, int length)
{
	if (file_idx+1 >= file_size)
		return -1;
	memcpy(buffer, &file_base[file_idx], length);
	file_idx += length;
	return 0;
}


int  get_line(void)
{
	int  		i;
	UINT8		ch;
	
	if (file_idx+1 >= file_size)
	{
		//sysprintf("EOF.\n");
		return -1;
	}
	
	memset(g_line, 0, sizeof(g_line));

	for (i = 0;	 ; i++)
	{
		if (read_file(&ch, 1) < 0)
			return 0;
			
		if ((ch == 0x0D) || (ch == 0x0A))
			break;
			
		g_line[i] = ch;
	}

	while (1)
	{
		if (read_file(&ch, 1) < 0)
			return 0;
		
		if ((ch != 0x0D) && (ch != 0x0A))
			break;
	}
	file_idx--;
	return 0;
}


int  is_hex_char(char c)
{
	if ((c >= '0') && (c <= '9'))
		return 1;
	if ((c >= 'a') && (c <= 'f'))
		return 1;
	if ((c >= 'A') && (c <= 'F'))
		return 1;
	return 0;
}


UINT8  char_to_hex(UINT8 c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	return 0;
}


int  str_to_hex(UINT8 *str, UINT8 *hex, int swap)
{
	int			i, count = 0, actual_len;
	UINT8  	val8;
	
	while (*str)
	{
		if (!is_hex_char(*str))
		{
			//sysprintf("ERROR - not hex!!\n");
			return count;
		}
		
		val8 = char_to_hex(*str);
		str++;
		
		if (!is_hex_char(*str))
		{
			//sysprintf("ERROR - not hex!!\n");
			return count;
		}

		val8 = (val8 << 4) | char_to_hex(*str);
		str++;
		
		hex[count] = val8;
		//sysprintf("hex = 0x%x\n", val8);
		count++;
	}
	
	actual_len = count;
	
	for ( ; count % 4 ; count++)
		hex[count] = 0;

	if (!swap)
		return actual_len;

	// SWAP
	for (i = 0; i < count; i+=4)
	{
		val8 = hex[i];
		hex[i] = hex[i+3];
		hex[i+3] = val8;
		
		val8 = hex[i+1];
		hex[i+1] = hex[i+2];
		hex[i+2] = val8;
	}
	
	return actual_len;
}


int  str_to_decimal(UINT8 *str)
{
	int		val32;
	
	val32 = 0;
	while (*str)
	{
		if ((*str < '0') || (*str > '9'))
		{
			return val32;
		}
		val32 = (val32 * 10) + (*str - '0');
		str++;
	}
	return val32;
}


int  get_next_pattern(void)
{
	int  		line_num = 1;
	int			blen;
	UINT8  	*p;

	/* get non-cachable buffer pointer */
	g_hmac_msg = (UINT8 *)((UINT32)g_hmac_msg_pool | 0x80000000);
	g_hmac_mac = (UINT8 *)((UINT32)g_hmac_mac_pool | 0x80000000);

	g_key_len = 0;
	
	memset(g_hmac_msg, 0x0, 128);
	
	while (get_line() == 0)
	{
		line_num++;
		
		if (g_line[0] == '#')
			continue;
			
		if (strncmp(g_line ,"Klen", 4) == 0)
		{
			p = (UINT8 *)&g_line[4];
			while ((*p < '0') || (*p > '9')) 
				p++;

			g_key_len = str_to_decimal(p);
			sysprintf("    Key length = %d\n", g_key_len);
			continue;
		}	

		if (strncmp(g_line ,"Tlen", 4) == 0)
		{
			p = (UINT8 *)&g_line[4];
			while ((*p < '0') || (*p > '9')) 
				p++;

			g_mac_len = str_to_decimal(p);
			sysprintf("    HMAC length = %d\n", g_mac_len);
			continue;
		}	

		if (strncmp(g_line ,"Key", 3) == 0)
		{
			p = (UINT8 *)&g_line[3];
			while (!is_hex_char(*p)) p++;
			if (str_to_hex(p, &g_hmac_msg[0], 0) != g_key_len)
			{
				sysprintf("key len mismatch!\n");
				return -1;
			}
			continue;
		}
		
		if (strncmp(g_line ,"Msg", 3) == 0)
		{
			p = (UINT8 *)&g_line[3];
			while (!is_hex_char(*p)) p++;
			g_msg_len = str_to_hex(p, &g_hmac_msg[(g_key_len+3)&0xfffffffc], 0);
			sysprintf("    Message length = %d\n", g_msg_len);
			continue;
		}

		if (strncmp(g_line ,"Mac", 3) == 0)
		{
			p = (UINT8 *)&g_line[3];
			while (!is_hex_char(*p)) p++;
			str_to_hex(p, &g_hmac_mac[0], 0);
			return 0;
		}

		if (strncmp(g_line ,"[L=", 3) == 0)
		{
			p = (UINT8 *)&g_line[3];
			while ((*p < '0') || (*p > '9')) 
				p++;
			blen = str_to_decimal(p);
			switch (blen)
			{
				case 20:
					g_sha_mode = SHA_MODE_SHA1;
					sysprintf("SHA1...\n");
					break;
				case 28:
					g_sha_mode = SHA_MODE_SHA224;
					sysprintf("SHA224...\n");
					break;
				case 32:
					sysprintf("SHA256...\n");
					g_sha_mode = SHA_MODE_SHA256;
					break;
				case 48:
					sysprintf("SHA384...\n");
					g_sha_mode = SHA_MODE_SHA384;
					break;
				case 64:
					sysprintf("SHA512...\n");
					g_sha_mode = SHA_MODE_SHA512;
					break;
			}
		}

		//sysprintf("LINE %d = %s\n", line_num, g_line);
	}
	return -1;
}




