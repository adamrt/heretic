#pragma once

#define MESSAGES_MAX_LEN (16384)

#include "span.h"

usize read_messages(span_t*, char*);
int message_count(char*);
int message_by_index(char* string, int index, char* buffer);
