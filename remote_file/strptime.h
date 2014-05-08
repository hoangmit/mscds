#pragma once

/**
function to parse format for time
*/

char * strptime(const char *buf, const char *fmt, struct tm *tm);