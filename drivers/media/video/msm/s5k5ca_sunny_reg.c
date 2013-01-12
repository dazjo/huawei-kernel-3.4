
/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "s5k5ca.h"
#define S5K5CA_ARRAY_SIZE_2(x) (sizeof(x) / sizeof(x[0]))

struct s5k5ca_i2c_reg_conf s5k5ca_init_reg_config_sunny_settings[] =
{
    // Start T&P part
    {0x0028, 0x7000},
    {0x002A, 0x2CF8},
    {0x0F12, 0xB510},
    {0x0F12, 0x4827},
    {0x0F12, 0x21C0},
    {0x0F12, 0x8041},
    {0x0F12, 0x4825},
    {0x0F12, 0x4A26},
    {0x0F12, 0x3020},
    {0x0F12, 0x8382},
    {0x0F12, 0x1D12},
    {0x0F12, 0x83C2},
    {0x0F12, 0x4822},
    {0x0F12, 0x3040},
    {0x0F12, 0x8041},
    {0x0F12, 0x4821},
    {0x0F12, 0x4922},
    {0x0F12, 0x3060},
    {0x0F12, 0x8381},
    {0x0F12, 0x1D09},
    {0x0F12, 0x83C1},
    {0x0F12, 0x4821},
    {0x0F12, 0x491D},
    {0x0F12, 0x8802},
    {0x0F12, 0x3980},
    {0x0F12, 0x804A},
    {0x0F12, 0x8842},
    {0x0F12, 0x808A},
    {0x0F12, 0x8882},
    {0x0F12, 0x80CA},
    {0x0F12, 0x88C2},
    {0x0F12, 0x810A},
    {0x0F12, 0x8902},
    {0x0F12, 0x491C},
    {0x0F12, 0x80CA},
    {0x0F12, 0x8942},
    {0x0F12, 0x814A},
    {0x0F12, 0x8982},
    {0x0F12, 0x830A},
    {0x0F12, 0x89C2},
    {0x0F12, 0x834A},
    {0x0F12, 0x8A00},
    {0x0F12, 0x4918},
    {0x0F12, 0x8188},
    {0x0F12, 0x4918},
    {0x0F12, 0x4819},
    {0x0F12, 0xF000},
    {0x0F12, 0xFA0E},
    {0x0F12, 0x4918},
    {0x0F12, 0x4819},
    {0x0F12, 0x6341},
    {0x0F12, 0x4919},
    {0x0F12, 0x4819},
    {0x0F12, 0xF000},
    {0x0F12, 0xFA07},
    {0x0F12, 0x4816},
    {0x0F12, 0x4918},
    {0x0F12, 0x3840},
    {0x0F12, 0x62C1},
    {0x0F12, 0x4918},
    {0x0F12, 0x3880},
    {0x0F12, 0x63C1},
    {0x0F12, 0x4917},
    {0x0F12, 0x6301},
    {0x0F12, 0x4917},
    {0x0F12, 0x3040},
    {0x0F12, 0x6181},
    {0x0F12, 0x4917},
    {0x0F12, 0x4817},
    {0x0F12, 0xF000},
    {0x0F12, 0xF9F7},
    {0x0F12, 0x4917},
    {0x0F12, 0x4817},
    {0x0F12, 0xF000},
    {0x0F12, 0xF9F3},
    {0x0F12, 0x4917},
    {0x0F12, 0x4817},
    {0x0F12, 0xF000},
    {0x0F12, 0xF9EF},
    {0x0F12, 0xBC10},
    {0x0F12, 0xBC08},
    {0x0F12, 0x4718},
    {0x0F12, 0x1100},
    {0x0F12, 0xD000},
    {0x0F12, 0x267C},
    {0x0F12, 0x0000},
    {0x0F12, 0x2CE8},
    {0x0F12, 0x0000},
    {0x0F12, 0x3274},
    {0x0F12, 0x7000},
    {0x0F12, 0xF400},
    {0x0F12, 0xD000},
    {0x0F12, 0xF520},
    {0x0F12, 0xD000},
    {0x0F12, 0x2DF1},
    {0x0F12, 0x7000},
    {0x0F12, 0x89A9},
    {0x0F12, 0x0000},
    {0x0F12, 0x2E43},
    {0x0F12, 0x7000},
    {0x0F12, 0x0140},
    {0x0F12, 0x7000},
    {0x0F12, 0x2E7D},
    {0x0F12, 0x7000},
    {0x0F12, 0xB4F7},
    {0x0F12, 0x0000},
    {0x0F12, 0x2F07},
    {0x0F12, 0x7000},
    {0x0F12, 0x2F2B},
    {0x0F12, 0x7000},
    {0x0F12, 0x2FD1},
    {0x0F12, 0x7000},
    {0x0F12, 0x2FE5},
    {0x0F12, 0x7000},
    {0x0F12, 0x2FB9},
    {0x0F12, 0x7000},
    {0x0F12, 0x013D},
    {0x0F12, 0x0001},
    {0x0F12, 0x306B},
    {0x0F12, 0x7000},
    {0x0F12, 0x5823},
    {0x0F12, 0x0000},
    {0x0F12, 0x30B9},
    {0x0F12, 0x7000},
    {0x0F12, 0xD789},
    {0x0F12, 0x0000},
    {0x0F12, 0xB570},
    {0x0F12, 0x6804},
    {0x0F12, 0x6845},
    {0x0F12, 0x6881},
    {0x0F12, 0x6840},
    {0x0F12, 0x2900},
    {0x0F12, 0x6880},
    {0x0F12, 0xD007},
    {0x0F12, 0x49C3},
    {0x0F12, 0x8949},
    {0x0F12, 0x084A},
    {0x0F12, 0x1880},
    {0x0F12, 0xF000},
    {0x0F12, 0xF9BA},
    {0x0F12, 0x80A0},
    {0x0F12, 0xE000},
    {0x0F12, 0x80A0},
    {0x0F12, 0x88A0},
    {0x0F12, 0x2800},
    {0x0F12, 0xD010},
    {0x0F12, 0x68A9},
    {0x0F12, 0x6828},
    {0x0F12, 0x084A},
    {0x0F12, 0x1880},
    {0x0F12, 0xF000},
    {0x0F12, 0xF9AE},
    {0x0F12, 0x8020},
    {0x0F12, 0x1D2D},
    {0x0F12, 0xCD03},
    {0x0F12, 0x084A},
    {0x0F12, 0x1880},
    {0x0F12, 0xF000},
    {0x0F12, 0xF9A7},
    {0x0F12, 0x8060},
    {0x0F12, 0xBC70},
    {0x0F12, 0xBC08},
    {0x0F12, 0x4718},
    {0x0F12, 0x2000},
    {0x0F12, 0x8060},
    {0x0F12, 0x8020},
    {0x0F12, 0xE7F8},
    {0x0F12, 0xB510},
    {0x0F12, 0xF000},
    {0x0F12, 0xF9A2},
    {0x0F12, 0x48B2},
    {0x0F12, 0x8A40},
    {0x0F12, 0x2800},
    {0x0F12, 0xD00C},
    {0x0F12, 0x48B1},
    {0x0F12, 0x49B2},
    {0x0F12, 0x8800},
    {0x0F12, 0x4AB2},
    {0x0F12, 0x2805},
    {0x0F12, 0xD003},
    {0x0F12, 0x4BB1},
    {0x0F12, 0x795B},
    {0x0F12, 0x2B00},
    {0x0F12, 0xD005},
    {0x0F12, 0x2001},
    {0x0F12, 0x8008},
    {0x0F12, 0x8010},
    {0x0F12, 0xBC10},
    {0x0F12, 0xBC08},
    {0x0F12, 0x4718},
    {0x0F12, 0x2800},
    {0x0F12, 0xD1FA},
    {0x0F12, 0x2000},
    {0x0F12, 0x8008},
    {0x0F12, 0x8010},
    {0x0F12, 0xE7F6},
    {0x0F12, 0xB5F8},
    {0x0F12, 0x2407},
    {0x0F12, 0x2C06},
    {0x0F12, 0xD035},
    {0x0F12, 0x2C07},
    {0x0F12, 0xD033},
    {0x0F12, 0x48A3},
    {0x0F12, 0x8BC1},
    {0x0F12, 0x2900},
    {0x0F12, 0xD02A},
    {0x0F12, 0x00A2},
    {0x0F12, 0x1815},
    {0x0F12, 0x4AA4},
    {0x0F12, 0x6DEE},
    {0x0F12, 0x8A92},
    {0x0F12, 0x4296},
    {0x0F12, 0xD923},
    {0x0F12, 0x0028},
    {0x0F12, 0x3080},
    {0x0F12, 0x0007},
    {0x0F12, 0x69C0},
    {0x0F12, 0xF000},
    {0x0F12, 0xF96B},
    {0x0F12, 0x1C71},
    {0x0F12, 0x0280},
    {0x0F12, 0xF000},
    {0x0F12, 0xF967},
    {0x0F12, 0x0006},
    {0x0F12, 0x4898},
    {0x0F12, 0x0061},
    {0x0F12, 0x1808},
    {0x0F12, 0x8D80},
    {0x0F12, 0x0A01},
    {0x0F12, 0x0600},
    {0x0F12, 0x0E00},
    {0x0F12, 0x1A08},
    {0x0F12, 0xF000},
    {0x0F12, 0xF96A},
    {0x0F12, 0x0002},
    {0x0F12, 0x6DE9},
    {0x0F12, 0x6FE8},
    {0x0F12, 0x1A08},
    {0x0F12, 0x4351},
    {0x0F12, 0x0300},
    {0x0F12, 0x1C49},
    {0x0F12, 0xF000},
    {0x0F12, 0xF953},
    {0x0F12, 0x0401},
    {0x0F12, 0x0430},
    {0x0F12, 0x0C00},
    {0x0F12, 0x4301},
    {0x0F12, 0x61F9},
    {0x0F12, 0xE004},
    {0x0F12, 0x00A2},
    {0x0F12, 0x4990},
    {0x0F12, 0x1810},
    {0x0F12, 0x3080},
    {0x0F12, 0x61C1},
    {0x0F12, 0x1E64},
    {0x0F12, 0xD2C5},
    {0x0F12, 0x2006},
    {0x0F12, 0xF000},
    {0x0F12, 0xF959},
    {0x0F12, 0x2007},
    {0x0F12, 0xF000},
    {0x0F12, 0xF956},
    {0x0F12, 0xBCF8},
    {0x0F12, 0xBC08},
    {0x0F12, 0x4718},
    {0x0F12, 0xB510},
    {0x0F12, 0xF000},
    {0x0F12, 0xF958},
    {0x0F12, 0x2800},
    {0x0F12, 0xD00A},
    {0x0F12, 0x4881},
    {0x0F12, 0x8B81},
    {0x0F12, 0x0089},
    {0x0F12, 0x1808},
    {0x0F12, 0x6DC1},
    {0x0F12, 0x4883},
    {0x0F12, 0x8A80},
    {0x0F12, 0x4281},
    {0x0F12, 0xD901},
    {0x0F12, 0x2001},
    {0x0F12, 0xE7A1},
    {0x0F12, 0x2000},
    {0x0F12, 0xE79F},
    {0x0F12, 0xB5F8},
    {0x0F12, 0x0004},
    {0x0F12, 0x4F80},
    {0x0F12, 0x227D},
    {0x0F12, 0x8938},
    {0x0F12, 0x0152},
    {0x0F12, 0x4342},
    {0x0F12, 0x487E},
    {0x0F12, 0x9000},
    {0x0F12, 0x8A01},
    {0x0F12, 0x0848},
    {0x0F12, 0x1810},
    {0x0F12, 0xF000},
    {0x0F12, 0xF91D},
    {0x0F12, 0x210F},
    {0x0F12, 0xF000},
    {0x0F12, 0xF940},
    {0x0F12, 0x497A},
    {0x0F12, 0x8C49},
    {0x0F12, 0x090E},
    {0x0F12, 0x0136},
    {0x0F12, 0x4306},
    {0x0F12, 0x4979},
    {0x0F12, 0x2C00},
    {0x0F12, 0xD003},
    {0x0F12, 0x2001},
    {0x0F12, 0x0240},
    {0x0F12, 0x4330},
    {0x0F12, 0x8108},
    {0x0F12, 0x4876},
    {0x0F12, 0x2C00},
    {0x0F12, 0x8D00},
    {0x0F12, 0xD001},
    {0x0F12, 0x2501},
    {0x0F12, 0xE000},
    {0x0F12, 0x2500},
    {0x0F12, 0x4972},
    {0x0F12, 0x4328},
    {0x0F12, 0x8008},
    {0x0F12, 0x207D},
    {0x0F12, 0x00C0},
    {0x0F12, 0xF000},
    {0x0F12, 0xF92E},
    {0x0F12, 0x2C00},
    {0x0F12, 0x496E},
    {0x0F12, 0x0328},
    {0x0F12, 0x4330},
    {0x0F12, 0x8108},
    {0x0F12, 0x88F8},
    {0x0F12, 0x2C00},
    {0x0F12, 0x01AA},
    {0x0F12, 0x4310},
    {0x0F12, 0x8088},
    {0x0F12, 0x9800},
    {0x0F12, 0x8A01},
    {0x0F12, 0x486A},
    {0x0F12, 0xF000},
    {0x0F12, 0xF8F1},
    {0x0F12, 0x496A},
    {0x0F12, 0x8809},
    {0x0F12, 0x4348},
    {0x0F12, 0x0400},
    {0x0F12, 0x0C00},
    {0x0F12, 0xF000},
    {0x0F12, 0xF918},
    {0x0F12, 0x0020},
    {0x0F12, 0xF000},
    {0x0F12, 0xF91D},
    {0x0F12, 0x4866},
    {0x0F12, 0x7004},
    {0x0F12, 0xE7A3},
    {0x0F12, 0xB510},
    {0x0F12, 0x0004},
    {0x0F12, 0xF000},
    {0x0F12, 0xF91E},
    {0x0F12, 0x6020},
    {0x0F12, 0x4963},
    {0x0F12, 0x8B49},
    {0x0F12, 0x0789},
    {0x0F12, 0xD001},
    {0x0F12, 0x0040},
    {0x0F12, 0x6020},
    {0x0F12, 0xE74C},
    {0x0F12, 0xB510},
    {0x0F12, 0xF000},
    {0x0F12, 0xF91B},
    {0x0F12, 0x485F},
    {0x0F12, 0x8880},
    {0x0F12, 0x0601},
    {0x0F12, 0x4854},
    {0x0F12, 0x1609},
    {0x0F12, 0x8141},
    {0x0F12, 0xE742},
    {0x0F12, 0xB5F8},
    {0x0F12, 0x000F},
    {0x0F12, 0x4C55},
    {0x0F12, 0x3420},
    {0x0F12, 0x2500},
    {0x0F12, 0x5765},
    {0x0F12, 0x0039},
    {0x0F12, 0xF000},
    {0x0F12, 0xF913},
    {0x0F12, 0x9000},
    {0x0F12, 0x2600},
    {0x0F12, 0x57A6},
    {0x0F12, 0x4C4C},
    {0x0F12, 0x42AE},
    {0x0F12, 0xD01B},
    {0x0F12, 0x4D54},
    {0x0F12, 0x8AE8},
    {0x0F12, 0x2800},
    {0x0F12, 0xD013},
    {0x0F12, 0x484D},
    {0x0F12, 0x8A01},
    {0x0F12, 0x8B80},
    {0x0F12, 0x4378},
    {0x0F12, 0xF000},
    {0x0F12, 0xF8B5},
    {0x0F12, 0x89A9},
    {0x0F12, 0x1A41},
    {0x0F12, 0x484E},
    {0x0F12, 0x3820},
    {0x0F12, 0x8AC0},
    {0x0F12, 0x4348},
    {0x0F12, 0x17C1},
    {0x0F12, 0x0D89},
    {0x0F12, 0x1808},
    {0x0F12, 0x1280},
    {0x0F12, 0x8961},
    {0x0F12, 0x1A08},
    {0x0F12, 0x8160},
    {0x0F12, 0xE003},
    {0x0F12, 0x88A8},
    {0x0F12, 0x0600},
    {0x0F12, 0x1600},
    {0x0F12, 0x8160},
    {0x0F12, 0x200A},
    {0x0F12, 0x5E20},
    {0x0F12, 0x42B0},
    {0x0F12, 0xD011},
    {0x0F12, 0xF000},
    {0x0F12, 0xF8AB},
    {0x0F12, 0x1D40},
    {0x0F12, 0x00C3},
    {0x0F12, 0x1A18},
    {0x0F12, 0x214B},
    {0x0F12, 0xF000},
    {0x0F12, 0xF897},
    {0x0F12, 0x211F},
    {0x0F12, 0xF000},
    {0x0F12, 0xF8BA},
    {0x0F12, 0x210A},
    {0x0F12, 0x5E61},
    {0x0F12, 0x0FC9},
    {0x0F12, 0x0149},
    {0x0F12, 0x4301},
    {0x0F12, 0x483D},
    {0x0F12, 0x81C1},
    {0x0F12, 0x9800},
    {0x0F12, 0xE74A},
    {0x0F12, 0xB5F1},
    {0x0F12, 0xB082},
    {0x0F12, 0x2500},
    {0x0F12, 0x483A},
    {0x0F12, 0x9001},
    {0x0F12, 0x2400},
    {0x0F12, 0x2028},
    {0x0F12, 0x4368},
    {0x0F12, 0x4A39},
    {0x0F12, 0x4925},
    {0x0F12, 0x1887},
    {0x0F12, 0x1840},
    {0x0F12, 0x9000},
    {0x0F12, 0x9800},
    {0x0F12, 0x0066},
    {0x0F12, 0x9A01},
    {0x0F12, 0x1980},
    {0x0F12, 0x218C},
    {0x0F12, 0x5A09},
    {0x0F12, 0x8A80},
    {0x0F12, 0x8812},
    {0x0F12, 0xF000},
    {0x0F12, 0xF8CA},
    {0x0F12, 0x53B8},
    {0x0F12, 0x1C64},
    {0x0F12, 0x2C14},
    {0x0F12, 0xDBF1},
    {0x0F12, 0x1C6D},
    {0x0F12, 0x2D03},
    {0x0F12, 0xDBE6},
    {0x0F12, 0x9802},
    {0x0F12, 0x6800},
    {0x0F12, 0x0600},
    {0x0F12, 0x0E00},
    {0x0F12, 0xF000},
    {0x0F12, 0xF8C5},
    {0x0F12, 0xBCFE},
    {0x0F12, 0xBC08},
    {0x0F12, 0x4718},
    {0x0F12, 0xB570},
    {0x0F12, 0x6805},
    {0x0F12, 0x2404},
    {0x0F12, 0xF000},
    {0x0F12, 0xF8C5},
    {0x0F12, 0x2800},
    {0x0F12, 0xD103},
    {0x0F12, 0xF000},
    {0x0F12, 0xF8C9},
    {0x0F12, 0x2800},
    {0x0F12, 0xD000},
    {0x0F12, 0x2400},
    {0x0F12, 0x3540},
    {0x0F12, 0x88E8},
    {0x0F12, 0x0500},
    {0x0F12, 0xD403},
    {0x0F12, 0x4822},
    {0x0F12, 0x89C0},
    {0x0F12, 0x2800},
    {0x0F12, 0xD002},
    {0x0F12, 0x2008},
    {0x0F12, 0x4304},
    {0x0F12, 0xE001},
    {0x0F12, 0x2010},
    {0x0F12, 0x4304},
    {0x0F12, 0x481F},
    {0x0F12, 0x8B80},
    {0x0F12, 0x0700},
    {0x0F12, 0x0F81},
    {0x0F12, 0x2001},
    {0x0F12, 0x2900},
    {0x0F12, 0xD000},
    {0x0F12, 0x4304},
    {0x0F12, 0x491C},
    {0x0F12, 0x8B0A},
    {0x0F12, 0x42A2},
    {0x0F12, 0xD004},
    {0x0F12, 0x0762},
    {0x0F12, 0xD502},
    {0x0F12, 0x4A19},
    {0x0F12, 0x3220},
    {0x0F12, 0x8110},
    {0x0F12, 0x830C},
    {0x0F12, 0xE691},
    {0x0F12, 0x0C3C},
    {0x0F12, 0x7000},
    {0x0F12, 0x3274},
    {0x0F12, 0x7000},
    {0x0F12, 0x26E8},
    {0x0F12, 0x7000},
    {0x0F12, 0x6100},
    {0x0F12, 0xD000},
    {0x0F12, 0x6500},
    {0x0F12, 0xD000},
    {0x0F12, 0x1A7C},
    {0x0F12, 0x7000},
    {0x0F12, 0x1120},
    {0x0F12, 0x7000},
    {0x0F12, 0xFFFF},
    {0x0F12, 0x0000},
    {0x0F12, 0x3374},
    {0x0F12, 0x7000},
    {0x0F12, 0x1D6C},
    {0x0F12, 0x7000},
    {0x0F12, 0x167C},
    {0x0F12, 0x7000},
    {0x0F12, 0xF400},
    {0x0F12, 0xD000},
    {0x0F12, 0x2C2C},
    {0x0F12, 0x7000},
    {0x0F12, 0x40A0},
    {0x0F12, 0x00DD},
    {0x0F12, 0xF520},
    {0x0F12, 0xD000},
    {0x0F12, 0x2C29},
    {0x0F12, 0x7000},
    {0x0F12, 0x1A54},
    {0x0F12, 0x7000},
    {0x0F12, 0x1564},
    {0x0F12, 0x7000},
    {0x0F12, 0xF2A0},
    {0x0F12, 0xD000},
    {0x0F12, 0x2440},
    {0x0F12, 0x7000},
    {0x0F12, 0x05A0},
    {0x0F12, 0x7000},
    {0x0F12, 0x2894},
    {0x0F12, 0x7000},
    {0x0F12, 0x1224},
    {0x0F12, 0x7000},
    {0x0F12, 0xB000},
    {0x0F12, 0xD000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0x1A3F},
    {0x0F12, 0x0001},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xF004},
    {0x0F12, 0xE51F},
    {0x0F12, 0x1F48},
    {0x0F12, 0x0001},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0x24BD},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0x36DD},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0xB4CF},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0xB5D7},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0x36ED},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0xF53F},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0xF5D9},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0x013D},
    {0x0F12, 0x0001},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0xF5C9},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0xFAA9},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0x3723},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0x5823},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0xD771},
    {0x0F12, 0x0000},
    {0x0F12, 0x4778},
    {0x0F12, 0x46C0},
    {0x0F12, 0xC000},
    {0x0F12, 0xE59F},
    {0x0F12, 0xFF1C},
    {0x0F12, 0xE12F},
    {0x0F12, 0xD75B},
    {0x0F12, 0x0000},
    {0x0F12, 0x8117},
    {0x0F12, 0x0000},

    // End T&P part

    // CIS/APS/Analog setting- 400LSBSYSCLK 45MHz
    {0x0028, 0x7000},
    {0x002A, 0x157A},
    {0x0F12, 0x0001},
    {0x002A, 0x1578},
    {0x0F12, 0x0001},
    {0x002A, 0x1576},
    {0x0F12, 0x0020},
    {0x002A, 0x1574},
    {0x0F12, 0x0006},
    {0x002A, 0x156E},
    {0x0F12, 0x0001}, // Slope calibration tolerance in units of 1/256
    {0x002A, 0x1568},
    {0x0F12, 0x00FC},

    //ADC control
    {0x002A, 0x155A},
    {0x0F12, 0x01CC}, //ADC SAT of 450mV for 10bit default in EVT1
    {0x002A, 0x157E},
    {0x0F12, 0x0C80}, // 3200 Max. Reset ramp DCLK counts (default 2048 0x800)
    {0x0F12, 0x0578}, // 1400 Max. Reset ramp DCLK counts for x3.5
    {0x002A, 0x157C},
    {0x0F12, 0x0190}, // 400 Reset ramp for x1 in DCLK counts
    {0x002A, 0x1570},
    {0x0F12, 0x00A0}, // 224 LSB
    {0x0F12, 0x0010}, // reset threshold
    {0x002A, 0x12C4},
    {0x0F12, 0x006A}, // 106 additional timing columns.
    {0x002A, 0x12C8},
    {0x0F12, 0x08AC}, //0834// 2100 ADC columns in normal mode including Hold & Latch    ***
    {0x0F12, 0x0050}, //0028// 40 addition of ADC columns in Y-ave mode (default 244 0x74) ***
    //WRITE #senHal_ForceModeType 0001 // Long exposure mode

    {0x002A, 0x1696},
    {0x0F12, 0x0000}, //0001// based on APS guidelines ****
    {0x0F12, 0x0000}, //0001// based on APS guidelines ****
    {0x0F12, 0x00C6}, // default. 1492 used for ADC dark characteristics
    {0x0F12, 0x00C6}, // default. 1492 used for ADC dark characteristics
    {0x002A, 0x1690},
    {0x0F12, 0x0001}, // when set double sampling is activated - requires different set of pointers
    {0x002A, 0x12B0},
    {0x0F12, 0x0055}, // comp and pixel bias control 0xF40E - default for EVT1
    {0x0F12, 0x005A}, // comp and pixel bias control 0xF40E for binning mode
    {0x002A, 0x337A},
    {0x0F12, 0x0006}, // [7] - is used for rest-only mode (EVT0 value is 0xD and HW 0x6)
    {0x0F12, 0x0068},
    {0x002A, 0x169E},
    {0x0F12, 0x0007}, //000D// [3:0]- specifies the target (default 7)- DCLK = 64MHz instead of 116MHz. ****
    {0x002A, 0x0BF6},
    {0x0F12, 0x0000},

    {0x002A, 0x327C},
    {0x0F12, 0x1000},
    {0x0F12, 0x6998},
    {0x0F12, 0x0078},
    {0x0F12, 0x04FE},
    {0x0F12, 0x8800},

    {0x002A, 0x3274},
    {0x0F12, 0x0155}, //set IO driving current 2mA for Gs500
    {0x0F12, 0x0155}, //set IO driving current
    {0x0F12, 0x1555}, //set IO driving current
    {0x0F12, 0x0555}, //set IO driving current
    {0x0028, 0x7000},
    {0x002A, 0x0572},
    {0x0F12, 0x0007}, //#skl_usConfigStbySettings // Enable T&P code after HW stby + skip ZI part on HW wakeup.

    {0x0028, 0x7000},
    {0x002A, 0x12D2},
    {0x0F12, 0x0003}, //0006 // #senHal_pContSenModesRegsArray[0][0]2 700012D2
    {0x0F12, 0x0003}, //0006 // #senHal_pContSenModesRegsArray[0][1]2 700012D4
    {0x0F12, 0x0003}, //0003 // #senHal_pContSenModesRegsArray[0][2]2 700012D6
    {0x0F12, 0x0003}, //0003 // #senHal_pContSenModesRegsArray[0][3]2 700012D8
    {0x0F12, 0x0884}, //0801 // #senHal_pContSenModesRegsArray[1][0]2 700012DA
    {0x0F12, 0x08CF}, //0829 // #senHal_pContSenModesRegsArray[1][1]2 700012DC
    {0x0F12, 0x0500}, //047D // #senHal_pContSenModesRegsArray[1][2]2 700012DE
    {0x0F12, 0x054B}, //04A5 // #senHal_pContSenModesRegsArray[1][3]2 700012E0
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[2][0]2 700012E2
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[2][1]2 700012E4
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[2][2]2 700012E6
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[2][3]2 700012E8
    {0x0F12, 0x0885}, //0802 // #senHal_pContSenModesRegsArray[3][0]2 700012EA
    {0x0F12, 0x0467}, //0415 // #senHal_pContSenModesRegsArray[3][1]2 700012EC
    {0x0F12, 0x0501}, //047E // #senHal_pContSenModesRegsArray[3][2]2 700012EE
    {0x0F12, 0x02A5}, //0253 // #senHal_pContSenModesRegsArray[3][3]2 700012F0
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[4][0]2 700012F2
    {0x0F12, 0x046A}, //0416 // #senHal_pContSenModesRegsArray[4][1]2 700012F4
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[4][2]2 700012F6
    {0x0F12, 0x02A8}, //0254 // #senHal_pContSenModesRegsArray[4][3]2 700012F8
    {0x0F12, 0x0885}, //0802 // #senHal_pContSenModesRegsArray[5][0]2 700012FA
    {0x0F12, 0x08D0}, //082A // #senHal_pContSenModesRegsArray[5][1]2 700012FC
    {0x0F12, 0x0501}, //047E // #senHal_pContSenModesRegsArray[5][2]2 700012FE
    {0x0F12, 0x054C}, //04A6 // #senHal_pContSenModesRegsArray[5][3]2 70001300
    {0x0F12, 0x0006}, //0010 // #senHal_pContSenModesRegsArray[6][0]2 70001302
    {0x0F12, 0x0020}, //0012 // #senHal_pContSenModesRegsArray[6][1]2 70001304
    {0x0F12, 0x0006}, //0006 // #senHal_pContSenModesRegsArray[6][2]2 70001306
    {0x0F12, 0x0020}, //000C // #senHal_pContSenModesRegsArray[6][3]2 70001308
    {0x0F12, 0x0881}, //07FE // #senHal_pContSenModesRegsArray[7][0]2 7000130A
    {0x0F12, 0x0463}, //0411 // #senHal_pContSenModesRegsArray[7][1]2 7000130C
    {0x0F12, 0x04FD}, //047A // #senHal_pContSenModesRegsArray[7][2]2 7000130E
    {0x0F12, 0x02A1}, //024F // #senHal_pContSenModesRegsArray[7][3]2 70001310
    {0x0F12, 0x0006}, //0010 // #senHal_pContSenModesRegsArray[8][0]2 70001312
    {0x0F12, 0x0489}, //0424 // #senHal_pContSenModesRegsArray[8][1]2 70001314
    {0x0F12, 0x0006}, //0006 // #senHal_pContSenModesRegsArray[8][2]2 70001316
    {0x0F12, 0x02C7}, //0262 // #senHal_pContSenModesRegsArray[8][3]2 70001318
    {0x0F12, 0x0881}, //07FE // #senHal_pContSenModesRegsArray[9][0]2 7000131A
    {0x0F12, 0x08CC}, //0826 // #senHal_pContSenModesRegsArray[9][1]2 7000131C
    {0x0F12, 0x04FD}, //047A // #senHal_pContSenModesRegsArray[9][2]2 7000131E
    {0x0F12, 0x0548}, //04A2 // #senHal_pContSenModesRegsArray[9][3]2 70001320
    {0x0F12, 0x03A2}, //036F // #senHal_pContSenModesRegsArray[10][0] 2 70001322
    {0x0F12, 0x01D3}, //01B7 // #senHal_pContSenModesRegsArray[10][1] 2 70001324
    {0x0F12, 0x01E0}, //01AD // #senHal_pContSenModesRegsArray[10][2] 2 70001326
    {0x0F12, 0x00F2}, //00D6 // #senHal_pContSenModesRegsArray[10][3] 2 70001328
    {0x0F12, 0x03F2}, //039B // #senHal_pContSenModesRegsArray[11][0] 2 7000132A
    {0x0F12, 0x0223}, //01E3 // #senHal_pContSenModesRegsArray[11][1] 2 7000132C
    {0x0F12, 0x0230}, //01D9 // #senHal_pContSenModesRegsArray[11][2] 2 7000132E
    {0x0F12, 0x0142}, //0102 // #senHal_pContSenModesRegsArray[11][3] 2 70001330
    {0x0F12, 0x03A2}, //036F // #senHal_pContSenModesRegsArray[12][0] 2 70001332
    {0x0F12, 0x063C}, //05CC // #senHal_pContSenModesRegsArray[12][1] 2 70001334
    {0x0F12, 0x01E0}, //01AD // #senHal_pContSenModesRegsArray[12][2] 2 70001336
    {0x0F12, 0x0399}, //0329 // #senHal_pContSenModesRegsArray[12][3] 2 70001338
    {0x0F12, 0x03F2}, //039B // #senHal_pContSenModesRegsArray[13][0] 2 7000133A
    {0x0F12, 0x068C}, //05F8 // #senHal_pContSenModesRegsArray[13][1] 2 7000133C
    {0x0F12, 0x0230}, //01D9 // #senHal_pContSenModesRegsArray[13][2] 2 7000133E
    {0x0F12, 0x03E9}, //0355 // #senHal_pContSenModesRegsArray[13][3] 2 70001340
    {0x0F12, 0x0002}, //0002 // #senHal_pContSenModesRegsArray[14][0] 2 70001342
    {0x0F12, 0x0002}, //0002 // #senHal_pContSenModesRegsArray[14][1] 2 70001344
    {0x0F12, 0x0002}, //0002 // #senHal_pContSenModesRegsArray[14][2] 2 70001346
    {0x0F12, 0x0002}, //0002 // #senHal_pContSenModesRegsArray[14][3] 2 70001348
    {0x0F12, 0x003C}, //0022 // #senHal_pContSenModesRegsArray[15][0] 2 7000134A
    {0x0F12, 0x003C}, //0020 // #senHal_pContSenModesRegsArray[15][1] 2 7000134C
    {0x0F12, 0x003C}, //0022 // #senHal_pContSenModesRegsArray[15][2] 2 7000134E
    {0x0F12, 0x003C}, //0020 // #senHal_pContSenModesRegsArray[15][3] 2 70001350
    {0x0F12, 0x01D3}, //01B9 // #senHal_pContSenModesRegsArray[16][0] 2 70001352
    {0x0F12, 0x01D3}, //01B7 // #senHal_pContSenModesRegsArray[16][1] 2 70001354
    {0x0F12, 0x00F2}, //00D8 // #senHal_pContSenModesRegsArray[16][2] 2 70001356
    {0x0F12, 0x00F2}, //00D6 // #senHal_pContSenModesRegsArray[16][3] 2 70001358
    {0x0F12, 0x020B}, //01D7 // #senHal_pContSenModesRegsArray[17][0] 2 7000135A
    {0x0F12, 0x024A}, //01F8 // #senHal_pContSenModesRegsArray[17][1] 2 7000135C
    {0x0F12, 0x012A}, //00F6 // #senHal_pContSenModesRegsArray[17][2] 2 7000135E
    {0x0F12, 0x0169}, //0117 // #senHal_pContSenModesRegsArray[17][3] 2 70001360
    {0x0F12, 0x0002}, //0002 // #senHal_pContSenModesRegsArray[18][0] 2 70001362
    {0x0F12, 0x046B}, //0417 // #senHal_pContSenModesRegsArray[18][1] 2 70001364
    {0x0F12, 0x0002}, //0002 // #senHal_pContSenModesRegsArray[18][2] 2 70001366
    {0x0F12, 0x02A9}, //0255 // #senHal_pContSenModesRegsArray[18][3] 2 70001368
    {0x0F12, 0x0419}, //03B0 // #senHal_pContSenModesRegsArray[19][0] 2 7000136A
    {0x0F12, 0x04A5}, //0435 // #senHal_pContSenModesRegsArray[19][1] 2 7000136C
    {0x0F12, 0x0257}, //01EE // #senHal_pContSenModesRegsArray[19][2] 2 7000136E
    {0x0F12, 0x02E3}, //0273 // #senHal_pContSenModesRegsArray[19][3] 2 70001370
    {0x0F12, 0x0630}, //05C7 // #senHal_pContSenModesRegsArray[20][0] 2 70001372
    {0x0F12, 0x063C}, //05CC // #senHal_pContSenModesRegsArray[20][1] 2 70001374
    {0x0F12, 0x038D}, //0324 // #senHal_pContSenModesRegsArray[20][2] 2 70001376
    {0x0F12, 0x0399}, //0329 // #senHal_pContSenModesRegsArray[20][3] 2 70001378
    {0x0F12, 0x0668}, //05E5 // #senHal_pContSenModesRegsArray[21][0] 2 7000137A
    {0x0F12, 0x06B3}, //060D // #senHal_pContSenModesRegsArray[21][1] 2 7000137C
    {0x0F12, 0x03C5}, //0342 // #senHal_pContSenModesRegsArray[21][2] 2 7000137E
    {0x0F12, 0x0410}, //036A // #senHal_pContSenModesRegsArray[21][3] 2 70001380
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[22][0] 2 70001382
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[22][1] 2 70001384
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[22][2] 2 70001386
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[22][3] 2 70001388
    {0x0F12, 0x03A2}, //036E // #senHal_pContSenModesRegsArray[23][0] 2 7000138A
    {0x0F12, 0x01D3}, //01B7 // #senHal_pContSenModesRegsArray[23][1] 2 7000138C
    {0x0F12, 0x01E0}, //01AC // #senHal_pContSenModesRegsArray[23][2] 2 7000138E
    {0x0F12, 0x00F2}, //00D6 // #senHal_pContSenModesRegsArray[23][3] 2 70001390
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[24][0] 2 70001392
    {0x0F12, 0x0461}, //040F // #senHal_pContSenModesRegsArray[24][1] 2 70001394
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[24][2] 2 70001396
    {0x0F12, 0x029F}, //024D // #senHal_pContSenModesRegsArray[24][3] 2 70001398
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[25][0] 2 7000139A
    {0x0F12, 0x063C}, //05CC // #senHal_pContSenModesRegsArray[25][1] 2 7000139C
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[25][2] 2 7000139E
    {0x0F12, 0x0399}, //0329 // #senHal_pContSenModesRegsArray[25][3] 2 700013A0
    {0x0F12, 0x003D}, //0023 // #senHal_pContSenModesRegsArray[26][0] 2 700013A2
    {0x0F12, 0x003D}, //0021 // #senHal_pContSenModesRegsArray[26][1] 2 700013A4
    {0x0F12, 0x003D}, //0023 // #senHal_pContSenModesRegsArray[26][2] 2 700013A6
    {0x0F12, 0x003D}, //0021 // #senHal_pContSenModesRegsArray[26][3] 2 700013A8
    {0x0F12, 0x01D0}, //01B6 // #senHal_pContSenModesRegsArray[27][0] 2 700013AA
    {0x0F12, 0x01D0}, //01B4 // #senHal_pContSenModesRegsArray[27][1] 2 700013AC
    {0x0F12, 0x00EF}, //00D5 // #senHal_pContSenModesRegsArray[27][2] 2 700013AE
    {0x0F12, 0x00EF}, //00D3 // #senHal_pContSenModesRegsArray[27][3] 2 700013B0
    {0x0F12, 0x020C}, //01D8 // #senHal_pContSenModesRegsArray[28][0] 2 700013B2
    {0x0F12, 0x024B}, //01F9 // #senHal_pContSenModesRegsArray[28][1] 2 700013B4
    {0x0F12, 0x012B}, //00F7 // #senHal_pContSenModesRegsArray[28][2] 2 700013B6
    {0x0F12, 0x016A}, //0118 // #senHal_pContSenModesRegsArray[28][3] 2 700013B8
    {0x0F12, 0x039F}, //036B // #senHal_pContSenModesRegsArray[29][0] 2 700013BA
    {0x0F12, 0x045E}, //040C // #senHal_pContSenModesRegsArray[29][1] 2 700013BC
    {0x0F12, 0x01DD}, //01A9 // #senHal_pContSenModesRegsArray[29][2] 2 700013BE
    {0x0F12, 0x029C}, //024A // #senHal_pContSenModesRegsArray[29][3] 2 700013C0
    {0x0F12, 0x041A}, //03B1 // #senHal_pContSenModesRegsArray[30][0] 2 700013C2
    {0x0F12, 0x04A6}, //0436 // #senHal_pContSenModesRegsArray[30][1] 2 700013C4
    {0x0F12, 0x0258}, //01EF // #senHal_pContSenModesRegsArray[30][2] 2 700013C6
    {0x0F12, 0x02E4}, //0274 // #senHal_pContSenModesRegsArray[30][3] 2 700013C8
    {0x0F12, 0x062D}, //05C4 // #senHal_pContSenModesRegsArray[31][0] 2 700013CA
    {0x0F12, 0x0639}, //05C9 // #senHal_pContSenModesRegsArray[31][1] 2 700013CC
    {0x0F12, 0x038A}, //0321 // #senHal_pContSenModesRegsArray[31][2] 2 700013CE
    {0x0F12, 0x0396}, //0326 // #senHal_pContSenModesRegsArray[31][3] 2 700013D0
    {0x0F12, 0x0669}, //05E6 // #senHal_pContSenModesRegsArray[32][0] 2 700013D2
    {0x0F12, 0x06B4}, //060E // #senHal_pContSenModesRegsArray[32][1] 2 700013D4
    {0x0F12, 0x03C6}, //0343 // #senHal_pContSenModesRegsArray[32][2] 2 700013D6
    {0x0F12, 0x0411}, //036B // #senHal_pContSenModesRegsArray[32][3] 2 700013D8
    {0x0F12, 0x087C}, //07F9 // #senHal_pContSenModesRegsArray[33][0] 2 700013DA
    {0x0F12, 0x08C7}, //0821 // #senHal_pContSenModesRegsArray[33][1] 2 700013DC
    {0x0F12, 0x04F8}, //0475 // #senHal_pContSenModesRegsArray[33][2] 2 700013DE
    {0x0F12, 0x0543}, //049D // #senHal_pContSenModesRegsArray[33][3] 2 700013E0
    {0x0F12, 0x0040}, //0026 // #senHal_pContSenModesRegsArray[34][0] 2 700013E2
    {0x0F12, 0x0040}, //0024 // #senHal_pContSenModesRegsArray[34][1] 2 700013E4
    {0x0F12, 0x0040}, //0026 // #senHal_pContSenModesRegsArray[34][2] 2 700013E6
    {0x0F12, 0x0040}, //0024 // #senHal_pContSenModesRegsArray[34][3] 2 700013E8
    {0x0F12, 0x01D0}, //01B6 // #senHal_pContSenModesRegsArray[35][0] 2 700013EA
    {0x0F12, 0x01D0}, //01B4 // #senHal_pContSenModesRegsArray[35][1] 2 700013EC
    {0x0F12, 0x00EF}, //00D5 // #senHal_pContSenModesRegsArray[35][2] 2 700013EE
    {0x0F12, 0x00EF}, //00D3 // #senHal_pContSenModesRegsArray[35][3] 2 700013F0
    {0x0F12, 0x020F}, //01DB // #senHal_pContSenModesRegsArray[36][0] 2 700013F2
    {0x0F12, 0x024E}, //01FC // #senHal_pContSenModesRegsArray[36][1] 2 700013F4
    {0x0F12, 0x012E}, //00FA // #senHal_pContSenModesRegsArray[36][2] 2 700013F6
    {0x0F12, 0x016D}, //011B // #senHal_pContSenModesRegsArray[36][3] 2 700013F8
    {0x0F12, 0x039F}, //036B // #senHal_pContSenModesRegsArray[37][0] 2 700013FA
    {0x0F12, 0x045E}, //040C // #senHal_pContSenModesRegsArray[37][1] 2 700013FC
    {0x0F12, 0x01DD}, //01A9 // #senHal_pContSenModesRegsArray[37][2] 2 700013FE
    {0x0F12, 0x029C}, //024A // #senHal_pContSenModesRegsArray[37][3] 2 70001400
    {0x0F12, 0x041D}, //03B4 // #senHal_pContSenModesRegsArray[38][0] 2 70001402
    {0x0F12, 0x04A9}, //0439 // #senHal_pContSenModesRegsArray[38][1] 2 70001404
    {0x0F12, 0x025B}, //01F2 // #senHal_pContSenModesRegsArray[38][2] 2 70001406
    {0x0F12, 0x02E7}, //0277 // #senHal_pContSenModesRegsArray[38][3] 2 70001408
    {0x0F12, 0x062D}, //05C4 // #senHal_pContSenModesRegsArray[39][0] 2 7000140A
    {0x0F12, 0x0639}, //05C9 // #senHal_pContSenModesRegsArray[39][1] 2 7000140C
    {0x0F12, 0x038A}, //0321 // #senHal_pContSenModesRegsArray[39][2] 2 7000140E
    {0x0F12, 0x0396}, //0326 // #senHal_pContSenModesRegsArray[39][3] 2 70001410
    {0x0F12, 0x066C}, //05E9 // #senHal_pContSenModesRegsArray[40][0] 2 70001412
    {0x0F12, 0x06B7}, //0611 // #senHal_pContSenModesRegsArray[40][1] 2 70001414
    {0x0F12, 0x03C9}, //0346 // #senHal_pContSenModesRegsArray[40][2] 2 70001416
    {0x0F12, 0x0414}, //036E // #senHal_pContSenModesRegsArray[40][3] 2 70001418
    {0x0F12, 0x087C}, //07F9 // #senHal_pContSenModesRegsArray[41][0] 2 7000141A
    {0x0F12, 0x08C7}, //0821 // #senHal_pContSenModesRegsArray[41][1] 2 7000141C
    {0x0F12, 0x04F8}, //0475 // #senHal_pContSenModesRegsArray[41][2] 2 7000141E
    {0x0F12, 0x0543}, //049D // #senHal_pContSenModesRegsArray[41][3] 2 70001420
    {0x0F12, 0x0040}, //0026 // #senHal_pContSenModesRegsArray[42][0] 2 70001422
    {0x0F12, 0x0040}, //0024 // #senHal_pContSenModesRegsArray[42][1] 2 70001424
    {0x0F12, 0x0040}, //0026 // #senHal_pContSenModesRegsArray[42][2] 2 70001426
    {0x0F12, 0x0040}, //0024 // #senHal_pContSenModesRegsArray[42][3] 2 70001428
    {0x0F12, 0x01D0}, //01B6 // #senHal_pContSenModesRegsArray[43][0] 2 7000142A
    {0x0F12, 0x01D0}, //01B4 // #senHal_pContSenModesRegsArray[43][1] 2 7000142C
    {0x0F12, 0x00EF}, //00D5 // #senHal_pContSenModesRegsArray[43][2] 2 7000142E
    {0x0F12, 0x00EF}, //00D3 // #senHal_pContSenModesRegsArray[43][3] 2 70001430
    {0x0F12, 0x020F}, //01DB // #senHal_pContSenModesRegsArray[44][0] 2 70001432
    {0x0F12, 0x024E}, //01FC // #senHal_pContSenModesRegsArray[44][1] 2 70001434
    {0x0F12, 0x012E}, //00FA // #senHal_pContSenModesRegsArray[44][2] 2 70001436
    {0x0F12, 0x016D}, //011B // #senHal_pContSenModesRegsArray[44][3] 2 70001438
    {0x0F12, 0x039F}, //036B // #senHal_pContSenModesRegsArray[45][0] 2 7000143A
    {0x0F12, 0x045E}, //040C // #senHal_pContSenModesRegsArray[45][1] 2 7000143C
    {0x0F12, 0x01DD}, //01A9 // #senHal_pContSenModesRegsArray[45][2] 2 7000143E
    {0x0F12, 0x029C}, //024A // #senHal_pContSenModesRegsArray[45][3] 2 70001440
    {0x0F12, 0x041D}, //03B4 // #senHal_pContSenModesRegsArray[46][0] 2 70001442
    {0x0F12, 0x04A9}, //0439 // #senHal_pContSenModesRegsArray[46][1] 2 70001444
    {0x0F12, 0x025B}, //01F2 // #senHal_pContSenModesRegsArray[46][2] 2 70001446
    {0x0F12, 0x02E7}, //0277 // #senHal_pContSenModesRegsArray[46][3] 2 70001448
    {0x0F12, 0x062D}, //05C4 // #senHal_pContSenModesRegsArray[47][0] 2 7000144A
    {0x0F12, 0x0639}, //05C9 // #senHal_pContSenModesRegsArray[47][1] 2 7000144C
    {0x0F12, 0x038A}, //0321 // #senHal_pContSenModesRegsArray[47][2] 2 7000144E
    {0x0F12, 0x0396}, //0326 // #senHal_pContSenModesRegsArray[47][3] 2 70001450
    {0x0F12, 0x066C}, //05E9 // #senHal_pContSenModesRegsArray[48][0] 2 70001452
    {0x0F12, 0x06B7}, //0611 // #senHal_pContSenModesRegsArray[48][1] 2 70001454
    {0x0F12, 0x03C9}, //0346 // #senHal_pContSenModesRegsArray[48][2] 2 70001456
    {0x0F12, 0x0414}, //036E // #senHal_pContSenModesRegsArray[48][3] 2 70001458
    {0x0F12, 0x087C}, //07F9 // #senHal_pContSenModesRegsArray[49][0] 2 7000145A
    {0x0F12, 0x08C7}, //0821 // #senHal_pContSenModesRegsArray[49][1] 2 7000145C
    {0x0F12, 0x04F8}, //0475 // #senHal_pContSenModesRegsArray[49][2] 2 7000145E
    {0x0F12, 0x0543}, //049D // #senHal_pContSenModesRegsArray[49][3] 2 70001460
    {0x0F12, 0x003D}, //0023 // #senHal_pContSenModesRegsArray[50][0] 2 70001462
    {0x0F12, 0x003D}, //0021 // #senHal_pContSenModesRegsArray[50][1] 2 70001464
    {0x0F12, 0x003D}, //0023 // #senHal_pContSenModesRegsArray[50][2] 2 70001466
    {0x0F12, 0x003D}, //0021 // #senHal_pContSenModesRegsArray[50][3] 2 70001468
    {0x0F12, 0x01D2}, //01B8 // #senHal_pContSenModesRegsArray[51][0] 2 7000146A
    {0x0F12, 0x01D2}, //01B6 // #senHal_pContSenModesRegsArray[51][1] 2 7000146C
    {0x0F12, 0x00F1}, //00D7 // #senHal_pContSenModesRegsArray[51][2] 2 7000146E
    {0x0F12, 0x00F1}, //00D5 // #senHal_pContSenModesRegsArray[51][3] 2 70001470
    {0x0F12, 0x020C}, //01D8 // #senHal_pContSenModesRegsArray[52][0] 2 70001472
    {0x0F12, 0x024B}, //01F9 // #senHal_pContSenModesRegsArray[52][1] 2 70001474
    {0x0F12, 0x012B}, //00F7 // #senHal_pContSenModesRegsArray[52][2] 2 70001476
    {0x0F12, 0x016A}, //0118 // #senHal_pContSenModesRegsArray[52][3] 2 70001478
    {0x0F12, 0x03A1}, //036D // #senHal_pContSenModesRegsArray[53][0] 2 7000147A
    {0x0F12, 0x0460}, //040E // #senHal_pContSenModesRegsArray[53][1] 2 7000147C
    {0x0F12, 0x01DF}, //01AB // #senHal_pContSenModesRegsArray[53][2] 2 7000147E
    {0x0F12, 0x029E}, //024C // #senHal_pContSenModesRegsArray[53][3] 2 70001480
    {0x0F12, 0x041A}, //03B1 // #senHal_pContSenModesRegsArray[54][0] 2 70001482
    {0x0F12, 0x04A6}, //0436 // #senHal_pContSenModesRegsArray[54][1] 2 70001484
    {0x0F12, 0x0258}, //01EF // #senHal_pContSenModesRegsArray[54][2] 2 70001486
    {0x0F12, 0x02E4}, //0274 // #senHal_pContSenModesRegsArray[54][3] 2 70001488
    {0x0F12, 0x062F}, //05C6 // #senHal_pContSenModesRegsArray[55][0] 2 7000148A
    {0x0F12, 0x063B}, //05CB // #senHal_pContSenModesRegsArray[55][1] 2 7000148C
    {0x0F12, 0x038C}, //0323 // #senHal_pContSenModesRegsArray[55][2] 2 7000148E
    {0x0F12, 0x0398}, //0328 // #senHal_pContSenModesRegsArray[55][3] 2 70001490
    {0x0F12, 0x0669}, //05E6 // #senHal_pContSenModesRegsArray[56][0] 2 70001492
    {0x0F12, 0x06B4}, //060E // #senHal_pContSenModesRegsArray[56][1] 2 70001494
    {0x0F12, 0x03C6}, //0343 // #senHal_pContSenModesRegsArray[56][2] 2 70001496
    {0x0F12, 0x0411}, //036B // #senHal_pContSenModesRegsArray[56][3] 2 70001498
    {0x0F12, 0x087E}, //07FB // #senHal_pContSenModesRegsArray[57][0] 2 7000149A
    {0x0F12, 0x08C9}, //0823 // #senHal_pContSenModesRegsArray[57][1] 2 7000149C
    {0x0F12, 0x04FA}, //0477 // #senHal_pContSenModesRegsArray[57][2] 2 7000149E
    {0x0F12, 0x0545}, //049F // #senHal_pContSenModesRegsArray[57][3] 2 700014A0
    {0x0F12, 0x03A2}, //036E // #senHal_pContSenModesRegsArray[58][0] 2 700014A2
    {0x0F12, 0x01D3}, //01B7 // #senHal_pContSenModesRegsArray[58][1] 2 700014A4
    {0x0F12, 0x01E0}, //01AC // #senHal_pContSenModesRegsArray[58][2] 2 700014A6
    {0x0F12, 0x00F2}, //00D6 // #senHal_pContSenModesRegsArray[58][3] 2 700014A8
    {0x0F12, 0x03AF}, //037B // #senHal_pContSenModesRegsArray[59][0] 2 700014AA
    {0x0F12, 0x01E0}, //01C4 // #senHal_pContSenModesRegsArray[59][1] 2 700014AC
    {0x0F12, 0x01ED}, //01B9 // #senHal_pContSenModesRegsArray[59][2] 2 700014AE
    {0x0F12, 0x00FF}, //00E3 // #senHal_pContSenModesRegsArray[59][3] 2 700014B0
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[60][0] 2 700014B2
    {0x0F12, 0x0461}, //040F // #senHal_pContSenModesRegsArray[60][1] 2 700014B4
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[60][2] 2 700014B6
    {0x0F12, 0x029F}, //024D // #senHal_pContSenModesRegsArray[60][3] 2 700014B8
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[61][0] 2 700014BA
    {0x0F12, 0x046E}, //041C // #senHal_pContSenModesRegsArray[61][1] 2 700014BC
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[61][2] 2 700014BE
    {0x0F12, 0x02AC}, //025A // #senHal_pContSenModesRegsArray[61][3] 2 700014C0
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[62][0] 2 700014C2
    {0x0F12, 0x063C}, //05CC // #senHal_pContSenModesRegsArray[62][1] 2 700014C4
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[62][2] 2 700014C6
    {0x0F12, 0x0399}, //0329 // #senHal_pContSenModesRegsArray[62][3] 2 700014C8
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[63][0] 2 700014CA
    {0x0F12, 0x0649}, //05D9 // #senHal_pContSenModesRegsArray[63][1] 2 700014CC
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[63][2] 2 700014CE
    {0x0F12, 0x03A6}, //0336 // #senHal_pContSenModesRegsArray[63][3] 2 700014D0
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[64][0] 2 700014D2
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[64][1] 2 700014D4
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[64][2] 2 700014D6
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[64][3] 2 700014D8
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[65][0] 2 700014DA
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[65][1] 2 700014DC
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[65][2] 2 700014DE
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[65][3] 2 700014E0
    {0x0F12, 0x03AA}, //0376 // #senHal_pContSenModesRegsArray[66][0] 2 700014E2
    {0x0F12, 0x01DB}, //01BF // #senHal_pContSenModesRegsArray[66][1] 2 700014E4
    {0x0F12, 0x01E8}, //01B4 // #senHal_pContSenModesRegsArray[66][2] 2 700014E6
    {0x0F12, 0x00FA}, //00DE // #senHal_pContSenModesRegsArray[66][3] 2 700014E8
    {0x0F12, 0x03B7}, //0383 // #senHal_pContSenModesRegsArray[67][0] 2 700014EA
    {0x0F12, 0x01E8}, //01CC // #senHal_pContSenModesRegsArray[67][1] 2 700014EC
    {0x0F12, 0x01F5}, //01C1 // #senHal_pContSenModesRegsArray[67][2] 2 700014EE
    {0x0F12, 0x0107}, //00EB // #senHal_pContSenModesRegsArray[67][3] 2 700014F0
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[68][0] 2 700014F2
    {0x0F12, 0x0469}, //0417 // #senHal_pContSenModesRegsArray[68][1] 2 700014F4
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[68][2] 2 700014F6
    {0x0F12, 0x02A7}, //0255 // #senHal_pContSenModesRegsArray[68][3] 2 700014F8
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[69][0] 2 700014FA
    {0x0F12, 0x0476}, //0424 // #senHal_pContSenModesRegsArray[69][1] 2 700014FC
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[69][2] 2 700014FE
    {0x0F12, 0x02B4}, //0262 // #senHal_pContSenModesRegsArray[69][3] 2 70001500
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[70][0] 2 70001502
    {0x0F12, 0x0644}, //05D4 // #senHal_pContSenModesRegsArray[70][1] 2 70001504
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[70][2] 2 70001506
    {0x0F12, 0x03A1}, //0331 // #senHal_pContSenModesRegsArray[70][3] 2 70001508
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[71][0] 2 7000150A
    {0x0F12, 0x0651}, //05E1 // #senHal_pContSenModesRegsArray[71][1] 2 7000150C
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[71][2] 2 7000150E
    {0x0F12, 0x03AE}, //033E // #senHal_pContSenModesRegsArray[71][3] 2 70001510
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[72][0] 2 70001512
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[72][1] 2 70001514
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[72][2] 2 70001516
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[72][3] 2 70001518
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[73][0] 2 7000151A
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[73][1] 2 7000151C
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[73][2] 2 7000151E
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[73][3] 2 70001520
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[74][0] 2 70001522
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[74][1] 2 70001524
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[74][2] 2 70001526
    {0x0F12, 0x0001}, //0001 // #senHal_pContSenModesRegsArray[74][3] 2 70001528
    {0x0F12, 0x000F}, //000F // #senHal_pContSenModesRegsArray[75][0] 2 7000152A
    {0x0F12, 0x000F}, //000F // #senHal_pContSenModesRegsArray[75][1] 2 7000152C
    {0x0F12, 0x000F}, //000F // #senHal_pContSenModesRegsArray[75][2] 2 7000152E
    {0x0F12, 0x000F}, //000F // #senHal_pContSenModesRegsArray[75][3] 2 70001530
    {0x0F12, 0x05AD}, //0544 // #senHal_pContSenModesRegsArray[76][0] 2 70001532
    {0x0F12, 0x03DE}, //038C // #senHal_pContSenModesRegsArray[76][1] 2 70001534
    {0x0F12, 0x030A}, //02A1 // #senHal_pContSenModesRegsArray[76][2] 2 70001536
    {0x0F12, 0x021C}, //01CA // #senHal_pContSenModesRegsArray[76][3] 2 70001538
    {0x0F12, 0x062F}, //05C6 // #senHal_pContSenModesRegsArray[77][0] 2 7000153A
    {0x0F12, 0x0460}, //040E // #senHal_pContSenModesRegsArray[77][1] 2 7000153C
    {0x0F12, 0x038C}, //0323 // #senHal_pContSenModesRegsArray[77][2] 2 7000153E
    {0x0F12, 0x029E}, //024C // #senHal_pContSenModesRegsArray[77][3] 2 70001540
    {0x0F12, 0x07FC}, //0779 // #senHal_pContSenModesRegsArray[78][0] 2 70001542
    {0x0F12, 0x0847}, //07A1 // #senHal_pContSenModesRegsArray[78][1] 2 70001544
    {0x0F12, 0x0478}, //03F5 // #senHal_pContSenModesRegsArray[78][2] 2 70001546
    {0x0F12, 0x04C3}, //041D // #senHal_pContSenModesRegsArray[78][3] 2 70001548
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[79][0] 2 7000154A
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[79][1] 2 7000154C
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[79][2] 2 7000154E
    {0x0F12, 0x0000}, //0000 // #senHal_pContSenModesRegsArray[79][3] 2 70001550

    {0x002A, 0x12B8}, //disable CINTR 0
    {0x0F12, 0x1000},

    //============================================================
    //ISP-FE Setting
    //============================================================
    {0x002A, 0x158A},
    {0x0F12, 0xEAF0},
    {0x002A, 0x15C6},
    {0x0F12, 0x0020},
    {0x0F12, 0x0060},
    {0x002A, 0x15BC},
    {0x0F12, 0x0200}, // added by Shy.

    //Analog Offset for MSM
    {0x002A, 0x1608},
    {0x0F12, 0x0100}, // #gisp_msm_sAnalogOffset[0]
    {0x0F12, 0x0100}, // #gisp_msm_sAnalogOffset[1]
    {0x0F12, 0x0100}, // #gisp_msm_sAnalogOffset[2]
    {0x0F12, 0x0100}, // #gisp_msm_sAnalogOffset[3]
    //================================================================================================
    // SET AE
    //================================================================================================
    // AE target
    {0x002A, 0x0F70},
    {0x0F12, 0x0038}, //#TVAR_ae_BrAve 091222
    // AE mode
    {0x002A, 0x0F76},
    {0x0F12, 0x000F}, //Disable illumination & contrast  // #ae_StatMode
    {0x002A, 0x051A},
    {0x0F12, 0x0111}, //#lt_uLimitHigh
    {0x0F12, 0x00F0}, //#lt_uLimitLow
    // AE weight
    {0x002A, 0x0F7E},
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_0_
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_1_
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_2_
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_3_
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_4_
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_5_
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_6_
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_7_
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_8_
    {0x0F12, 0x0303}, // #ae_WeightTbl_16_9_	0303
    {0x0F12, 0x0303}, // #ae_WeightTbl_16_10	0303
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_11
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_12
    {0x0F12, 0x0303}, // #ae_WeightTbl_16_13	0303
    {0x0F12, 0x0303}, // #ae_WeightTbl_16_14	0303
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_15
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_16
    {0x0F12, 0x0303}, // #ae_WeightTbl_16_17	0303
    {0x0F12, 0x0303}, // #ae_WeightTbl_16_18	0303
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_19
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_20
    {0x0F12, 0x0303}, // #ae_WeightTbl_16_21	0303
    {0x0F12, 0x0303}, // #ae_WeightTbl_16_22	0303
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_23
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_24
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_25
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_26
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_27
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_28
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_29
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_30
    {0x0F12, 0x0101}, // #ae_WeightTbl_16_31

    //================================================================================================
    // SET FLICKER
    //================================================================================================
    {0x002A, 0x0C18},
    {0x0F12, 0x0000}, // 0001: 60Hz start auto / 0000: 50Hz start auto
    {0x002A, 0x04D2},
    {0x0F12, 0x067F},

    //002A   04BA
    //0F12   0001//0001(50Hz) 0002(60Hz)
    //0F12   0001//REG_SF_USER_FlickerQuantChanged

    //================================================================================================
    // SET GAS
    //================================================================================================
    // GAS alpha
    // R, Gr, Gb, B per light source
    {0x002A, 0x06CE},
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[0] // Horizon
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[1]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[2]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[3]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[4] // IncandA
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[5]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[6]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[7]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[8] // WW
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[9]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[10]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[11] ///
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[12]// CWF
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[13]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[14]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[15] ///
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[16]// D50
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[17]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[18]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[19]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[20]// D65
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[21]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[22]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[23]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[24]// D75
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[25]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[26]
    {0x0F12, 0x0100}, // #TVAR_ash_GASAlpha[27]
    {0x0F12, 0x0100}, // #TVAR_ash_GASOutdoorAlpha[0] // Outdoor
    {0x0F12, 0x0100}, // #TVAR_ash_GASOutdoorAlpha[1]
    {0x0F12, 0x0100}, // #TVAR_ash_GASOutdoorAlpha[2]
    {0x0F12, 0x0100}, // #TVAR_ash_GASOutdoorAlpha[3] // GAS beta
    {0x0F12, 0x0000}, // #ash_GASBeta[0]// Horizon
    {0x0F12, 0x0000}, // #ash_GASBeta[1]
    {0x0F12, 0x0000}, // #ash_GASBeta[2]
    {0x0F12, 0x0000}, // #ash_GASBeta[3]
    {0x0F12, 0x0000}, // #ash_GASBeta[4]// IncandA
    {0x0F12, 0x0000}, // #ash_GASBeta[5]
    {0x0F12, 0x0000}, // #ash_GASBeta[6]
    {0x0F12, 0x0000}, // #ash_GASBeta[7]
    {0x0F12, 0x0000}, // #ash_GASBeta[8]// WW
    {0x0F12, 0x0000}, // #ash_GASBeta[9]
    {0x0F12, 0x0000}, // #ash_GASBeta[10]
    {0x0F12, 0x0000}, // #ash_GASBeta[11]
    {0x0F12, 0x0000}, // #ash_GASBeta[12] // CWF
    {0x0F12, 0x0000}, // #ash_GASBeta[13]
    {0x0F12, 0x0000}, // #ash_GASBeta[14]
    {0x0F12, 0x0000}, // #ash_GASBeta[15]
    {0x0F12, 0x0000}, // #ash_GASBeta[16] // D50
    {0x0F12, 0x0000}, // #ash_GASBeta[17]
    {0x0F12, 0x0000}, // #ash_GASBeta[18]
    {0x0F12, 0x0000}, // #ash_GASBeta[19]
    {0x0F12, 0x0000}, // #ash_GASBeta[20] // D65
    {0x0F12, 0x0000}, // #ash_GASBeta[21]
    {0x0F12, 0x0000}, // #ash_GASBeta[22]
    {0x0F12, 0x0000}, // #ash_GASBeta[23]
    {0x0F12, 0x0000}, // #ash_GASBeta[24] // D75
    {0x0F12, 0x0000}, // #ash_GASBeta[25]
    {0x0F12, 0x0000}, // #ash_GASBeta[26]
    {0x0F12, 0x0000}, // #ash_GASBeta[27]
    {0x0F12, 0x0000}, // #ash_GASOutdoorBeta[0] // Outdoor
    {0x0F12, 0x0000}, // #ash_GASOutdoorBeta[1]
    {0x0F12, 0x0000}, // #ash_GASOutdoorBeta[2]
    {0x0F12, 0x0000}, // #ash_GASOutdoorBeta[3]
    {0x002A, 0x06B4},
    {0x0F12, 0x0000}, // #wbt_bUseOutdoorASH ON:1 OFF:0

    // Parabloic function
    {0x002A, 0x075A},
    {0x0F12, 0x0000}, // #ash_bParabolicEstimation
    {0x0F12, 0x0400}, // #ash_uParabolicCenterX
    {0x0F12, 0x0300}, // #ash_uParabolicCenterY
    {0x0F12, 0x0010}, // #ash_uParabolicScalingA
    {0x0F12, 0x0011}, // #ash_uParabolicScalingB
    {0x002A, 0x06C6},
    {0x0F12, 0x0100}, //ash_CGrasAlphas_0_
    {0x0F12, 0x0100}, //ash_CGrasAlphas_1_
    {0x0F12, 0x0100}, //ash_CGrasAlphas_2_
    {0x0F12, 0x0100}, //ash_CGrasAlphas_3_
    {0x002A, 0x0E3C},
    {0x0F12, 0x00C0}, // #awbb_Alpha_Comp_Mode
    {0x002A, 0x074E},
    {0x0F12, 0x0000}, // #ash_bLumaMode //use Beta : 0001 not use Beta : 0000

    // GAS LUT start address // 7000_347C
    {0x002A, 0x0754},
    {0x0F12, 0x347C},
    {0x0F12, 0x7000},

    // GAS LUT
    {0x002A, 0x347C}, ///
    {0x0F12, 0x020B}, // #TVAR_ash_pGAS[0]
    {0x0F12, 0x019E}, // #TVAR_ash_pGAS[1]
    {0x0F12, 0x0155}, // #TVAR_ash_pGAS[2]
    {0x0F12, 0x0121}, // #TVAR_ash_pGAS[3]
    {0x0F12, 0x00FA}, // #TVAR_ash_pGAS[4]
    {0x0F12, 0x00E2}, // #TVAR_ash_pGAS[5]
    {0x0F12, 0x00D7}, // #TVAR_ash_pGAS[6]
    {0x0F12, 0x00DC}, // #TVAR_ash_pGAS[7]
    {0x0F12, 0x00F0}, // #TVAR_ash_pGAS[8]
    {0x0F12, 0x0114}, // #TVAR_ash_pGAS[9]
    {0x0F12, 0x0148}, // #TVAR_ash_pGAS[10]
    {0x0F12, 0x0196}, // #TVAR_ash_pGAS[11]
    {0x0F12, 0x01F5}, // #TVAR_ash_pGAS[12]
    {0x0F12, 0x01B0}, // #TVAR_ash_pGAS[13]
    {0x0F12, 0x0160}, // #TVAR_ash_pGAS[14]
    {0x0F12, 0x0118}, // #TVAR_ash_pGAS[15]
    {0x0F12, 0x00E0}, // #TVAR_ash_pGAS[16]
    {0x0F12, 0x00B6}, // #TVAR_ash_pGAS[17]
    {0x0F12, 0x009B}, // #TVAR_ash_pGAS[18]
    {0x0F12, 0x008F}, // #TVAR_ash_pGAS[19]
    {0x0F12, 0x0094}, // #TVAR_ash_pGAS[20]
    {0x0F12, 0x00AD}, // #TVAR_ash_pGAS[21]
    {0x0F12, 0x00D8}, // #TVAR_ash_pGAS[22]
    {0x0F12, 0x0110}, // #TVAR_ash_pGAS[23]
    {0x0F12, 0x0156}, // #TVAR_ash_pGAS[24]
    {0x0F12, 0x01AC}, // #TVAR_ash_pGAS[25]
    {0x0F12, 0x0172}, // #TVAR_ash_pGAS[26]
    {0x0F12, 0x0124}, // #TVAR_ash_pGAS[27]
    {0x0F12, 0x00DB}, // #TVAR_ash_pGAS[28]
    {0x0F12, 0x009F}, // #TVAR_ash_pGAS[29]
    {0x0F12, 0x0073}, // #TVAR_ash_pGAS[30]
    {0x0F12, 0x0057}, // #TVAR_ash_pGAS[31]
    {0x0F12, 0x004C}, // #TVAR_ash_pGAS[32]
    {0x0F12, 0x0054}, // #TVAR_ash_pGAS[33]
    {0x0F12, 0x006C}, // #TVAR_ash_pGAS[34]
    {0x0F12, 0x0097}, // #TVAR_ash_pGAS[35]
    {0x0F12, 0x00D5}, // #TVAR_ash_pGAS[36]
    {0x0F12, 0x0120}, // #TVAR_ash_pGAS[37]
    {0x0F12, 0x0170}, // #TVAR_ash_pGAS[38]
    {0x0F12, 0x0147}, // #TVAR_ash_pGAS[39]
    {0x0F12, 0x00FC}, // #TVAR_ash_pGAS[40]
    {0x0F12, 0x00AC}, // #TVAR_ash_pGAS[41]
    {0x0F12, 0x006F}, // #TVAR_ash_pGAS[42]
    {0x0F12, 0x0044}, // #TVAR_ash_pGAS[43]
    {0x0F12, 0x002A}, // #TVAR_ash_pGAS[44]
    {0x0F12, 0x0020}, // #TVAR_ash_pGAS[45]
    {0x0F12, 0x0027}, // #TVAR_ash_pGAS[46]
    {0x0F12, 0x0040}, // #TVAR_ash_pGAS[47]
    {0x0F12, 0x006C}, // #TVAR_ash_pGAS[48]
    {0x0F12, 0x00AB}, // #TVAR_ash_pGAS[49]
    {0x0F12, 0x00FB}, // #TVAR_ash_pGAS[50]
    {0x0F12, 0x014B}, // #TVAR_ash_pGAS[51]
    {0x0F12, 0x0131}, // #TVAR_ash_pGAS[52]
    {0x0F12, 0x00E4}, // #TVAR_ash_pGAS[53]
    {0x0F12, 0x0090}, // #TVAR_ash_pGAS[54]
    {0x0F12, 0x0052}, // #TVAR_ash_pGAS[55]
    {0x0F12, 0x0027}, // #TVAR_ash_pGAS[56]
    {0x0F12, 0x0010}, // #TVAR_ash_pGAS[57]
    {0x0F12, 0x0008}, // #TVAR_ash_pGAS[58]
    {0x0F12, 0x000F}, // #TVAR_ash_pGAS[59]
    {0x0F12, 0x0027}, // #TVAR_ash_pGAS[60]
    {0x0F12, 0x0053}, // #TVAR_ash_pGAS[61]
    {0x0F12, 0x0093}, // #TVAR_ash_pGAS[62]
    {0x0F12, 0x00EA}, // #TVAR_ash_pGAS[63]
    {0x0F12, 0x0138}, // #TVAR_ash_pGAS[64]
    {0x0F12, 0x0129}, // #TVAR_ash_pGAS[65]
    {0x0F12, 0x00DA}, // #TVAR_ash_pGAS[66]
    {0x0F12, 0x0085}, // #TVAR_ash_pGAS[67]
    {0x0F12, 0x0046}, // #TVAR_ash_pGAS[68]
    {0x0F12, 0x001D}, // #TVAR_ash_pGAS[69]
    {0x0F12, 0x0007}, // #TVAR_ash_pGAS[70]
    {0x0F12, 0x0000}, // #TVAR_ash_pGAS[71]
    {0x0F12, 0x0007}, // #TVAR_ash_pGAS[72]
    {0x0F12, 0x001F}, // #TVAR_ash_pGAS[73]
    {0x0F12, 0x004B}, // #TVAR_ash_pGAS[74]
    {0x0F12, 0x008E}, // #TVAR_ash_pGAS[75]
    {0x0F12, 0x00E4}, // #TVAR_ash_pGAS[76]
    {0x0F12, 0x0138}, // #TVAR_ash_pGAS[77]
    {0x0F12, 0x0130}, // #TVAR_ash_pGAS[78]
    {0x0F12, 0x00E2}, // #TVAR_ash_pGAS[79]
    {0x0F12, 0x008D}, // #TVAR_ash_pGAS[80]
    {0x0F12, 0x004E}, // #TVAR_ash_pGAS[81]
    {0x0F12, 0x0025}, // #TVAR_ash_pGAS[82]
    {0x0F12, 0x000E}, // #TVAR_ash_pGAS[83]
    {0x0F12, 0x0007}, // #TVAR_ash_pGAS[84]
    {0x0F12, 0x000F}, // #TVAR_ash_pGAS[85]
    {0x0F12, 0x0027}, // #TVAR_ash_pGAS[86]
    {0x0F12, 0x0055}, // #TVAR_ash_pGAS[87]
    {0x0F12, 0x009A}, // #TVAR_ash_pGAS[88]
    {0x0F12, 0x00F2}, // #TVAR_ash_pGAS[89]
    {0x0F12, 0x0142}, // #TVAR_ash_pGAS[90]
    {0x0F12, 0x0145}, // #TVAR_ash_pGAS[91]
    {0x0F12, 0x00F9}, // #TVAR_ash_pGAS[92]
    {0x0F12, 0x00A6}, // #TVAR_ash_pGAS[93]
    {0x0F12, 0x0067}, // #TVAR_ash_pGAS[94]
    {0x0F12, 0x003C}, // #TVAR_ash_pGAS[95]
    {0x0F12, 0x0024}, // #TVAR_ash_pGAS[96]
    {0x0F12, 0x001D}, // #TVAR_ash_pGAS[97]
    {0x0F12, 0x0025}, // #TVAR_ash_pGAS[98]
    {0x0F12, 0x0041}, // #TVAR_ash_pGAS[99]
    {0x0F12, 0x0071}, // #TVAR_ash_pGAS[100]
    {0x0F12, 0x00B5}, // #TVAR_ash_pGAS[101]
    {0x0F12, 0x010B}, // #TVAR_ash_pGAS[102]
    {0x0F12, 0x015A}, // #TVAR_ash_pGAS[103]
    {0x0F12, 0x0169}, // #TVAR_ash_pGAS[104]
    {0x0F12, 0x011F}, // #TVAR_ash_pGAS[105]
    {0x0F12, 0x00CF}, // #TVAR_ash_pGAS[106]
    {0x0F12, 0x0092}, // #TVAR_ash_pGAS[107]
    {0x0F12, 0x0066}, // #TVAR_ash_pGAS[108]
    {0x0F12, 0x004D}, // #TVAR_ash_pGAS[109]
    {0x0F12, 0x0044}, // #TVAR_ash_pGAS[110]
    {0x0F12, 0x004F}, // #TVAR_ash_pGAS[111]
    {0x0F12, 0x006B}, // #TVAR_ash_pGAS[112]
    {0x0F12, 0x009E}, // #TVAR_ash_pGAS[113]
    {0x0F12, 0x00E2}, // #TVAR_ash_pGAS[114]
    {0x0F12, 0x0136}, // #TVAR_ash_pGAS[115]
    {0x0F12, 0x0183}, // #TVAR_ash_pGAS[116]
    {0x0F12, 0x01A9}, // #TVAR_ash_pGAS[117]
    {0x0F12, 0x0157}, // #TVAR_ash_pGAS[118]
    {0x0F12, 0x010E}, // #TVAR_ash_pGAS[119]
    {0x0F12, 0x00D2}, // #TVAR_ash_pGAS[120]
    {0x0F12, 0x00A7}, // #TVAR_ash_pGAS[121]
    {0x0F12, 0x008C}, // #TVAR_ash_pGAS[122]
    {0x0F12, 0x0086}, // #TVAR_ash_pGAS[123]
    {0x0F12, 0x0090}, // #TVAR_ash_pGAS[124]
    {0x0F12, 0x00AE}, // #TVAR_ash_pGAS[125]
    {0x0F12, 0x00E0}, // #TVAR_ash_pGAS[126]
    {0x0F12, 0x0121}, // #TVAR_ash_pGAS[127]
    {0x0F12, 0x0170}, // #TVAR_ash_pGAS[128]
    {0x0F12, 0x01C5}, // #TVAR_ash_pGAS[129]
    {0x0F12, 0x01F7}, // #TVAR_ash_pGAS[130]
    {0x0F12, 0x0193}, // #TVAR_ash_pGAS[131]
    {0x0F12, 0x014B}, // #TVAR_ash_pGAS[132]
    {0x0F12, 0x0114}, // #TVAR_ash_pGAS[133]
    {0x0F12, 0x00F0}, // #TVAR_ash_pGAS[134]
    {0x0F12, 0x00D7}, // #TVAR_ash_pGAS[135]
    {0x0F12, 0x00D0}, // #TVAR_ash_pGAS[136]
    {0x0F12, 0x00D7}, // #TVAR_ash_pGAS[137]
    {0x0F12, 0x00F4}, // #TVAR_ash_pGAS[138]
    {0x0F12, 0x0125}, // #TVAR_ash_pGAS[139]
    {0x0F12, 0x0160}, // #TVAR_ash_pGAS[140]
    {0x0F12, 0x01B2}, // #TVAR_ash_pGAS[141]
    {0x0F12, 0x021A}, // #TVAR_ash_pGAS[142]
    {0x0F12, 0x01DF}, // #TVAR_ash_pGAS[143]
    {0x0F12, 0x0172}, // #TVAR_ash_pGAS[144]
    {0x0F12, 0x012C}, // #TVAR_ash_pGAS[145]
    {0x0F12, 0x00FB}, // #TVAR_ash_pGAS[146]
    {0x0F12, 0x00D5}, // #TVAR_ash_pGAS[147]
    {0x0F12, 0x00BB}, // #TVAR_ash_pGAS[148]
    {0x0F12, 0x00AF}, // #TVAR_ash_pGAS[149]
    {0x0F12, 0x00B3}, // #TVAR_ash_pGAS[150]
    {0x0F12, 0x00C7}, // #TVAR_ash_pGAS[151]
    {0x0F12, 0x00E7}, // #TVAR_ash_pGAS[152]
    {0x0F12, 0x0110}, // #TVAR_ash_pGAS[153]
    {0x0F12, 0x0155}, // #TVAR_ash_pGAS[154]
    {0x0F12, 0x01B2}, // #TVAR_ash_pGAS[155]
    {0x0F12, 0x018C}, // #TVAR_ash_pGAS[156]
    {0x0F12, 0x0135}, // #TVAR_ash_pGAS[157]
    {0x0F12, 0x00F5}, // #TVAR_ash_pGAS[158]
    {0x0F12, 0x00C0}, // #TVAR_ash_pGAS[159]
    {0x0F12, 0x0097}, // #TVAR_ash_pGAS[160]
    {0x0F12, 0x007C}, // #TVAR_ash_pGAS[161]
    {0x0F12, 0x0072}, // #TVAR_ash_pGAS[162]
    {0x0F12, 0x0078}, // #TVAR_ash_pGAS[163]
    {0x0F12, 0x008D}, // #TVAR_ash_pGAS[164]
    {0x0F12, 0x00B2}, // #TVAR_ash_pGAS[165]
    {0x0F12, 0x00E0}, // #TVAR_ash_pGAS[166]
    {0x0F12, 0x011E}, // #TVAR_ash_pGAS[167]
    {0x0F12, 0x0170}, // #TVAR_ash_pGAS[168]
    {0x0F12, 0x0159}, // #TVAR_ash_pGAS[169]
    {0x0F12, 0x0106}, // #TVAR_ash_pGAS[170]
    {0x0F12, 0x00C2}, // #TVAR_ash_pGAS[171]
    {0x0F12, 0x008B}, // #TVAR_ash_pGAS[172]
    {0x0F12, 0x0061}, // #TVAR_ash_pGAS[173]
    {0x0F12, 0x0046}, // #TVAR_ash_pGAS[174]
    {0x0F12, 0x003C}, // #TVAR_ash_pGAS[175]
    {0x0F12, 0x0043}, // #TVAR_ash_pGAS[176]
    {0x0F12, 0x0058}, // #TVAR_ash_pGAS[177]
    {0x0F12, 0x007E}, // #TVAR_ash_pGAS[178]
    {0x0F12, 0x00B3}, // #TVAR_ash_pGAS[179]
    {0x0F12, 0x00F4}, // #TVAR_ash_pGAS[180]
    {0x0F12, 0x0141}, // #TVAR_ash_pGAS[181]
    {0x0F12, 0x0132}, // #TVAR_ash_pGAS[182]
    {0x0F12, 0x00E5}, // #TVAR_ash_pGAS[183]
    {0x0F12, 0x009C}, // #TVAR_ash_pGAS[184]
    {0x0F12, 0x0062}, // #TVAR_ash_pGAS[185]
    {0x0F12, 0x0039}, // #TVAR_ash_pGAS[186]
    {0x0F12, 0x0021}, // #TVAR_ash_pGAS[187]
    {0x0F12, 0x0018}, // #TVAR_ash_pGAS[188]
    {0x0F12, 0x0020}, // #TVAR_ash_pGAS[189]
    {0x0F12, 0x0034}, // #TVAR_ash_pGAS[190]
    {0x0F12, 0x005B}, // #TVAR_ash_pGAS[191]
    {0x0F12, 0x0092}, // #TVAR_ash_pGAS[192]
    {0x0F12, 0x00D6}, // #TVAR_ash_pGAS[193]
    {0x0F12, 0x0121}, // #TVAR_ash_pGAS[194]
    {0x0F12, 0x011D}, // #TVAR_ash_pGAS[195]
    {0x0F12, 0x00CC}, // #TVAR_ash_pGAS[196]
    {0x0F12, 0x0083}, // #TVAR_ash_pGAS[197]
    {0x0F12, 0x004A}, // #TVAR_ash_pGAS[198]
    {0x0F12, 0x0023}, // #TVAR_ash_pGAS[199]
    {0x0F12, 0x000C}, // #TVAR_ash_pGAS[200]
    {0x0F12, 0x0005}, // #TVAR_ash_pGAS[201]
    {0x0F12, 0x000D}, // #TVAR_ash_pGAS[202]
    {0x0F12, 0x0022}, // #TVAR_ash_pGAS[203]
    {0x0F12, 0x0048}, // #TVAR_ash_pGAS[204]
    {0x0F12, 0x007F}, // #TVAR_ash_pGAS[205]
    {0x0F12, 0x00C9}, // #TVAR_ash_pGAS[206]
    {0x0F12, 0x0115}, // #TVAR_ash_pGAS[207]
    {0x0F12, 0x0115}, // #TVAR_ash_pGAS[208]
    {0x0F12, 0x00C4}, // #TVAR_ash_pGAS[209]
    {0x0F12, 0x007A}, // #TVAR_ash_pGAS[210]
    {0x0F12, 0x0041}, // #TVAR_ash_pGAS[211]
    {0x0F12, 0x001B}, // #TVAR_ash_pGAS[212]
    {0x0F12, 0x0006}, // #TVAR_ash_pGAS[213]
    {0x0F12, 0x0000}, // #TVAR_ash_pGAS[214]
    {0x0F12, 0x0008}, // #TVAR_ash_pGAS[215]
    {0x0F12, 0x001E}, // #TVAR_ash_pGAS[216]
    {0x0F12, 0x0044}, // #TVAR_ash_pGAS[217]
    {0x0F12, 0x007D}, // #TVAR_ash_pGAS[218]
    {0x0F12, 0x00C9}, // #TVAR_ash_pGAS[219]
    {0x0F12, 0x0115}, // #TVAR_ash_pGAS[220]
    {0x0F12, 0x011C}, // #TVAR_ash_pGAS[221]
    {0x0F12, 0x00CB}, // #TVAR_ash_pGAS[222]
    {0x0F12, 0x0081}, // #TVAR_ash_pGAS[223]
    {0x0F12, 0x0048}, // #TVAR_ash_pGAS[224]
    {0x0F12, 0x0022}, // #TVAR_ash_pGAS[225]
    {0x0F12, 0x000D}, // #TVAR_ash_pGAS[226]
    {0x0F12, 0x0007}, // #TVAR_ash_pGAS[227]
    {0x0F12, 0x0010}, // #TVAR_ash_pGAS[228]
    {0x0F12, 0x0027}, // #TVAR_ash_pGAS[229]
    {0x0F12, 0x004E}, // #TVAR_ash_pGAS[230]
    {0x0F12, 0x0088}, // #TVAR_ash_pGAS[231]
    {0x0F12, 0x00D5}, // #TVAR_ash_pGAS[232]
    {0x0F12, 0x011C}, // #TVAR_ash_pGAS[233]
    {0x0F12, 0x012D}, // #TVAR_ash_pGAS[234]
    {0x0F12, 0x00E0}, // #TVAR_ash_pGAS[235]
    {0x0F12, 0x0097}, // #TVAR_ash_pGAS[236]
    {0x0F12, 0x005F}, // #TVAR_ash_pGAS[237]
    {0x0F12, 0x0038}, // #TVAR_ash_pGAS[238]
    {0x0F12, 0x0021}, // #TVAR_ash_pGAS[239]
    {0x0F12, 0x001C}, // #TVAR_ash_pGAS[240]
    {0x0F12, 0x0026}, // #TVAR_ash_pGAS[241]
    {0x0F12, 0x003D}, // #TVAR_ash_pGAS[242]
    {0x0F12, 0x0068}, // #TVAR_ash_pGAS[243]
    {0x0F12, 0x00A3}, // #TVAR_ash_pGAS[244]
    {0x0F12, 0x00EE}, // #TVAR_ash_pGAS[245]
    {0x0F12, 0x0138}, // #TVAR_ash_pGAS[246]
    {0x0F12, 0x0151}, // #TVAR_ash_pGAS[247]
    {0x0F12, 0x0102}, // #TVAR_ash_pGAS[248]
    {0x0F12, 0x00BC}, // #TVAR_ash_pGAS[249]
    {0x0F12, 0x0083}, // #TVAR_ash_pGAS[250]
    {0x0F12, 0x005C}, // #TVAR_ash_pGAS[251]
    {0x0F12, 0x0046}, // #TVAR_ash_pGAS[252]
    {0x0F12, 0x0041}, // #TVAR_ash_pGAS[253]
    {0x0F12, 0x004B}, // #TVAR_ash_pGAS[254]
    {0x0F12, 0x0066}, // #TVAR_ash_pGAS[255]
    {0x0F12, 0x0092}, // #TVAR_ash_pGAS[256]
    {0x0F12, 0x00CD}, // #TVAR_ash_pGAS[257]
    {0x0F12, 0x0115}, // #TVAR_ash_pGAS[258]
    {0x0F12, 0x015F}, // #TVAR_ash_pGAS[259]
    {0x0F12, 0x018A}, // #TVAR_ash_pGAS[260]
    {0x0F12, 0x0133}, // #TVAR_ash_pGAS[261]
    {0x0F12, 0x00F0}, // #TVAR_ash_pGAS[262]
    {0x0F12, 0x00BB}, // #TVAR_ash_pGAS[263]
    {0x0F12, 0x0097}, // #TVAR_ash_pGAS[264]
    {0x0F12, 0x007F}, // #TVAR_ash_pGAS[265]
    {0x0F12, 0x0079}, // #TVAR_ash_pGAS[266]
    {0x0F12, 0x0084}, // #TVAR_ash_pGAS[267]
    {0x0F12, 0x00A1}, // #TVAR_ash_pGAS[268]
    {0x0F12, 0x00CD}, // #TVAR_ash_pGAS[269]
    {0x0F12, 0x0105}, // #TVAR_ash_pGAS[270]
    {0x0F12, 0x014C}, // #TVAR_ash_pGAS[271]
    {0x0F12, 0x01A0}, // #TVAR_ash_pGAS[272]
    {0x0F12, 0x01D5}, // #TVAR_ash_pGAS[273]
    {0x0F12, 0x0171}, // #TVAR_ash_pGAS[274]
    {0x0F12, 0x012D}, // #TVAR_ash_pGAS[275]
    {0x0F12, 0x00FE}, // #TVAR_ash_pGAS[276]
    {0x0F12, 0x00D9}, // #TVAR_ash_pGAS[277]
    {0x0F12, 0x00C2}, // #TVAR_ash_pGAS[278]
    {0x0F12, 0x00BD}, // #TVAR_ash_pGAS[279]
    {0x0F12, 0x00C9}, // #TVAR_ash_pGAS[280]
    {0x0F12, 0x00E5}, // #TVAR_ash_pGAS[281]
    {0x0F12, 0x0111}, // #TVAR_ash_pGAS[282]
    {0x0F12, 0x0146}, // #TVAR_ash_pGAS[283]
    {0x0F12, 0x018E}, // #TVAR_ash_pGAS[284]
    {0x0F12, 0x01EE}, // #TVAR_ash_pGAS[285]
    {0x0F12, 0x01CF}, // #TVAR_ash_pGAS[286]
    {0x0F12, 0x0162}, // #TVAR_ash_pGAS[287]
    {0x0F12, 0x011E}, // #TVAR_ash_pGAS[288]
    {0x0F12, 0x00F1}, // #TVAR_ash_pGAS[289]
    {0x0F12, 0x00CF}, // #TVAR_ash_pGAS[290]
    {0x0F12, 0x00B9}, // #TVAR_ash_pGAS[291]
    {0x0F12, 0x00B0}, // #TVAR_ash_pGAS[292]
    {0x0F12, 0x00BB}, // #TVAR_ash_pGAS[293]
    {0x0F12, 0x00D6}, // #TVAR_ash_pGAS[294]
    {0x0F12, 0x00F9}, // #TVAR_ash_pGAS[295]
    {0x0F12, 0x0128}, // #TVAR_ash_pGAS[296]
    {0x0F12, 0x016F}, // #TVAR_ash_pGAS[297]
    {0x0F12, 0x01D3}, // #TVAR_ash_pGAS[298]
    {0x0F12, 0x017B}, // #TVAR_ash_pGAS[299]
    {0x0F12, 0x0127}, // #TVAR_ash_pGAS[300]
    {0x0F12, 0x00E9}, // #TVAR_ash_pGAS[301]
    {0x0F12, 0x00B7}, // #TVAR_ash_pGAS[302]
    {0x0F12, 0x0091}, // #TVAR_ash_pGAS[303]
    {0x0F12, 0x007B}, // #TVAR_ash_pGAS[304]
    {0x0F12, 0x0074}, // #TVAR_ash_pGAS[305]
    {0x0F12, 0x007F}, // #TVAR_ash_pGAS[306]
    {0x0F12, 0x0099}, // #TVAR_ash_pGAS[307]
    {0x0F12, 0x00C2}, // #TVAR_ash_pGAS[308]
    {0x0F12, 0x00F6}, // #TVAR_ash_pGAS[309]
    {0x0F12, 0x0139}, // #TVAR_ash_pGAS[310]
    {0x0F12, 0x018E}, // #TVAR_ash_pGAS[311]
    {0x0F12, 0x014A}, // #TVAR_ash_pGAS[312]
    {0x0F12, 0x00FA}, // #TVAR_ash_pGAS[313]
    {0x0F12, 0x00B9}, // #TVAR_ash_pGAS[314]
    {0x0F12, 0x0084}, // #TVAR_ash_pGAS[315]
    {0x0F12, 0x005D}, // #TVAR_ash_pGAS[316]
    {0x0F12, 0x0046}, // #TVAR_ash_pGAS[317]
    {0x0F12, 0x003E}, // #TVAR_ash_pGAS[318]
    {0x0F12, 0x0049}, // #TVAR_ash_pGAS[319]
    {0x0F12, 0x0061}, // #TVAR_ash_pGAS[320]
    {0x0F12, 0x008C}, // #TVAR_ash_pGAS[321]
    {0x0F12, 0x00C5}, // #TVAR_ash_pGAS[322]
    {0x0F12, 0x0107}, // #TVAR_ash_pGAS[323]
    {0x0F12, 0x0155}, // #TVAR_ash_pGAS[324]
    {0x0F12, 0x0129}, // #TVAR_ash_pGAS[325]
    {0x0F12, 0x00DA}, // #TVAR_ash_pGAS[326]
    {0x0F12, 0x0096}, // #TVAR_ash_pGAS[327]
    {0x0F12, 0x005F}, // #TVAR_ash_pGAS[328]
    {0x0F12, 0x0037}, // #TVAR_ash_pGAS[329]
    {0x0F12, 0x0021}, // #TVAR_ash_pGAS[330]
    {0x0F12, 0x001A}, // #TVAR_ash_pGAS[331]
    {0x0F12, 0x0023}, // #TVAR_ash_pGAS[332]
    {0x0F12, 0x003B}, // #TVAR_ash_pGAS[333]
    {0x0F12, 0x0065}, // #TVAR_ash_pGAS[334]
    {0x0F12, 0x009F}, // #TVAR_ash_pGAS[335]
    {0x0F12, 0x00E5}, // #TVAR_ash_pGAS[336]
    {0x0F12, 0x012F}, // #TVAR_ash_pGAS[337]
    {0x0F12, 0x0116}, // #TVAR_ash_pGAS[338]
    {0x0F12, 0x00C7}, // #TVAR_ash_pGAS[339]
    {0x0F12, 0x0080}, // #TVAR_ash_pGAS[340]
    {0x0F12, 0x0049}, // #TVAR_ash_pGAS[341]
    {0x0F12, 0x0022}, // #TVAR_ash_pGAS[342]
    {0x0F12, 0x000C}, // #TVAR_ash_pGAS[343]
    {0x0F12, 0x0006}, // #TVAR_ash_pGAS[344]
    {0x0F12, 0x000E}, // #TVAR_ash_pGAS[345]
    {0x0F12, 0x0026}, // #TVAR_ash_pGAS[346]
    {0x0F12, 0x004E}, // #TVAR_ash_pGAS[347]
    {0x0F12, 0x0086}, // #TVAR_ash_pGAS[348]
    {0x0F12, 0x00CF}, // #TVAR_ash_pGAS[349]
    {0x0F12, 0x011C}, // #TVAR_ash_pGAS[350]
    {0x0F12, 0x0113}, // #TVAR_ash_pGAS[351]
    {0x0F12, 0x00C4}, // #TVAR_ash_pGAS[352]
    {0x0F12, 0x007A}, // #TVAR_ash_pGAS[353]
    {0x0F12, 0x0042}, // #TVAR_ash_pGAS[354]
    {0x0F12, 0x001D}, // #TVAR_ash_pGAS[355]
    {0x0F12, 0x0007}, // #TVAR_ash_pGAS[356]
    {0x0F12, 0x0000}, // #TVAR_ash_pGAS[357]
    {0x0F12, 0x0008}, // #TVAR_ash_pGAS[358]
    {0x0F12, 0x001F}, // #TVAR_ash_pGAS[359]
    {0x0F12, 0x0045}, // #TVAR_ash_pGAS[360]
    {0x0F12, 0x007F}, // #TVAR_ash_pGAS[361]
    {0x0F12, 0x00C9}, // #TVAR_ash_pGAS[362]
    {0x0F12, 0x0112}, // #TVAR_ash_pGAS[363]
    {0x0F12, 0x011C}, // #TVAR_ash_pGAS[364]
    {0x0F12, 0x00CF}, // #TVAR_ash_pGAS[365]
    {0x0F12, 0x0086}, // #TVAR_ash_pGAS[366]
    {0x0F12, 0x004C}, // #TVAR_ash_pGAS[367]
    {0x0F12, 0x0025}, // #TVAR_ash_pGAS[368]
    {0x0F12, 0x000F}, // #TVAR_ash_pGAS[369]
    {0x0F12, 0x0007}, // #TVAR_ash_pGAS[370]
    {0x0F12, 0x000E}, // #TVAR_ash_pGAS[371]
    {0x0F12, 0x0025}, // #TVAR_ash_pGAS[372]
    {0x0F12, 0x004B}, // #TVAR_ash_pGAS[373]
    {0x0F12, 0x0084}, // #TVAR_ash_pGAS[374]
    {0x0F12, 0x00CD}, // #TVAR_ash_pGAS[375]
    {0x0F12, 0x0115}, // #TVAR_ash_pGAS[376]
    {0x0F12, 0x0134}, // #TVAR_ash_pGAS[377]
    {0x0F12, 0x00E7}, // #TVAR_ash_pGAS[378]
    {0x0F12, 0x009E}, // #TVAR_ash_pGAS[379]
    {0x0F12, 0x0065}, // #TVAR_ash_pGAS[380]
    {0x0F12, 0x003C}, // #TVAR_ash_pGAS[381]
    {0x0F12, 0x0024}, // #TVAR_ash_pGAS[382]
    {0x0F12, 0x001B}, // #TVAR_ash_pGAS[383]
    {0x0F12, 0x0022}, // #TVAR_ash_pGAS[384]
    {0x0F12, 0x0039}, // #TVAR_ash_pGAS[385]
    {0x0F12, 0x0062}, // #TVAR_ash_pGAS[386]
    {0x0F12, 0x0099}, // #TVAR_ash_pGAS[387]
    {0x0F12, 0x00DF}, // #TVAR_ash_pGAS[388]
    {0x0F12, 0x0126}, // #TVAR_ash_pGAS[389]
    {0x0F12, 0x0157}, // #TVAR_ash_pGAS[390]
    {0x0F12, 0x010C}, // #TVAR_ash_pGAS[391]
    {0x0F12, 0x00C6}, // #TVAR_ash_pGAS[392]
    {0x0F12, 0x008D}, // #TVAR_ash_pGAS[393]
    {0x0F12, 0x0063}, // #TVAR_ash_pGAS[394]
    {0x0F12, 0x0049}, // #TVAR_ash_pGAS[395]
    {0x0F12, 0x0041}, // #TVAR_ash_pGAS[396]
    {0x0F12, 0x0047}, // #TVAR_ash_pGAS[397]
    {0x0F12, 0x005F}, // #TVAR_ash_pGAS[398]
    {0x0F12, 0x0087}, // #TVAR_ash_pGAS[399]
    {0x0F12, 0x00BF}, // #TVAR_ash_pGAS[400]
    {0x0F12, 0x0100}, // #TVAR_ash_pGAS[401]
    {0x0F12, 0x0149}, // #TVAR_ash_pGAS[402]
    {0x0F12, 0x0194}, // #TVAR_ash_pGAS[403]
    {0x0F12, 0x0140}, // #TVAR_ash_pGAS[404]
    {0x0F12, 0x00FF}, // #TVAR_ash_pGAS[405]
    {0x0F12, 0x00CA}, // #TVAR_ash_pGAS[406]
    {0x0F12, 0x00A0}, // #TVAR_ash_pGAS[407]
    {0x0F12, 0x0083}, // #TVAR_ash_pGAS[408]
    {0x0F12, 0x007B}, // #TVAR_ash_pGAS[409]
    {0x0F12, 0x007F}, // #TVAR_ash_pGAS[410]
    {0x0F12, 0x0097}, // #TVAR_ash_pGAS[411]
    {0x0F12, 0x00BE}, // #TVAR_ash_pGAS[412]
    {0x0F12, 0x00F2}, // #TVAR_ash_pGAS[413]
    {0x0F12, 0x0131}, // #TVAR_ash_pGAS[414]
    {0x0F12, 0x0181}, // #TVAR_ash_pGAS[415]
    {0x0F12, 0x01E4}, // #TVAR_ash_pGAS[416]
    {0x0F12, 0x017E}, // #TVAR_ash_pGAS[417]
    {0x0F12, 0x013B}, // #TVAR_ash_pGAS[418]
    {0x0F12, 0x010C}, // #TVAR_ash_pGAS[419]
    {0x0F12, 0x00E5}, // #TVAR_ash_pGAS[420]
    {0x0F12, 0x00CA}, // #TVAR_ash_pGAS[421]
    {0x0F12, 0x00BF}, // #TVAR_ash_pGAS[422]
    {0x0F12, 0x00C4}, // #TVAR_ash_pGAS[423]
    {0x0F12, 0x00D8}, // #TVAR_ash_pGAS[424]
    {0x0F12, 0x00FE}, // #TVAR_ash_pGAS[425]
    {0x0F12, 0x012D}, // #TVAR_ash_pGAS[426]
    {0x0F12, 0x016E}, // #TVAR_ash_pGAS[427]
    {0x0F12, 0x01CC}, // #TVAR_ash_pGAS[428]
    {0x0F12, 0x0194}, // #TVAR_ash_pGAS[429]
    {0x0F12, 0x0138}, // #TVAR_ash_pGAS[430]
    {0x0F12, 0x00FA}, // #TVAR_ash_pGAS[431]
    {0x0F12, 0x00D2}, // #TVAR_ash_pGAS[432]
    {0x0F12, 0x00B5}, // #TVAR_ash_pGAS[433]
    {0x0F12, 0x00A4}, // #TVAR_ash_pGAS[434]
    {0x0F12, 0x009E}, // #TVAR_ash_pGAS[435]
    {0x0F12, 0x00A7}, // #TVAR_ash_pGAS[436]
    {0x0F12, 0x00BC}, // #TVAR_ash_pGAS[437]
    {0x0F12, 0x00DC}, // #TVAR_ash_pGAS[438]
    {0x0F12, 0x0106}, // #TVAR_ash_pGAS[439]
    {0x0F12, 0x0147}, // #TVAR_ash_pGAS[440]
    {0x0F12, 0x0199}, // #TVAR_ash_pGAS[441]
    {0x0F12, 0x0145}, // #TVAR_ash_pGAS[442]
    {0x0F12, 0x0101}, // #TVAR_ash_pGAS[443]
    {0x0F12, 0x00CA}, // #TVAR_ash_pGAS[444]
    {0x0F12, 0x00A0}, // #TVAR_ash_pGAS[445]
    {0x0F12, 0x0081}, // #TVAR_ash_pGAS[446]
    {0x0F12, 0x006D}, // #TVAR_ash_pGAS[447]
    {0x0F12, 0x0069}, // #TVAR_ash_pGAS[448]
    {0x0F12, 0x0072}, // #TVAR_ash_pGAS[449]
    {0x0F12, 0x0089}, // #TVAR_ash_pGAS[450]
    {0x0F12, 0x00AD}, // #TVAR_ash_pGAS[451]
    {0x0F12, 0x00DA}, // #TVAR_ash_pGAS[452]
    {0x0F12, 0x0117}, // #TVAR_ash_pGAS[453]
    {0x0F12, 0x0160}, // #TVAR_ash_pGAS[454]
    {0x0F12, 0x0117}, // #TVAR_ash_pGAS[455]
    {0x0F12, 0x00D7}, // #TVAR_ash_pGAS[456]
    {0x0F12, 0x009D}, // #TVAR_ash_pGAS[457]
    {0x0F12, 0x0070}, // #TVAR_ash_pGAS[458]
    {0x0F12, 0x0050}, // #TVAR_ash_pGAS[459]
    {0x0F12, 0x003F}, // #TVAR_ash_pGAS[460]
    {0x0F12, 0x0038}, // #TVAR_ash_pGAS[461]
    {0x0F12, 0x0041}, // #TVAR_ash_pGAS[462]
    {0x0F12, 0x0057}, // #TVAR_ash_pGAS[463]
    {0x0F12, 0x0079}, // #TVAR_ash_pGAS[464]
    {0x0F12, 0x00AD}, // #TVAR_ash_pGAS[465]
    {0x0F12, 0x00EA}, // #TVAR_ash_pGAS[466]
    {0x0F12, 0x0129}, // #TVAR_ash_pGAS[467]
    {0x0F12, 0x00F4}, // #TVAR_ash_pGAS[468]
    {0x0F12, 0x00B7}, // #TVAR_ash_pGAS[469]
    {0x0F12, 0x007C}, // #TVAR_ash_pGAS[470]
    {0x0F12, 0x004E}, // #TVAR_ash_pGAS[471]
    {0x0F12, 0x002F}, // #TVAR_ash_pGAS[472]
    {0x0F12, 0x001D}, // #TVAR_ash_pGAS[473]
    {0x0F12, 0x0018}, // #TVAR_ash_pGAS[474]
    {0x0F12, 0x001F}, // #TVAR_ash_pGAS[475]
    {0x0F12, 0x0033}, // #TVAR_ash_pGAS[476]
    {0x0F12, 0x0056}, // #TVAR_ash_pGAS[477]
    {0x0F12, 0x0088}, // #TVAR_ash_pGAS[478]
    {0x0F12, 0x00C6}, // #TVAR_ash_pGAS[479]
    {0x0F12, 0x0104}, // #TVAR_ash_pGAS[480]
    {0x0F12, 0x00E2}, // #TVAR_ash_pGAS[481]
    {0x0F12, 0x00A5}, // #TVAR_ash_pGAS[482]
    {0x0F12, 0x0066}, // #TVAR_ash_pGAS[483]
    {0x0F12, 0x0038}, // #TVAR_ash_pGAS[484]
    {0x0F12, 0x001A}, // #TVAR_ash_pGAS[485]
    {0x0F12, 0x0009}, // #TVAR_ash_pGAS[486]
    {0x0F12, 0x0006}, // #TVAR_ash_pGAS[487]
    {0x0F12, 0x000C}, // #TVAR_ash_pGAS[488]
    {0x0F12, 0x001E}, // #TVAR_ash_pGAS[489]
    {0x0F12, 0x003F}, // #TVAR_ash_pGAS[490]
    {0x0F12, 0x006F}, // #TVAR_ash_pGAS[491]
    {0x0F12, 0x00B2}, // #TVAR_ash_pGAS[492]
    {0x0F12, 0x00EE}, // #TVAR_ash_pGAS[493]
    {0x0F12, 0x00DD}, // #TVAR_ash_pGAS[494]
    {0x0F12, 0x009E}, // #TVAR_ash_pGAS[495]
    {0x0F12, 0x0060}, // #TVAR_ash_pGAS[496]
    {0x0F12, 0x0030}, // #TVAR_ash_pGAS[497]
    {0x0F12, 0x0013}, // #TVAR_ash_pGAS[498]
    {0x0F12, 0x0004}, // #TVAR_ash_pGAS[499]
    {0x0F12, 0x0000}, // #TVAR_ash_pGAS[500]
    {0x0F12, 0x0005}, // #TVAR_ash_pGAS[501]
    {0x0F12, 0x0017}, // #TVAR_ash_pGAS[502]
    {0x0F12, 0x0036}, // #TVAR_ash_pGAS[503]
    {0x0F12, 0x0066}, // #TVAR_ash_pGAS[504]
    {0x0F12, 0x00A7}, // #TVAR_ash_pGAS[505]
    {0x0F12, 0x00E4}, // #TVAR_ash_pGAS[506]
    {0x0F12, 0x00E5}, // #TVAR_ash_pGAS[507]
    {0x0F12, 0x00A6}, // #TVAR_ash_pGAS[508]
    {0x0F12, 0x0067}, // #TVAR_ash_pGAS[509]
    {0x0F12, 0x0039}, // #TVAR_ash_pGAS[510]
    {0x0F12, 0x001B}, // #TVAR_ash_pGAS[511]
    {0x0F12, 0x000B}, // #TVAR_ash_pGAS[512]
    {0x0F12, 0x0006}, // #TVAR_ash_pGAS[513]
    {0x0F12, 0x000B}, // #TVAR_ash_pGAS[514]
    {0x0F12, 0x001C}, // #TVAR_ash_pGAS[515]
    {0x0F12, 0x003B}, // #TVAR_ash_pGAS[516]
    {0x0F12, 0x006B}, // #TVAR_ash_pGAS[517]
    {0x0F12, 0x00AC}, // #TVAR_ash_pGAS[518]
    {0x0F12, 0x00E6}, // #TVAR_ash_pGAS[519]
    {0x0F12, 0x00F9}, // #TVAR_ash_pGAS[520]
    {0x0F12, 0x00BC}, // #TVAR_ash_pGAS[521]
    {0x0F12, 0x007E}, // #TVAR_ash_pGAS[522]
    {0x0F12, 0x0051}, // #TVAR_ash_pGAS[523]
    {0x0F12, 0x0030}, // #TVAR_ash_pGAS[524]
    {0x0F12, 0x001E}, // #TVAR_ash_pGAS[525]
    {0x0F12, 0x0018}, // #TVAR_ash_pGAS[526]
    {0x0F12, 0x001E}, // #TVAR_ash_pGAS[527]
    {0x0F12, 0x002F}, // #TVAR_ash_pGAS[528]
    {0x0F12, 0x0050}, // #TVAR_ash_pGAS[529]
    {0x0F12, 0x007E}, // #TVAR_ash_pGAS[530]
    {0x0F12, 0x00BD}, // #TVAR_ash_pGAS[531]
    {0x0F12, 0x00F7}, // #TVAR_ash_pGAS[532]
    {0x0F12, 0x011A}, // #TVAR_ash_pGAS[533]
    {0x0F12, 0x00DF}, // #TVAR_ash_pGAS[534]
    {0x0F12, 0x00A5}, // #TVAR_ash_pGAS[535]
    {0x0F12, 0x0076}, // #TVAR_ash_pGAS[536]
    {0x0F12, 0x0054}, // #TVAR_ash_pGAS[537]
    {0x0F12, 0x0041}, // #TVAR_ash_pGAS[538]
    {0x0F12, 0x003A}, // #TVAR_ash_pGAS[539]
    {0x0F12, 0x003E}, // #TVAR_ash_pGAS[540]
    {0x0F12, 0x0051}, // #TVAR_ash_pGAS[541]
    {0x0F12, 0x0073}, // #TVAR_ash_pGAS[542]
    {0x0F12, 0x00A1}, // #TVAR_ash_pGAS[543]
    {0x0F12, 0x00DB}, // #TVAR_ash_pGAS[544]
    {0x0F12, 0x0115}, // #TVAR_ash_pGAS[545]
    {0x0F12, 0x0157}, // #TVAR_ash_pGAS[546]
    {0x0F12, 0x0112}, // #TVAR_ash_pGAS[547]
    {0x0F12, 0x00DA}, // #TVAR_ash_pGAS[548]
    {0x0F12, 0x00AE}, // #TVAR_ash_pGAS[549]
    {0x0F12, 0x008D}, // #TVAR_ash_pGAS[550]
    {0x0F12, 0x0078}, // #TVAR_ash_pGAS[551]
    {0x0F12, 0x006F}, // #TVAR_ash_pGAS[552]
    {0x0F12, 0x0074}, // #TVAR_ash_pGAS[553]
    {0x0F12, 0x0087}, // #TVAR_ash_pGAS[554]
    {0x0F12, 0x00A7}, // #TVAR_ash_pGAS[555]
    {0x0F12, 0x00D4}, // #TVAR_ash_pGAS[556]
    {0x0F12, 0x010E}, // #TVAR_ash_pGAS[557]
    {0x0F12, 0x014F}, // #TVAR_ash_pGAS[558]
    {0x0F12, 0x0195}, // #TVAR_ash_pGAS[559]
    {0x0F12, 0x0147}, // #TVAR_ash_pGAS[560]
    {0x0F12, 0x0111}, // #TVAR_ash_pGAS[561]
    {0x0F12, 0x00E6}, // #TVAR_ash_pGAS[562]
    {0x0F12, 0x00C9}, // #TVAR_ash_pGAS[563]
    {0x0F12, 0x00B4}, // #TVAR_ash_pGAS[564]
    {0x0F12, 0x00AA}, // #TVAR_ash_pGAS[565]
    {0x0F12, 0x00AE}, // #TVAR_ash_pGAS[566]
    {0x0F12, 0x00C0}, // #TVAR_ash_pGAS[567]
    {0x0F12, 0x00DF}, // #TVAR_ash_pGAS[568]
    {0x0F12, 0x0106}, // #TVAR_ash_pGAS[569]
    {0x0F12, 0x0143}, // #TVAR_ash_pGAS[570]
    {0x0F12, 0x018F}, // #TVAR_ash_pGAS[571]
    {0x002A, 0x0D30},
    {0x0F12, 0x02A7}, // #awbb_GLocusR
    {0x0F12, 0x0343}, // #awbb_GLocusB
    {0x002A, 0x06B8},
    {0x0F12, 0x00C0}, // #TVAR_ash_AwbAshCord_0_
    {0x0F12, 0x00E0}, // #TVAR_ash_AwbAshCord_1_
    {0x0F12, 0x00FA}, // #TVAR_ash_AwbAshCord_2_
    {0x0F12, 0x011D}, // #TVAR_ash_AwbAshCord_3_
    {0x0F12, 0x0144}, // #TVAR_ash_AwbAshCord_4_
    {0x0F12, 0x0173}, // #TVAR_ash_AwbAshCord_5_
    {0x0F12, 0x0180}, // #TVAR_ash_AwbAshCord_6_
    //================================================================================================
    // SET CCM
    //================================================================================================
    // CCM start address // 7000_33A4
    {0x002A, 0x0698},
    {0x0F12, 0x33A4},
    {0x0F12, 0x7000},
    {0x002A, 0x33A4},
    {0x0F12, 0x01C3}, //01CB //#TVAR_wbt_pBaseCcms// Horizon
    {0x0F12, 0xFF89}, //FF8E //#TVAR_wbt_pBaseCcms
    {0x0F12, 0xFFE5}, //FFD2 //#TVAR_wbt_pBaseCcms
    {0x0F12, 0xFF26}, //FF3C  //FF64 //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x028E}, //0305  //01B2 //#TVAR_wbt_pBaseCcms
    {0x0F12, 0xFF80}, //FED8  //FF35 //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x0002}, //FFEC  //FFDF //#TVAR_wbt_pBaseCcms
    {0x0F12, 0xFFA8}, //FFFD  //FFE9 //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x01F0}, //0354  //01BD //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x0125}, //011C //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x0119}, //011B //#TVAR_wbt_pBaseCcms
    {0x0F12, 0xFE5A}, //FF43 //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x0179}, //019D //#TVAR_wbt_pBaseCcms
    {0x0F12, 0xFF8A}, //FF4C //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x0180}, //01CC //#TVAR_wbt_pBaseCcms
    {0x0F12, 0xFEC2}, //FF33 //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x0176}, //0173 //#TVAR_wbt_pBaseCcms
    {0x0F12, 0x0094}, //#TVAR_wbt_pBaseCcms /

    {0x0F12, 0x01C3}, //0206 //01C8 //#TVAR_wbt_pBaseCcms[18]// Inca
    {0x0F12, 0xFF89}, //FF84 //FF7F //#TVAR_wbt_pBaseCcms[19]
    {0x0F12, 0xFFE5}, //FFC7 //FFE4 //#TVAR_wbt_pBaseCcms[20]
    {0x0F12, 0xFF26}, //FF3C  //FF42 //FF64 //#TVAR_wbt_pBaseCcms[21]
    {0x0F12, 0x028E}, //0305  //0319 //01B2 //#TVAR_wbt_pBaseCcms[22]
    {0x0F12, 0xFF80}, //FED8  //FEEE //FF35 //#TVAR_wbt_pBaseCcms[23]
    {0x0F12, 0x0002}, //FFEC  //001F //FFDF //#TVAR_wbt_pBaseCcms[24]
    {0x0F12, 0xFFA8}, //FFFD  //001E //FFE9 //#TVAR_wbt_pBaseCcms[25]
    {0x0F12, 0x01F0}, //0354  //0396 //01BD //#TVAR_wbt_pBaseCcms[26]
    {0x0F12, 0x0125}, //0184 //011C //#TVAR_wbt_pBaseCcms[27]
    {0x0F12, 0x0119}, //018A //011B //#TVAR_wbt_pBaseCcms[28]
    {0x0F12, 0xFE5A}, //FDBA //FF43 //#TVAR_wbt_pBaseCcms[29]
    {0x0F12, 0x0179}, //0215 //019D //#TVAR_wbt_pBaseCcms[30] //
    {0x0F12, 0xFF8A}, //FEC1 //FF4C //#TVAR_wbt_pBaseCcms[31]
    {0x0F12, 0x0180}, //0356 //01CC //#TVAR_wbt_pBaseCcms[32]
    {0x0F12, 0xFEC2}, //FF89 //FF33 //#TVAR_wbt_pBaseCcms[33]
    {0x0F12, 0x0176}, //021A //0173 //#TVAR_wbt_pBaseCcms[34]
    {0x0F12, 0x0094}, //008E //012F //#TVAR_wbt_pBaseCcms[35]

    {0x0F12, 0x01CA}, //01F4  //01C8 //#TVAR_wbt_pBaseCcms[36]
    {0x0F12, 0xFF89}, //FF6B  //FF7F //#TVAR_wbt_pBaseCcms[37]
    {0x0F12, 0xFFE0}, //FFD9  //FFE4 //#TVAR_wbt_pBaseCcms[38]
    {0x0F12, 0xFF26}, //FF66  //FF64 //#TVAR_wbt_pBaseCcms[39]
    {0x0F12, 0x028E}, //01FA  //01B2 //#TVAR_wbt_pBaseCcms[40]
    {0x0F12, 0xFF80}, //FEF8  //FF35 //#TVAR_wbt_pBaseCcms[41]
    {0x0F12, 0x0020}, //FFB8  //FFDF //#TVAR_wbt_pBaseCcms[42]
    {0x0F12, 0xFFF8}, //FFE3  //FFE9 //#TVAR_wbt_pBaseCcms[43]
    {0x0F12, 0x01E0}, //01F7  //01BD //#TVAR_wbt_pBaseCcms[44]
    {0x0F12, 0x0120}, //0290  //011C //#TVAR_wbt_pBaseCcms[45]
    {0x0F12, 0x00FA}, //0189  //011B //#TVAR_wbt_pBaseCcms[46]
    {0x0F12, 0xFF12}, //FD6F  //FF43 //#TVAR_wbt_pBaseCcms[47]
    {0x0F12, 0x0179}, //019D //#TVAR_wbt_pBaseCcms[48]
    {0x0F12, 0xFF8A}, //FF4C //#TVAR_wbt_pBaseCcms[49]
    {0x0F12, 0x0180}, //01CC //#TVAR_wbt_pBaseCcms[50]
    {0x0F12, 0xFEC2}, //FEF8  //FF33 //#TVAR_wbt_pBaseCcms[51]
    {0x0F12, 0x0176}, //019B  //0173 //#TVAR_wbt_pBaseCcms[52]
    {0x0F12, 0x0094}, //014E  //012F //#TVAR_wbt_pBaseCcms[53]
    // CWF
    {0x0F12, 0x01CA}, //01C8  //01C8 //#TVAR_wbt_pBaseCcms[54]
    {0x0F12, 0xFF89}, //FFA5  //FF7F //#TVAR_wbt_pBaseCcms[55]
    {0x0F12, 0xFFE0}, //FFFC  //FFE4 //#TVAR_wbt_pBaseCcms[56]
    {0x0F12, 0xFF26}, //FFAC  //FF64 //#TVAR_wbt_pBaseCcms[57]
    {0x0F12, 0x028E}, //0229  //01B2 //#TVAR_wbt_pBaseCcms[58]
    {0x0F12, 0xFF80}, //FF0E  //FF35 //#TVAR_wbt_pBaseCcms[59]
    {0x0F12, 0x0020}, //FF89  //FFDF //#TVAR_wbt_pBaseCcms[60]
    {0x0F12, 0xFFF8}, //0010  //FFE9 //#TVAR_wbt_pBaseCcms[61]
    {0x0F12, 0x01E0}, //0314  //01BD //#TVAR_wbt_pBaseCcms[62]
    {0x0F12, 0x0120}, //0252  //011C //#TVAR_wbt_pBaseCcms[63]
    {0x0F12, 0x00FA}, //00F5  //011B //#TVAR_wbt_pBaseCcms[64]
    {0x0F12, 0xFF12}, //FE15  //FF43 //#TVAR_wbt_pBaseCcms[65]
    {0x0F12, 0x0179}, //#TVAR_wbt_pBaseCcms[66]
    {0x0F12, 0xFF8A}, //#TVAR_wbt_pBaseCcms[67]
    {0x0F12, 0x0180}, //#TVAR_wbt_pBaseCcms[68]
    {0x0F12, 0xFEC2}, //FF32  //FF33 //#TVAR_wbt_pBaseCcms[69]
    {0x0F12, 0x0176}, //019E  //0173 //#TVAR_wbt_pBaseCcms[70]
    {0x0F12, 0x0094}, //0159  //012F //#TVAR_wbt_pBaseCcms[71]
    // D50
    {0x0F12, 0x018A}, //01C8  //#TVAR_wbt_pBaseCcms[72]
    {0x0F12, 0xFFC9}, //FF7F  //#TVAR_wbt_pBaseCcms[73]
    {0x0F12, 0x0005}, //FFE4  //#TVAR_wbt_pBaseCcms[74]
    {0x0F12, 0xFF56}, //#TVAR_wbt_pBaseCcms[75]
    {0x0F12, 0x0230}, //#TVAR_wbt_pBaseCcms[76]
    {0x0F12, 0xFFA0}, //#TVAR_wbt_pBaseCcms[77]
    {0x0F12, 0x0020}, //FFDF  //#TVAR_wbt_pBaseCcms[78]
    {0x0F12, 0xFFF8}, //FFE9  //#TVAR_wbt_pBaseCcms[79]
    {0x0F12, 0x01E0}, //01BD  //#TVAR_wbt_pBaseCcms[80]
    {0x0F12, 0x00FF}, //011C  //#TVAR_wbt_pBaseCcms[81]
    {0x0F12, 0x00E2}, //011B  //#TVAR_wbt_pBaseCcms[82]
    {0x0F12, 0xFF4D}, //FF43  //#TVAR_wbt_pBaseCcms[83]
    {0x0F12, 0x0179}, //#TVAR_wbt_pBaseCcms[84]
    {0x0F12, 0xFF8A}, //#TVAR_wbt_pBaseCcms[85]
    {0x0F12, 0x0180}, //#TVAR_wbt_pBaseCcms[86]
    {0x0F12, 0xFEC2}, //#TVAR_wbt_pBaseCcms[87]
    {0x0F12, 0x0176}, //#TVAR_wbt_pBaseCcms[88]
    {0x0F12, 0x0094}, //#TVAR_wbt_pBaseCcms[89]
    //  D65
    {0x0F12, 0x018A}, //01DC  //01C8 //#TVAR_wbt_pBaseCcms[90]
    {0x0F12, 0xFFC9}, //FF76  //FF7F //#TVAR_wbt_pBaseCcms[91]
    {0x0F12, 0x0005}, //FFE0  //FFE4 //#TVAR_wbt_pBaseCcms[92]
    {0x0F12, 0xFF56}, //FF4F  //#TVAR_wbt_pBaseCcms[93]
    {0x0F12, 0x0230}, //01F5  //#TVAR_wbt_pBaseCcms[94]
    {0x0F12, 0xFFA0}, //FF77  //#TVAR_wbt_pBaseCcms[95]
    {0x0F12, 0x0020}, //FFCA  //FFDF //#TVAR_wbt_pBaseCcms[96]
    {0x0F12, 0xFFF8}, //FFD5  //FFE9 //#TVAR_wbt_pBaseCcms[97]
    {0x0F12, 0x01E0}, //01EC  //01BD //#TVAR_wbt_pBaseCcms[98]
    {0x0F12, 0x00FF}, //0126  //011C //#TVAR_wbt_pBaseCcms[99]
    {0x0F12, 0x00E2}, //0125  //011B //#TVAR_wbt_pBaseCcms[100]
    {0x0F12, 0xFF4D}, //FF35  //FF43 //#TVAR_wbt_pBaseCcms[101]
    {0x0F12, 0x0179}, //01BE  //#TVAR_wbt_pBaseCcms[102]
    {0x0F12, 0xFF8A}, //FF44  //#TVAR_wbt_pBaseCcms[103]
    {0x0F12, 0x0180}, //013E  //#TVAR_wbt_pBaseCcms[104]
    {0x0F12, 0xFEC2}, //FF2E  //#TVAR_wbt_pBaseCcms[105]
    {0x0F12, 0x0176}, //01C0  //#TVAR_wbt_pBaseCcms[106]
    {0x0F12, 0x0094}, //011C  //#TVAR_wbt_pBaseCcms[107]
    {0x002A, 0x06A0}, // Outdoor CCM address // 7000_3380
    {0x0F12, 0x3380},
    {0x0F12, 0x7000},
    {0x002A, 0x3380}, // Outdoor CCM
    {0x0F12, 0x01E0}, //#TVAR_wbt_pOutdoorCcm[0]
    {0x0F12, 0xFF80}, //#TVAR_wbt_pOutdoorCcm[1]
    {0x0F12, 0xFFD0}, //#TVAR_wbt_pOutdoorCcm[2]
    {0x0F12, 0xFF61}, //#TVAR_wbt_pOutdoorCcm[3]
    {0x0F12, 0x01BD}, //#TVAR_wbt_pOutdoorCcm[4]
    {0x0F12, 0xFF34}, //#TVAR_wbt_pOutdoorCcm[5]
    {0x0F12, 0xFFFE}, //#TVAR_wbt_pOutdoorCcm[6]
    {0x0F12, 0xFFF6}, //#TVAR_wbt_pOutdoorCcm[7]
    {0x0F12, 0x019D}, //#TVAR_wbt_pOutdoorCcm[8]
    {0x0F12, 0x0107}, //#TVAR_wbt_pOutdoorCcm[9]
    {0x0F12, 0x010F}, //#TVAR_wbt_pOutdoorCcm[10]
    {0x0F12, 0xFF67}, //#TVAR_wbt_pOutdoorCcm[11]
    {0x0F12, 0x016C}, //#TVAR_wbt_pOutdoorCcm[12]
    {0x0F12, 0xFF54}, //#TVAR_wbt_pOutdoorCcm[13]
    {0x0F12, 0x01FC}, //#TVAR_wbt_pOutdoorCcm[14]
    {0x0F12, 0xFF82}, //#TVAR_wbt_pOutdoorCcm[15]
    {0x0F12, 0x015D}, //#TVAR_wbt_pOutdoorCcm[16]
    {0x0F12, 0x00FD}, //#TVAR_wbt_pOutdoorCcm[17]
    //White balance
    //AWB Rgain initial value
    //{0x002A   0E44//
    //{0x0F12   05B0//06CC//053C //awbb_GainsInit[0] R
    //{0x0F12   0400//0400 //awbb_GainsInit[1] G
    //{0x0F12   05A0//055C//0600//055C //awbb_GainsInit[2] B

    //AWB offset
    //{0x002A   0E36
    //{0x0F12   0030 //R	0x0030
    //{0x0F12   FFD0 //B
    //{0x0F12   0000 //G

    // param_start awbb_IndoorGrZones_m_BGrid
    {0x002A, 0x0C48},
    {0x0F12, 0x038B}, //awbb_IndoorGrZones_m_BGrid[0]
    {0x0F12, 0x03C0}, //awbb_IndoorGrZones_m_BGrid[1]
    {0x0F12, 0x033D}, //awbb_IndoorGrZones_m_BGrid[2]
    {0x0F12, 0x03C5}, //awbb_IndoorGrZones_m_BGrid[3]
    {0x0F12, 0x0303}, //awbb_IndoorGrZones_m_BGrid[4]
    {0x0F12, 0x03AE}, //awbb_IndoorGrZones_m_BGrid[5]
    {0x0F12, 0x02CF}, //awbb_IndoorGrZones_m_BGrid[6]
    {0x0F12, 0x0387}, //awbb_IndoorGrZones_m_BGrid[7]
    {0x0F12, 0x02A0}, //awbb_IndoorGrZones_m_BGrid[8]
    {0x0F12, 0x0360}, //awbb_IndoorGrZones_m_BGrid[9]
    {0x0F12, 0x027C}, //awbb_IndoorGrZones_m_BGrid[10]
    {0x0F12, 0x0335}, //awbb_IndoorGrZones_m_BGrid[11]
    {0x0F12, 0x025D}, //awbb_IndoorGrZones_m_BGrid[12]
    {0x0F12, 0x030A}, //awbb_IndoorGrZones_m_BGrid[13]
    {0x0F12, 0x0243}, //awbb_IndoorGrZones_m_BGrid[14]
    {0x0F12, 0x02E5}, //awbb_IndoorGrZones_m_BGrid[15]
    {0x0F12, 0x0227}, //awbb_IndoorGrZones_m_BGrid[16]
    {0x0F12, 0x02BD}, //awbb_IndoorGrZones_m_BGrid[17]
    {0x0F12, 0x020E}, //awbb_IndoorGrZones_m_BGrid[18]
    {0x0F12, 0x029E}, //awbb_IndoorGrZones_m_BGrid[19]
    {0x0F12, 0x01F7}, //awbb_IndoorGrZones_m_BGrid[20]
    {0x0F12, 0x027F}, //awbb_IndoorGrZones_m_BGrid[21]
    {0x0F12, 0x01E3}, //awbb_IndoorGrZones_m_BGrid[22]
    {0x0F12, 0x0262}, //awbb_IndoorGrZones_m_BGrid[23]
    {0x0F12, 0x01D1}, //awbb_IndoorGrZones_m_BGrid[24]
    {0x0F12, 0x024D}, //awbb_IndoorGrZones_m_BGrid[25]
    {0x0F12, 0x01BD}, //awbb_IndoorGrZones_m_BGrid[26]
    {0x0F12, 0x0232}, //awbb_IndoorGrZones_m_BGrid[27]
    {0x0F12, 0x01B2}, //awbb_IndoorGrZones_m_BGrid[28]
    {0x0F12, 0x021A}, //awbb_IndoorGrZones_m_BGrid[29]
    {0x0F12, 0x01B3}, //awbb_IndoorGrZones_m_BGrid[30]
    {0x0F12, 0x0201}, //awbb_IndoorGrZones_m_BGrid[31]
    {0x0F12, 0x01BC}, //awbb_IndoorGrZones_m_BGrid[32]
    {0x0F12, 0x01DD}, //awbb_IndoorGrZones_m_BGrid[33]
    {0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid[34]
    {0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid[35]
    {0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid[36]
    {0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid[37]
    {0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid[38]
    {0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid[39]
    {0x0F12, 0x0005}, //awbb_IndoorGrZones_m_GridStep // param_end awbb_IndoorGrZones_m_BGrid
    {0x0F12, 0x0000},
    {0x002A, 0x0CA0},
    {0x0F12, 0x011A}, //awbb_IndoorGrZones_m_Boffs
    {0x0F12, 0x0000},
    {0x002A, 0x0CE0}, // param_start awbb_LowBrGrZones_m_BGrid
    {0x0F12, 0x0376}, //awbb_LowBrGrZones_m_BGrid[0]
    {0x0F12, 0x03F4}, //awbb_LowBrGrZones_m_BGrid[1]
    {0x0F12, 0x0304}, //awbb_LowBrGrZones_m_BGrid[2]
    {0x0F12, 0x03F4}, //awbb_LowBrGrZones_m_BGrid[3]
    {0x0F12, 0x029A}, //awbb_LowBrGrZones_m_BGrid[4]
    {0x0F12, 0x03E6}, //awbb_LowBrGrZones_m_BGrid[5]
    {0x0F12, 0x024E}, //awbb_LowBrGrZones_m_BGrid[6]
    {0x0F12, 0x039A}, //awbb_LowBrGrZones_m_BGrid[7]
    {0x0F12, 0x020E}, //awbb_LowBrGrZones_m_BGrid[8]
    {0x0F12, 0x034C}, //awbb_LowBrGrZones_m_BGrid[9]
    {0x0F12, 0x01E0}, //awbb_LowBrGrZones_m_BGrid[10]
    {0x0F12, 0x02FF}, //awbb_LowBrGrZones_m_BGrid[11]
    {0x0F12, 0x01AD}, //awbb_LowBrGrZones_m_BGrid[12]
    {0x0F12, 0x02B8}, //awbb_LowBrGrZones_m_BGrid[13]
    {0x0F12, 0x018A}, //awbb_LowBrGrZones_m_BGrid[14]
    {0x0F12, 0x0284}, //awbb_LowBrGrZones_m_BGrid[15]
    {0x0F12, 0x0187}, //awbb_LowBrGrZones_m_BGrid[16]
    {0x0F12, 0x025A}, //awbb_LowBrGrZones_m_BGrid[17]
    {0x0F12, 0x018D}, //awbb_LowBrGrZones_m_BGrid[18]
    {0x0F12, 0x01F6}, //awbb_LowBrGrZones_m_BGrid[19]
    {0x0F12, 0x0000}, //awbb_LowBrGrZones_m_BGrid[20]
    {0x0F12, 0x0000}, //awbb_LowBrGrZones_m_BGrid[21]
    {0x0F12, 0x0000}, //awbb_LowBrGrZones_m_BGrid[22]
    {0x0F12, 0x0000}, //awbb_LowBrGrZones_m_BGrid[23]
    {0x0F12, 0x0006}, //awbb_LowBrGrZones_m_GridStep // param_end awbb_LowBrGrZones_m_BGrid
    {0x0F12, 0x0000},
    {0x002A, 0x0D18},
    {0x0F12, 0x00FA}, //awbb_LowBrGrZones_m_Boffs
    {0x0F12, 0x0000},
    {0x002A, 0x0CA4}, // param_start awbb_OutdoorGrZones_m_BGrid
    {0x0F12, 0x026F}, //awbb_OutdoorGrZones_m_BGrid[0]
    {0x0F12, 0x029C}, //awbb_OutdoorGrZones_m_BGrid[1]
    {0x0F12, 0x0238}, //awbb_OutdoorGrZones_m_BGrid[2]
    {0x0F12, 0x0284}, //awbb_OutdoorGrZones_m_BGrid[3]
    {0x0F12, 0x0206}, //awbb_OutdoorGrZones_m_BGrid[4]
    {0x0F12, 0x0250}, //awbb_OutdoorGrZones_m_BGrid[5]
    {0x0F12, 0x01D6}, //awbb_OutdoorGrZones_m_BGrid[6]
    {0x0F12, 0x0226}, //awbb_OutdoorGrZones_m_BGrid[7]
    {0x0F12, 0x01BC}, //awbb_OutdoorGrZones_m_BGrid[8]
    {0x0F12, 0x01F6}, //awbb_OutdoorGrZones_m_BGrid[9]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[10]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[11]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[12]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[13]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[14]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[15]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[16]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[17]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[18]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[19]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[20]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[21]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[22]
    {0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[23]
    {0x0F12, 0x0006}, //awbb_OutdoorGrZones_m_GridStep // param_end awbb_OutdoorGrZones_m_BGrid
    {0x0F12, 0x0000},
    {0x002A, 0x0CDC},
    {0x0F12, 0x0212}, //awbb_OutdoorGrZones_m_Boffs
    {0x0F12, 0x0000},
    {0x002A, 0x0D1C},
    {0x0F12, 0x034D}, //awbb_CrclLowT_R_c
    {0x0F12, 0x0000},
    {0x002A, 0x0D20},
    {0x0F12, 0x016C}, //awbb_CrclLowT_B_c
    {0x0F12, 0x0000},
    {0x002A, 0x0D24},
    {0x0F12, 0x49D5}, //awbb_CrclLowT_Rad_c
    {0x0F12, 0x0000},
    {0x002A, 0x0D46},
    {0x0F12, 0x0470}, //awbb_MvEq_RBthresh
    {0x002A, 0x0D5C},
    {0x0F12, 0x0534}, //awbb_LowTempRB
    {0x002A, 0x0D2C},
    {0x0F12, 0x0131}, //awbb_IntcR
    {0x0F12, 0x012C}, //awbb_IntcB
    {0x002A, 0x0E4A}, //Grid Correction Settings - new
    {0x0F12, 0x0002}, //awbb_GridEnable
    //	param_start	awbb_GridConst_2
    {0x002A, 0x0E22},
    {0x0F12, 0x0EC6}, //awbb_GridConst_2[0]
    {0x0F12, 0x0F3B}, //awbb_GridConst_2[1]
    {0x0F12, 0x0FC7}, //awbb_GridConst_2[2]
    {0x0F12, 0x107E}, //awbb_GridConst_2[3]
    {0x0F12, 0x10F4}, //awbb_GridConst_2[4]
    {0x0F12, 0x1198}, //awbb_GridConst_2[5]
    //	param_end	awbb_GridConst_2
    {0x002A, 0x0E2E},
    {0x0F12, 0x00B2}, //awbb_GridCoeff_R_1
    {0x0F12, 0x00B8}, //awbb_GridCoeff_B_1
    {0x0F12, 0x00A6}, //awbb_GridCoeff_R_2
    {0x0F12, 0x00C3}, //awbb_GridCoeff_B_2

    //	param_start	awbb_GridConst_1
    {0x002A, 0x0E1C},
    {0x0F12, 0x02F4}, //awbb_GridConst_1[0]
    {0x0F12, 0x0347}, //awbb_GridConst_1[1]
    {0x0F12, 0x0390}, //awbb_GridConst_1[2]
    //	param_end	awbb_GridConst_1
    //	param_start	awbb_GridCorr_R
    {0x002A, 0x0DD4},
    {0x0F12, 0x0000}, //awbb_GridCorr_R[0]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[1]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[2]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[3]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[4]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[5]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[6]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[7]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[8]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[9]
    {0x0F12, 0xFFB0}, //awbb_GridCorr_R[10]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[11]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[12]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[13]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[14]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[15]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[16]
    {0x0F12, 0x0000}, //awbb_GridCorr_R[17]
    //	param_end	awbb_GridCorr_R
    //	param_start	awbb_GridCorr_B
    {0x002A, 0x0DF8},
    {0x0F12, 0x0000}, //awbb_GridCorr_B[0]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[1]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[2]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[3]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[4]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[5]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[6]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[7]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[8]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[9]
    {0x0F12, 0x0050}, //awbb_GridCorr_B[10]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[11]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[12]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[13]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[14]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[15]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[16]
    {0x0F12, 0x0000}, //awbb_GridCorr_B[17]
    //	param_end	awbb_GridCorr_B

    //================================================================================================
    // SET GAMMA
    //================================================================================================
    //Our //old	//STW
    {0x002A, 0x3288},
    {0x0F12, 0x0000}, //0000	//#SARR_usDualGammaLutRGBIndoor_0__0_ 0x70003288
    {0x0F12, 0x0004}, //0004	//#SARR_usDualGammaLutRGBIndoor_0__1_ 0x7000328A
    {0x0F12, 0x0010}, //0010	//#SARR_usDualGammaLutRGBIndoor_0__2_ 0x7000328C
    {0x0F12, 0x002A}, //002A	//#SARR_usDualGammaLutRGBIndoor_0__3_ 0x7000328E
    {0x0F12, 0x0062}, //0062	//#SARR_usDualGammaLutRGBIndoor_0__4_ 0x70003290
    {0x0F12, 0x00D5}, //00D5	//#SARR_usDualGammaLutRGBIndoor_0__5_ 0x70003292
    {0x0F12, 0x0138}, //0138	//#SARR_usDualGammaLutRGBIndoor_0__6_ 0x70003294
    {0x0F12, 0x0161}, //0161	//#SARR_usDualGammaLutRGBIndoor_0__7_ 0x70003296
    {0x0F12, 0x0186}, //0186	//#SARR_usDualGammaLutRGBIndoor_0__8_ 0x70003298
    {0x0F12, 0x01CA}, //01BC	//#SARR_usDualGammaLutRGBIndoor_0__9_ 0x7000329A
    {0x0F12, 0x01F8}, //01E8	//#SARR_usDualGammaLutRGBIndoor_0__10_ 0x7000329C
    {0x0F12, 0x0225}, //020F	//#SARR_usDualGammaLutRGBIndoor_0__11_ 0x7000329E
    {0x0F12, 0x0251}, //0232	//#SARR_usDualGammaLutRGBIndoor_0__12_ 0x700032A0
    {0x0F12, 0x0296}, //0273	//#SARR_usDualGammaLutRGBIndoor_0__13_ 0x700032A2
    {0x0F12, 0x02D3}, //02AF	//#SARR_usDualGammaLutRGBIndoor_0__14_ 0x700032A4
    {0x0F12, 0x032D}, //0309	//#SARR_usDualGammaLutRGBIndoor_0__15_ 0x700032A6
    {0x0F12, 0x0379}, //0355	//#SARR_usDualGammaLutRGBIndoor_0__16_ 0x700032A8
    {0x0F12, 0x03B3}, //0394	//#SARR_usDualGammaLutRGBIndoor_0__17_ 0x700032AA
    {0x0F12, 0x03DC}, //03CE	//#SARR_usDualGammaLutRGBIndoor_0__18_ 0x700032AC
    {0x0F12, 0x03FF}, //03FF	//#SARR_usDualGammaLutRGBIndoor_0__19_ 0x700032AE
    {0x0F12, 0x0000}, //0000	//#SARR_usDualGammaLutRGBIndoor_1__0_ 0x700032B0
    {0x0F12, 0x0004}, //0004	//#SARR_usDualGammaLutRGBIndoor_1__1_ 0x700032B2
    {0x0F12, 0x0010}, //0010	//#SARR_usDualGammaLutRGBIndoor_1__2_ 0x700032B4
    {0x0F12, 0x002A}, //002A	//#SARR_usDualGammaLutRGBIndoor_1__3_ 0x700032B6
    {0x0F12, 0x0062}, //0062	//#SARR_usDualGammaLutRGBIndoor_1__4_ 0x700032B8
    {0x0F12, 0x00D5}, //00D5	//#SARR_usDualGammaLutRGBIndoor_1__5_ 0x700032BA
    {0x0F12, 0x0138}, //0138	//#SARR_usDualGammaLutRGBIndoor_1__6_ 0x700032BC
    {0x0F12, 0x0161}, //0161	//#SARR_usDualGammaLutRGBIndoor_1__7_ 0x700032BE
    {0x0F12, 0x0186}, //0186	//#SARR_usDualGammaLutRGBIndoor_1__8_ 0x700032C0
    {0x0F12, 0x01CA}, //01BC	//#SARR_usDualGammaLutRGBIndoor_1__9_ 0x700032C2
    {0x0F12, 0x01F8}, //01E8	//#SARR_usDualGammaLutRGBIndoor_1__10_ 0x700032C4
    {0x0F12, 0x0225}, //020F	//#SARR_usDualGammaLutRGBIndoor_1__11_ 0x700032C6
    {0x0F12, 0x0251}, //0232	//#SARR_usDualGammaLutRGBIndoor_1__12_ 0x700032C8
    {0x0F12, 0x0296}, //0273	//#SARR_usDualGammaLutRGBIndoor_1__13_ 0x700032CA
    {0x0F12, 0x02D3}, //02AF	//#SARR_usDualGammaLutRGBIndoor_1__14_ 0x700032CC
    {0x0F12, 0x032D}, //0309	//#SARR_usDualGammaLutRGBIndoor_1__15_ 0x700032CE
    {0x0F12, 0x0379}, //0355	//#SARR_usDualGammaLutRGBIndoor_1__16_ 0x700032D0
    {0x0F12, 0x03B3}, //0394	//#SARR_usDualGammaLutRGBIndoor_1__17_ 0x700032D2
    {0x0F12, 0x03DC}, //03CE	//#SARR_usDualGammaLutRGBIndoor_1__18_ 0x700032D4
    {0x0F12, 0x03FF}, //03FF	//#SARR_usDualGammaLutRGBIndoor_1__19_ 0x700032D6
    {0x0F12, 0x0000}, //0000	//#SARR_usDualGammaLutRGBIndoor_2__0_ 0x700032D8
    {0x0F12, 0x0004}, //0004	//#SARR_usDualGammaLutRGBIndoor_2__1_ 0x700032DA
    {0x0F12, 0x0010}, //0010	//#SARR_usDualGammaLutRGBIndoor_2__2_ 0x700032DC
    {0x0F12, 0x002A}, //002A	//#SARR_usDualGammaLutRGBIndoor_2__3_ 0x700032DE
    {0x0F12, 0x0062}, //0062	//#SARR_usDualGammaLutRGBIndoor_2__4_ 0x700032E0
    {0x0F12, 0x00D5}, //00D5	//#SARR_usDualGammaLutRGBIndoor_2__5_ 0x700032E2
    {0x0F12, 0x0138}, //0138	//#SARR_usDualGammaLutRGBIndoor_2__6_ 0x700032E4
    {0x0F12, 0x0161}, //0161	//#SARR_usDualGammaLutRGBIndoor_2__7_ 0x700032E6
    {0x0F12, 0x0186}, //0186	//#SARR_usDualGammaLutRGBIndoor_2__8_ 0x700032E8
    {0x0F12, 0x01CA}, //01BC	//#SARR_usDualGammaLutRGBIndoor_2__9_ 0x700032EA
    {0x0F12, 0x01F8}, //01E8	//#SARR_usDualGammaLutRGBIndoor_2__10_ 0x700032EC
    {0x0F12, 0x0225}, //020F	//#SARR_usDualGammaLutRGBIndoor_2__11_ 0x700032EE
    {0x0F12, 0x0251}, //0232	//#SARR_usDualGammaLutRGBIndoor_2__12_ 0x700032F0
    {0x0F12, 0x0296}, //0273	//#SARR_usDualGammaLutRGBIndoor_2__13_ 0x700032F2
    {0x0F12, 0x02D3}, //02AF	//#SARR_usDualGammaLutRGBIndoor_2__14_ 0x700032F4
    {0x0F12, 0x032D}, //0309	//#SARR_usDualGammaLutRGBIndoor_2__15_ 0x700032F6
    {0x0F12, 0x0379}, //0355	//#SARR_usDualGammaLutRGBIndoor_2__16_ 0x700032F8
    {0x0F12, 0x03B3}, //0394	//#SARR_usDualGammaLutRGBIndoor_2__17_ 0x700032FA
    {0x0F12, 0x03DC}, //03CE	//#SARR_usDualGammaLutRGBIndoor_2__18_ 0x700032FC
    {0x0F12, 0x03FF}, //03FF	//#SARR_usDualGammaLutRGBIndoor_2__19_ 0x700032FE

    {0x0F12, 0x0000}, //#SARR_usDualGammaLutRGBOutdoor_0__0_ 0x70003300
    {0x0F12, 0x0004}, //#SARR_usDualGammaLutRGBOutdoor_0__1_ 0x70003302
    {0x0F12, 0x0010}, //#SARR_usDualGammaLutRGBOutdoor_0__2_ 0x70003304
    {0x0F12, 0x002A}, //#SARR_usDualGammaLutRGBOutdoor_0__3_ 0x70003306
    {0x0F12, 0x0062}, //#SARR_usDualGammaLutRGBOutdoor_0__4_ 0x70003308
    {0x0F12, 0x00D5}, //#SARR_usDualGammaLutRGBOutdoor_0__5_ 0x7000330A
    {0x0F12, 0x0138}, //#SARR_usDualGammaLutRGBOutdoor_0__6_ 0x7000330C
    {0x0F12, 0x0161}, //#SARR_usDualGammaLutRGBOutdoor_0__7_ 0x7000330E
    {0x0F12, 0x0186}, //#SARR_usDualGammaLutRGBOutdoor_0__8_ 0x70003310
    {0x0F12, 0x01BC}, //#SARR_usDualGammaLutRGBOutdoor_0__9_ 0x70003312
    {0x0F12, 0x01E8}, //#SARR_usDualGammaLutRGBOutdoor_0__10_0x70003314
    {0x0F12, 0x020F}, //#SARR_usDualGammaLutRGBOutdoor_0__11_0x70003316
    {0x0F12, 0x0232}, //#SARR_usDualGammaLutRGBOutdoor_0__12_0x70003318
    {0x0F12, 0x0273}, //#SARR_usDualGammaLutRGBOutdoor_0__13_0x7000331A
    {0x0F12, 0x02AF}, //#SARR_usDualGammaLutRGBOutdoor_0__14_0x7000331C
    {0x0F12, 0x0309}, //#SARR_usDualGammaLutRGBOutdoor_0__15_0x7000331E
    {0x0F12, 0x0355}, //#SARR_usDualGammaLutRGBOutdoor_0__16_0x70003320
    {0x0F12, 0x0394}, //#SARR_usDualGammaLutRGBOutdoor_0__17_0x70003322
    {0x0F12, 0x03CE}, //#SARR_usDualGammaLutRGBOutdoor_0__18_0x70003324
    {0x0F12, 0x03FF}, //#SARR_usDualGammaLutRGBOutdoor_0__19_0x70003326
    {0x0F12, 0x0000}, //#SARR_usDualGammaLutRGBOutdoor_1__0_ 0x70003328
    {0x0F12, 0x0004}, //#SARR_usDualGammaLutRGBOutdoor_1__1_ 0x7000332A
    {0x0F12, 0x0010}, //#SARR_usDualGammaLutRGBOutdoor_1__2_ 0x7000332C
    {0x0F12, 0x002A}, //#SARR_usDualGammaLutRGBOutdoor_1__3_ 0x7000332E
    {0x0F12, 0x0062}, //#SARR_usDualGammaLutRGBOutdoor_1__4_ 0x70003330
    {0x0F12, 0x00D5}, //#SARR_usDualGammaLutRGBOutdoor_1__5_ 0x70003332
    {0x0F12, 0x0138}, //#SARR_usDualGammaLutRGBOutdoor_1__6_ 0x70003334
    {0x0F12, 0x0161}, //#SARR_usDualGammaLutRGBOutdoor_1__7_ 0x70003336
    {0x0F12, 0x0186}, //#SARR_usDualGammaLutRGBOutdoor_1__8_ 0x70003338
    {0x0F12, 0x01BC}, //#SARR_usDualGammaLutRGBOutdoor_1__9_ 0x7000333A
    {0x0F12, 0x01E8}, //#SARR_usDualGammaLutRGBOutdoor_1__10_0x7000333C
    {0x0F12, 0x020F}, //#SARR_usDualGammaLutRGBOutdoor_1__11_0x7000333E
    {0x0F12, 0x0232}, //#SARR_usDualGammaLutRGBOutdoor_1__12_0x70003340
    {0x0F12, 0x0273}, //#SARR_usDualGammaLutRGBOutdoor_1__13_0x70003342
    {0x0F12, 0x02AF}, //#SARR_usDualGammaLutRGBOutdoor_1__14_0x70003344
    {0x0F12, 0x0309}, //#SARR_usDualGammaLutRGBOutdoor_1__15_0x70003346
    {0x0F12, 0x0355}, //#SARR_usDualGammaLutRGBOutdoor_1__16_0x70003348
    {0x0F12, 0x0394}, //#SARR_usDualGammaLutRGBOutdoor_1__17_0x7000334A
    {0x0F12, 0x03CE}, //#SARR_usDualGammaLutRGBOutdoor_1__18_0x7000334C
    {0x0F12, 0x03FF}, //#SARR_usDualGammaLutRGBOutdoor_1__19_0x7000334E
    {0x0F12, 0x0000}, //#SARR_usDualGammaLutRGBOutdoor_2__0_ 0x70003350
    {0x0F12, 0x0004}, //#SARR_usDualGammaLutRGBOutdoor_2__1_ 0x70003352
    {0x0F12, 0x0010}, //#SARR_usDualGammaLutRGBOutdoor_2__2_ 0x70003354
    {0x0F12, 0x002A}, //#SARR_usDualGammaLutRGBOutdoor_2__3_ 0x70003356
    {0x0F12, 0x0062}, //#SARR_usDualGammaLutRGBOutdoor_2__4_ 0x70003358
    {0x0F12, 0x00D5}, //#SARR_usDualGammaLutRGBOutdoor_2__5_ 0x7000335A
    {0x0F12, 0x0138}, //#SARR_usDualGammaLutRGBOutdoor_2__6_ 0x7000335C
    {0x0F12, 0x0161}, //#SARR_usDualGammaLutRGBOutdoor_2__7_ 0x7000335E
    {0x0F12, 0x0186}, //#SARR_usDualGammaLutRGBOutdoor_2__8_ 0x70003360
    {0x0F12, 0x01BC}, //#SARR_usDualGammaLutRGBOutdoor_2__9_ 0x70003362
    {0x0F12, 0x01E8}, //#SARR_usDualGammaLutRGBOutdoor_2__10_0x70003364
    {0x0F12, 0x020F}, //#SARR_usDualGammaLutRGBOutdoor_2__11_0x70003366
    {0x0F12, 0x0232}, //#SARR_usDualGammaLutRGBOutdoor_2__12_0x70003368
    {0x0F12, 0x0273}, //#SARR_usDualGammaLutRGBOutdoor_2__13_0x7000336A
    {0x0F12, 0x02AF}, //#SARR_usDualGammaLutRGBOutdoor_2__14_0x7000336C
    {0x0F12, 0x0309}, //#SARR_usDualGammaLutRGBOutdoor_2__15_0x7000336E
    {0x0F12, 0x0355}, //#SARR_usDualGammaLutRGBOutdoor_2__16_0x70003370
    {0x0F12, 0x0394}, //#SARR_usDualGammaLutRGBOutdoor_2__17_0x70003372
    {0x0F12, 0x03CE}, //#SARR_usDualGammaLutRGBOutdoor_2__18_0x70003374
    {0x0F12, 0x03FF}, //#SARR_usDualGammaLutRGBOutdoor_2__19_0x70003376

    {0x002A, 0x06A6},
    {0x0F12, 0x00C0}, // #SARR_AwbCcmCord_0_
    {0x0F12, 0x00F8}, // #SARR_AwbCcmCord_1_
    {0x0F12, 0x0112}, // #SARR_AwbCcmCord_2_
    {0x0F12, 0x014A}, // #SARR_AwbCcmCord_3_
    {0x0F12, 0x0156}, // #SARR_AwbCcmCord_4_
    {0x0F12, 0x017F}, // #SARR_AwbCcmCord_5_

    {0x002A, 0x1034}, // Hong  1123
    {0x0F12, 0x00B5}, // #SARR_IllumType[0]
    {0x0F12, 0x00CF}, // #SARR_IllumType[1]
    {0x0F12, 0x0116}, // #SARR_IllumType[2]
    {0x0F12, 0x0140}, // #SARR_IllumType[3]
    {0x0F12, 0x0150}, // #SARR_IllumType[4]
    {0x0F12, 0x0174}, // #SARR_IllumType[5]
    {0x0F12, 0x018E}, // #SARR_IllumType[6]

    {0x0F12, 0x00B8}, // #SARR_IllumTypeF[0]
    {0x0F12, 0x00BA}, // #SARR_IllumTypeF[1]
    {0x0F12, 0x00C0}, // #SARR_IllumTypeF[2]
    {0x0F12, 0x00F0}, // #SARR_IllumTypeF[3]
    {0x0F12, 0x0100}, // #SARR_IllumTypeF[4]
    {0x0F12, 0x0100}, // #SARR_IllumTypeF[5]
    {0x0F12, 0x0100}, // #SARR_IllumTypeF[6]

    //================================================================================================
    // SET AFIT
    //================================================================================================
    // Noise index
    {0x002A, 0x0764},
    {0x0F12, 0x0041}, //afit_uNoiseIndInDoor[0] //
    {0x0F12, 0x0063}, //afit_uNoiseIndInDoor[1] //
    {0x0F12, 0x00BB}, //afit_uNoiseIndInDoor[2] // 203//
    {0x0F12, 0x0193}, //afit_uNoiseIndInDoor[3] // Indoor_NB below 1500 _Noise index 300-400d //
    {0x0F12, 0x02BC}, //afit_uNoiseIndInDoor[4] // DNP NB 4600 _ Noisenidex :560d-230h //
    // AFIT table start address // 7000_07C4
    {0x002A, 0x0770},
    {0x0F12, 0x07C4},
    {0x0F12, 0x7000},

    // AFIT table (Variables)
    {0x002A, 0x07C4},
    {0x0F12, 0x0000}, //700007C4 //TVAR_afit_pBaseValS[0] // AFIT16_BRIGHTNESS
    {0x0F12, 0x0000}, //700007C6 //TVAR_afit_pBaseValS[1] // AFIT16_CONTRAST
    {0x0F12, 0x0001}, //700007C8 //TVAR_afit_pBaseValS[2] // AFIT16_SATURATION
    {0x0F12, 0x0005}, //700007CA //TVAR_afit_pBaseValS[3] // AFIT16_SHARP_BLUR
    {0x0F12, 0xFFF6}, //700007CC //TVAR_afit_pBaseValS[4] // AFIT16_GLAMOUR
    {0x0F12, 0x00C4}, //700007CE //TVAR_afit_pBaseValS[5] // AFIT16_sddd8a_edge_high
    {0x0F12, 0x03FF}, //700007D0 //TVAR_afit_pBaseValS[6] // AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x009C}, //700007D2 //TVAR_afit_pBaseValS[7] // AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x017C}, //700007D4 //TVAR_afit_pBaseValS[8] // AFIT16_demsharpmix1_iRGBOffset
    {0x0F12, 0x03FF}, //700007D6 //TVAR_afit_pBaseValS[9] // AFIT16_demsharpmix1_iDemClamp
    {0x0F12, 0x000C}, //700007D8 //TVAR_afit_pBaseValS[10] //AFIT16_demsharpmix1_iLowThreshold
    {0x0F12, 0x0010}, //700007DA //TVAR_afit_pBaseValS[11] //AFIT16_demsharpmix1_iHighThreshold
    {0x0F12, 0x012C}, //700007DC //TVAR_afit_pBaseValS[12] //AFIT16_demsharpmix1_iLowBright
    {0x0F12, 0x03E8}, //700007DE //TVAR_afit_pBaseValS[13] //AFIT16_demsharpmix1_iHighBright
    {0x0F12, 0x0046}, //700007E0 //TVAR_afit_pBaseValS[14] //AFIT16_demsharpmix1_iLowSat
    {0x0F12, 0x005A}, //700007E2 //TVAR_afit_pBaseValS[15] //AFIT16_demsharpmix1_iHighSat
    {0x0F12, 0x0070}, //700007E4 //TVAR_afit_pBaseValS[16] //AFIT16_demsharpmix1_iTune
    {0x0F12, 0x0019}, //700007E6 //TVAR_afit_pBaseValS[17] //AFIT16_demsharpmix1_iHystThLow
    {0x0F12, 0x0019}, //700007E8 //TVAR_afit_pBaseValS[18] //AFIT16_demsharpmix1_iHystThHigh
    {0x0F12, 0x01AA}, //700007EA //TVAR_afit_pBaseValS[19] //AFIT16_demsharpmix1_iHystCenter
    {0x0F12, 0x0064}, //700007EC //TVAR_afit_pBaseValS[20] //AFIT16_YUV422_DENOISE_iUVLowThresh
    {0x0F12, 0x0064}, //700007EE //TVAR_afit_pBaseValS[21] //AFIT16_YUV422_DENOISE_iUVHighThresh
    {0x0F12, 0x000A}, //700007F0 //TVAR_afit_pBaseValS[22] //AFIT16_YUV422_DENOISE_iYLowThresh
    {0x0F12, 0x000A}, //700007F2 //TVAR_afit_pBaseValS[23] //AFIT16_YUV422_DENOISE_iYHighThresh
    {0x0F12, 0x0032}, //700007F4 //TVAR_afit_pBaseValS[24] //AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x0012}, //700007F6 //TVAR_afit_pBaseValS[25] //AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x002A}, //700007F8 //TVAR_afit_pBaseValS[26] //AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x0024}, //700007FA //TVAR_afit_pBaseValS[27] //AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x002A}, //700007FC //TVAR_afit_pBaseValS[28] //AFIT16_Sharpening_iLowSharpClamp_sBin
    {0x0F12, 0x0024}, //700007FE //TVAR_afit_pBaseValS[29] //AFIT16_Sharpening_iHighSharpClamp_sBin
    {0x0F12, 0x0A24}, //70000800 //TVAR_afit_pBaseValS[30] //AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
    {0x0F12, 0x1701}, //70000802 //TVAR_afit_pBaseValS[31] //AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
    {0x0F12, 0x0229}, //70000804 //TVAR_afit_pBaseValS[32] //AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
    {0x0F12, 0x1403}, //70000806 //TVAR_afit_pBaseValS[33] //AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
    {0x0F12, 0x0000}, //70000808 //TVAR_afit_pBaseValS[34] //AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]
    {0x0F12, 0x0000}, //7000080A //TVAR_afit_pBaseValS[35] //AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
    {0x0F12, 0x0000}, //7000080C //TVAR_afit_pBaseValS[36] //AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]
    {0x0F12, 0x00FF}, //7000080E //TVAR_afit_pBaseValS[37] //AFIT8_sddd8a_iSatSat[7:0],	AFIT8_sddd8a_iRadialTune [15:8]
    {0x0F12, 0x043B}, //70000810 //TVAR_afit_pBaseValS[38] //AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]
    {0x0F12, 0x1414}, //70000812 //TVAR_afit_pBaseValS[39] //AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],	AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
    {0x0F12, 0x0301}, //70000814 //TVAR_afit_pBaseValS[40] //AFIT8_sddd8a_iLowSlopeThresh[7:0],	AFIT8_sddd8a_iHighSlopeThresh [15:8]
    {0x0F12, 0xFF07}, //70000816 //TVAR_afit_pBaseValS[41] //AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]
    {0x0F12, 0x051E}, //70000818 //TVAR_afit_pBaseValS[42] //AFIT8_Demosaicing_iMonochrom [7:0],	 AFIT8_Demosaicing_iDecisionThresh [15:8]
    {0x0F12, 0x0A1E}, //7000081A //TVAR_afit_pBaseValS[43] //AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]
    {0x0F12, 0x0F0F}, //7000081C //TVAR_afit_pBaseValS[44] //AFIT8_Demosaicing_iGRDenoiseVal [7:0],	AFIT8_Demosaicing_iGBDenoiseVal [15:8]
    {0x0F12, 0x0A05}, //7000081E //TVAR_afit_pBaseValS[45] //AFIT8_Demosaicing_iNearGrayDesat[7:0],	AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
    {0x0F12, 0x0A3C}, //70000820 //TVAR_afit_pBaseValS[46] //AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]
    {0x0F12, 0x0A28}, //70000822 //TVAR_afit_pBaseValS[47] //AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]
    {0x0F12, 0x0002}, //70000824 //TVAR_afit_pBaseValS[48] //AFIT8_Sharpening_nSharpWidth [7:0],	 AFIT8_Sharpening_iReduceNegative [15:8]
    {0x0F12, 0x00FF}, //70000826 //TVAR_afit_pBaseValS[49] //AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
    {0x0F12, 0x1102}, //70000828 //TVAR_afit_pBaseValS[50] //AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
    {0x0F12, 0x001B}, //7000082A //TVAR_afit_pBaseValS[51] //AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]
    {0x0F12, 0x0900}, //7000082C //TVAR_afit_pBaseValS[52] //AFIT8_demsharpmix1_iNarrMult [7:0],	 AFIT8_demsharpmix1_iHystFalloff [15:8]
    {0x0F12, 0x0600}, //7000082E //TVAR_afit_pBaseValS[53] //AFIT8_demsharpmix1_iHystMinMult [7:0],	AFIT8_demsharpmix1_iHystWidth [15:8]
    {0x0F12, 0x0504}, //70000830 //TVAR_afit_pBaseValS[54] //AFIT8_demsharpmix1_iHystFallLow [7:0],	AFIT8_demsharpmix1_iHystFallHigh [15:8]
    {0x0F12, 0x0305}, //70000832 //TVAR_afit_pBaseValS[55] //AFIT8_demsharpmix1_iHystTune [7:0],	* AFIT8_YUV422_DENOISE_iUVSupport [15:8]
    {0x0F12, 0x3C03}, //70000834 //TVAR_afit_pBaseValS[56] //AFIT8_YUV422_DENOISE_iYSupport [7:0],   AFIT8_byr_cgras_iShadingPower [15:8]
    {0x0F12, 0x006E}, //70000836 //TVAR_afit_pBaseValS[57] //AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]
    {0x0F12, 0x0178}, //70000838 //TVAR_afit_pBaseValS[58] //AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]
    {0x0F12, 0x0080}, //7000083A //TVAR_afit_pBaseValS[59] //AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]
    {0x0F12, 0x1414}, //7000083C //TVAR_afit_pBaseValS[60] //AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
    {0x0F12, 0x0101}, //7000083E //TVAR_afit_pBaseValS[61] //AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]
    {0x0F12, 0x5002}, //70000840 //TVAR_afit_pBaseValS[62] //AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]
    {0x0F12, 0x7850}, //70000842 //TVAR_afit_pBaseValS[63] //AFIT8_sddd8a_DispTH_High [7:0],	 AFIT8_sddd8a_iDenThreshLow [15:8]
    {0x0F12, 0x2878}, //70000844 //TVAR_afit_pBaseValS[64] //AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]
    {0x0F12, 0x0A00}, //70000846 //TVAR_afit_pBaseValS[65] //AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
    {0x0F12, 0x1403}, //70000848 //TVAR_afit_pBaseValS[66] //AFIT8_Demosaicing_iEdgeDesatLimit[7:0],	AFIT8_Demosaicing_iDemSharpenLow [15:8]
    {0x0F12, 0x1E0C}, //7000084A //TVAR_afit_pBaseValS[67] //AFIT8_Demosaicing_iDemSharpenHigh[7:0],	 AFIT8_Demosaicing_iDemSharpThresh [15:8]
    {0x0F12, 0x070A}, //7000084C //TVAR_afit_pBaseValS[68] //AFIT8_Demosaicing_iDemShLowLimit [7:0],	 AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
    {0x0F12, 0x32FF}, //7000084E //TVAR_afit_pBaseValS[69] //AFIT8_Demosaicing_iDemBlurLow[7:0],	 AFIT8_Demosaicing_iDemBlurHigh [15:8]
    {0x0F12, 0x4104}, //70000850 //TVAR_afit_pBaseValS[70] //AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]
    {0x0F12, 0x123C}, //70000852 //TVAR_afit_pBaseValS[71] //AFIT8_Sharpening_iHighSharpPower[7:0],	AFIT8_Sharpening_iLowShDenoise [15:8]
    {0x0F12, 0x4012}, //70000854 //TVAR_afit_pBaseValS[72] //AFIT8_Sharpening_iHighShDenoise [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
    {0x0F12, 0x0204}, //70000856 //TVAR_afit_pBaseValS[73] //AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
    {0x0F12, 0x1E03}, //70000858 //TVAR_afit_pBaseValS[74] //AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
    {0x0F12, 0x011E}, //7000085A //TVAR_afit_pBaseValS[75] //AFIT8_sddd8a_iClustThresh_C_Bin [7:0],	AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
    {0x0F12, 0x0201}, //7000085C //TVAR_afit_pBaseValS[76] //AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
    {0x0F12, 0x5050}, //7000085E //TVAR_afit_pBaseValS[77] //AFIT8_sddd8a_DispTH_Low_Bin [7:0],	AFIT8_sddd8a_DispTH_High_Bin [15:8]
    {0x0F12, 0x3C3C}, //70000860 //TVAR_afit_pBaseValS[78] //AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
    {0x0F12, 0x0028}, //70000862 //TVAR_afit_pBaseValS[79] //AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],	AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
    {0x0F12, 0x030A}, //70000864 //TVAR_afit_pBaseValS[80] //AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
    {0x0F12, 0x0714}, //70000866 //TVAR_afit_pBaseValS[81] //AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],	AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
    {0x0F12, 0x0A1E}, //70000868 //TVAR_afit_pBaseValS[82] //AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
    {0x0F12, 0xFF07}, //7000086A //TVAR_afit_pBaseValS[83] //AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
    {0x0F12, 0x0432}, //7000086C //TVAR_afit_pBaseValS[84] //AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
    {0x0F12, 0x4050}, //7000086E //TVAR_afit_pBaseValS[85] //AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
    {0x0F12, 0x0F0F}, //70000870 //TVAR_afit_pBaseValS[86] //AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
    {0x0F12, 0x0440}, //70000872 //TVAR_afit_pBaseValS[87] //AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
    {0x0F12, 0x0302}, //70000874 //TVAR_afit_pBaseValS[88] //AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
    {0x0F12, 0x1E1E}, //70000876 //TVAR_afit_pBaseValS[89] //AFIT8_sddd8a_iClustThresh_H_sBin[7:0],	AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
    {0x0F12, 0x0101}, //70000878 //TVAR_afit_pBaseValS[90] //AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
    {0x0F12, 0x5002}, //7000087A //TVAR_afit_pBaseValS[91] //AFIT8_sddd8a_nClustLevel_H_sBin [7:0],	AFIT8_sddd8a_DispTH_Low_sBin [15:8]
    {0x0F12, 0x3C50}, //7000087C //TVAR_afit_pBaseValS[92] //AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
    {0x0F12, 0x283C}, //7000087E //TVAR_afit_pBaseValS[93] //AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],	AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
    {0x0F12, 0x0A00}, //70000880 //TVAR_afit_pBaseValS[94] //AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
    {0x0F12, 0x1403}, //70000882 //TVAR_afit_pBaseValS[95] //AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
    {0x0F12, 0x1E07}, //70000884 //TVAR_afit_pBaseValS[96] //AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
    {0x0F12, 0x070A}, //70000886 //TVAR_afit_pBaseValS[97] //AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
    {0x0F12, 0x32FF}, //70000888 //TVAR_afit_pBaseValS[98] //AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
    {0x0F12, 0x5004}, //7000088A //TVAR_afit_pBaseValS[99] //AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],	AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
    {0x0F12, 0x0F40}, //7000088C //TVAR_afit_pBaseValS[100] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
    {0x0F12, 0x400F}, //7000088E //TVAR_afit_pBaseValS[101] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
    {0x0F12, 0x0204}, //70000890 //TVAR_afit_pBaseValS[102] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
    {0x0F12, 0x0003}, //70000892 //TVAR_afit_pBaseValS[103] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]
    {0x0F12, 0x0000}, //70000894 //TVAR_afit_pBaseValS[104] /AFIT16_BRIGHTNESS
    {0x0F12, 0x0000}, //70000896 //TVAR_afit_pBaseValS[105] /AFIT16_CONTRAST
    {0x0F12, 0x0005}, //70000898 //TVAR_afit_pBaseValS[106] /AFIT16_SATURATION
    {0x0F12, 0x0005}, //7000089A //TVAR_afit_pBaseValS[107] /AFIT16_SHARP_BLUR
    {0x0F12, 0xFFF6}, //7000089C //TVAR_afit_pBaseValS[108] /AFIT16_GLAMOUR
    {0x0F12, 0x00C4}, //7000089E //TVAR_afit_pBaseValS[109] /AFIT16_sddd8a_edge_high
    {0x0F12, 0x03FF}, //700008A0 //TVAR_afit_pBaseValS[110] /AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x009C}, //700008A2 //TVAR_afit_pBaseValS[111] /AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x017C}, //700008A4 //TVAR_afit_pBaseValS[112] /AFIT16_demsharpmix1_iRGBOffset
    {0x0F12, 0x03FF}, //700008A6 //TVAR_afit_pBaseValS[113] /AFIT16_demsharpmix1_iDemClamp
    {0x0F12, 0x000C}, //700008A8 //TVAR_afit_pBaseValS[114] /AFIT16_demsharpmix1_iLowThreshold
    {0x0F12, 0x0010}, //700008AA //TVAR_afit_pBaseValS[115] /AFIT16_demsharpmix1_iHighThreshold
    {0x0F12, 0x012C}, //700008AC //TVAR_afit_pBaseValS[116] /AFIT16_demsharpmix1_iLowBright
    {0x0F12, 0x03E8}, //700008AE //TVAR_afit_pBaseValS[117] /AFIT16_demsharpmix1_iHighBright
    {0x0F12, 0x0046}, //700008B0 //TVAR_afit_pBaseValS[118] /AFIT16_demsharpmix1_iLowSat
    {0x0F12, 0x005A}, //700008B2 //TVAR_afit_pBaseValS[119] /AFIT16_demsharpmix1_iHighSat
    {0x0F12, 0x0070}, //700008B4 //TVAR_afit_pBaseValS[120] /AFIT16_demsharpmix1_iTune
    {0x0F12, 0x000F}, //700008B6 //TVAR_afit_pBaseValS[121] /AFIT16_demsharpmix1_iHystThLow
    {0x0F12, 0x000F}, //700008B8 //TVAR_afit_pBaseValS[122] /AFIT16_demsharpmix1_iHystThHigh
    {0x0F12, 0x01AA}, //700008BA //TVAR_afit_pBaseValS[123] /AFIT16_demsharpmix1_iHystCenter
    {0x0F12, 0x003C}, //700008BC //TVAR_afit_pBaseValS[124] /AFIT16_YUV422_DENOISE_iUVLowThresh
    {0x0F12, 0x003C}, //700008BE //TVAR_afit_pBaseValS[125] /AFIT16_YUV422_DENOISE_iUVHighThresh
    {0x0F12, 0x0005}, //700008C0 //TVAR_afit_pBaseValS[126] /AFIT16_YUV422_DENOISE_iYLowThresh
    {0x0F12, 0x0005}, //700008C2 //TVAR_afit_pBaseValS[127] /AFIT16_YUV422_DENOISE_iYHighThresh
    {0x0F12, 0x0046}, //700008C4 //TVAR_afit_pBaseValS[128] /AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x0019}, //700008C6 //TVAR_afit_pBaseValS[129] /AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x002A}, //700008C8 //TVAR_afit_pBaseValS[130] /AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x0024}, //700008CA //TVAR_afit_pBaseValS[131] /AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x002A}, //700008CC //TVAR_afit_pBaseValS[132] /AFIT16_Sharpening_iLowSharpClamp_sBin
    {0x0F12, 0x0024}, //700008CE //TVAR_afit_pBaseValS[133] /AFIT16_Sharpening_iHighSharpClamp_sBin
    {0x0F12, 0x0A24}, //700008D0 //TVAR_afit_pBaseValS[134] /AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
    {0x0F12, 0x1701}, //700008D2 //TVAR_afit_pBaseValS[135] /AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
    {0x0F12, 0x0229}, //700008D4 //TVAR_afit_pBaseValS[136] /AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
    {0x0F12, 0x1403}, //700008D6 //TVAR_afit_pBaseValS[137] /AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
    {0x0F12, 0x0000}, //700008D8 //TVAR_afit_pBaseValS[138] /AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]
    {0x0F12, 0x0000}, //700008DA //TVAR_afit_pBaseValS[139] /AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
    {0x0F12, 0x0000}, //700008DC //TVAR_afit_pBaseValS[140] /AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]
    {0x0F12, 0x00FF}, //700008DE //TVAR_afit_pBaseValS[141] /AFIT8_sddd8a_iSatSat[7:0],	AFIT8_sddd8a_iRadialTune [15:8]
    {0x0F12, 0x043B}, //700008E0 //TVAR_afit_pBaseValS[142] /AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]
    {0x0F12, 0x1414}, //700008E2 //TVAR_afit_pBaseValS[143] /AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],	AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
    {0x0F12, 0x0301}, //700008E4 //TVAR_afit_pBaseValS[144] /AFIT8_sddd8a_iLowSlopeThresh[7:0],	AFIT8_sddd8a_iHighSlopeThresh [15:8]
    {0x0F12, 0xFF07}, //700008E6 //TVAR_afit_pBaseValS[145] /AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]
    {0x0F12, 0x051E}, //700008E8 //TVAR_afit_pBaseValS[146] /AFIT8_Demosaicing_iMonochrom [7:0],	 AFIT8_Demosaicing_iDecisionThresh [15:8]
    {0x0F12, 0x0A1E}, //700008EA //TVAR_afit_pBaseValS[147] /AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]
    {0x0F12, 0x0F0F}, //700008EC //TVAR_afit_pBaseValS[148] /AFIT8_Demosaicing_iGRDenoiseVal [7:0],	AFIT8_Demosaicing_iGBDenoiseVal [15:8]
    {0x0F12, 0x0A03}, //700008EE //TVAR_afit_pBaseValS[149] /AFIT8_Demosaicing_iNearGrayDesat[7:0],	AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
    {0x0F12, 0x0A3C}, //700008F0 //TVAR_afit_pBaseValS[150] /AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]
    {0x0F12, 0x0A28}, //700008F2 //TVAR_afit_pBaseValS[151] /AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]
    {0x0F12, 0x0002}, //700008F4 //TVAR_afit_pBaseValS[152] /AFIT8_Sharpening_nSharpWidth [7:0],	 AFIT8_Sharpening_iReduceNegative [15:8]
    {0x0F12, 0x00FF}, //700008F6 //TVAR_afit_pBaseValS[153] /AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
    {0x0F12, 0x1102}, //700008F8 //TVAR_afit_pBaseValS[154] /AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
    {0x0F12, 0x001B}, //700008FA //TVAR_afit_pBaseValS[155] /AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]
    {0x0F12, 0x0900}, //700008FC //TVAR_afit_pBaseValS[156] /AFIT8_demsharpmix1_iNarrMult [7:0],	 AFIT8_demsharpmix1_iHystFalloff [15:8]
    {0x0F12, 0x0600}, //700008FE //TVAR_afit_pBaseValS[157] /AFIT8_demsharpmix1_iHystMinMult [7:0],	AFIT8_demsharpmix1_iHystWidth [15:8]
    {0x0F12, 0x0504}, //70000900 //TVAR_afit_pBaseValS[158] /AFIT8_demsharpmix1_iHystFallLow [7:0],	AFIT8_demsharpmix1_iHystFallHigh [15:8]
    {0x0F12, 0x0305}, //70000902 //TVAR_afit_pBaseValS[159] /AFIT8_demsharpmix1_iHystTune [7:0],	* AFIT8_YUV422_DENOISE_iUVSupport [15:8]
    {0x0F12, 0x4603}, //70000904 //TVAR_afit_pBaseValS[160] /AFIT8_YUV422_DENOISE_iYSupport [7:0],   AFIT8_byr_cgras_iShadingPower [15:8]
    {0x0F12, 0x0080}, //70000906 //TVAR_afit_pBaseValS[161] /AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]
    {0x0F12, 0x0180}, //70000908 //TVAR_afit_pBaseValS[162] /AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]
    {0x0F12, 0x0080}, //7000090A //TVAR_afit_pBaseValS[163] /AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]
    {0x0F12, 0x1919}, //7000090C //TVAR_afit_pBaseValS[164] /AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
    {0x0F12, 0x0101}, //7000090E //TVAR_afit_pBaseValS[165] /AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]
    {0x0F12, 0x3C02}, //70000910 //TVAR_afit_pBaseValS[166] /AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]
    {0x0F12, 0x643C}, //70000912 //TVAR_afit_pBaseValS[167] /AFIT8_sddd8a_DispTH_High [7:0],	 AFIT8_sddd8a_iDenThreshLow [15:8]
    {0x0F12, 0x2864}, //70000914 //TVAR_afit_pBaseValS[168] /AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]
    {0x0F12, 0x0A00}, //70000916 //TVAR_afit_pBaseValS[169] /AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
    {0x0F12, 0x1403}, //70000918 //TVAR_afit_pBaseValS[170] /AFIT8_Demosaicing_iEdgeDesatLimit[7:0],	AFIT8_Demosaicing_iDemSharpenLow [15:8]
    {0x0F12, 0x1E0C}, //7000091A //TVAR_afit_pBaseValS[171] /AFIT8_Demosaicing_iDemSharpenHigh[7:0],	 AFIT8_Demosaicing_iDemSharpThresh [15:8]
    {0x0F12, 0x070A}, //7000091C //TVAR_afit_pBaseValS[172] /AFIT8_Demosaicing_iDemShLowLimit [7:0],	 AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
    {0x0F12, 0x32FF}, //7000091E //TVAR_afit_pBaseValS[173] /AFIT8_Demosaicing_iDemBlurLow[7:0],	 AFIT8_Demosaicing_iDemBlurHigh [15:8]
    {0x0F12, 0x4104}, //70000920 //TVAR_afit_pBaseValS[174] /AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]
    {0x0F12, 0x123C}, //70000922 //TVAR_afit_pBaseValS[175] /AFIT8_Sharpening_iHighSharpPower[7:0],	AFIT8_Sharpening_iLowShDenoise [15:8]
    {0x0F12, 0x4012}, //70000924 //TVAR_afit_pBaseValS[176] /AFIT8_Sharpening_iHighShDenoise [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
    {0x0F12, 0x0204}, //70000926 //TVAR_afit_pBaseValS[177] /AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
    {0x0F12, 0x1E03}, //70000928 //TVAR_afit_pBaseValS[178] /AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
    {0x0F12, 0x011E}, //7000092A //TVAR_afit_pBaseValS[179] /AFIT8_sddd8a_iClustThresh_C_Bin [7:0],	AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
    {0x0F12, 0x0201}, //7000092C //TVAR_afit_pBaseValS[180] /AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
    {0x0F12, 0x3232}, //7000092E //TVAR_afit_pBaseValS[181] /AFIT8_sddd8a_DispTH_Low_Bin [7:0],	AFIT8_sddd8a_DispTH_High_Bin [15:8]
    {0x0F12, 0x3C3C}, //70000930 //TVAR_afit_pBaseValS[182] /AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
    {0x0F12, 0x0028}, //70000932 //TVAR_afit_pBaseValS[183] /AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],	AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
    {0x0F12, 0x030A}, //70000934 //TVAR_afit_pBaseValS[184] /AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
    {0x0F12, 0x0714}, //70000936 //TVAR_afit_pBaseValS[185] /AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],	AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
    {0x0F12, 0x0A1E}, //70000938 //TVAR_afit_pBaseValS[186] /AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
    {0x0F12, 0xFF07}, //7000093A //TVAR_afit_pBaseValS[187] /AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
    {0x0F12, 0x0432}, //7000093C //TVAR_afit_pBaseValS[188] /AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
    {0x0F12, 0x4050}, //7000093E //TVAR_afit_pBaseValS[189] /AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
    {0x0F12, 0x0F0F}, //70000940 //TVAR_afit_pBaseValS[190] /AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
    {0x0F12, 0x0440}, //70000942 //TVAR_afit_pBaseValS[191] /AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
    {0x0F12, 0x0302}, //70000944 //TVAR_afit_pBaseValS[192] /AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
    {0x0F12, 0x1E1E}, //70000946 //TVAR_afit_pBaseValS[193] /AFIT8_sddd8a_iClustThresh_H_sBin[7:0],	AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
    {0x0F12, 0x0101}, //70000948 //TVAR_afit_pBaseValS[194] /AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
    {0x0F12, 0x3202}, //7000094A //TVAR_afit_pBaseValS[195] /AFIT8_sddd8a_nClustLevel_H_sBin [7:0],	AFIT8_sddd8a_DispTH_Low_sBin [15:8]
    {0x0F12, 0x3C32}, //7000094C //TVAR_afit_pBaseValS[196] /AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
    {0x0F12, 0x283C}, //7000094E //TVAR_afit_pBaseValS[197] /AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],	AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
    {0x0F12, 0x0A00}, //70000950 //TVAR_afit_pBaseValS[198] /AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
    {0x0F12, 0x1403}, //70000952 //TVAR_afit_pBaseValS[199] /AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
    {0x0F12, 0x1E07}, //70000954 //TVAR_afit_pBaseValS[200] /AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
    {0x0F12, 0x070A}, //70000956 //TVAR_afit_pBaseValS[201] /AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
    {0x0F12, 0x32FF}, //70000958 //TVAR_afit_pBaseValS[202] /AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
    {0x0F12, 0x5004}, //7000095A //TVAR_afit_pBaseValS[203] /AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],	AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
    {0x0F12, 0x0F40}, //7000095C //TVAR_afit_pBaseValS[204] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
    {0x0F12, 0x400F}, //7000095E //TVAR_afit_pBaseValS[205] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
    {0x0F12, 0x0204}, //70000960 //TVAR_afit_pBaseValS[206] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
    {0x0F12, 0x0003}, //70000962 //TVAR_afit_pBaseValS[207] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]
    {0x0F12, 0x0000}, //70000964 //TVAR_afit_pBaseValS[208] /AFIT16_BRIGHTNESS
    {0x0F12, 0x0000}, //70000966 //TVAR_afit_pBaseValS[209] /AFIT16_CONTRAST
    /*increase the power shading, reduce the saturation*/
    {0x0F12, 0x0014}, //70000968 //TVAR_afit_pBaseValS[210] /AFIT16_SATURATION
    {0x0F12, 0x0005}, //7000096A //TVAR_afit_pBaseValS[211] /AFIT16_SHARP_BLUR
    {0x0F12, 0xFFF6}, //7000096C //TVAR_afit_pBaseValS[212] /AFIT16_GLAMOUR
    {0x0F12, 0x00C4}, //7000096E //TVAR_afit_pBaseValS[213] /AFIT16_sddd8a_edge_high
    {0x0F12, 0x03FF}, //70000970 //TVAR_afit_pBaseValS[214] /AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x009C}, //70000972 //TVAR_afit_pBaseValS[215] /AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x017C}, //70000974 //TVAR_afit_pBaseValS[216] /AFIT16_demsharpmix1_iRGBOffset
    {0x0F12, 0x03FF}, //70000976 //TVAR_afit_pBaseValS[217] /AFIT16_demsharpmix1_iDemClamp
    {0x0F12, 0x000C}, //70000978 //TVAR_afit_pBaseValS[218] /AFIT16_demsharpmix1_iLowThreshold
    {0x0F12, 0x0010}, //7000097A //TVAR_afit_pBaseValS[219] /AFIT16_demsharpmix1_iHighThreshold
    {0x0F12, 0x012C}, //7000097C //TVAR_afit_pBaseValS[220] /AFIT16_demsharpmix1_iLowBright
    {0x0F12, 0x03E8}, //7000097E //TVAR_afit_pBaseValS[221] /AFIT16_demsharpmix1_iHighBright
    {0x0F12, 0x0046}, //70000980 //TVAR_afit_pBaseValS[222] /AFIT16_demsharpmix1_iLowSat
    {0x0F12, 0x0078}, //70000982 //TVAR_afit_pBaseValS[223] /AFIT16_demsharpmix1_iHighSat
    {0x0F12, 0x0070}, //70000984 //TVAR_afit_pBaseValS[224] /AFIT16_demsharpmix1_iTune
    {0x0F12, 0x0004}, //70000986 //TVAR_afit_pBaseValS[225] /AFIT16_demsharpmix1_iHystThLow
    {0x0F12, 0x0004}, //70000988 //TVAR_afit_pBaseValS[226] /AFIT16_demsharpmix1_iHystThHigh
    {0x0F12, 0x01AA}, //7000098A //TVAR_afit_pBaseValS[227] /AFIT16_demsharpmix1_iHystCenter
    {0x0F12, 0x001E}, //7000098C //TVAR_afit_pBaseValS[228] /AFIT16_YUV422_DENOISE_iUVLowThresh
    {0x0F12, 0x001E}, //7000098E //TVAR_afit_pBaseValS[229] /AFIT16_YUV422_DENOISE_iUVHighThresh
    {0x0F12, 0x0005}, //70000990 //TVAR_afit_pBaseValS[230] /AFIT16_YUV422_DENOISE_iYLowThresh
    {0x0F12, 0x0005}, //70000992 //TVAR_afit_pBaseValS[231] /AFIT16_YUV422_DENOISE_iYHighThresh
    {0x0F12, 0x0064}, //70000994 //TVAR_afit_pBaseValS[232] /AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x001B}, //70000996 //TVAR_afit_pBaseValS[233] /AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x002A}, //70000998 //TVAR_afit_pBaseValS[234] /AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x0024}, //7000099A //TVAR_afit_pBaseValS[235] /AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x002A}, //7000099C //TVAR_afit_pBaseValS[236] /AFIT16_Sharpening_iLowSharpClamp_sBin
    {0x0F12, 0x0024}, //7000099E //TVAR_afit_pBaseValS[237] /AFIT16_Sharpening_iHighSharpClamp_sBin
    {0x0F12, 0x0A24}, //700009A0 //TVAR_afit_pBaseValS[238] /AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
    {0x0F12, 0x1701}, //700009A2 //TVAR_afit_pBaseValS[239] /AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
    {0x0F12, 0x0229}, //700009A4 //TVAR_afit_pBaseValS[240] /AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
    {0x0F12, 0x1403}, //700009A6 //TVAR_afit_pBaseValS[241] /AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
    {0x0F12, 0x0000}, //700009A8 //TVAR_afit_pBaseValS[242] /AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]
    {0x0F12, 0x0000}, //700009AA //TVAR_afit_pBaseValS[243] /AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
    {0x0F12, 0x0000}, //700009AC //TVAR_afit_pBaseValS[244] /AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]
    {0x0F12, 0x00FF}, //700009AE //TVAR_afit_pBaseValS[245] /AFIT8_sddd8a_iSatSat[7:0],	AFIT8_sddd8a_iRadialTune [15:8]
    {0x0F12, 0x043B}, //700009B0 //TVAR_afit_pBaseValS[246] /AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]
    {0x0F12, 0x1414}, //700009B2 //TVAR_afit_pBaseValS[247] /AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],	AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
    {0x0F12, 0x0301}, //700009B4 //TVAR_afit_pBaseValS[248] /AFIT8_sddd8a_iLowSlopeThresh[7:0],	AFIT8_sddd8a_iHighSlopeThresh [15:8]
    {0x0F12, 0xFF07}, //700009B6 //TVAR_afit_pBaseValS[249] /AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]
    {0x0F12, 0x051E}, //700009B8 //TVAR_afit_pBaseValS[250] /AFIT8_Demosaicing_iMonochrom [7:0],	 AFIT8_Demosaicing_iDecisionThresh [15:8]
    {0x0F12, 0x0A1E}, //700009BA //TVAR_afit_pBaseValS[251] /AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]
    {0x0F12, 0x0F0F}, //700009BC //TVAR_afit_pBaseValS[252] /AFIT8_Demosaicing_iGRDenoiseVal [7:0],	AFIT8_Demosaicing_iGBDenoiseVal [15:8]
    {0x0F12, 0x0A03}, //700009BE //TVAR_afit_pBaseValS[253] /AFIT8_Demosaicing_iNearGrayDesat[7:0],	AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
    {0x0F12, 0x0A3C}, //700009C0 //TVAR_afit_pBaseValS[254] /AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]
    {0x0F12, 0x0528}, //700009C2 //TVAR_afit_pBaseValS[255] /AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]
    {0x0F12, 0x0002}, //700009C4 //TVAR_afit_pBaseValS[256] /AFIT8_Sharpening_nSharpWidth [7:0],	 AFIT8_Sharpening_iReduceNegative [15:8]
    {0x0F12, 0x00FF}, //700009C6 //TVAR_afit_pBaseValS[257] /AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
    {0x0F12, 0x1102}, //700009C8 //TVAR_afit_pBaseValS[258] /AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
    {0x0F12, 0x001B}, //700009CA //TVAR_afit_pBaseValS[259] /AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]
    {0x0F12, 0x0900}, //700009CC //TVAR_afit_pBaseValS[260] /AFIT8_demsharpmix1_iNarrMult [7:0],	 AFIT8_demsharpmix1_iHystFalloff [15:8]
    {0x0F12, 0x0600}, //700009CE //TVAR_afit_pBaseValS[261] /AFIT8_demsharpmix1_iHystMinMult [7:0],	AFIT8_demsharpmix1_iHystWidth [15:8]
    {0x0F12, 0x0504}, //700009D0 //TVAR_afit_pBaseValS[262] /AFIT8_demsharpmix1_iHystFallLow [7:0],	AFIT8_demsharpmix1_iHystFallHigh [15:8]
    {0x0F12, 0x0305}, //700009D2 //TVAR_afit_pBaseValS[263] /AFIT8_demsharpmix1_iHystTune [7:0],	* AFIT8_YUV422_DENOISE_iUVSupport [15:8]
    {0x0F12, 0x5A03}, //4603  //// #TVAR_afit_pBaseVals[264]
    {0x0F12, 0x0080}, //700009D6 //TVAR_afit_pBaseValS[265] /AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]
    {0x0F12, 0x0180}, //700009D8 //TVAR_afit_pBaseValS[266] /AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]
    {0x0F12, 0x0080}, //700009DA //TVAR_afit_pBaseValS[267] /AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]
    {0x0F12, 0x2323}, //700009DC //TVAR_afit_pBaseValS[268] /AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
    {0x0F12, 0x0101}, //700009DE //TVAR_afit_pBaseValS[269] /AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]
    {0x0F12, 0x2A02}, //700009E0 //TVAR_afit_pBaseValS[270] /AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]
    {0x0F12, 0x3C2A}, //700009E2 //TVAR_afit_pBaseValS[271] /AFIT8_sddd8a_DispTH_High [7:0],	 AFIT8_sddd8a_iDenThreshLow [15:8]
    {0x0F12, 0x283C}, //700009E4 //TVAR_afit_pBaseValS[272] /AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]
    {0x0F12, 0x0A00}, //700009E6 //TVAR_afit_pBaseValS[273] /AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
    {0x0F12, 0x1403}, //700009E8 //TVAR_afit_pBaseValS[274] /AFIT8_Demosaicing_iEdgeDesatLimit[7:0],	AFIT8_Demosaicing_iDemSharpenLow [15:8]
    {0x0F12, 0x1E0C}, //700009EA //TVAR_afit_pBaseValS[275] /AFIT8_Demosaicing_iDemSharpenHigh[7:0],	 AFIT8_Demosaicing_iDemSharpThresh [15:8]
    {0x0F12, 0x070A}, //700009EC //TVAR_afit_pBaseValS[276] /AFIT8_Demosaicing_iDemShLowLimit [7:0],	 AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
    {0x0F12, 0x32FF}, //700009EE //TVAR_afit_pBaseValS[277] /AFIT8_Demosaicing_iDemBlurLow[7:0],	 AFIT8_Demosaicing_iDemBlurHigh [15:8]
    {0x0F12, 0x4B04}, //700009F0 //TVAR_afit_pBaseValS[278] /AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]
    {0x0F12, 0x0F40}, //700009F2 //TVAR_afit_pBaseValS[279] /AFIT8_Sharpening_iHighSharpPower[7:0],	AFIT8_Sharpening_iLowShDenoise [15:8]
    {0x0F12, 0x400F}, //700009F4 //TVAR_afit_pBaseValS[280] /AFIT8_Sharpening_iHighShDenoise [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
    {0x0F12, 0x0204}, //700009F6 //TVAR_afit_pBaseValS[281] /AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
    {0x0F12, 0x2303}, //700009F8 //TVAR_afit_pBaseValS[282] /AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
    {0x0F12, 0x0123}, //700009FA //TVAR_afit_pBaseValS[283] /AFIT8_sddd8a_iClustThresh_C_Bin [7:0],	AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
    {0x0F12, 0x0201}, //700009FC //TVAR_afit_pBaseValS[284] /AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
    {0x0F12, 0x262A}, //700009FE //TVAR_afit_pBaseValS[285] /AFIT8_sddd8a_DispTH_Low_Bin [7:0],	AFIT8_sddd8a_DispTH_High_Bin [15:8]
    {0x0F12, 0x2C2C}, //70000A00 //TVAR_afit_pBaseValS[286] /AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
    {0x0F12, 0x0028}, //70000A02 //TVAR_afit_pBaseValS[287] /AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],	AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
    {0x0F12, 0x030A}, //70000A04 //TVAR_afit_pBaseValS[288] /AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
    {0x0F12, 0x0714}, //70000A06 //TVAR_afit_pBaseValS[289] /AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],	AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
    {0x0F12, 0x0A1E}, //70000A08 //TVAR_afit_pBaseValS[290] /AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
    {0x0F12, 0xFF07}, //70000A0A //TVAR_afit_pBaseValS[291] /AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
    {0x0F12, 0x0432}, //70000A0C //TVAR_afit_pBaseValS[292] /AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
    {0x0F12, 0x4050}, //70000A0E //TVAR_afit_pBaseValS[293] /AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
    {0x0F12, 0x0F0F}, //70000A10 //TVAR_afit_pBaseValS[294] /AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
    {0x0F12, 0x0440}, //70000A12 //TVAR_afit_pBaseValS[295] /AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
    {0x0F12, 0x0302}, //70000A14 //TVAR_afit_pBaseValS[296] /AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
    {0x0F12, 0x2323}, //70000A16 //TVAR_afit_pBaseValS[297] /AFIT8_sddd8a_iClustThresh_H_sBin[7:0],	AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
    {0x0F12, 0x0101}, //70000A18 //TVAR_afit_pBaseValS[298] /AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
    {0x0F12, 0x2A02}, //70000A1A //TVAR_afit_pBaseValS[299] /AFIT8_sddd8a_nClustLevel_H_sBin [7:0],	AFIT8_sddd8a_DispTH_Low_sBin [15:8]
    {0x0F12, 0x2C26}, //70000A1C //TVAR_afit_pBaseValS[300] /AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
    {0x0F12, 0x282C}, //70000A1E //TVAR_afit_pBaseValS[301] /AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],	AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
    {0x0F12, 0x0A00}, //70000A20 //TVAR_afit_pBaseValS[302] /AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
    {0x0F12, 0x1403}, //70000A22 //TVAR_afit_pBaseValS[303] /AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
    {0x0F12, 0x1E07}, //70000A24 //TVAR_afit_pBaseValS[304] /AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
    {0x0F12, 0x070A}, //70000A26 //TVAR_afit_pBaseValS[305] /AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
    {0x0F12, 0x32FF}, //70000A28 //TVAR_afit_pBaseValS[306] /AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
    {0x0F12, 0x5004}, //70000A2A //TVAR_afit_pBaseValS[307] /AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],	AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
    {0x0F12, 0x0F40}, //70000A2C //TVAR_afit_pBaseValS[308] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
    {0x0F12, 0x400F}, //70000A2E //TVAR_afit_pBaseValS[309] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
    {0x0F12, 0x0204}, //70000A30 //TVAR_afit_pBaseValS[310] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
    {0x0F12, 0x0003}, //70000A32 //TVAR_afit_pBaseValS[311] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]
    {0x0F12, 0x0000}, //70000A34 //TVAR_afit_pBaseValS[312] /AFIT16_BRIGHTNESS
    {0x0F12, 0x0000}, //70000A36 //TVAR_afit_pBaseValS[313] /AFIT16_CONTRAST
    {0x0F12, 0x0014}, //70000A38 //TVAR_afit_pBaseValS[314] /AFIT16_SATURATION
    {0x0F12, 0x0000}, //70000A3A //TVAR_afit_pBaseValS[315] /AFIT16_SHARP_BLUR
    {0x0F12, 0x0000}, //70000A3C //TVAR_afit_pBaseValS[316] /AFIT16_GLAMOUR
    {0x0F12, 0x00C4}, //70000A3E //TVAR_afit_pBaseValS[317] /AFIT16_sddd8a_edge_high
    {0x0F12, 0x03FF}, //70000A40 //TVAR_afit_pBaseValS[318] /AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x009C}, //70000A42 //TVAR_afit_pBaseValS[319] /AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x017C}, //70000A44 //TVAR_afit_pBaseValS[320] /AFIT16_demsharpmix1_iRGBOffset
    {0x0F12, 0x03FF}, //70000A46 //TVAR_afit_pBaseValS[321] /AFIT16_demsharpmix1_iDemClamp
    {0x0F12, 0x000C}, //70000A48 //TVAR_afit_pBaseValS[322] /AFIT16_demsharpmix1_iLowThreshold
    {0x0F12, 0x0010}, //70000A4A //TVAR_afit_pBaseValS[323] /AFIT16_demsharpmix1_iHighThreshold
    {0x0F12, 0x00C8}, //70000A4C //TVAR_afit_pBaseValS[324] /AFIT16_demsharpmix1_iLowBright
    {0x0F12, 0x0384}, //70000A4E //TVAR_afit_pBaseValS[325] /AFIT16_demsharpmix1_iHighBright
    {0x0F12, 0x0046}, //70000A50 //TVAR_afit_pBaseValS[326] /AFIT16_demsharpmix1_iLowSat
    {0x0F12, 0x0082}, //70000A52 //TVAR_afit_pBaseValS[327] /AFIT16_demsharpmix1_iHighSat
    {0x0F12, 0x0070}, //70000A54 //TVAR_afit_pBaseValS[328] /AFIT16_demsharpmix1_iTune
    {0x0F12, 0x0000}, //70000A56 //TVAR_afit_pBaseValS[329] /AFIT16_demsharpmix1_iHystThLow
    {0x0F12, 0x0000}, //70000A58 //TVAR_afit_pBaseValS[330] /AFIT16_demsharpmix1_iHystThHigh
    {0x0F12, 0x01AA}, //70000A5A //TVAR_afit_pBaseValS[331] /AFIT16_demsharpmix1_iHystCenter
    {0x0F12, 0x001E}, //70000A5C //TVAR_afit_pBaseValS[332] /AFIT16_YUV422_DENOISE_iUVLowThresh
    {0x0F12, 0x001E}, //70000A5E //TVAR_afit_pBaseValS[333] /AFIT16_YUV422_DENOISE_iUVHighThresh
    {0x0F12, 0x000A}, //70000A60 //TVAR_afit_pBaseValS[334] /AFIT16_YUV422_DENOISE_iYLowThresh
    {0x0F12, 0x000A}, //70000A62 //TVAR_afit_pBaseValS[335] /AFIT16_YUV422_DENOISE_iYHighThresh
    {0x0F12, 0x010E}, //70000A64 //TVAR_afit_pBaseValS[336] /AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x0028}, //70000A66 //TVAR_afit_pBaseValS[337] /AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x0032}, //70000A68 //TVAR_afit_pBaseValS[338] /AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x0028}, //70000A6A //TVAR_afit_pBaseValS[339] /AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x0032}, //70000A6C //TVAR_afit_pBaseValS[340] /AFIT16_Sharpening_iLowSharpClamp_sBin
    {0x0F12, 0x0028}, //70000A6E //TVAR_afit_pBaseValS[341] /AFIT16_Sharpening_iHighSharpClamp_sBin
    {0x0F12, 0x0A24}, //70000A70 //TVAR_afit_pBaseValS[342] /AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
    {0x0F12, 0x1701}, //70000A72 //TVAR_afit_pBaseValS[343] /AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
    {0x0F12, 0x0229}, //70000A74 //TVAR_afit_pBaseValS[344] /AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
    {0x0F12, 0x1403}, //70000A76 //TVAR_afit_pBaseValS[345] /AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
    {0x0F12, 0x0000}, //70000A78 //TVAR_afit_pBaseValS[346] /AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]
    {0x0F12, 0x0000}, //70000A7A //TVAR_afit_pBaseValS[347] /AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
    {0x0F12, 0x0504}, //70000A7C //TVAR_afit_pBaseValS[348] /AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]
    {0x0F12, 0x00FF}, //70000A7E //TVAR_afit_pBaseValS[349] /AFIT8_sddd8a_iSatSat[7:0],	AFIT8_sddd8a_iRadialTune [15:8]
    {0x0F12, 0x043B}, //70000A80 //TVAR_afit_pBaseValS[350] /AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]
    {0x0F12, 0x1414}, //70000A82 //TVAR_afit_pBaseValS[351] /AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],	AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
    {0x0F12, 0x0301}, //70000A84 //TVAR_afit_pBaseValS[352] /AFIT8_sddd8a_iLowSlopeThresh[7:0],	AFIT8_sddd8a_iHighSlopeThresh [15:8]
    {0x0F12, 0xFF07}, //70000A86 //TVAR_afit_pBaseValS[353] /AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]
    {0x0F12, 0x051E}, //70000A88 //TVAR_afit_pBaseValS[354] /AFIT8_Demosaicing_iMonochrom [7:0],	 AFIT8_Demosaicing_iDecisionThresh [15:8]
    {0x0F12, 0x0A1E}, //70000A8A //TVAR_afit_pBaseValS[355] /AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]
    {0x0F12, 0x0F0F}, //70000A8C //TVAR_afit_pBaseValS[356] /AFIT8_Demosaicing_iGRDenoiseVal [7:0],	AFIT8_Demosaicing_iGBDenoiseVal [15:8]
    {0x0F12, 0x0A00}, //70000A8E //TVAR_afit_pBaseValS[357] /AFIT8_Demosaicing_iNearGrayDesat[7:0],	AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
    {0x0F12, 0x0A3C}, //70000A90 //TVAR_afit_pBaseValS[358] /AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]
    {0x0F12, 0x0532}, //70000A92 //TVAR_afit_pBaseValS[359] /AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]
    {0x0F12, 0x0002}, //70000A94 //TVAR_afit_pBaseValS[360] /AFIT8_Sharpening_nSharpWidth [7:0],	 AFIT8_Sharpening_iReduceNegative [15:8]
    {0x0F12, 0x00FF}, //70000A96 //TVAR_afit_pBaseValS[361] /AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
    {0x0F12, 0x1002}, //70000A98 //TVAR_afit_pBaseValS[362] /AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
    {0x0F12, 0x001E}, //70000A9A //TVAR_afit_pBaseValS[363] /AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]
    {0x0F12, 0x0900}, //70000A9C //TVAR_afit_pBaseValS[364] /AFIT8_demsharpmix1_iNarrMult [7:0],	 AFIT8_demsharpmix1_iHystFalloff [15:8]
    {0x0F12, 0x0600}, //70000A9E //TVAR_afit_pBaseValS[365] /AFIT8_demsharpmix1_iHystMinMult [7:0],	AFIT8_demsharpmix1_iHystWidth [15:8]
    {0x0F12, 0x0504}, //70000AA0 //TVAR_afit_pBaseValS[366] /AFIT8_demsharpmix1_iHystFallLow [7:0],	AFIT8_demsharpmix1_iHystFallHigh [15:8]
    {0x0F12, 0x0305}, //70000AA2 //TVAR_afit_pBaseValS[367] /AFIT8_demsharpmix1_iHystTune [7:0],	* AFIT8_YUV422_DENOISE_iUVSupport [15:8]
    {0x0F12, 0x6E02}, //4602  //// #TVAR_afit_pBaseVals[368]
    {0x0F12, 0x0080}, //70000AA6 //TVAR_afit_pBaseValS[369] /AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]
    {0x0F12, 0x0180}, //70000AA8 //TVAR_afit_pBaseValS[370] /AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]
    {0x0F12, 0x0080}, //70000AAA //TVAR_afit_pBaseValS[371] /AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]
    {0x0F12, 0x2328}, //70000AAC //TVAR_afit_pBaseValS[372] /AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
    {0x0F12, 0x0101}, //70000AAE //TVAR_afit_pBaseValS[373] /AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]
    {0x0F12, 0x2A02}, //70000AB0 //TVAR_afit_pBaseValS[374] /AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]
    {0x0F12, 0x2228}, //70000AB2 //TVAR_afit_pBaseValS[375] /AFIT8_sddd8a_DispTH_High [7:0],	 AFIT8_sddd8a_iDenThreshLow [15:8]
    {0x0F12, 0x2822}, //70000AB4 //TVAR_afit_pBaseValS[376] /AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]
    {0x0F12, 0x0A00}, //70000AB6 //TVAR_afit_pBaseValS[377] /AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
    {0x0F12, 0x1903}, //70000AB8 //TVAR_afit_pBaseValS[378] /AFIT8_Demosaicing_iEdgeDesatLimit[7:0],	AFIT8_Demosaicing_iDemSharpenLow [15:8]
    {0x0F12, 0x1E0F}, //70000ABA //TVAR_afit_pBaseValS[379] /AFIT8_Demosaicing_iDemSharpenHigh[7:0],	 AFIT8_Demosaicing_iDemSharpThresh [15:8]
    {0x0F12, 0x070A}, //70000ABC //TVAR_afit_pBaseValS[380] /AFIT8_Demosaicing_iDemShLowLimit [7:0],	 AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
    {0x0F12, 0x32FF}, //70000ABE //TVAR_afit_pBaseValS[381] /AFIT8_Demosaicing_iDemBlurLow[7:0],	 AFIT8_Demosaicing_iDemBlurHigh [15:8]
    {0x0F12, 0x9604}, //70000AC0 //TVAR_afit_pBaseValS[382] /AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]
    {0x0F12, 0x0F42}, //70000AC2 //TVAR_afit_pBaseValS[383] /AFIT8_Sharpening_iHighSharpPower[7:0],	AFIT8_Sharpening_iLowShDenoise [15:8]
    {0x0F12, 0x400F}, //70000AC4 //TVAR_afit_pBaseValS[384] /AFIT8_Sharpening_iHighShDenoise [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
    {0x0F12, 0x0504}, //70000AC6 //TVAR_afit_pBaseValS[385] /AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
    {0x0F12, 0x2805}, //70000AC8 //TVAR_afit_pBaseValS[386] /AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
    {0x0F12, 0x0123}, //70000ACA //TVAR_afit_pBaseValS[387] /AFIT8_sddd8a_iClustThresh_C_Bin [7:0],	AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
    {0x0F12, 0x0201}, //70000ACC //TVAR_afit_pBaseValS[388] /AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
    {0x0F12, 0x2024}, //70000ACE //TVAR_afit_pBaseValS[389] /AFIT8_sddd8a_DispTH_Low_Bin [7:0],	AFIT8_sddd8a_DispTH_High_Bin [15:8]
    {0x0F12, 0x1C1C}, //70000AD0 //TVAR_afit_pBaseValS[390] /AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
    {0x0F12, 0x0028}, //70000AD2 //TVAR_afit_pBaseValS[391] /AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],	AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
    {0x0F12, 0x030A}, //70000AD4 //TVAR_afit_pBaseValS[392] /AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
    {0x0F12, 0x0A0A}, //70000AD6 //TVAR_afit_pBaseValS[393] /AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],	AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
    {0x0F12, 0x0A2D}, //70000AD8 //TVAR_afit_pBaseValS[394] /AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
    {0x0F12, 0xFF07}, //70000ADA //TVAR_afit_pBaseValS[395] /AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
    {0x0F12, 0x0432}, //70000ADC //TVAR_afit_pBaseValS[396] /AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
    {0x0F12, 0x4050}, //70000ADE //TVAR_afit_pBaseValS[397] /AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
    {0x0F12, 0x0F0F}, //70000AE0 //TVAR_afit_pBaseValS[398] /AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
    {0x0F12, 0x0440}, //70000AE2 //TVAR_afit_pBaseValS[399] /AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
    {0x0F12, 0x0302}, //70000AE4 //TVAR_afit_pBaseValS[400] /AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
    {0x0F12, 0x2328}, //70000AE6 //TVAR_afit_pBaseValS[401] /AFIT8_sddd8a_iClustThresh_H_sBin[7:0],	AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
    {0x0F12, 0x0101}, //70000AE8 //TVAR_afit_pBaseValS[402] /AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
    {0x0F12, 0x3C02}, //70000AEA //TVAR_afit_pBaseValS[403] /AFIT8_sddd8a_nClustLevel_H_sBin [7:0],	AFIT8_sddd8a_DispTH_Low_sBin [15:8]
    {0x0F12, 0x1C3C}, //70000AEC //TVAR_afit_pBaseValS[404] /AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
    {0x0F12, 0x281C}, //70000AEE //TVAR_afit_pBaseValS[405] /AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],	AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
    {0x0F12, 0x0A00}, //70000AF0 //TVAR_afit_pBaseValS[406] /AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
    {0x0F12, 0x0A03}, //70000AF2 //TVAR_afit_pBaseValS[407] /AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
    {0x0F12, 0x2D0A}, //70000AF4 //TVAR_afit_pBaseValS[408] /AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
    {0x0F12, 0x070A}, //70000AF6 //TVAR_afit_pBaseValS[409] /AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
    {0x0F12, 0x32FF}, //70000AF8 //TVAR_afit_pBaseValS[410] /AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
    {0x0F12, 0x5004}, //70000AFA //TVAR_afit_pBaseValS[411] /AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],	AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
    {0x0F12, 0x0F40}, //70000AFC //TVAR_afit_pBaseValS[412] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
    {0x0F12, 0x400F}, //70000AFE //TVAR_afit_pBaseValS[413] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
    {0x0F12, 0x0204}, //70000B00 //TVAR_afit_pBaseValS[414] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
    {0x0F12, 0x0003}, //70000B02 //TVAR_afit_pBaseValS[415] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]
    {0x0F12, 0x0000}, //70000B04 //TVAR_afit_pBaseValS[416] /AFIT16_BRIGHTNESS
    {0x0F12, 0x0000}, //70000B06 //TVAR_afit_pBaseValS[417] /AFIT16_CONTRAST
    {0x0F12, 0x0014}, //70000B08 //TVAR_afit_pBaseValS[418] /AFIT16_SATURATION
    {0x0F12, 0x0000}, //70000B0A //TVAR_afit_pBaseValS[419] /AFIT16_SHARP_BLUR
    {0x0F12, 0x0000}, //70000B0C //TVAR_afit_pBaseValS[420] /AFIT16_GLAMOUR
    {0x0F12, 0x00C4}, //70000B0E //TVAR_afit_pBaseValS[421] /AFIT16_sddd8a_edge_high
    {0x0F12, 0x03FF}, //70000B10 //TVAR_afit_pBaseValS[422] /AFIT16_Demosaicing_iSatVal
    {0x0F12, 0x009C}, //70000B12 //TVAR_afit_pBaseValS[423] /AFIT16_Sharpening_iReduceEdgeThresh
    {0x0F12, 0x017C}, //70000B14 //TVAR_afit_pBaseValS[424] /AFIT16_demsharpmix1_iRGBOffset
    {0x0F12, 0x03FF}, //70000B16 //TVAR_afit_pBaseValS[425] /AFIT16_demsharpmix1_iDemClamp
    {0x0F12, 0x000C}, //70000B18 //TVAR_afit_pBaseValS[426] /AFIT16_demsharpmix1_iLowThreshold
    {0x0F12, 0x0010}, //70000B1A //TVAR_afit_pBaseValS[427] /AFIT16_demsharpmix1_iHighThreshold
    {0x0F12, 0x00C8}, //70000B1C //TVAR_afit_pBaseValS[428] /AFIT16_demsharpmix1_iLowBright
    {0x0F12, 0x0320}, //70000B1E //TVAR_afit_pBaseValS[429] /AFIT16_demsharpmix1_iHighBright
    {0x0F12, 0x0046}, //70000B20 //TVAR_afit_pBaseValS[430] /AFIT16_demsharpmix1_iLowSat
    {0x0F12, 0x015E}, //70000B22 //TVAR_afit_pBaseValS[431] /AFIT16_demsharpmix1_iHighSat
    {0x0F12, 0x0070}, //70000B24 //TVAR_afit_pBaseValS[432] /AFIT16_demsharpmix1_iTune
    {0x0F12, 0x0000}, //70000B26 //TVAR_afit_pBaseValS[433] /AFIT16_demsharpmix1_iHystThLow
    {0x0F12, 0x0000}, //70000B28 //TVAR_afit_pBaseValS[434] /AFIT16_demsharpmix1_iHystThHigh
    {0x0F12, 0x01AA}, //70000B2A //TVAR_afit_pBaseValS[435] /AFIT16_demsharpmix1_iHystCenter
    {0x0F12, 0x0014}, //70000B2C //TVAR_afit_pBaseValS[436] /AFIT16_YUV422_DENOISE_iUVLowThresh
    {0x0F12, 0x0014}, //70000B2E //TVAR_afit_pBaseValS[437] /AFIT16_YUV422_DENOISE_iUVHighThresh
    {0x0F12, 0x000A}, //70000B30 //TVAR_afit_pBaseValS[438] /AFIT16_YUV422_DENOISE_iYLowThresh
    {0x0F12, 0x000A}, //70000B32 //TVAR_afit_pBaseValS[439] /AFIT16_YUV422_DENOISE_iYHighThresh
    {0x0F12, 0x0140}, //70000B34 //TVAR_afit_pBaseValS[440] /AFIT16_Sharpening_iLowSharpClamp
    {0x0F12, 0x003C}, //70000B36 //TVAR_afit_pBaseValS[441] /AFIT16_Sharpening_iHighSharpClamp
    {0x0F12, 0x0032}, //70000B38 //TVAR_afit_pBaseValS[442] /AFIT16_Sharpening_iLowSharpClamp_Bin
    {0x0F12, 0x0023}, //70000B3A //TVAR_afit_pBaseValS[443] /AFIT16_Sharpening_iHighSharpClamp_Bin
    {0x0F12, 0x0023}, //70000B3C //TVAR_afit_pBaseValS[444] /AFIT16_Sharpening_iLowSharpClamp_sBin
    {0x0F12, 0x0032}, //70000B3E //TVAR_afit_pBaseValS[445] /AFIT16_Sharpening_iHighSharpClamp_sBin
    {0x0F12, 0x0A24}, //70000B40 //TVAR_afit_pBaseValS[446] /AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
    {0x0F12, 0x1701}, //70000B42 //TVAR_afit_pBaseValS[447] /AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
    {0x0F12, 0x0229}, //70000B44 //TVAR_afit_pBaseValS[448] /AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
    {0x0F12, 0x1403}, //70000B46 //TVAR_afit_pBaseValS[449] /AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
    {0x0F12, 0x0000}, //70000B48 //TVAR_afit_pBaseValS[450] /AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]
    {0x0F12, 0x0000}, //70000B4A //TVAR_afit_pBaseValS[451] /AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
    {0x0F12, 0x0505}, //70000B4C //TVAR_afit_pBaseValS[452] /AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]
    {0x0F12, 0x00FF}, //70000B4E //TVAR_afit_pBaseValS[453] /AFIT8_sddd8a_iSatSat[7:0],	AFIT8_sddd8a_iRadialTune [15:8]
    {0x0F12, 0x043B}, //70000B50 //TVAR_afit_pBaseValS[454] /AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]
    {0x0F12, 0x1414}, //70000B52 //TVAR_afit_pBaseValS[455] /AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],	AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
    {0x0F12, 0x0301}, //70000B54 //TVAR_afit_pBaseValS[456] /AFIT8_sddd8a_iLowSlopeThresh[7:0],	AFIT8_sddd8a_iHighSlopeThresh [15:8]
    {0x0F12, 0xFF07}, //70000B56 //TVAR_afit_pBaseValS[457] /AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]
    {0x0F12, 0x051E}, //70000B58 //TVAR_afit_pBaseValS[458] /AFIT8_Demosaicing_iMonochrom [7:0],	 AFIT8_Demosaicing_iDecisionThresh [15:8]
    {0x0F12, 0x0A1E}, //70000B5A //TVAR_afit_pBaseValS[459] /AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]
    {0x0F12, 0x0000}, //70000B5C //TVAR_afit_pBaseValS[460] /AFIT8_Demosaicing_iGRDenoiseVal [7:0],	AFIT8_Demosaicing_iGBDenoiseVal [15:8]
    {0x0F12, 0x0A00}, //70000B5E //TVAR_afit_pBaseValS[461] /AFIT8_Demosaicing_iNearGrayDesat[7:0],	AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
    {0x0F12, 0x143C}, //70000B60 //TVAR_afit_pBaseValS[462] /AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]
    {0x0F12, 0x0532}, //70000B62 //TVAR_afit_pBaseValS[463] /AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]
    {0x0F12, 0x0002}, //70000B64 //TVAR_afit_pBaseValS[464] /AFIT8_Sharpening_nSharpWidth [7:0],	 AFIT8_Sharpening_iReduceNegative [15:8]
    {0x0F12, 0x0096}, //70000B66 //TVAR_afit_pBaseValS[465] /AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
    {0x0F12, 0x1002}, //70000B68 //TVAR_afit_pBaseValS[466] /AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
    {0x0F12, 0x001E}, //70000B6A //TVAR_afit_pBaseValS[467] /AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]
    {0x0F12, 0x0900}, //70000B6C //TVAR_afit_pBaseValS[468] /AFIT8_demsharpmix1_iNarrMult [7:0],	 AFIT8_demsharpmix1_iHystFalloff [15:8]
    {0x0F12, 0x0600}, //70000B6E //TVAR_afit_pBaseValS[469] /AFIT8_demsharpmix1_iHystMinMult [7:0],	AFIT8_demsharpmix1_iHystWidth [15:8]
    {0x0F12, 0x0504}, //70000B70 //TVAR_afit_pBaseValS[470] /AFIT8_demsharpmix1_iHystFallLow [7:0],	AFIT8_demsharpmix1_iHystFallHigh [15:8]
    {0x0F12, 0x0305}, //70000B72 //TVAR_afit_pBaseValS[471] /AFIT8_demsharpmix1_iHystTune [7:0],	* AFIT8_YUV422_DENOISE_iUVSupport [15:8]
    {0x0F12, 0x7802}, //70000B74 //TVAR_afit_pBaseValS[472] /AFIT8_YUV422_DENOISE_iYSupport [7:0],   AFIT8_byr_cgras_iShadingPower [15:8]
    {0x0F12, 0x0080}, //70000B76 //TVAR_afit_pBaseValS[473] /AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]
    {0x0F12, 0x0180}, //70000B78 //TVAR_afit_pBaseValS[474] /AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]
    {0x0F12, 0x0080}, //70000B7A //TVAR_afit_pBaseValS[475] /AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]
    {0x0F12, 0x5050}, //70000B7C //TVAR_afit_pBaseValS[476] /AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
    {0x0F12, 0x0101}, //70000B7E //TVAR_afit_pBaseValS[477] /AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]
    {0x0F12, 0x1C02}, //70000B80 //TVAR_afit_pBaseValS[478] /AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]
    {0x0F12, 0x191C}, //70000B82 //TVAR_afit_pBaseValS[479] /AFIT8_sddd8a_DispTH_High [7:0],	 AFIT8_sddd8a_iDenThreshLow [15:8]
    {0x0F12, 0x2819}, //70000B84 //TVAR_afit_pBaseValS[480] /AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]
    {0x0F12, 0x0A00}, //70000B86 //TVAR_afit_pBaseValS[481] /AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
    {0x0F12, 0x1E03}, //70000B88 //TVAR_afit_pBaseValS[482] /AFIT8_Demosaicing_iEdgeDesatLimit[7:0],	AFIT8_Demosaicing_iDemSharpenLow [15:8]
    {0x0F12, 0x1E0F}, //70000B8A //TVAR_afit_pBaseValS[483] /AFIT8_Demosaicing_iDemSharpenHigh[7:0],	 AFIT8_Demosaicing_iDemSharpThresh [15:8]
    {0x0F12, 0x0508}, //70000B8C //TVAR_afit_pBaseValS[484] /AFIT8_Demosaicing_iDemShLowLimit [7:0],	 AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
    {0x0F12, 0x32FF}, //70000B8E //TVAR_afit_pBaseValS[485] /AFIT8_Demosaicing_iDemBlurLow[7:0],	 AFIT8_Demosaicing_iDemBlurHigh [15:8]
    {0x0F12, 0xAA04}, //70000B90 //TVAR_afit_pBaseValS[486] /AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]
    {0x0F12, 0x1452}, //70000B92 //TVAR_afit_pBaseValS[487] /AFIT8_Sharpening_iHighSharpPower[7:0],	AFIT8_Sharpening_iLowShDenoise [15:8]
    {0x0F12, 0x4015}, //70000B94 //TVAR_afit_pBaseValS[488] /AFIT8_Sharpening_iHighShDenoise [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
    {0x0F12, 0x0604}, //70000B96 //TVAR_afit_pBaseValS[489] /AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
    {0x0F12, 0x5006}, //70000B98 //TVAR_afit_pBaseValS[490] /AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
    {0x0F12, 0x0150}, //70000B9A //TVAR_afit_pBaseValS[491] /AFIT8_sddd8a_iClustThresh_C_Bin [7:0],	AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
    {0x0F12, 0x0201}, //70000B9C //TVAR_afit_pBaseValS[492] /AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
    {0x0F12, 0x1E1E}, //70000B9E //TVAR_afit_pBaseValS[493] /AFIT8_sddd8a_DispTH_Low_Bin [7:0],	AFIT8_sddd8a_DispTH_High_Bin [15:8]
    {0x0F12, 0x1212}, //70000BA0 //TVAR_afit_pBaseValS[494] /AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
    {0x0F12, 0x0028}, //70000BA2 //TVAR_afit_pBaseValS[495] /AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],	AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
    {0x0F12, 0x030A}, //70000BA4 //TVAR_afit_pBaseValS[496] /AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
    {0x0F12, 0x0A10}, //70000BA6 //TVAR_afit_pBaseValS[497] /AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],	AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
    {0x0F12, 0x0819}, //70000BA8 //TVAR_afit_pBaseValS[498] /AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
    {0x0F12, 0xFF05}, //70000BAA //TVAR_afit_pBaseValS[499] /AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
    {0x0F12, 0x0432}, //70000BAC //TVAR_afit_pBaseValS[500] /AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
    {0x0F12, 0x4052}, //70000BAE //TVAR_afit_pBaseValS[501] /AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
    {0x0F12, 0x1514}, //70000BB0 //TVAR_afit_pBaseValS[502] /AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
    {0x0F12, 0x0440}, //70000BB2 //TVAR_afit_pBaseValS[503] /AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
    {0x0F12, 0x0302}, //70000BB4 //TVAR_afit_pBaseValS[504] /AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
    {0x0F12, 0x5050}, //70000BB6 //TVAR_afit_pBaseValS[505] /AFIT8_sddd8a_iClustThresh_H_sBin[7:0],	AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
    {0x0F12, 0x0101}, //70000BB8 //TVAR_afit_pBaseValS[506] /AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
    {0x0F12, 0x1E02}, //70000BBA //TVAR_afit_pBaseValS[507] /AFIT8_sddd8a_nClustLevel_H_sBin [7:0],	AFIT8_sddd8a_DispTH_Low_sBin [15:8]
    {0x0F12, 0x121E}, //70000BBC //TVAR_afit_pBaseValS[508] /AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
    {0x0F12, 0x2812}, //70000BBE //TVAR_afit_pBaseValS[509] /AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],	AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
    {0x0F12, 0x0A00}, //70000BC0 //TVAR_afit_pBaseValS[510] /AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
    {0x0F12, 0x1003}, //70000BC2 //TVAR_afit_pBaseValS[511] /AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
    {0x0F12, 0x190A}, //70000BC4 //TVAR_afit_pBaseValS[512] /AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
    {0x0F12, 0x0508}, //70000BC6 //TVAR_afit_pBaseValS[513] /AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
    {0x0F12, 0x32FF}, //70000BC8 //TVAR_afit_pBaseValS[514] /AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
    {0x0F12, 0x5204}, //70000BCA //TVAR_afit_pBaseValS[515] /AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],	AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
    {0x0F12, 0x1440}, //70000BCC //TVAR_afit_pBaseValS[516] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
    {0x0F12, 0x4015}, //70000BCE //TVAR_afit_pBaseValS[517] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],	AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
    {0x0F12, 0x0204}, //70000BD0 //TVAR_afit_pBaseValS[518] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
    {0x0F12, 0x0003}, //70000BD2 //TVAR_afit_pBaseValS[519] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]

    {0x0F12, 0x7F7A}, //// #afit_pConstBaseVals[0]
    {0x0F12, 0x7F9D}, //afit_pConstBaseValS[1]
    {0x0F12, 0xBEFC}, //afit_pConstBaseValS[2]
    {0x0F12, 0xF7BC}, //afit_pConstBaseValS[3]
    {0x0F12, 0x7E06}, //afit_pConstBaseValS[4]
    {0x0F12, 0x0053}, //afit_pConstBaseValS[5]

    // Update Changed Registers
    {0x002A, 0x0664},
    {0x0F12, 0x013E}, //seti_uContrastCenter

    {0x002A, 0x04D6},
    {0x0F12, 0x0001}, // #REG_TC_DBG_ReInitCmd
    {0x0028, 0xD000},
    {0x002A, 0x1102},
    {0x0F12, 0x00C0}, // Use T&P index 22 and 23
    {0x002A, 0x113C},
    {0x0F12, 0x267C}, // Trap 22 address 0x71aa
    {0x0F12, 0x2680}, // Trap 23 address 0x71b4
    {0x002A, 0x1142},
    {0x0F12, 0x00C0}, // Trap Up Set (trap Addr are > 0x10000)
    {0x002A, 0x117C},
    {0x0F12, 0x2CE8}, // Patch 22 address (TrapAndPatchOpCodes array index 22)
    {0x0F12, 0x2CeC}, // Patch 23 address (TrapAndPatchOpCodes array index 23)
    // Fill RAM with alternative op-codes
    {0x0028, 0x7000}, // start add MSW
    {0x002A, 0x2CE8}, // start add LSW
    {0x0F12, 0x0007}, // Modify LSB to control AWBB_YThreshLow
    {0x0F12, 0x00e2}, //
    {0x0F12, 0x0005}, // Modify LSB to control AWBB_YThreshLowBrLow
    {0x0F12, 0x00e2}, //
    // Update T&P tuning parameters
    {0x002A, 0x337A},
    {0x0F12, 0x0006}, // #Tune_TP_atop_dbus_reg // 6 is the default HW value

    //================================================================================================
    // SET PLL
    //================================================================================================
    //How to set
    //h)ex(CLK you want) * 1000)
    //h)ex((CLK you want) * 1000 / 4)
    //h)ex((CLK you want) * 1000 / 4)
    //===============================================================================================
    //Set input CLK // 26MHz
    {0x002A, 0x01CC},
    {0x0F12, 0x5DC0}, //5FB4 // #REG_TC_IPRM_InClockLSBs  //Set input CLK // 24MHz
    {0x0F12, 0x0000}, // #REG_TC_IPRM_InClockMSBs
                      //{0x002A   01EE
                      //{0x0F12   0000  // #REG_TC_IPRM_UseNPviClocks // Number of PLL setting  0x0002
                      //{0x0F12   0003  // #REG_TC_IPRM_UseNMiPiClocks
                      //   //Set system CLK // 50MHz
                      //{0x002A   01F6
                      //{0x0F12   1F40  //34BC //2710 //2904	//2BF2 // #REG_TC_IPRM_OpClk4KHz_0
                      //   //Set pixel CLK // 42MHz
                      //{0x0F12   28C3  //157C //3A88 // #REG_TC_IPRM_MinOutRate4KHz_0
                      //{0x0F12   2C06  //157C //3AA8 // #REG_TC_IPRM_MaxOutRate4KHz_0
                      //{0x0F12   1F40  //34BC //2710 //2904	//2BF2 // #REG_TC_IPRM_OpClk4KHz_1
                      //   //Set pixel CLK // 42MHz
                      //{0x0F12   28C3  //157C //3A88 // #REG_TC_IPRM_MinOutRate4KHz_1
                      //{0x0F12   2C06  //157C //3AA8 // #REG_TC_IPRM_MaxOutRate4KHz_1

    {0x002A, 0x01EE},
    {0x0F12, 0x0000}, //#REG_TC_IPRM_UseNPviClocks		// Number of PLL setting
    {0x0F12, 0x0003}, //#REG_TC_IPRM_UseNMiPiClocks		// Number of PLL setting

    {0x002A, 0x01F6},
    {0x0F12, 0x1F40}, //REG_TC_IPRM_OpClk4KHz_0                   	2   700001F6
    {0x0F12, 0x32A8}, //3
    {0x0F12, 0x32E8},

    {0x0F12, 0x1F40}, //REG_TC_IPRM_OpClk4KHz_1                   	2   700001FC
    {0x0F12, 0x2ea0}, //REG_TC_IPRM_MinOutRate4KHz_1              	2   700001FE
    {0x0F12, 0x2f00}, //REG_TC_IPRM_MaxOutRate4KHz_1              	2   70000200

    {0x0F12, 0x0BB8}, //REG_TC_IPRM_OpClk4KHz_2                   	2   70000202
    {0x0F12, 0x05DC}, //REG_TC_IPRM_MinOutRate4KHz_2              	2   70000204
    {0x0F12, 0x1770}, //REG_TC_IPRM_MaxOutRate4KHz_2              	2   70000206

    //Update PLL
    {0x002A, 0x0208},
    {0x0F12, 0x0001}, // #REG_TC_IPRM_InitParamsUpdated

    //============================================================
    // Frame rate setting
    //============================================================
    // How to set
    // 1. Exposure value
    // dec2hex((1 / (frame rate you want(ms))) * 100d * 4d)
    // 2. Analog Digital gain
    // dec2hex((Analog gain you want) * 256d)
    //============================================================
    // Set preview exposure time
    {0x002A, 0x0530},
    {0x0F12, 0x5DC0}, // #lt_uMaxExp1			60ms
    {0x0F12, 0x0000},
    {0x0F12, 0x6590}, // #lt_uMaxExp2			70ms
    {0x0F12, 0x0000},
    {0x002A, 0x167C},
    {0x0F12, 0x8CA0}, // #evt1_lt_uMaxExp3	100ms
    {0x0F12, 0x0000},
    {0x0F12, 0xABE0}, // #evt1_lt_uMaxExp4	120ms
    {0x0F12, 0x0000},

    // Set capture exposure time
    {0x002A, 0x0538},
    {0x0F12, 0x5DC0}, // #lt_uCapMaxExp1 		60ms
    {0x0F12, 0x0000},
    {0x0F12, 0x6590}, // #lt_uCapMaxExp2 	 70ms
    {0x0F12, 0x0000},
    {0x002A, 0x1684},
    {0x0F12, 0x8CA0}, // #evt1_lt_uCapMaxExp3 100ms
    {0x0F12, 0x0000},
    {0x0F12, 0xABE0}, // #evt1_lt_uCapMaxExp4 120ms
    {0x0F12, 0x0000},

    // Set gain
    {0x002A, 0x0540},
    {0x0F12, 0x0150}, // #lt_uMaxAnGain1
    {0x0F12, 0x0280}, // #lt_uMaxAnGain2
    {0x002A, 0x168C},
    {0x0F12, 0x0350}, // #evt1_lt_uMaxAnGain3
    {0x0F12, 0x0800}, // #evt1_lt_uMaxAnGain4

    {0x002A, 0x0544},
    {0x0F12, 0x0100}, // #lt_uMaxDigGain
    {0x0F12, 0x0A00}, //#lt_uMaxTotGain

    {0x002A, 0x1694},
    {0x0F12, 0x0001}, // #evt1_senHal_bExpandForbid

    //delay 10ms
};

struct s5k5ca_i2c_reg_conf s5k5ca_init_reg_config_sunny_settings_2[] =
{
    //===============================================================================================
    //SET PREVIEW CONFIGURATION_0
    //# Foramt : YUV422
    //# Size: VGA
    //# FPS : 7.5~15fps for normal mode
    //===============================================================================================
    {0x002A, 0x026C},
    {0x0F12, 0x0400}, //0320//// +10 //0400 //#REG_0TC_PCFG_usWidth//1024
    {0x0F12, 0x0300}, //0258//// +8//0300 //#REG_0TC_PCFG_usHeight //768	026E
    {0x0F12, 0x0005}, //#REG_0TC_PCFG_Format			0270
    {0x0F12, 0x32E8}, //157C //3AA8 //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
    {0x0F12, 0x32a8}, //157C //3A88 //#REG_0TC_PCFG_usMinOut4KHzRate  0274
    {0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88	0276
    {0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88 		027
    {0x0F12, 0x0030}, //0052 //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
    {0x0F12, 0x0010}, //#REG_0TC_PCFG_OIFMask
    {0x0F12, 0x01E0}, //01E0 //#REG_0TC_PCFG_usJpegPacketSize  jpeg_packet_size = 1024	400
    {0x0F12, 0x0000}, //0000 //#REG_0TC_PCFG_usJpegTotalPackets    jpeg_packets = 600		258
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
    {0x0F12, 0x0001}, //2  Jack 20100420//#REG_0TC_PCFG_FrRateQualityType 01:Always achieve the best frame rate. 02:Always achieve the best possible image quality (no-binning mode)
    {0x0F12, 0x0535}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps
    {0x0F12, 0x029A}, //01C6 //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //22fps
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uPrevMirror  0x0003
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror  0x0003
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation
    //===============================================================================================
    //SET PREVIEW CONFIGURATION_1
    //# Foramt : YUV422
    //# Size: VGA
    //# FPS : 5~15fps for Night mode
    //===============================================================================================
    {0x002A, 0x029C},
    {0x0F12, 0x0400}, //0320////  +10//0400 //#REG_0TC_PCFG_usWidth//1024
    {0x0F12, 0x0300}, //0258////  +8//0300 //#REG_0TC_PCFG_usHeight //768	 026E
    {0x0F12, 0x0005}, //#REG_0TC_PCFG_Format			0270
    {0x0F12, 0x32E8}, //157C //3AA8 //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
    {0x0F12, 0x32a8}, //157C //3A88 //#REG_0TC_PCFG_usMinOut4KHzRate  0274
    {0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88	0276
    {0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88 		027
    {0x0F12, 0x0050}, //0052 //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
    {0x0F12, 0x0010}, //#REG_0TC_PCFG_OIFMask
    {0x0F12, 0x01E0}, //01E0 //#REG_0TC_PCFG_usJpegPacketSize
    {0x0F12, 0x0000}, //0000 //#REG_0TC_PCFG_usJpegTotalPackets
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
    {0x0F12, 0x0001},
    {0x0F12, 0x07D0}, //0535 //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //5fps
    {0x0F12, 0x029A}, //01C6 //#REG_0TC_PCFG_usMinFrTimeMsecMult10 //22fps
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uPrevMirror  0x0003
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror  0x0003
    {0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

    //===============================================================================================
    //APPLY PREVIEW CONFIGURATION & RUN PREVIEW
    //===============================================================================================
    {0x002A, 0x023C},
    {0x0F12, 0x0000}, // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
    {0x002A, 0x0240},
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevOpenAfterChange
    {0x002A, 0x0230},
    {0x0F12, 0x0001}, // #REG_TC_GP_NewConfigSync // Update preview configuration
    {0x002A, 0x023E},
    {0x0F12, 0x0001}, // #REG_TC_GP_PrevConfigChanged
    {0x002A, 0x0220},
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreview // Start preview
    {0x0F12, 0x0001}, // #REG_TC_GP_EnablePreviewChanged
    //===============================================================================================
    //SET CAPTURE CONFIGURATION_0
    //# Foramt :YUV
    //# Size: 2048*1536
    //# FPS : 5fps
    //===============================================================================================
    {0x002A, 0x035C},
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
    {0x0F12, 0x0800}, //0800 //#REG_0TC_CCFG_usWidth
    {0x0F12, 0x0600}, //0600 //#REG_0TC_CCFG_usHeight
    {0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
    {0x0F12, 0x32E8}, //157C //3AA8 //#REG_0TC_CCFG_usMaxOut4KHzRate
    {0x0F12, 0x32a8}, //157C //3A88 //#REG_0TC_CCFG_usMinOut4KHzRate
    {0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
    {0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
    {0x0F12, 0x0030}, //0052 //#REG_0TC_CCFG_PVIMask
    {0x0F12, 0x0010}, //#REG_0TC_CCFG_OIFMask
    {0x0F12, 0x01E0}, //01E0 //#REG_0TC_CCFG_usJpegPacketSize
    {0x0F12, 0x08FC}, //08fc //#REG_0TC_CCFG_usJpegTotalPackets
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
    {0x0F12, 0x0002}, //2 //#REG_0TC_CCFG_FrRateQualityType  0000 Jack 20100420
    {0x0F12, 0x07D0}, //029A //0535 //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
    {0x0F12, 0x07D0}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //10fps
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
    {0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex

    {0x0028, 0xD000},
    {0x002A, 0x1000},
    {0x0F12, 0x0001},
    {0x0028, 0x7000},
};

const static struct s5k5ca_i2c_reg_conf s5k5ca_init_reg_sensor_start_settings[] =
{
    //#ARM GO
    //#Direct mode
    {0xFCFC, 0xD000},
    {0x0010, 0x0001}, //Reset
    {0x1030, 0x0000}, //Clear host interrupt so main will wait
    {0x0014, 0x0001}, //ARM go
    //p100 //delay 100ms
};

/*modify the register settings of effect aqua*/
struct s5k5ca_i2c_reg_conf s5k5ca_effect_off_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x021E},
    {0x0F12, 0x0000}, // #REG_TC_GP_SpecialEffects
    {0x002A, 0x04D2},
    {0x0F12, 0x067F},
};
struct s5k5ca_i2c_reg_conf s5k5ca_effect_mono_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x021E},
    {0x0F12, 0x0001}, // #REG_TC_GP_SpecialEffects
    {0x002A, 0x04D2},
    {0x0F12, 0x067F},
};

struct s5k5ca_i2c_reg_conf s5k5ca_effect_negative_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x021E},
    {0x0F12, 0x0003}, // #REG_TC_GP_SpecialEffects
    {0x002A, 0x04D2},
    {0x0F12, 0x067F},
};
struct s5k5ca_i2c_reg_conf s5k5ca_effect_solarize_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x021E},
    {0x0F12, 0x0002}, // #REG_TC_GP_SpecialEffects
};

struct s5k5ca_i2c_reg_conf s5k5ca_effect_posterize_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x021E},
    {0x0F12, 0x0006}, // #REG_TC_GP_SpecialEffects
};

struct s5k5ca_i2c_reg_conf s5k5ca_effect_sepia_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x021E},
    {0x0F12, 0x0004}, // #REG_TC_GP_SpecialEffects
    {0x002A, 0x04D2},
    {0x0F12, 0x067F},
};
struct s5k5ca_i2c_reg_conf s5k5ca_effect_aqua_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x021E},
    {0x0F12, 0x0005}, // #REG_TC_GP_SpecialEffects

    {0x002A, 0x04D2},
    {0x0F12, 0x0677},
    {0x002A, 0x04a0},
    {0x0F12, 0x0100}, //R_gain 
    {0x0F12, 0x0001},
    {0x0F12, 0x0100},
    {0x0F12, 0x0001},
    {0x0F12, 0x0300}, //B_gain
    {0x0F12, 0x0001},
};
/*delete some lines*/

struct s5k5ca_i2c_reg_conf s5k5ca_preview_reg_config_settings[] =
{
    //================================================================================================
    // APPLY PREVIEW CONFIGURATION & RUN PREVIEW
    //================================================================================================
    {0x0028, 0x7000},
    {0x002A, 0x023C},
    {0x0F12, 0x0000},
    {0x002A, 0x0240},
    {0x0F12, 0x0001},
    {0x002A, 0x0230},
    {0x0F12, 0x0001},
    {0x002A, 0x023E},
    {0x0F12, 0x0001},
    {0x002A, 0x0220},
    {0x0F12, 0x0001},
    {0x0028, 0xD000},
    {0x002A, 0xB0A0},
    {0x0F12, 0x0000},
    {0x0028, 0x7000},
    {0x002A, 0x0222},
    {0x0F12, 0x0001},
};

struct s5k5ca_i2c_reg_conf s5k5ca_snapshot_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x0244},
    {0x0F12, 0x0000}, //REG_TC_GP_ActiveCapConfig
    {0x002A, 0x0230},
    {0x0F12, 0x0001}, //REG_TC_GP_NewConfigSync
    {0x002A, 0x0246},
    {0x0F12, 0x0001}, //REG_TC_GP_CapConfigChanged
    {0x002A, 0x0224},
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCapture       g change
    {0x0028, 0xD000},
    {0x002A, 0xB0A0},
    {0x0F12, 0x0000}, // Clear cont. clock befor confi
    {0x0028, 0x7000},
    {0x002A, 0x0226}, //0224
    {0x0F12, 0x0001}, //REG_TC_GP_EnableCaptureChanged
};

