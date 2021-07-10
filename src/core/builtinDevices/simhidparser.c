/*
 * simhidparser.c
 *  Author: Hiroshi Murayama <opiopan@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "simhidparser.h"

/*========================================================
 Error message
========================================================*/
static const char *ERR_SYNTAX = "syntax error";
static const char* ERR_TOOLONG = "too long line";
static const char* ERR_TOOMANYPARAM = "parameters of command must be less than 10";
static const char* ERR_NOCMD = "not supported command";
static const char* ERR_NOOPT = "unknown option name";
static const char *ERR_INVBOOLOPT = "option value must be 0 or 1";
static const char *ERR_INVINTOPT = "option value must be integer";
static const char *ERR_TOOLONGOPT = "too long option value";

/*========================================================
 Command parser implementation
========================================================*/
void simhid_parser_init(SimhidParserCtx * ctx, char *buf, int len)
{
    ctx->phase = SIMHID_PARSE_INIT;
    ctx->linebuf = buf;
    ctx->linebuflen = len;
    ctx->parsedlen = 0;
    ctx->paramnum = 0;
    ctx->command = -1;
    ctx->err = NULL;
}

static inline bool isSeparator(int c)
{
    return c == ' ' || c == '\t';
}

static inline bool isCommand(int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static inline bool isNumeric(int c)
{
    return c >= '0' && c <= '9';
}

bool simhid_parser_parse(SimhidParserCtx *ctx, int c)
{
    bool rc = false;

    if (ctx->phase == SIMHID_PARSE_END){
        ctx->phase = SIMHID_PARSE_INIT;
        ctx->parsedlen = 0;
        ctx->paramnum = 0;
        ctx->command = -1;
        ctx->err = NULL;
    }

    if (ctx->parsedlen == ctx->linebuflen && ctx->phase != SIMHID_PARSE_SKIP){
        ctx->phase = SIMHID_PARSE_SKIP;
        ctx->err = ERR_TOOLONG;
    }
    if (ctx->phase != SIMHID_PARSE_SKIP || ctx->phase != SIMHID_PARSE_EOL){
        ctx->linebuf[ctx->parsedlen++] = c;
    }

    switch (ctx->phase){
    case SIMHID_PARSE_INIT:{
        if (isSeparator(c)){
            /* nothing to do */
        }else if (isCommand(c)){
            ctx->phase = SIMHID_PARSE_POSTCOMMAND;
            ctx->command = c;
        }else if (c == '\r'){
            ctx->phase = SIMHID_PARSE_EOL;
        }else{
            ctx->phase = SIMHID_PARSE_SKIP;
            ctx->err = ERR_SYNTAX;
        }
        break;
    }
    case SIMHID_PARSE_POSTCOMMAND:{
        if (isSeparator(c)){
            ctx->phase = SIMHID_PARSE_SEPARATOR;
        }else if (c == '\r'){
            ctx->phase = SIMHID_PARSE_EOL;
        }else{
            ctx->phase = SIMHID_PARSE_SKIP;
            ctx->err = ERR_SYNTAX;
        }
        break;
    }
    case SIMHID_PARSE_SEPARATOR:{
        if (isSeparator(c)){
            /* nothing to do */
        }else if (c == '\r'){
            ctx->phase = SIMHID_PARSE_EOL;
        }else if (ctx->paramnum >= COMMAND_MAX_PARAM){
            ctx->phase = SIMHID_PARSE_SKIP;
            ctx->err = ERR_TOOMANYPARAM;
        }else{
            SimhidCommandParam* param = ctx->params + ctx->paramnum++;
            param->strvalue = ctx->linebuf + ctx->parsedlen - 1;
            param->isNumber = false;
            param->len = 1;
            if (c == '-'){
                ctx->phase = SIMHID_PARSE_MINUSNUMBER;
            }else if (isNumeric(c)){
                ctx->phase = SIMHID_PARSE_NUMBER;
                param->numvalue = c - '0';
            }else{
                ctx->phase = SIMHID_PARSE_STRING;
            }
        }
        break;
    }
    case SIMHID_PARSE_MINUSNUMBER:{
        if (isSeparator(c)){
            ctx->phase = SIMHID_PARSE_SEPARATOR;
        }else if (c == '\r'){
            ctx->phase = SIMHID_PARSE_EOL;
        }else{
            SimhidCommandParam *param = ctx->params + ctx->paramnum - 1;
            param->len++;
            if (isNumeric(c)){
                ctx->phase = SIMHID_PARSE_NUMBER;
                param->numvalue = -(c - '0');
            }else{
                ctx->phase = SIMHID_PARSE_STRING;
            }
        }
        break;
    }
    case SIMHID_PARSE_NUMBER:{
        SimhidCommandParam *param = ctx->params + ctx->paramnum - 1;
        if (isSeparator(c)){
            ctx->phase = SIMHID_PARSE_SEPARATOR;
            param->isNumber = true;
            ctx->linebuf[ctx->parsedlen - 1] = '\0';
        }else if (c == '\r'){
            ctx->phase = SIMHID_PARSE_EOL;
            param->isNumber = true;
            ctx->linebuf[ctx->parsedlen - 1] = '\0';
        }else{
            param->len++;
            if (isNumeric(c)){
                param->numvalue *= 10;
                param->numvalue += param->numvalue > 0 ? 
                                   c - '0' : -(c - '0');
            }else{
                ctx->phase = SIMHID_PARSE_STRING;
            }
        }
        break;
    }
    case SIMHID_PARSE_STRING:{
        SimhidCommandParam *param = ctx->params + ctx->paramnum - 1;
        if (isSeparator(c)){
            ctx->phase = SIMHID_PARSE_SEPARATOR;
            ctx->linebuf[ctx->parsedlen - 1] = '\0';
        }else if (c == '\r'){
            ctx->linebuf[ctx->parsedlen - 1] = '\0';
            ctx->phase = SIMHID_PARSE_EOL;
        }else{
            param->len++;
        }
        break;
    }
    case SIMHID_PARSE_SKIP:{
        if (c == '\r'){
            ctx->phase = SIMHID_PARSE_EOL;
        }
        break;
    }
    case SIMHID_PARSE_EOL:{
        if (c == '\n'){
            ctx->phase = SIMHID_PARSE_END;
            rc = true;
        }else{
            ctx->phase = SIMHID_PARSE_SKIP;
            ctx->err = ERR_SYNTAX;
        }
        break;
    }
    case SIMHID_PARSE_END:{
        /* no condition reach here */
        break;
    }
    }

    return rc;
}
