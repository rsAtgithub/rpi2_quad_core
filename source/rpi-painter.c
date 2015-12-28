/*
    Copyright (c) 2015-2016, Rushikesh Shingnapurkar
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

*/


#include "rpi-mailbox.h"
#include "rpi-painter.h"

#define __wfi() __asm__ __volatile__ ("wfi" : : : "memory")

// stack for secondary CPUs
unsigned int cpu1_stack[1000];
unsigned int cpu2_stack[1000];
unsigned int cpu3_stack[1000];

// storage for xy markers for 4 cpus.
// e.g. xyMarkers[CPU_0][XS] : starting X
//      xyMarkers[CPU_0][YS] : starting Y
//      xyMarkers[CPU_0][XE] : ending X
//      xyMarkers[CPU_0][YE] : ending Y
unsigned int xyMarkers[4][4];
unsigned char *gFb;
int gBpp;
int gPitch;

int gShift;

volatile unsigned int task_done[4];

void trigger1(void){
    volatile unsigned int* ptr = (volatile unsigned int*)CORE_1_MAILBOX_3_SET;
    *ptr = _trigger_cpu_1;
}

void trigger2(void){
    volatile unsigned int* ptr = (volatile unsigned int*)CORE_2_MAILBOX_3_SET;
    *ptr = _trigger_cpu_2;
}

void trigger3(void){
    volatile unsigned int* ptr = (volatile unsigned int*)CORE_3_MAILBOX_3_SET;
    *ptr = _trigger_cpu_3;
}

void fill_task(void){
    unsigned int x, y;
    int pixel_offset;
    unsigned short color;
    int r, g, b;
    volatile unsigned char* fb = gFb;
    int bpp = gBpp;
    int pitch = gPitch;
    int area;

    while(1){

        int cpuid = _get_cpu_id();
        area = (gShift + cpuid) % 4;
        switch(cpuid){
            case 0:     // RED
                r = COLOR_SET;
                g = 0;
                b = 0;
                break;
            case 1:     // GREEN
                r = 0;
                g = COLOR_SET;
                b = 0;
                break;
            case 2:     // BLUE
                r = 0;
                g = 0;
                b = COLOR_SET;
                break;
            case 3:     // YELLOW
                r = COLOR_SET;
                g = COLOR_SET;
                b = 0;
                break;
            }

        color = ( (r >> 3) << 11 ) | ( ( g >> 2 ) << 5 ) | ( b >> 3 );

        for(y = xyMarkers[area][YS]; y < xyMarkers[area][YE]; y++){
            for(x = xyMarkers[area][XS]; x < xyMarkers[area][XE]; x++){
                pixel_offset = ( x * ( bpp >> 3 ) ) + ( y * pitch );
                *(unsigned short*)&fb[pixel_offset] = color;
            }
        }

        if(cpuid == 0){
            return;
        }

        if(cpuid == 1){
            __wfi();
            continue;
        }

        task_done[cpuid] = 1;

        do{
        }while(task_done[cpuid] == 1);

    }

}

// interrupt setup for core1
void __start_core_1(void){
    volatile unsigned int* core_1_mailbox_interrupt_control = (volatile unsigned int*)CORE1_MAILBOXES_INTERRUPT_CONTROL;
    volatile unsigned int* core_1_mailbox_0_clr = (volatile unsigned int*)CORE_1_MAILBOX_0_RD_CLR;

    *core_1_mailbox_0_clr = 0;

    *core_1_mailbox_interrupt_control = 0x01;

    fill_task();
}

void revoke_core_1(void){
    volatile unsigned int* core_1_mailbox_0_set = (volatile unsigned int*)CORE_1_MAILBOX_0_SET;
    *core_1_mailbox_0_set = 1;
}


void clrscr(volatile unsigned char* fb, int height, int width, int bpp, int pitch){
    unsigned int x, y;
    int pixel_offset;
    for(y = 0; y < height; y++){
        for(x = 0; x < width; x++){
            pixel_offset = ( x * ( bpp >> 3 ) ) + ( y * pitch );
            *(unsigned short*)&fb[pixel_offset] = WHITE_16BIT;
        }
    }
}

void drawCross(volatile unsigned char* fb, int height, int width, int bpp, int pitch){
    unsigned int x, y;
    unsigned int mid_height = height / 2;
    unsigned int mid_width  = width  / 2;
    unsigned int h_line_start = mid_height - (CROSS_HEIGHT / 2);
    unsigned int v_line_start = mid_width - (CROSS_WIDTH / 2);
    int pixel_offset;

    clrscr(fb, height, width, bpp, pitch);

    // first draw h line
    for(y = h_line_start; y < h_line_start + CROSS_HEIGHT; y++){
        for(x = 0; x < width; x++){
            pixel_offset = ( x * ( bpp >> 3 ) ) + ( y * pitch );
            *(unsigned short*)&fb[pixel_offset] = BLACK_16BIT;
        }
    }

    // Now h line
    for(x = v_line_start; x < v_line_start + CROSS_WIDTH; x++){
        for(y = 0; y < height; y++){
            pixel_offset = ( x * ( bpp >> 3 ) ) + ( y * pitch );
            *(unsigned short*)&fb[pixel_offset] = BLACK_16BIT;
        }
    }


}

/** Main function - we'll never return from here */
void painter_main( volatile unsigned char* fb, unsigned int width, unsigned int height, unsigned int bpp, unsigned int pitch )
{
    // Top Left
    xyMarkers[CPU_0][XS] = 0;
    xyMarkers[CPU_0][YS] = 0;
    xyMarkers[CPU_0][XE] = (width / 2)  - (CROSS_WIDTH / 2);
    xyMarkers[CPU_0][YE] = (height / 2) - (CROSS_HEIGHT / 2);

    // Top Right
    xyMarkers[CPU_1][XS] = (width / 2)  + (CROSS_WIDTH / 2);
    xyMarkers[CPU_1][YS] = 0;
    xyMarkers[CPU_1][XE] = width;
    xyMarkers[CPU_1][YE] = (height / 2) - (CROSS_HEIGHT / 2);

    // Bottom Right
    xyMarkers[CPU_2][XS] = (width / 2)  + (CROSS_WIDTH / 2);
    xyMarkers[CPU_2][YS] = (height / 2) + (CROSS_HEIGHT / 2);
    xyMarkers[CPU_2][XE] = width;
    xyMarkers[CPU_2][YE] = height;

    // Bottom Left
    xyMarkers[CPU_3][XS] = 0;
    xyMarkers[CPU_3][YS] = (height / 2) + (CROSS_HEIGHT / 2);
    xyMarkers[CPU_3][XE] = (width / 2)  - (CROSS_WIDTH / 2);
    xyMarkers[CPU_3][YE] = height;

    // Draw cross to divide 4 CPUs
    drawCross(fb, height, width, bpp, pitch);

    gFb = fb;
    gBpp = bpp;
    gPitch = pitch;

    gShift = 0;

    trigger1 ();
    trigger2 ();
    trigger3 ();

    // Infinite loop for our main.
    while(1){
        fill_task();
        RPI_WaitMicroSeconds(2000000);
        /*do{
            }while(task_done[1] != 1);*/
        do{
            }while(task_done[2] != 1);
        do{
            }while(task_done[3] != 1);
        gShift++;
        /*task_done[1] = 0;*/
        task_done[2] = 0;
        task_done[3] = 0;
        revoke_core_1();
    }
}