struct s5k5ca_i2c_reg_conf s5k5ca_wb_auto_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x04D2},
    {0x0F12, 0x067F}, // #REG_TC_DBG_AutoAlgEnBits
};

struct s5k5ca_i2c_reg_conf s5k5ca_wb_a_reg_config_settings[] =
{
    {0x002A, 0x04D2},
    {0x0F12, 0x0677}, // #REG_TC_DBG_AutoAlgEnBits
    {0x002A, 0x04A0},
    {0x0F12, 0x0420}, // #REG_SF_USER_Rgain
    {0x0F12, 0x0001}, // #REG_SF_USER_RgainChanged
    {0x002A, 0x04A4},
    {0x0F12, 0x0400}, // #REG_SF_USER_Ggain
    {0x0F12, 0x0001}, // #REG_SF_USER_GgainChanged
    {0x002A, 0x04A8},
    {0x0F12, 0x0880}, // #REG_SF_USER_Bgain
    {0x0F12, 0x0001}, // #REG_SF_USER_BgainChanged
};

struct s5k5ca_i2c_reg_conf s5k5ca_wb_tl84_reg_config_settings[] =
{
    {0x002A, 0x04D2},
    {0x0F12, 0x0677}, // #REG_TC_DBG_AutoAlgEnBits
    {0x002A, 0x04A0},
    {0x0F12, 0x0480}, // #REG_SF_USER_Rgain
    {0x0F12, 0x0001}, // #REG_SF_USER_RgainChanged
    {0x002A, 0x04A4},
    {0x0F12, 0x0400}, // #REG_SF_USER_Ggain
    {0x0F12, 0x0001}, // #REG_SF_USER_GgainChanged
    {0x002A, 0x04A8},
    {0x0F12, 0x07C0}, // #REG_SF_USER_Bgain
    {0x0F12, 0x0001}, // #REG_SF_USER_BgainChanged
};

