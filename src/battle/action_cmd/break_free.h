#ifndef _BREAK_FREE_H_
#define _BREAK_FREE_H_

#include "common_structs.h"

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

// args: none
API_CALLABLE(action_command_break_free_init);

// args: prep time, duration, effectiveness, difficulty
API_CALLABLE(action_command_break_free_start);

void action_command_break_free_update(void);
void action_command_break_free_draw(void);
void action_command_break_free_free(void);

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif
