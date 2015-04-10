#include "encode_and_decode.h"

char Char2Int(char ch)
{
    if(ch>='0' && ch<='9')return (char)(ch-'0');
    if(ch>='a' && ch<='f')return (char)(ch-'a'+10);
    if(ch>='A' && ch<='F')return (char)(ch-'A'+10);
    return -1;
}

char Str2Bin(char *str)
{
    char tempWord[2];
    char chn;

    tempWord[0] = Char2Int(str[0]);                                //make the B to 11 -- 00001011
    tempWord[1] = Char2Int(str[1]);                                //make the 0 to 0  -- 00000000

    chn = (tempWord[0] << 4) | tempWord[1];                //to change the BO to 10110000

    return chn;
}

uint8_t* UrlDecode(char* str)
{
	if(NULL==str)
		return NULL;
    char tmp[2];
    int i=0,idx=0,ndx;
    uint len=strlen(str);
    uint8_t* out=(uint8_t*)malloc(len+1);
	if(NULL==out)
		return NULL;
    memset(out,0,len+1);
    uint8_t* output=out;

    while(i<len)
    {
        if(str[i]=='%')
        {
            tmp[0]=str[i+1];
            tmp[1]=str[i+2];
            *(out++)=Str2Bin(tmp);
            i=i+3;
        }
        else if(str[i]=='+')
        {
            *(out++)=' ';
            i++;
        }
        else
        {
            *(out++)=str[i];
            i++;
        }
    }
    return output;
}
 
uint8_t* UrlEncode(char* str)
{ 
	if(NULL==str)
		return NULL; 
	int i,j = 0; 
	char ch;
	uint strSize=strlen(str);
	uint8_t* result=(uint8_t*)malloc(3*strSize+1);
	if(NULL==result)
		return NULL;
	memset(result,0,3*strSize+1);
	for (i=0; (i<strSize); i++)
	{   
		ch = str[i];   
		if ((ch >= 'A') && (ch <= 'Z'))
		{   
			result[j++] = ch;   
		} 
		else if ((ch >= 'a') && (ch <= 'z'))
		{   
			result[j++] = ch;   
		}
		else if ((ch >= '0') && (ch <= '9'))
		{   
			result[j++] = ch;   
		}
		else if(ch == ' ')
		{   
			result[j++] = '+';   
		}
		else
		{    
			sprintf(result+j, "%%%02x", (unsigned char)ch);   
			j += 3;     
		}   
	}      
	return result;   
}

int8_t* base64_encode(uint8_t* bindata, uint binlength, int8_t* base64)
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int base64_decode(int8_t* base64, uint8_t* bindata)
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}

int md5_encode(char* src,uint srclen,char* des)
{
	unsigned char md[16];
	char* de=MD5(src,srclen,md);
	if(NULL==de)
	{
		return -1;
	}
	int i;
	char tmp[3]={0};
	for (i = 0; i < 16; i++)
	{
        sprintf(tmp,"%2.2x",md[i]);
        strcat(des,tmp);
    }
	return 0;
}