struct s5k5ca_i2c_reg_conf s5k5ca_wb_f_reg_config_settings[] =
{
    {0x002A, 0x04D2},
    {0x0F12, 0x0677}, // #REG_TC_DBG_AutoAlgEnBits
    {0x002A, 0x04A0},
    {0x0F12, 0x054C}, // #REG_SF_USER_Rgain
    {0x0F12, 0x0001}, // #REG_SF_USER_RgainChanged
    {0x002A, 0x04A4},
    {0x0F12, 0x0400}, // #REG_SF_USER_Ggain
    {0x0F12, 0x0001}, // #REG_SF_USER_GgainChanged
    {0x002A, 0x04A8},
    {0x0F12, 0x085A}, // #REG_SF_USER_Bgain
    {0x0F12, 0x0001}, // #REG_SF_USER_BgainChanged
};

struct s5k5ca_i2c_reg_conf s5k5ca_wb_d65_reg_config_settings[] =
{
    {0x002A, 0x04D2},
    {0x0F12, 0x0677}, // #REG_TC_DBG_AutoAlgEnBits
    {0x002A, 0x04A0},
    {0x0F12, 0x0584}, // #REG_SF_USER_Rgain
    {0x0F12, 0x0001}, // #REG_SF_USER_RgainChanged
    {0x002A, 0x04A4},
    {0x0F12, 0x0400}, // #REG_SF_USER_Ggain
    {0x0F12, 0x0001}, // #REG_SF_USER_GgainChanged
    {0x002A, 0x04A8},
    {0x0F12, 0x060A}, // #REG_SF_USER_Bgain
    {0x0F12, 0x0001}, // #REG_SF_USER_BgainChanged
};

