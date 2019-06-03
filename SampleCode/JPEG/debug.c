INT32 ParsingJPEG(PUINT8 JPEG_Buffer, UINT32 Length, PUINT32 pu32Width, PUINT32 pu32Height, PUINT32 u32Format, BOOL bPrimary)
{
    //  HByte,LByte     :   For JPEG Marker decode
    //  MLength         :   Length of Marker (all data in the marker)
    //  index           :   The address index of the JPEG bit-stream
    //  QT_Count        :   Quantization Table Counter

    unsigned char HByte, LByte;
    unsigned short int MLength;
    int index;
    int HuffTable = 0xF;
    int HuffCount = 0;
    int HuffIndex = 0;
    PUINT8 pu8Addr;
    int HuffmanIndex[4] = {0};
    int HuffmanSize[5] = {0};
    INT32 i32Result = ERR_SUCCESS;
    BOOL bWorkaround = FALSE;
    index = 0;

    while (index < Length)
    {
        HByte = JPEG_Buffer[index++];

        if (HByte == 0xFF)
        {
            LByte = JPEG_Buffer[index++];

            switch (LByte)      //May be a Marker
            {
                case 0xD8:      //SOI Marker (Start Of Image)
                    break;

                case 0xC1:
                case 0xC2:
                case 0xC3:
                case 0xC5:
                case 0xC6:
                case 0xC7:
                case 0xC8:
                case 0xC9:
                case 0xCA:
                case 0xCB:
                case 0xCD:
                case 0xCE:
                case 0xCF:
                    return ERR_MODE;                /* SOF Marker(Not Baseline Mode) */
                    break;

                case 0xDB:                              /* DQT Marker (Define Quantization Table) */
                    if (index + 1 > Length)
                        return ERR_FORMAT;          /* Wrong file format */

                    HByte = JPEG_Buffer[index++];
                    LByte = JPEG_Buffer[index++];
                    MLength = (HByte << 8) + LByte;
                    index += MLength - 2;           /* Skip DQT Data */

                    if (index > Length)
                        return ERR_FORMAT;          /* Wrong file format */

                    break;

                case 0xC0:                              /* SOF Marker (Baseline mode Start Of Frame) */
                {
                    UINT16 u16Height, u16Width, end_index, Nf, Ci, HSF[3], VSF[3], i;

                    if (index + 1 > Length)
                        return ERR_FORMAT;          /* Wrong file format */

                    HByte = JPEG_Buffer[index++];
                    LByte = JPEG_Buffer[index++];
                    MLength = (HByte << 8) + LByte;
                    end_index = index + MLength - 2;
                    index++;
                    HByte = JPEG_Buffer[index++];
                    LByte = JPEG_Buffer[index++];
                    u16Height = (HByte << 8) + LByte;   /* Get Image Height */
                    HByte = JPEG_Buffer[index++];
                    LByte = JPEG_Buffer[index++];
                    u16Width = (HByte << 8) + LByte;    /* Get Width Height */

                    sysprintf("Width %d, Height %d\n", u16Width, u16Height);
                    *pu32Width = u16Width;
                    *pu32Height = u16Height;
                    Nf = JPEG_Buffer[index++];

                    //sysprintf("Component  %d\n", Nf);
                    if (Nf != 3)
                        return ERR_MODE;

                    for (i = 0; i < Nf; i++)            /* Get Sampling factors */
                    {
                        Ci = JPEG_Buffer[index++];
                        HByte = JPEG_Buffer[index++];
                        HSF[i] = HByte / 16;
                        VSF[i] = HByte % 16;
                        index++;

                    }

                    /*for(i=0;i<Nf;i++)
                    {
                        sysprintf("Component %d, Hi %d, Vi %d\n",i,HSF[i],VSF[i]);
                    }*/


                    if (HSF[1] == VSF[1] == HSF[2] == VSF[2] == 1)
                    {
                        if (HSF[0] == 2 && VSF[0] == 2)
                        {
                            //sysprintf("YUV420\n");
                            *u32Format = JPEG_DEC_YUV420;
                        }
                        else if (HSF[0] == 2 && VSF[0] == 1)
                        {
                            //sysprintf("YUV422\n");
                            *u32Format = JPEG_DEC_YUV422;

                            if ((u16Height % 16) <= 8)
                                i32Result = ERR_ONLY_NORMAL;
                        }
                        else if (HSF[0] == VSF[0] == 1)
                        {
                            //sysprintf("YUV444\n");
                            *u32Format = JPEG_DEC_YUV444;

                            if ((u16Height % 16) <= 8)
                                i32Result = ERR_ONLY_NORMAL;
                        }
                        else
                            return ERR_MODE;
                    }
                    else
                        return ERR_MODE;


                    if (end_index != index)
                        return ERR_FORMAT;          /* Wrong file format */

                    if (index > Length)
                        return ERR_FORMAT;          /* Wrong file format */

                    break;
                }

                case 0xDA:                              /* Scan Header */
                {
                    int i, Ns, Td[3], Ta[3], end_index, Tda[3];

                    if (index + 1 > Length)
                        return ERR_FORMAT;          /* Wrong file format */

                    HByte = JPEG_Buffer[index++];
                    LByte = JPEG_Buffer[index++];
                    MLength = (HByte << 8) + LByte;

                    end_index = index + MLength - 2;

                    Ns = JPEG_Buffer[index++];

                    // sysprintf("Ns %d\n", Ns);
                    for (i = 0; i < Ns; i++)
                    {
                        index++;
                        Tda[i] = index;
                        HByte = JPEG_Buffer[index++];
                        Td[i] = HByte / 16;
                        Ta[i] = HByte % 16;
                    }

                    if (bWorkaround)
                    {
                        /*  for(i=0;i<Ns;i++)
                          sysprintf("Component %d Td %d Ta %d\n", i ,Td[i],Ta[i]);*/
                        if (Td[0] == 1 && Td[1] == 0 && Td[2] == 0)
                        {
                            JPEG_Buffer[Tda[0]] =  JPEG_Buffer[Tda[0]] & ~0xF0;
                            JPEG_Buffer[Tda[1]] =  JPEG_Buffer[Tda[1]] | 0x10;
                            JPEG_Buffer[Tda[2]] =  JPEG_Buffer[Tda[2]] | 0x10;
                            JPEG_Buffer[HuffmanIndex[0] - 1] = 0x01;
                            JPEG_Buffer[HuffmanIndex[1] - 1] = 0x00;

                        }

                        if (Ta[0] == 1 && Ta[1] == 0 && Ta[2] == 0)
                        {
                            JPEG_Buffer[Tda[0]] =  JPEG_Buffer[Tda[0]] & ~0x0F;
                            JPEG_Buffer[Tda[1]] =  JPEG_Buffer[Tda[1]] | 0x01;
                            JPEG_Buffer[Tda[2]] =  JPEG_Buffer[Tda[2]] | 0x01;
                            JPEG_Buffer[HuffmanIndex[2] - 1] = 0x11;
                            JPEG_Buffer[HuffmanIndex[3] - 1] = 0x10;
                        }
                    }

                    index += 3;

                    if (end_index != index)
                        return ERR_FORMAT;          /* Wrong file format */

                    if (index > Length)
                        return ERR_FORMAT;          /* Wrong file format */

                    if (bWorkaround)
                    {
                        index = 0;

                        pu8Addr = (PUINT8)JPEG_Buffer;

                        if (HuffCount == 2 || HuffCount == 3)
                        {
                            UINT32 u32Length = 0;

                            if (HuffTable != 0xA)
                            {
                                pu8Addr[256] = 0x00;        /* Clear Original 0xFF */
                                pu8Addr[257] = 0x00;        /* Clear Original 0xD8 */
                                pu8Addr[index++] = 0xFF;
                                pu8Addr[index++] = 0xD8;
                                pu8Addr[index++] = 0xFF;
                                pu8Addr[index++] = 0xC4;

                                index += 2;

                                for (i = 0; i < 4; i++)
                                {
                                    if (HuffTable & (1 << i))
                                    {
                                        switch (i)
                                        {
                                            case 0:
                                                pu8Addr[index++] = 0x00;
                                                u32Length += HuffmanSize[1] + 1;
                                                memcpy(pu8Addr + index, pu8Addr + HuffmanIndex[1], HuffmanSize[1]);
                                                index += HuffmanSize[1];
                                                break;

                                            case 1:
                                                pu8Addr[index++] = 0x01;
                                                u32Length += HuffmanSize[0] + 1;
                                                memcpy(pu8Addr + index, pu8Addr + HuffmanIndex[0], HuffmanSize[0]);
                                                index += HuffmanSize[0];

                                                break;

                                            case 2:
                                                pu8Addr[index++] = 0x10;
                                                u32Length += HuffmanSize[3] + 1;
                                                memcpy(pu8Addr + index, pu8Addr + HuffmanIndex[3], HuffmanSize[3]);
                                                index += HuffmanSize[3];
                                                break;

                                            case 3:
                                                pu8Addr[index++] = 0x11;
                                                u32Length += HuffmanSize[2] + 1;
                                                memcpy(pu8Addr + index, pu8Addr + HuffmanIndex[2], HuffmanSize[2]);
                                                index += HuffmanSize[2];
                                                break;
                                        }
                                    }
                                }

                                u32Length += 2;
                                pu8Addr[4] = (u32Length >> 8) & 0xFF;
                                pu8Addr[5] = u32Length & 0xFF;
                            }
                        }
                    }

                    return i32Result;
                    break;
                }

                case 0xC4:      //DHT Marker (Define Huffman Table)
                {
                    int Li, i;

                    if (index + 1 > Length)
                        return ERR_FORMAT;              /* Wrong file format */

                    HByte = JPEG_Buffer[index++];
                    LByte = JPEG_Buffer[index++];
                    MLength = (HByte << 8) + LByte - 2;

                    if ((index + MLength) > Length)
                        return ERR_FORMAT;              /* Wrong file format */

                    do
                    {
                        switch (JPEG_Buffer[index++])   /* Tc & Th */
                        {
                            case 0x00:
                                HuffmanIndex[0] = index;
                                HuffTable ^= 0x1;
                                HuffIndex = 0;
                                break;

                            case 0x01:
                                HuffmanIndex[1] = index;
                                HuffTable ^= 0x2;
                                HuffIndex = 1;
                                break;

                            case 0x10:
                                HuffmanIndex[2] = index;
                                HuffTable ^= 0x4;
                                HuffIndex = 2;
                                break;

                            case 0x11:
                                HuffmanIndex[3] = index;
                                HuffTable ^= 0x8;
                                HuffIndex = 3;
                                break;
                        }

                        HuffCount++;

                        Li = 0;

                        for (i = 1; i <= 16; i++)
                        {

                            Li += JPEG_Buffer[index++];
                        }

                        HuffmanSize[HuffIndex] = Li + 16;
                        MLength = MLength - 17 - Li;
                        index = index + Li;

                    } while (MLength != 0);

                    break;
                }

                case 0xE0:
                case 0xE1:
                case 0xE2:
                case 0xE3:
                case 0xE4:
                case 0xE5:
                case 0xE6:
                case 0xE7:
                case 0xE8:
                case 0xE9:
                case 0xEA:
                case 0xEB:
                case 0xEC:
                case 0xED:
                case 0xEE:
                case 0xEF:
                case 0xFE:
                {
                    /* Application Marker && Comment */
                    int tmp;

                    if (index + 1 > Length)
                        return ERR_FORMAT;          /* Wrong file format */

                    tmp = index - 2;
                    HByte = JPEG_Buffer[index++];
                    LByte = JPEG_Buffer[index++];
                    MLength = (HByte << 8) + LByte;

                    if (bPrimary)
                        index += MLength - 2;           /* Skip Application or Comment Data */

                    break;
                }
            }
        }
    }

    return ERR_FORMAT;      //Wrong file format
}