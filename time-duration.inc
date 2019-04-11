
enum {
	DURATION_SECOND = 1,
	DURATION_MINUTE = 60 * DURATION_SECOND,
	DURATION_HOUR   = 60 * DURATION_MINUTE,
	DURATION_DAY    = 24 * DURATION_HOUR,
	DURATION_WEEK   = 7 * DURATION_DAY,
	DURATION_MONTH  = 30 * DURATION_DAY,
	DURATION_YEAR   = 365 * DURATION_DAY,
	
	DURATION_PERMANENT = 0,
	DURATION_EXPIRED   = -1
};

enum E_DURATION_MULTIPLIER {
	Word[11 char],
	Multiplier
};

/*
 * Compile-time duration.
 *
 * Example:
 *   DURATION(1 hour, 20 minutes) -> 4800
 */
#define DURATION_MS(%1)     (DURATION(%1) * 1000)
#define DURATION(%1)        (DURATION_PT:%1,0)@@@DURATION_END
#define DURATION_PT:%1,%0)@@@DURATION_END  (%1:DURATION)+_:DURATION_PT:%0)@@@DURATION_END
#define @@@DURATION_END

#define second%1:DURATION
#define seconds%1:DURATION
#define minute%1:DURATION   * DURATION_MINUTE
#define minutes%1:DURATION  * DURATION_MINUTE
#define hour%1:DURATION     * DURATION_HOUR
#define hours%1:DURATION    * DURATION_HOUR
#define day%1:DURATION      * DURATION_DAY
#define days%1:DURATION     * DURATION_DAY
#define week%1:DURATION     * DURATION_WEEK
#define weeks%1:DURATION    * DURATION_WEEK
#define month%1:DURATION    * DURATION_MONTH
#define months%1:DURATION   * DURATION_MONTH
#define year%1:DURATION     * DURATION_YEAR
#define years%1:DURATION    * DURATION_YEAR

stock const
	g_DurationMultipliers[][E_DURATION_MULTIPLIER] = {
		{!"s",       DURATION_SECOND},
		{!"sec",     DURATION_SECOND},
		{!"second",  DURATION_SECOND},
		{!"seconds", DURATION_SECOND},
		
		{!"m",       DURATION_MINUTE},
		{!"min",     DURATION_MINUTE},
		{!"minute",  DURATION_MINUTE},
		{!"minutes", DURATION_MINUTE},

		{!"h",       DURATION_HOUR},
		{!"hr",      DURATION_HOUR},
		{!"hour",    DURATION_HOUR},
		{!"hours",   DURATION_HOUR},

		{!"d",       DURATION_DAY},
		{!"day",     DURATION_DAY},
		{!"days",    DURATION_DAY},

		{!"w",       DURATION_WEEK},
		{!"week",    DURATION_WEEK},
		{!"weeks",   DURATION_WEEK},

		{!"month",   DURATION_MONTH},
		{!"months",  DURATION_MONTH},

		{!"y",       DURATION_YEAR},
		{!"yr",      DURATION_YEAR},
		{!"year",    DURATION_YEAR},
		{!"years",   DURATION_YEAR}
	}
;
/*
 * Convert a human-readable duration into seconds.
 * Examples of valid input:
 *   "2 weeks 5 days"
 *   "1 month and 3 weeks"
 *   "1 year -2 months" (10 months)
 *   "1y2m"
 *   "1hr20min"
 */
