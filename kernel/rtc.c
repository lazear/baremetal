// rtc.c
// Kryos Real Time Clock Driver

#include <x86.h>
#include <types.h>

#define RTC_BASE	0x70
#define RTC_DATA	0x71

void rtc_send(uint8_t b)
{
	outb(RTC_BASE, b);
}

uint8_t rtc_readin(void)
{
	return inb(RTC_DATA);
}

uint32_t rtc_read()
{
	uint32_t in = inb(RTC_DATA);
	uint32_t out;
	out = ((in >> 4) & 0x0F) * 10;
	out += (in & 0x0F);
	return out;
}

uint32_t RtcGetSecond(void)
{
	rtc_send(0x00);
	return rtc_read();
}

uint32_t RtcGetMinute(void)
{
	rtc_send(0x02);
	return rtc_read();
}

uint32_t RtcGetHour(void)
{
	rtc_send(0x04);
	return rtc_read();
}

uint32_t RtcGetDay(void)
{
	rtc_send(0x07);
	return rtc_read();
}

uint32_t RtcGetMonth(void)
{
	rtc_send(0x08);
	return rtc_read();
}

uint32_t RtcGetYear(void)
{
	rtc_send(0x09);
	return rtc_read();
}

char *RtcTranslateMonth(uint32_t month)
{
	char *months[12] = {
		"January", "February", "March", "April",
		"May", "June", "July", "August", "September",
		"October", "November", "December" };
	return months[month - 1];
}

void PrintRtcTime(void)
{
	uint32_t sec, min, hour;
	sec = RtcGetSecond();
	min = RtcGetMinute();
	hour = RtcGetHour();

	printf("Current time is: %i:%i:%i\n", hour, min, sec);
}

void PrintRtcDate(void)
{
		uint32_t day, mon, year;
		day = RtcGetDay();
		mon = RtcGetMonth();
		year = RtcGetYear() + 2000;

		printf("Current date is: %s %i, %i\n", RtcTranslateMonth(mon), day, year);
}

// taken from Ominos
uint64_t RtcEpochTime(void)
{
	uint64_t time;

	uint32_t sec = RtcGetSecond();
	uint32_t min = RtcGetMinute();
	uint32_t hour = RtcGetHour();
	uint32_t day = RtcGetDay();
	uint32_t mon = RtcGetMonth();
	uint32_t year = RtcGetYear();

	uint32_t days;
	int i;

	uint32_t months[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 304, 334};

	days = day;

	if (year < 70) year += 100;	
	year += 1900;

	for (i = 1970; i < year; i++)
	{
		if ((i % 4) != 0)
			days += 365;
		else if ((i % 400) != 0)
			days += 366;
		else if ((i % 100) != 0)
			days += 365;
		else 
			days += 366;
	}

	days += months[mon-1];
	days++;

	time = days * 24;	// hours
	time += hour;		// current hour
	time *= 60;			// minutes
	time += min;		// current minute
	time *= 60;			// seconds
	time += sec;		// current second
	return time;
}



