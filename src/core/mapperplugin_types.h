//
// mapperplugin.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

typedef enum
{
    FSMLOG_ERROR,
    FSMLOG_WARNING,
    FSMLOG_INFO,
    FSMLOG_MESSAGE,
    FSMLOG_DEBUG,
} FSMAPPER_LOG_TYPE;

typedef unsigned long long FSMAPPER_EVENT_ID;
