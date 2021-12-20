/*
 * simhidparser.h
 *  Author: Hiroshi Murayama <opiopan@gmail.com>
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*========================================================
 Command parser
========================================================*/
#define COMMAND_MAX_PARAM 32

typedef enum{
    SIMHID_PARSE_INIT = 0,
    SIMHID_PARSE_POSTCOMMAND,
    SIMHID_PARSE_SEPARATOR,
    SIMHID_PARSE_MINUSNUMBER,
    SIMHID_PARSE_NUMBER,
    SIMHID_PARSE_STRING,
    SIMHID_PARSE_SKIP,
    SIMHID_PARSE_EOL,
    SIMHID_PARSE_END,
} SIMHID_CMDPARSE_PHASE;

typedef struct{
    bool isNumber;
    const char *strvalue;
    int len;
    int numvalue;
}SimhidCommandParam;

typedef struct{
    char* linebuf;
    int linebuflen;
    int parsedlen;
    SIMHID_CMDPARSE_PHASE phase;
    int command;
    int paramnum;
    SimhidCommandParam params[COMMAND_MAX_PARAM];
    const char* err;
} SimhidParserCtx;

void simhid_parser_init(SimhidParserCtx* ctx, char* buf, int len);
bool simhid_parser_parse(SimhidParserCtx* ctx, int c);

#ifdef __cplusplus
}
#endif
