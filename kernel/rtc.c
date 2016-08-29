// rtc.c
// Kryos Real Time Clock Driver

#include <x86.h>
#include <types.h>

#define RTC_BASE	0x70
#define RTC_DATA	0x71
#define RTC_SEC		0x00
#define RTC_MIN		0x02
#define RTC_HOUR	0x04
#define RTC_DAY		0x07
#define RTC_MON		0x08
#define RTC_YEAR	0x09

/* 2000-03-01 (mod 400 year, immediately after feb29 */
#define LEAPOCH (946684800LL + 86400*(31+29))
#define DAYS_PER_400Y (365*400 + 97)

typedef uint32_t time_t;

uint32_t rtc_read(uint8_t b) {
	outb(RTC_BASE, b);
	uint32_t in = inb(RTC_DATA);

	uint32_t out;
	out = ((in >> 4) & 0x0F) * 10;
	out += (in & 0x0F);
	return out;
}

char *rtc_month(uint32_t month) {
	char *months[12] = {
		"January", "February", "March", "April",
		"May", "June", "July", "August", "September",
		"October", "November", "December" };
	return months[month - 1];
}

char* ctime(time_t t) {
	// t is a value in seconds
	uint32_t secs = t - LEAPOCH;
	uint32_t days = secs / (60*60*24);
	uint32_t remsec = secs % (60*60*24);

	if (remsec < 0)  {
		remsec += 86400;
		days--;
	}
	

	//printf("t: %x Year: %d Month: %d Day: %d Hour: %d", t, years, i, days, hours); 
}


time_t time(void) {
	time_t t;

	uint32_t sec = rtc_read(RTC_SEC);
	uint32_t min = rtc_read(RTC_MIN);
	uint32_t hour = rtc_read(RTC_HOUR);
	uint32_t day = rtc_read(RTC_DAY);
	uint32_t mon = rtc_read(RTC_MON);
	uint32_t year = rtc_read(RTC_YEAR);
	int i;
	uint32_t months[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 304, 334};


	if (year < 70) 
		year += 100;	
	year += 1900;

	for (i = 1970; i < year; i++)
	{
		if ((i % 4) != 0)
			day += 365;
		else if ((i % 400) != 0)
			day += 366;
		else if ((i % 100) != 0)
			day += 365;
		else 
			day += 366;
	}

	day += months[mon-1];
	day++;

	t = day * 24;	// hours
	t += hour;		// current hour
	t *= 60;			// minutes
	t += min;		// current minute
	t *= 60;			// seconds
	t += sec;		// current second
	return t;
}



