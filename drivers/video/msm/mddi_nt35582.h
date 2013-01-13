/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*creat by lijuan 00152865 2010/03/30*/
#ifndef MDDI_MDDI_NT35582_H
#define MDDI_MDDI_NT35582_H

#if 0
typedef struct {
	uint32 reg;
	uint32 val;
} s_reg_val_pair_t;

static s_reg_val_pair_t 
s_seq_init_setup[]={
	{0,	30},
	{0xC100,0x40},
	{0xC200,0x21},
	{0xC202,0x02},
	{0xB600,0x30},
	{0xB602,0x30},
	{0xC000,0x86},
	{0xC001,0x00},
	{0xC002,0x86},
	{0xC003,0x00},
	{0xC700,0x8D},
	{0xE000,0X0E},
	{0xE001,0X20},
	{0xE002,0X28},
	{0xE003,0X39},
	{0xE004,0X1D},
	{0xE005,0X30},
	{0xE006,0X61},
	{0xE007,0X3F},
	{0xE008,0X1F},
	{0xE009,0X28},
	{0xE00A,0X89},
	{0xE00B,0X18},
	{0xE00C,0X39},
	{0xE00D,0X56},
	{0xE00E,0X78},
	{0xE00F,0X89},
	{0xE010,0X32},
	{0xE011,0X4D},
	{0xE100,0X0E},
	{0xE101,0X20},
	{0xE102,0X28},
	{0xE103,0X39},
	{0xE104,0X1D},
	{0xE105,0X30},
	{0xE106,0X61},
	{0xE107,0X3F},
	{0xE108,0X1F},
	{0xE109,0X28},
	{0xE10A,0X89},
	{0xE10B,0X18},
	{0xE10C,0X39},
	{0xE10D,0X56},
	{0xE10E,0X78},
	{0xE10F,0X89},
	{0xE110,0X32},
	{0xE111,0X4D},
	{0xE200,0X0E},
	{0xE201,0X20},
	{0xE202,0X28},
	{0xE203,0X39},
	{0xE204,0X1D},
	{0xE205,0X30},
	{0xE206,0X61},
	{0xE207,0X3F},
	{0xE208,0X1F},
	{0xE209,0X28},
	{0xE20A,0X89},
	{0xE20B,0X18},
	{0xE20C,0X39},
	{0xE20D,0X56},
	{0xE20E,0X78},
	{0xE20F,0X89},
	{0xE210,0X32},
	{0xE211,0X4D},
	{0xE300,0X0E},
	{0xE301,0X20},
	{0xE302,0X28},
	{0xE303,0X39},
	{0xE304,0X1D},
	{0xE305,0X30},
	{0xE306,0X61},
	{0xE307,0X3F},
	{0xE308,0X1F},
	{0xE309,0X28},
	{0xE30A,0X89},
	{0xE30B,0X18},
	{0xE30C,0X39},
	{0xE30D,0X56},
	{0xE30E,0X78},
	{0xE30F,0X89},
	{0xE310,0X32},
	{0xE311,0X4D},
	{0xE400,0X0E},
	{0xE401,0X20},
	{0xE402,0X28},
	{0xE403,0X39},
	{0xE404,0X1D},
	{0xE405,0X30},
	{0xE406,0X61},
	{0xE407,0X3F},
	{0xE408,0X1F},
	{0xE409,0X28},
	{0xE40A,0X89},
	{0xE40B,0X18},
	{0xE40C,0X39},
	{0xE40D,0X56},
	{0xE40E,0X78},
	{0xE40F,0X89},
	{0xE410,0X32},
	{0xE411,0X4D},
	{0xE500,0X0E},
	{0xE501,0X20},
	{0xE502,0X28},
	{0xE503,0X39},
	{0xE504,0X1D},
	{0xE505,0X30},
	{0xE506,0X61},
	{0xE507,0X3F},
	{0xE508,0X1F},
	{0xE509,0X28},
	{0xE50A,0X89},
	{0xE50B,0X18},
	{0xE50C,0X39},
	{0xE50D,0X56},
	{0xE50E,0X78},
	{0xE50F,0X89},
	{0xE510,0X32},
	{0xE511,0X4D},
	{0x5301,0x04},/*add for the backlight */
	/*{0x3600,0xc0},*/
	{0x1100,0X00},
	{0,	150},
	{0x2900,0X00},
	{0,	150},

};
#endif

#endif


