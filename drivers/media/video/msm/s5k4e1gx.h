/*
Copyright (c) 2010, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
 * Neither the name of Code Aurora Forum, Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


 */

#ifndef CAMSENSOR_S5K4E1GX
#define CAMSENSOR_S5K4E1GX

#include <mach/board.h>
extern struct s5k4e1_reg s5k4e1_regs;

struct s5k4e1gx_i2c_reg_conf
{
    unsigned short waddr;
    unsigned char  bdata;
};

struct s5k4e1_reg
{
    struct s5k4e1gx_i2c_reg_conf *reg_mipi;
    unsigned short                reg_mipi_size;
    struct s5k4e1gx_i2c_reg_conf *rec_settings;
    unsigned short                rec_size;
    struct s5k4e1gx_i2c_reg_conf *reg_pll_p;
    unsigned short                reg_pll_p_size;
    struct s5k4e1gx_i2c_reg_conf *reg_pll_s;
    unsigned short                reg_pll_s_size;
    struct s5k4e1gx_i2c_reg_conf *reg_prev;
    unsigned short                reg_prev_size;
    struct s5k4e1gx_i2c_reg_conf *reg_snap;
    unsigned short                reg_snap_size;
};

#endif /* CAMSENSOR_S5K4E1GX */