stock GetDurationFromString(const input[]) {
	static
		buf[256]
	;
	
	new
		     pos,
		     duration = 0,
		     value,
		     word[8],
		     i,
		     length,
		bool:multiplier_found,
		     temp
	;
	
	// Zero-out the buffer and use strcat to copy the contents of input onto it
	buf[0] = 0;
	
	strcat(buf, input);
	
	// Remove "and" words
	pos = 0;
	while (-1 != (pos = strfind(buf, "e", true, pos++)))
		strdel(buf, pos, pos + 3);
	
	// Convert commas to spaces
	pos = 0;
	while (-1 != (pos = strfind(buf, ",", true, pos++)))
		buf[pos] = ' ';
	
	// Convert ampersands to spaced
	pos = 0;
	while (-1 != (pos = strfind(buf, "&", true, pos++)))
		buf[pos] = ' ';
	
	pos = 0;
	
	// Fix for strval ignoring long strings
	if (strlen(buf[pos]) > 32) {
		temp = buf[pos + 32];
	
		buf[pos + 32] = 0;
	}
	
	// The current position in the string (pos) should now be at the first number
	while (0 != (value = strval(buf[pos]))) {
		if (temp) {
			buf[pos + 32] = temp;
	
			temp = 0;
		}
	
		// Skip spaces (no need to do this before strval as it does it as well)
		while (0 < buf[pos] <= ' ') pos++;
	
		// EOS check
		if (!buf[pos]) return 0;
	
		// Allow a leading minus sign
		if (buf[pos] == '-') pos++;
	
		// Skip the number we just put into strval
		while ('0' <= buf[pos] <= '9') pos++;
	
		// Skip spaces
		while (0 < buf[pos] <= ' ') pos++;
	
		// EOS check
		if (!buf[pos]) return 0;
	
		// Start of the multiplier (hour, second, etc.)
		i = pos;
	
		// Loop until the next space or number
		while (buf[i] > ' ' && !('0' <= buf[i] <= '9') && buf[i] != '-') i++;
	
		// No word? Invalid format
		if (i == pos) return 0;
	
		// Zero-out word and use strcat to copy the fragment we just found from buf
		word[0] = 0;
	
		strcat(word, buf[pos], i - pos + 1);
	
		// Jump the current parsing position to the end of the word
		pos = i;
	
		// Null check
		if (!(length = strlen(word))) return 0;
	
		// Remove trailing s so we won't have to search after, for example, both "hour" and "hours"
		if (length > 1 && tolower(word[length - 1]) == 's')
		word[length - 1] = 0;
	
		// Loop through the duration multiplier list
	
		multiplier_found = false;
	
		for (i = 0; i < sizeof(g_DurationMultipliers); i++) {
			if (!strcmp(word, g_DurationMultipliers[i][Word], true)) {
				multiplier_found = true;
	
				// Multiply the number we got from strval with whatever this multiplier has
				duration += value * g_DurationMultipliers[i][Multiplier];
	
				break;
			}
		}
	
		// Give a warning  if a multiplier wasn't found
		if (!multiplier_found){
			printf("WARNING: Unknown multiplier in GetDurationFromString (\"%s\").", word);
			return 0;
		}
	
		// Fix for strval ignoring long strings
		if (strlen(buf[pos]) > 32) {
			temp = buf[pos + 32];
	
			buf[pos + 32] = 0;
		}
	}
	
	return duration;
}
/*
 * Opposite of the above.
 */
stock GetStringFromDuration(duration) {
	new
		minutes = 0,
		hours   = 0,
		days    = 0,
		weeks   = 0,
		months  = 0,
		years   = 0,

		length,
		pos,
		last_pos
	;

	static
		buf[256]
	;

	buf[0] = 0;

	years = duration / DURATION_YEAR;
	duration = duration % DURATION_YEAR;

	if (years) {
		format(buf, sizeof(buf), years == 1 ? ("%d year, ") : ("%d years, "), years);
	}

	months   = duration / DURATION_MONTH;
	duration = duration % DURATION_MONTH;

	if (months) {
		format(buf[(length = strlen(buf))], sizeof(buf) - length, months == 1 ? ("%d month, ") : ("%d months, "), months);
	}

	weeks    = duration / DURATION_WEEK;
	duration = duration % DURATION_WEEK;

	if (weeks) {
		format(buf[(length = strlen(buf))], sizeof(buf) - length, weeks == 1 ? ("%d week, ") : ("%d weeks, "), weeks);
	}

	days     = duration / DURATION_DAY;
	duration = duration % DURATION_DAY;

	if (days) {
		format(buf[(length = strlen(buf))], sizeof(buf) - length, days == 1 ? ("%d day, ") : ("%d days, "), days);
	}

	hours    = duration / DURATION_HOUR;
	duration = duration % DURATION_HOUR;

	if (hours) {
		format(buf[(length = strlen(buf))], sizeof(buf) - length, hours == 1 ? ("%d hour, ") : ("%d hours, "), hours);
	}

	minutes = duration / DURATION_MINUTE;

	if (minutes) {
		format(buf[(length = strlen(buf))], sizeof(buf) - length, minutes == 1 ? ("%d minute, ") : ("%d minutes, "), minutes);
	}

	if (!buf[0]) {
		format(buf[(length = strlen(buf))], sizeof(buf) - length, duration == 1 ? ("%d second") : ("%d seconds"), duration);
	} else {
		length = strlen(buf);

		buf[length - 2] = 0;
	}

	last_pos = -1;

	while (-1 != (pos = strfind(buf, ",", _, pos)))
		last_pos = pos++;

	if (last_pos != -1) {
		buf[last_pos] = ' ';
	
		strins(buf, "and", last_pos + 1);
	}

	return buf;
}