struct s5k5ca_i2c_reg_conf s5k5ca_wb_d50_reg_config_settings[] =
{
    {0x002A, 0x04D2},
    {0x0F12, 0x0677}, // #REG_TC_DBG_AutoAlgEnBits
    {0x002A, 0x04A0},
    {0x0F12, 0x0631}, // #REG_SF_USER_Rgain
    {0x0F12, 0x0001}, // #REG_SF_USER_RgainChanged
    {0x002A, 0x04A4},
    {0x0F12, 0x0400}, // #REG_SF_USER_Ggain
    {0x0F12, 0x0001}, // #REG_SF_USER_GgainChanged
    {0x002A, 0x04A8},
    {0x0F12, 0x04E6}, // #REG_SF_USER_Bgain
    {0x0F12, 0x0001}, // #REG_SF_USER_BgainChanged
};

struct s5k5ca_i2c_reg_conf s5k5ca_antibanding_off_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x04BA},
    {0x0F12, 0x0000}, // #REG_SF_USER_FlickerQuant
};

struct s5k5ca_i2c_reg_conf s5k5ca_antibanding_auto_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x0C18}, //#AFC_Default60Hz
    {0x0F12, 0x0001}, // #AFC_Default60Hz  1: Auto Flicker 60Hz start 0: Auto Flicker 50Hz start
    {0x002A, 0x04D2}, // #REG_TC_DBG_AutoAlgEnBits
    {0x0F12, 0x067F},
};

