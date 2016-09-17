/*
cpuid.c
===============================================================================
MIT License
Copyright (c) 2007-2016 Michael Lazear

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
*/

#include <types.h>

#define cpuid(in, a, b, c, d) \
	asm volatile("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

char *Intel[] = {
	"Brand ID Not Supported.", 
	"Intel(R) Celeron(R) processor", 
	"Intel(R) Pentium(R) III processor", 
	"Intel(R) Pentium(R) III Xeon(R) processor", 
	"Intel(R) Pentium(R) III processor", 
	"Reserved", 
	"Mobile Intel(R) Pentium(R) III processor-M", 
	"Mobile Intel(R) Celeron(R) processor", 
	"Intel(R) Pentium(R) 4 processor", 
	"Intel(R) Pentium(R) 4 processor", 
	"Intel(R) Celeron(R) processor", 
	"Intel(R) Xeon(R) Processor", 
	"Intel(R) Xeon(R) processor MP", 
	"Reserved", 
	"Mobile Intel(R) Pentium(R) 4 processor-M", 
	"Mobile Intel(R) Pentium(R) Celeron(R) processor", 
	"Reserved", 
	"Mobile Genuine Intel(R) processor", 
	"Intel(R) Celeron(R) M processor", 
	"Mobile Intel(R) Celeron(R) processor", 
	"Intel(R) Celeron(R) processor", 
	"Mobile Geniune Intel(R) processor", 
	"Intel(R) Pentium(R) M processor", 
	"Mobile Intel(R) Celeron(R) processor"
};

char *Intel_Other[] = {
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Celeron(R) processor", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Xeon(R) processor MP", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Xeon(R) processor", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved"
};

void cpuid_vendor()
{
	int i;
	uint32_t unused, ebx, ecx, edx;
	cpuid(0, unused, ebx, ecx, edx);
	char vendor[13];
	vendor[12] = '\0';
	for (i = 0; i < 4; i++)
	{
		vendor[i]		= ebx >> (8 * i);
		vendor[i + 4]	= edx >> (8 * i);
		vendor[i + 8]	= ecx >> (8 * i);
	}
	printf("%s\n", vendor);
}

void cpuid_psn(void)
{
	uint32_t unused, eax, ecx, edx;
	cpuid(1, eax, unused, unused, unused);	// signature
	cpuid(3, unused, unused, ecx, edx);
	printf("%w %w %w %w %w %w\n", 
			(eax >> 16), (eax & 0xFFFF),
			(ecx >> 16), (ecx & 0xFFFF),
			(edx >> 16), (edx & 0xFFFF));
}

void cpuid_features(void)
{
	uint32_t unused, edx;
	cpuid(1, unused, unused, unused, edx);
	printf("CPU Features (0x%X):\n", edx);
	if (edx & 0x00000001)	printf("\tFloating Point Unit\n");
	if (edx & 0x00000002)	printf("\tVirtual Mode Extensions\n");
	if (edx & 0x00000040)	printf("\tPhysical Address Extensions\n");
	if (edx & 0x00000200)	printf("\tOn-chip APIC\n");
	if (edx & 0x00040000)	cpuid_psn();
	if (edx & 0x40000000)	printf("\tIA-64 CPU\n");
	cpuid(0x80000001, unused, unused, unused, edx);
	printf("Extended Features (0x%X):\n", edx);
	if (edx & (1<<29))		printf("\tAMD64 Compliant\n");

}

void cpuid_detect(void)
{
	uint32_t eax, ebx, ecx, edx;
	cpuid(0, eax, ebx, ecx, edx);
	cpuid_vendor();
	switch (ebx)
	{
	case 0x756E6547:
		cpuid_intel();
		cpuid_features();
		break;
	case 0x68747541:
		break;
	default:
		printf("Unknown x86 CPU (0x%X) Detected\n", ebx);
		break;
	}
}

void cpuid_intel(void)
{
	uint32_t eax, ebx, ecx, edx, max_eax, unused, signature;
	int model, family, type, brand, stepping, reserved;
	int extended_family = -1;

	cpuid(1, eax, ebx, unused, unused);
	model		= (eax >> 4) & 0xF;
	family		= (eax >> 8) & 0xF;
	type		= (eax >> 12) & 0x3;
	brand		= (ebx & 0xFF);
	stepping	= (eax & 0xF);
	reserved	= (eax >> 14);
	signature	= (eax);

	switch (type)
	{
	case 0:
		printf("Original OEM ");
		break;
	case 1:
		printf("Overdrive ");
		break;
	case 2:
		printf("Dual Capable ");
		break;
	case 3:
		printf("Reserved ");
		break;
	}
	switch (family)
	{
	case 3:
		printf("i386\n");
		break;
	case 4:
		printf("i486\n");
		break;
	case 5:
		printf("Pentium\n");
		break;
	case 6:
		printf("Pentium Pro\n");
		break;
	case 15:
		printf("Pentium 4 ");
		break;
	}
	if(family == 15) 
	{
		extended_family = (eax >> 20) & 0xff;
		printf("Extended family %d\n", extended_family);
	}
	if (family != 15) 
	{
		printf("Model ");
		switch(family) 
		{
		case 3:
			break;
		case 4:
			switch(model)
			{
			case 0:
			case 1:
				printf("DX");
				break;
			case 2:
				printf("SX");
				break;
			case 3:
				printf("487/DX2");
				break;
			case 4:
				printf("SL");
				break;
			case 5:
				printf("SX2");
				break;
			case 7:
				printf("Write-back enhanced DX2");
				break;
			case 8:
				printf("DX4");
				break;
			}
			break;
		case 5:
				switch(model) 
				{
				case 1:
					printf("60/66");
					break;
				case 2:
					printf("75-200");
					break;
				case 3:
					printf("for 486 system");
					break;
				case 4:
					printf("MMX");
					break;
				}
			break;
		case 6:
			switch(model) 
			{
			case 1:
				printf("Pentium Pro");
				break;
			case 3:
				printf("Pentium II Model 3");
				break;
			case 5:
				printf("Pentium II Model 5/Xeon/Celeron");
				break;
			case 6:
				printf("Celeron");
				break;
			case 7:
				printf("Pentium III/Pentium III Xeon - external L2 cache");
				break;
			case 8:
				printf("Pentium III/Pentium III Xeon - internal L2 cache");
				break;
			}
			break;
		}
	}

	cpuid(0x80000000, max_eax, unused, unused, unused);
	if(brand > 0) 
	{
		printf("Brand %d - ", brand);
		if(brand < 0x18) 
		{
			if(signature == 0x000006B1 || signature == 0x00000F13) 
			{
				printf("%s\n", Intel_Other[brand]);
			} else {
				printf("%s\n", Intel[brand]);
			}
		} else {
			printf("Reserved\n");
		}
	}
	printf("Stepping: %d Reserved: %d\n", stepping, reserved);
}
