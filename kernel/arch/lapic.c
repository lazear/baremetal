/*
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
#include <acpi.h>
#include <lapic.h>



/*
The local APIC can be enabled or disabled in either of two ways:
1. Using the APIC global enable/disable flag in the IA32_APIC_BASE MSR (MSR address 1BH; see Figure 10-5):
	— When IA32_APIC_BASE[11] is 0, the processor is functionally equivalent to an IA-32 processor without an
	on-chip APIC. The CPUID feature flag for the APIC (see Section 10.4.2, “Presence of the Local APIC”) is also
	set to 0.
	— When IA32_APIC_BASE[11] is set to 0, processor APICs based on the 3-wire APIC bus cannot be generally
	re-enabled until a system hardware reset. The 3-wire bus loses track of arbitration that would be necessary
	for complete re-enabling. Certain APIC functionality can be enabled (for example: performance and
	thermal monitoring interrupt generation).
	— For processors that use Front Side Bus (FSB) delivery of interrupts, software may disable or enable the
	APIC by setting and resetting IA32_APIC_BASE[11]. A hardware reset is not required to re-start APIC
	functionality, if software guarantees no interrupt will be sent to the APIC as IA32_APIC_BASE[11] is
	cleared.
— When IA32_APIC_BASE[11] is set to 0, prior initialization to the APIC may be lost and the APIC may return
to the state described in Section 10.4.7.1, “Local APIC State After Power-Up or Reset.”
2. Using the APIC software enable/disable flag in the spurious-interrupt vector register (see Figure 10-23):
	— If IA32_APIC_BASE[11] is 1, software can temporarily disable a local APIC at any time by clearing the APIC
	software enable/disable flag in the spurious-interrupt vector register (see Figure 10-23). The state of the
	local APIC when in this software-disabled state is described in Section 10.4.7.2, “Local APIC State After It
	Has Been Software Disabled.”
	— When the local APIC is in the software-disabled state, it can be re-enabled at any time by setting the APIC
	software enable/disable flag to 1


We're going for Method #2
*/



/* Initilize the local advanced programmable interrupt chip */
void lapic_init() {

}