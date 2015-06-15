/**
 * @author: cangmu
 * @description: C 语言使用配置文件库.
 */
#ifndef UTIL__STRING_H
#define UTIL__STRING_H

#include <string.h>

static int is_whitespace(const int ch)
{
	return strchr(" \t\r\n", ch) != NULL;
}

static int is_alpha(const int ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static int is_digit(const int ch){
	return (ch >= '0' && ch <= '9');
}

static int is_wordchar(const int ch)
{
	return (ch == '_') || is_alpha(ch) || is_digit(ch);
}

static int is_empty_str(const char *str)
{
	const char *p = str;
	while(*p && is_whitespace(*p))
	{
		p++;
	}
	return *p == '\0';
}

/* 返回左边不包含空白字符的字符串的指针 */
static char *ltrim(char *str)
{
	char *p = str;
	while(*p && is_whitespace(*p))
	{
		p++;
	}
	return p;
}

/* 返回右边不包含空白字符的字符串的指针 */
static char *rtrim(char *str)
{
	char *p;
	p = str + strlen(str) - 1;
	while(p >= str && is_whitespace(*p))
	{
		p--;
	}
	*(p + 1) = '\0';
	return str;
}

/* 返回左右两边不包含空白字符的字符串的指针 */
static char *trim(char *str)
{
	char *p;
	p = ltrim(str);
	rtrim(p);
	return p;
}

#endif
