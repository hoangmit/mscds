#pragma once

/**
function to parse time format
*/
char * strptime(const char *buf, const char *fmt, struct tm *tm);