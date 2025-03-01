#ifndef _masterp2_events_h_included_
#define _masterp2_events_h_included_

#include <stdint.h>

typedef struct mp_event 
{
    /// @brief event code
    uint16_t code;
    /// @brief correlation id (or just id understood by sender and receiver)
    int correl_id;
    union {
        int   i_arg;
        float f_arg;
        char* s_arg;
        void* p_arg;
    };

    union {
        int   i_arg2;
        float f_arg2;
        char* s_arg2;
        void* p_arg2;
    };
    
} mp_event_t;


#endif