struct s5k5ca_i2c_reg_conf s5k5ca_antibanding_50hz_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x04BA},
    {0x0F12, 0x0001}, // #REG_SF_USER_FlickerQuant
};

struct s5k5ca_i2c_reg_conf s5k5ca_antibanding_60hz_reg_config_settings[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x04BA},
    {0x0F12, 0x0002}, // #REG_SF_USER_FlickerQuant
};
struct s5k5ca_i2c_reg_conf s5k5ca_mirror_mode_reg_config_setting[] =
{
    {0x0028, 0x7000},
    {0x002A, 0x0296}, //configure0
    {0x0F12, 0x0003}, //#REG_0TC_PCFG_uPrevMirror  0x0003
    {0x0F12, 0x0003}, //#REG_0TC_PCFG_uCaptureMirror  0x0003
    {0x002A, 0x023E}, //REG_TC_GP_PrevConfigChanged
    {0x0F12, 0x0001}, 
    {0x002A, 0x0246}, //REG_TC_GP_CapConfigChanged
    {0x0F12, 0x0001},
    
};
struct s5k5ca_reg s5k5ca_regs =
{
    .s5k5ca_init_reg_config_sunny             = s5k5ca_init_reg_config_sunny_settings,
    .s5k5ca_init_reg_config_sunny_size        = S5K5CA_ARRAY_SIZE_2(s5k5ca_init_reg_config_sunny_settings),
    .s5k5ca_init_reg_config_sunny_2           = s5k5ca_init_reg_config_sunny_settings_2,
    .s5k5ca_init_reg_config_sunny_2_size      = S5K5CA_ARRAY_SIZE_2(s5k5ca_init_reg_config_sunny_settings_2),
    .s5k5ca_init_reg_sensor_start             = s5k5ca_init_reg_sensor_start_settings,
    .s5k5ca_init_reg_sensor_start_size        = S5K5CA_ARRAY_SIZE_2(s5k5ca_init_reg_sensor_start_settings),
    .s5k5ca_effect_off_reg_config             = s5k5ca_effect_off_reg_config_settings,
    .s5k5ca_effect_off_reg_config_size        = S5K5CA_ARRAY_SIZE_2(s5k5ca_effect_off_reg_config_settings),
    .s5k5ca_effect_mono_reg_config            = s5k5ca_effect_mono_reg_config_settings,
    .s5k5ca_effect_mono_reg_config_size       = S5K5CA_ARRAY_SIZE_2(s5k5ca_effect_mono_reg_config_settings),
    .s5k5ca_effect_negative_reg_config        = s5k5ca_effect_negative_reg_config_settings,
    .s5k5ca_effect_negative_reg_config_size   = S5K5CA_ARRAY_SIZE_2(s5k5ca_effect_negative_reg_config_settings),
    .s5k5ca_effect_sepia_reg_config           = s5k5ca_effect_sepia_reg_config_settings,
    .s5k5ca_effect_sepia_reg_config_size      = S5K5CA_ARRAY_SIZE_2(s5k5ca_effect_sepia_reg_config_settings),
    .s5k5ca_effect_aqua_reg_config            = s5k5ca_effect_aqua_reg_config_settings,
    .s5k5ca_effect_aqua_reg_config_size       = S5K5CA_ARRAY_SIZE_2(s5k5ca_effect_aqua_reg_config_settings),
    .s5k5ca_effect_solarize_reg_config      = s5k5ca_effect_solarize_reg_config_settings,
    .s5k5ca_effect_solarize_reg_config_size = S5K5CA_ARRAY_SIZE_2(s5k5ca_effect_solarize_reg_config_settings),
    .s5k5ca_effect_posterize_reg_config      = s5k5ca_effect_posterize_reg_config_settings,
    .s5k5ca_effect_posterize_reg_config_size = S5K5CA_ARRAY_SIZE_2(s5k5ca_effect_posterize_reg_config_settings),
    .s5k5ca_preview_reg_config                = s5k5ca_preview_reg_config_settings,
    .s5k5ca_preview_reg_config_size           = S5K5CA_ARRAY_SIZE_2(s5k5ca_preview_reg_config_settings),
    .s5k5ca_snapshot_reg_config               = s5k5ca_snapshot_reg_config_settings,
    .s5k5ca_snapshot_reg_config_size          = S5K5CA_ARRAY_SIZE_2(s5k5ca_snapshot_reg_config_settings),
    .s5k5ca_wb_auto_reg_config                = s5k5ca_wb_auto_reg_config_settings,
    .s5k5ca_wb_auto_reg_config_size           = S5K5CA_ARRAY_SIZE_2(s5k5ca_wb_auto_reg_config_settings),
    .s5k5ca_wb_a_reg_config                   = s5k5ca_wb_a_reg_config_settings,
    .s5k5ca_wb_a_reg_config_size              = S5K5CA_ARRAY_SIZE_2(s5k5ca_wb_a_reg_config_settings),
    .s5k5ca_wb_tl84_reg_config                = s5k5ca_wb_tl84_reg_config_settings,
    .s5k5ca_wb_tl84_reg_config_size           = S5K5CA_ARRAY_SIZE_2(s5k5ca_wb_tl84_reg_config_settings),
    .s5k5ca_wb_f_reg_config                   = s5k5ca_wb_f_reg_config_settings,
    .s5k5ca_wb_f_reg_config_size              = S5K5CA_ARRAY_SIZE_2(s5k5ca_wb_f_reg_config_settings),
    .s5k5ca_wb_d65_reg_config                 = s5k5ca_wb_d65_reg_config_settings,
    .s5k5ca_wb_d65_reg_config_size            = S5K5CA_ARRAY_SIZE_2(s5k5ca_wb_d65_reg_config_settings),
    .s5k5ca_wb_d50_reg_config                 = s5k5ca_wb_d50_reg_config_settings,
    .s5k5ca_wb_d50_reg_config_size            = S5K5CA_ARRAY_SIZE_2(s5k5ca_wb_d50_reg_config_settings),
    .s5k5ca_mirror_mode_reg_config                 = s5k5ca_mirror_mode_reg_config_setting,
    .s5k5ca_mirror_mode_reg_config_size            = S5K5CA_ARRAY_SIZE_2(s5k5ca_mirror_mode_reg_config_setting),
    /*delete some lines*/
};

