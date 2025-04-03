//
// Transputer emulator
//
// by Oscar Toledo G.
// All rights reserved.
// https://nanochess.org/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Creation date: Mar/17/2025.
// Revision date: Mar/31/2025. Runs far enough to read the disk.
// Revision date: Apr/01/2025. Functional (missing floating point unit, so the
//                             ray tracer and modeler cannot be run).
//

//
// All the Javascript bitwise operators work as 32-bit signed integers,
// this makes very difficult the port because my C code depended on
// unsigned integers. Also the number handling of Javascript is ALWAYS
// 64-bit floating point, and this means 53-bit signed integers.
//
// To avoid getting negative numbers, the operator >>> 0 is used, and
// also to limit values to 32-bit unsigned.
//

const ROUND_MINUS = 0;
const ROUND_NEAREST = 1;
const ROUND_POSITIVE = 2;
const ROUND_ZERO = 3;

const MEMORY_MASK = 0x0003ffff;
const MAX_INT = 0xffffffff;

const GotoSNPBit = 0x02;
const ErrorFlag = 0x80000000;
const HaltOnErrorBit = 0x80;

const NotProcess_p = 0x80000000;

var channel0 = new Uint8Array(65536);
var offset_channel0 = 0;
var bytes_channel0 = 0;

var output_buffer = new Uint8Array(65536);
var output_pointer = 0;

var key_buffer = new Uint8Array(256);
var key_read = 0;
var key_write = 0;

function handle_input(addr, channel, bytes)
{
    if (bytes == 0)
        return;
    if (bytes > bytes_channel0)
        bytes = bytes_channel0;
    while (bytes) {
        write_memory(addr, channel0[offset_channel0]);
        addr++;
        offset_channel0 = (offset_channel0 + 1) & 0xffff;
        bytes_channel0--;
        bytes--;
    }
}

var cursor_row;
var cursor_column;

function handle_output(addr, channel, bytes)
{
    var c;
    var offset;
    var date;
    var v;
    var v2;
    var p;
    var str;
    var x;
    var y;
    var r;
    var g;
    var b;
    var canvas;
    var ctx;
    var color = [0, 4, 2, 6, 1, 5, 3, 7];

    while (bytes) {
        if (output_pointer >= output_buffer.length) {
            window.alert("Error: Too much data unprocessed\n");
            throw "Oops!";
        }
        output_buffer[output_pointer++] = read_memory(addr++);
        bytes--;
    }
    if (output_pointer < 2)
        return;
    c = (output_buffer[0] & 0x0f) * 16 + (output_buffer[1] & 0x0f);
    switch (c) {
        case 1: /* Code 01: Read sector */
            if (output_pointer < 12)
                return;
            sector = 0;
            for (c = 4; c < 12; c++) {
                sector = (sector << 4) | (output_buffer[c] & 0x0f);
            }
            sector = sector >>> 0;
            c = (output_buffer[2] & 0x0f) * 16 + (output_buffer[3] & 0x0f);
            console.log("Reading sector " + sector + " drive " + c);
            if (c == 0) {   // Floppy disk
                if (sector >= 2880) {
                    channel0[0] = 1;    // All bad
                    for (c = 1; c < 513; c++)
                        channel0[c] = 0xfc;
                } else {
                    offset = sector * 512;
                    channel0[0] = 0;
                    for (c = 1; c < 513; c++)
                        channel0[c] = floppy[offset + c - 1];
                }
            } else if (c == 1) {    // RAM disk
                if (sector >= 1024) {
                    channel0[0] = 1;    // All bad
                    for (c = 1; c < 513; c++)
                        channel0[c] = 0xfc;
                } else {
                    offset = sector * 512;
                    channel0[0] = 0;
                    for (c = 1; c < 513; c++)
                        channel0[c] = ram_disk[offset + c - 1];
                }
            } else if (c == 2) {    // Hard disk drive
                if (sector >= 40 * 1048576 / 512) {
                    channel0[0] = 1;    // All bad
                    for (c = 1; c < 513; c++)
                        channel0[c] = 0xfc;
                } else {
                    offset = sector * 512;
                    channel0[0] = 0;
                    for (c = 1; c < 513; c++)
                        channel0[c] = harddisk[offset + c - 1];
                }
            } else if (c == 3) {    // CD-ROM (unimplemented)
                channel0[0] = 1;    // All bad
                for (c = 1; c < 513; c++)
                    channel0[c] = 0xfc;
            } else {
                channel0[0] = 1;    // All bad
                for (c = 1; c < 513; c++)
                    channel0[c] = 0xfc;
            }
            offset_channel0 = 0;
            bytes_channel0 = 513;
            output_pointer = 0;
            return;
        case 2: /* Code 02: Write sector */
            if (output_pointer < 1036)
                return;
            sector = 0;
            for (c = 4; c < 12; c++) {
                sector = (sector << 4) | (output_buffer[c] & 0x0f);
            }
            sector = sector >>> 0;
            c = (output_buffer[2] & 0x0f) * 16 + (output_buffer[3] & 0x0f);
            if (c == 0) {   /* Floppy disk */
                if (sector >= 2880) {
                    channel0[0] = 1;    // All bad
                } else {
                    offset = sector * 512;
                    channel0[0] = 0;
                    for (c = 0; c < 512; c++)
                        floppy[offset + c] = (output_buffer[12 + c * 2] & 0x0f) * 16 + (output_buffer[13 + c * 2] & 0x0f);
                }
            } else if (c == 1) {    /* RAM disk */
                if (sector >= 1024) {
                    channel0[0] = 1;    // All bad
                } else {
                    offset = sector * 512;
                    channel0[0] = 0;
                    for (c = 0; c < 512; c++)
                        ram_disk[offset + c] = (output_buffer[12 + c * 2] & 0x0f) * 16 + (output_buffer[13 + c * 2] & 0x0f);
                }
            } else if (c == 2) {    /* Hard disk drive */
                if (sector >= 40 * 1048576 / 512) {
                    channel0[0] = 1;    // All bad
                } else {
                    offset = sector * 512;
                    channel0[0] = 0;
                    for (c = 0; c < 512; c++)
                        harddisk[offset + c] = (output_buffer[12 + c * 2] & 0x0f) * 16 + (output_buffer[13 + c * 2] & 0x0f);
                }
            } else {
                channel0[0] = 1;
            }
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 3: /* Code 03: Format track (floopy disk) */
            if (output_pointer < 6)
                return;
            /* Code / Track / Head */
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 4: /* Code 04: Update screen */
            if (output_pointer < 323)
                return;
            viewer.readBytes("\x1b[" + (output_buffer[2] + 1) + ";1f");
            str = "";
            c = -1;
            p = 3;
            while (p < 323) {
                v = (output_buffer[p] & 0x0f) * 16 + (output_buffer[p + 1] & 0x0f);
                p += 2;
                v2 = (output_buffer[p] & 0x0f) * 16 + (output_buffer[p + 1] & 0x0f);
                p += 2;
                if (v2 != c) {
                    var something = 0;
                    
                    str += "\x1b[";
                    if ((c & 0x88) != (v2 & 0x88)) {
                        str += "0";
                        something = 1;
                        c = ~v2;
                    }
                    if (v2 & 8) {
                        if (something != 0)
                            str += ";";
                        str += "1";
                        something = 1;
                    }
                    if (v2 & 0x80) {
                        if (something != 0)
                            str += ";";
                        str += "5";
                        something = 1;
                    }
                    if ((c & 7) != (v2 & 7)) {
                        if (something != 0)
                            str += ";";
                        str += 30 + color[v2 & 7];
                        something = 1;
                    }
                    if ((c & 0x70) != (v2 & 0x70)) {
                        if (something != 0)
                            str += ";";
                        str += 40 + color[(v2 & 0x70) / 16];
                        something = 1;
                    }
                    str += "m";
                    
                    c = v2;
                }
                if (v == 0)
                    v = 32;
                str += String.fromCharCode(v);
            }
            viewer.readBytes(str);
            viewer.readBytes("\x1b[" + cursor_row + ";" + cursor_column + "H");
            output_pointer = 0;
            return;
        case 5: /* Code 05: Pressed key */
            if (key_read == key_write) {
                channel0[0] = 0;
            } else {
                channel0[0] = key_buffer[key_read];
                key_read = (key_read + 1) % key_buffer.length;
            }
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 6: /* Code 06: Get time */
            date = new Date();
            channel0[0] = date.getSeconds();
            channel0[1] = date.getMinutes();
            channel0[2] = date.getHours();
            channel0[3] = date.getDate();
            channel0[4] = date.getMonth() + 1;
            channel0[5] = date.getFullYear() - 1900;
            offset_channel0 = 0;
            bytes_channel0 = 6;
            output_pointer = 0;
            return;
        case 7: /* Code 07: Cursor position */
            if (output_pointer < 6)
                return;
            c = (output_buffer[2] & 0x0f) * 16 + (output_buffer[3] & 0x0f) + (output_buffer[4] & 0x0f) * 4096 + (output_buffer[5] & 0x0f) * 256;
            cursor_row = Math.floor(c / 80) + 1;
            cursor_column = c % 80 + 1;
            viewer.readBytes("\x1b[" + cursor_row + ";" + cursor_column + "H");
            output_pointer = 0;
            return;
        case 8: /* Code 08: Cursor shape */
            if (output_pointer < 6)
                return;
            c = (output_buffer[2] & 0x0f) * 16 + (output_buffer[3] & 0x0f);
            if (c == 16) {    /* 16,0 turn off cursor */
                viewer.removeCursor();
                viewer.cursor_enabled = 0;
            } else {    /* 0,15 turn on block cursor */
                viewer.cursor_enabled = 1;
                viewer.drawCursor();
            }
            output_pointer = 0;
            return;
        case 9: /* Code 09: Printer */
            if (output_pointer < 4)
                return;
            /* Byte for printer */
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 16: /* Code 10: Enter graphics mode */
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 17: /* Code 11: Exit graphics mode */
            /* !!! Could save canvas here */
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 18: /* Code 12: Draw pixel */
            if (output_pointer < 16)
                return;
            x = (output_buffer[2] & 0x0f) * 16 + (output_buffer[3] & 0x0f) + (output_buffer[4] & 0x0f) * 4096 + (output_buffer[5] & 0x0f) * 256;
            y = (output_buffer[6] & 0x0f) * 16 + (output_buffer[7] & 0x0f) + (output_buffer[8] & 0x0f) * 4096 + (output_buffer[9] & 0x0f) * 256;
            r = (output_buffer[10] & 0x0f) * 16 + (output_buffer[11] & 0x0f);
            g = (output_buffer[12] & 0x0f) * 16 + (output_buffer[13] & 0x0f);
            b = (output_buffer[14] & 0x0f) * 16 + (output_buffer[15] & 0x0f);
//            console.log(x + "," + y + "," + r + "," + g + "," + b);
            if (x >= 0 && x < 640 && y >= 0 && y < 480) {
                canvas = document.getElementById("canvas");
                ctx = canvas.getContext("2d");
                ctx.fillStyle = "rgb(" + r + "," + g + "," + b + ")";
                ctx.fillRect(x, y, 1, 1);
            }
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 19: /* Code 13: Draw grey pixel */
            if (output_pointer < 12)
                return;
            x = (output_buffer[2] & 0x0f) * 16 + (output_buffer[3] & 0x0f) + (output_buffer[4] & 0x0f) * 4096 + (output_buffer[5] & 0x0f) * 256;
            y = (output_buffer[6] & 0x0f) * 16 + (output_buffer[7] & 0x0f) + (output_buffer[8] & 0x0f) * 4096 + (output_buffer[9] & 0x0f) * 256;
            r = (output_buffer[10] & 0x0f) * 16 + (output_buffer[11] & 0x0f);
            g = r;
            b = r;
//            console.log(x + "," + y + "," + r + "," + g + "," + b);
            if (x >= 0 && x < 640 && y >= 0 && y < 480) {
                canvas = document.getElementById("canvas");
                ctx = canvas.getContext("2d");
                ctx.fillStyle = "rgb(" + r + "," + g + "," + b + ")";
                ctx.fillRect(x, y, 1, 1);
            }
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 24: /* Code 18: Read serial port */
            channel0[0] = 0;    /* Byte from serial port */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 25: /* Code 19: Write serial port */
            if (output_pointer < 4)
                return;
            /* Byte for serial port */
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 26: /* Code 1a: Read tape */
            channel0[0] = 0;    /* Byte from tape */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 27: /* Code 1b: Write tape */
            if (output_pointer < 4)
                return;
            /* Byte for tape */
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        case 28: /* Code 1c: Rewind tape */
            channel0[0] = 0;    /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = 0;
            return;
        default:
            throw "Code " + c.toString(16) + " not defined!";
    }
}

/*
 ** Memory access
 */
const float32 = new Float32Array(4);
const float64 = new Float64Array(4);
const shared32 = new Uint32Array(float32.buffer);
const shared64 = new Uint32Array(float64.buffer);

// float32[0] = 5.0;
// console.log(float32[0]);
// console.log(shared32[0].toString(16));
// float64[0] = 5.0;
// console.log(float64[0]);
// console.log(shared64[0].toString(16) + "," + shared64[1].toString(16));

function read_memory(addr)
{
    return memory[addr & MEMORY_MASK];
}

function write_memory(addr, byte)
{
    memory[addr & MEMORY_MASK] = byte;
}

function read_memory_32(addr)
{
    addr &= MEMORY_MASK;
    return (memory[addr] | (memory[addr + 1] << 8) | (memory[addr + 2] << 16) | (memory[addr + 3] << 24)) >>> 0;
}

function write_memory_32(addr, word)
{
    addr &= MEMORY_MASK;
    memory[addr] = word;
    memory[addr + 1] = word >> 8;
    memory[addr + 2] = word >> 16;
    memory[addr + 3] = word >> 24;
}

function read_memory_fp32(addr)
{
    addr &= MEMORY_MASK;
    shared32[3] = (memory[addr] | (memory[addr + 1] << 8) | (memory[addr + 2] << 16) | (memory[addr + 3] << 24)) >>> 0;
    return float32[3];
}

function read_memory_fp64(addr)
{
    addr &= MEMORY_MASK;
    shared64[6] = (memory[addr] | (memory[addr + 1] << 8) | (memory[addr + 2] << 16) | (memory[addr + 3] << 24)) >>> 0;
    shared64[7] = (memory[addr + 4] | (memory[addr + 5] << 8) | (memory[addr + 6] << 16) | (memory[addr + 7] << 24)) >>> 0;
    return float64[3];
}

function write_memory_fp32(addr, value)
{
    var word;
    
    float32[3] = value;
    word = shared32[3];
    addr &= MEMORY_MASK;
    memory[addr] = word;
    memory[addr + 1] = word >> 8;
    memory[addr + 2] = word >> 16;
    memory[addr + 3] = word >> 24;
}

function write_memory_fp64(addr, value)
{
    var word;
    
    float64[3] = value;
    addr &= MEMORY_MASK;
    word = shared64[6];
    memory[addr] = word;
    memory[addr + 1] = word >> 8;
    memory[addr + 2] = word >> 16;
    memory[addr + 3] = word >> 24;
    word = shared64[7];
    memory[addr + 4] = word;
    memory[addr + 5] = word >> 8;
    memory[addr + 6] = word >> 16;
    memory[addr + 7] = word >> 24;
}

/*
 ** Boot program
 **
 ** The transputer board had 128 KB. of high-speed RAM, however,
 ** we need to take in account the internal 4 KB. of transputer RAM.
 ** So we can start our workspace at 0x80021000 because the lower
 ** 4 KB. ram of the external RAM are accesible there (mirroring).
 */
var boot_image = [
    0x24, 0xf2,                     /* mint */
    0x22, 0x20, 0x2f, 0x2f, 0x88,   /* adc 0x20ff8 */
    0x23, 0xfc,                     /* gajw */
    0x22, 0xf9,                     /* testerr */
    0x25, 0xf7,                     /* clrhalterr */
    0x29, 0xfc,                     /* fptesterr */
    0x24, 0xf2,                     /* mint */
    0x21, 0xf8,                     /* sthf */
    0x24, 0xf2,                     /* mint */
    0x21, 0xfc,                     /* stlf */
    0x24, 0xf2,                     /* mint */
    0x24, 0xf2,                     /* mint */
    0xe9,                           /* stnl 9 */
    0x24, 0xf2,                     /* mint */
    0x24, 0xf2,                     /* mint */
    0xea,                           /* stnl 10 */
    0x40,                           /* ldc 0 */
    0x25, 0xf4,                     /* sttimer */
    0x27, 0xfa,                     /* timerdisableh */
    0x27, 0xfb,                     /* timerdisablel */
    0x40,                           /* ldc 0 */
    0xd0,                           /* stl 0 */
    0x49,                           /* ldc 9 */
    0xd1,                           /* stl 1 */
    0x70,                           /* ldl 0 */
    0x24, 0xf2,                     /* mint */
    0xfa,                           /* wsub */
    0x21, 0xf2,                     /* resetch */
    0x10,                           /* ldlp 0 */
    0x4a,                           /* ldc 10 */
    0x22, 0xf1,                     /* lend */
    0x24, 0xf2,                     /* mint */
    0x24, 0x50,                     /* ldnlp 64 */
    0x24, 0xf2,                     /* mint */
    0x54,                           /* ldnlp 4 */
    0x20, 0x20, 0x20, 0x40,         /* ldc initial_code_length */
    0xf7,                           /* in */
    0x24, 0x9c,                     /* call 0x80000100 */
    0x2f, 0xff, /*0x60, 0x0c*/
];

/*
 ** RAM memory for transputer.
 **
 ** Only 132K used.
 */
var memory = new Uint8Array(262144);

for (c = 0; c < 262144; c++)
    memory[c] = 0xff;

/*
 ** Transputer interface code MAESTRO
 ** (MAnejador de Entrada y Salida para TRansputer por Oscar)
 */
var maestro = [
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
    0x24, 0xf2, 0x24, 0xf2, 0x24, 0xe0, 0x24, 0xf2,
    0x24, 0xf2, 0x24, 0xe1, 0x24, 0xf2, 0x24, 0xf2,
    0x24, 0xe2, 0x24, 0xf2, 0x24, 0xf2, 0x24, 0xe3,
    0x24, 0xf2, 0x24, 0xf2, 0x24, 0xe8, 0x24, 0xf2,
    0x24, 0xf2, 0x24, 0xe9, 0x24, 0x60, 0xbf, 0x10,
    0x24, 0xf2, 0x24, 0xe4, 0x60, 0xbe, 0x12, 0xd0,
    0x23, 0x2e, 0x48, 0xd1, 0x27, 0x22, 0x20, 0x20,
    0x27, 0x22, 0x40, 0x70, 0xe0, 0x70, 0x83, 0xd0,
    0x10, 0x21, 0x41, 0x22, 0xf1, 0x23, 0x2e, 0x1a,
    0xd0, 0x21, 0x49, 0xd1, 0x40, 0x70, 0xe0, 0x70,
    0x83, 0xd0, 0x10, 0x4a, 0x22, 0xf1, 0xb2, 0x40,
    0x24, 0xf2, 0x24, 0xe5, 0x40, 0x24, 0xf2, 0x24,
    0xe6, 0x40, 0x24, 0xf2, 0x24, 0xe7, 0x40, 0x24,
    0xf2, 0x24, 0xeb, 0x21, 0x49, 0x21, 0xfb, 0x60,
    0x10, 0x60, 0xef, 0x60, 0x10, 0x23, 0xf9, 0x29,
    0x43, 0x21, 0xfb, 0x61, 0x10, 0x60, 0xef, 0x61,
    0x10, 0x23, 0xf9, 0x62, 0xb0, 0x21, 0x28, 0x0c,
    0x60, 0xb8, 0x24, 0xf2, 0x24, 0x34, 0xd6, 0x24,
    0xf2, 0x24, 0x34, 0x23, 0x2e, 0x58, 0xd1, 0x21,
    0x2e, 0x28, 0x24, 0x48, 0x22, 0xf2, 0x25, 0xf2,
    0x22, 0xfb, 0x76, 0xd2, 0x71, 0xd3, 0x40, 0xd4,
    0x74, 0x21, 0x49, 0xf4, 0x61, 0xa9, 0x60, 0x4f,
    0xd7, 0x40, 0xd5, 0x75, 0x2a, 0x40, 0xf4, 0x21,
    0xa1, 0x21, 0x23, 0x47, 0x77, 0x72, 0x30, 0x27,
    0xf4, 0xd7, 0x72, 0x51, 0xd2, 0x75, 0x51, 0xd5,
    0x61, 0x09, 0x77, 0x73, 0x30, 0xf4, 0xc0, 0xa8,
    0x73, 0x51, 0xd3, 0x74, 0x81, 0xd4, 0x62, 0x00,
    0x77, 0x73, 0xe0, 0x24, 0xf2, 0x24, 0x35, 0xaa,
    0x2c, 0x48, 0x22, 0xf2, 0x25, 0xf2, 0x22, 0xfb,
    0x61, 0x0e, 0x41, 0x24, 0xf2, 0x24, 0xe5, 0x24,
    0xf2, 0x50, 0x23, 0x40, 0xfe, 0x24, 0xf2, 0x50,
    0x23, 0x44, 0xfe, 0x24, 0xf2, 0x50, 0x74, 0xfe,
    0x72, 0x75, 0xf4, 0x2a, 0x40, 0x2e, 0x96, 0x40,
    0x24, 0xf2, 0x24, 0xe5, 0x64, 0x0c, 0x60, 0xbc,
    0x10, 0x24, 0xf2, 0x24, 0x52, 0x44, 0xf7, 0x24,
    0xf2, 0x24, 0x35, 0xab, 0x21, 0x21, 0x43, 0x22,
    0xf2, 0x25, 0xf2, 0x22, 0xfb, 0x60, 0x00, 0x41,
    0x24, 0xf2, 0x24, 0xe5, 0x70, 0xf1, 0xd1, 0x71,
    0x41, 0xf4, 0x28, 0xa5, 0x71, 0x42, 0xf4, 0x28,
    0xa8, 0x71, 0x43, 0xf4, 0x28, 0xab, 0x71, 0x45,
    0xf4, 0x28, 0xac, 0x71, 0x46, 0xf4, 0x28, 0xad,
    0x71, 0x47, 0xf4, 0x28, 0xae, 0x71, 0x48, 0xf4,
    0x28, 0xa9, 0x71, 0x49, 0xf4, 0x28, 0xaa, 0x71,
    0x21, 0x40, 0xf4, 0x24, 0xaa, 0x71, 0x21, 0x41,
    0xf4, 0x24, 0xa4, 0x71, 0x21, 0x42, 0xf4, 0x24,
    0xa4, 0x71, 0x21, 0x43, 0xf4, 0x24, 0xa4, 0x71,
    0x21, 0x48, 0xf4, 0x25, 0xaa, 0x71, 0x21, 0x49,
    0xf4, 0x26, 0xa6, 0x71, 0x21, 0x4a, 0xf4, 0x26,
    0xa6, 0x71, 0x21, 0x4b, 0xf4, 0x25, 0xaa, 0x71,
    0x21, 0x4c, 0xf4, 0x21, 0xaa, 0x0c, 0x70, 0x71,
    0x25, 0x9b, 0x72, 0xa6, 0x70, 0x24, 0xf2, 0x54,
    0x72, 0xf7, 0x24, 0xf2, 0x24, 0x53, 0x40, 0xfe,
    0x40, 0x24, 0xf2, 0x24, 0xe5, 0x69, 0x09, 0x41,
    0xd1, 0x40, 0xd2, 0x61, 0x01, 0x48, 0xd1, 0x40,
    0xd2, 0x62, 0x0b, 0x46, 0xd1, 0x40, 0xd2, 0x62,
    0x05, 0x46, 0xd1, 0x22, 0x20, 0x41, 0xd2, 0x63,
    0x0d, 0x22, 0x20, 0x46, 0xd1, 0x41, 0xd2, 0x63,
    0x05, 0x44, 0xd1, 0x41, 0xd2, 0x64, 0x0f, 0x41,
    0xd1, 0x41, 0xd2, 0x64, 0x09, 0x41, 0xd1, 0x46,
    0xd2, 0x64, 0x03, 0x43, 0xd1, 0x40, 0xd2, 0x65,
    0x0d, 0x42, 0xd1, 0x40, 0xd2, 0x65, 0x07, 0x41,
    0xd1, 0x42, 0xd2, 0x65, 0x01, 0x60, 0xbf, 0x72,
    0x21, 0xaf, 0x24, 0xf2, 0x50, 0x73, 0xf1, 0x44,
    0x24, 0xf0, 0x23, 0x80, 0xfe, 0x24, 0xf2, 0x50,
    0x73, 0xf1, 0x4f, 0x24, 0xf6, 0x23, 0x80, 0xfe,
    0x73, 0x81, 0xd3, 0x72, 0x60, 0x8f, 0xd2, 0x62,
    0x0e, 0xb1, 0x22, 0xf0, 0x68, 0xb0, 0x40, 0x21,
    0x10, 0x26, 0x93, 0x21, 0x10, 0xd0, 0x2d, 0x47,
    0x21, 0xfb, 0x8f, 0x60, 0x40, 0x24, 0xf6, 0xd1,
    0x28, 0x7f, 0x21, 0x40, 0x24, 0xf0, 0x23, 0x24,
    0x21, 0xc2, 0x21, 0xa1, 0x21, 0x10, 0x71, 0x22,
    0x20, 0x40, 0x24, 0xfa, 0x71, 0x24, 0xf2, 0x24,
    0xea, 0x71, 0x29, 0xb0, 0xf6, 0x24, 0xf2, 0x24,
    0x34, 0x26, 0x4f, 0x21, 0xfb, 0x94, 0x21, 0x94,
    0x63, 0x04, 0x71, 0xf1, 0xad, 0x71, 0xf1, 0x72,
    0x23, 0xfb, 0x71, 0x81, 0xd1, 0x72, 0x82, 0xd2,
    0x60, 0x00, 0x22, 0xf0, 0x60, 0xbe, 0x45, 0xd0,
    0x10, 0xd1, 0x11, 0x24, 0xf2, 0x24, 0x52, 0x44,
    0xfb, 0x11, 0x24, 0xf2, 0x24, 0x53, 0x41, 0xf7,
    0x70, 0x61, 0xab, 0xb2, 0x22, 0xf0, 0x71, 0x60,
    0x8f, 0xd1, 0x41, 0x71, 0x23, 0xfb, 0x40, 0x71,
    0x81, 0x23, 0xfb, 0x72, 0x21, 0x48, 0x24, 0xf0,
    0x71, 0x82, 0x23, 0xfb, 0x72, 0x21, 0x40, 0x24,
    0xf0, 0x71, 0x83, 0x23, 0xfb, 0x72, 0x48, 0x24,
    0xf0, 0x71, 0x84, 0x23, 0xfb, 0x72, 0x71, 0x85,
    0x23, 0xfb, 0x71, 0xd2, 0x12, 0x24, 0xf2, 0x24,
    0x52, 0x44, 0xfb, 0x12, 0x24, 0xf2, 0x24, 0x53,
    0x41, 0xf7, 0x22, 0xf0, 0x49, 0x6e, 0x73, 0x65,
    0x72, 0x74, 0x65, 0x20, 0x75, 0x6e, 0x20, 0x64,
    0x69, 0x73, 0x63, 0x6f, 0x20, 0x63, 0x6f, 0x6e,
    0x20, 0x73, 0x69, 0x73, 0x74, 0x65, 0x6d, 0x61,
    0x20, 0x6f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69,
    0x76, 0x6f, 0x2c, 0x20, 0x79, 0x20, 0x70, 0x75,
    0x6c, 0x73, 0x65, 0x20, 0x75, 0x6e, 0x61, 0x20,
    0x74, 0x65, 0x63, 0x6c, 0x61, 0x2e, 0x2e, 0x2e,
    0x00,
];

var ram_disk_boot = [
    0x24, 0x0e, 0x53, 0x4f, 0x4d, 0x33, 0x32, 0x20,
    0x76, 0x31, 0x2e, 0x30, 0xa7, 0xf1, 0xd9, 0x2a,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x44, 0x69, 0x73, 0x63, 0x6f, 0x20, 0x52, 0x41,
    0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0x12, 0x34,
    0xfd, 0xff, 0xff, 0xff];

/*
 ** Initial floppy disk
 */
var floppy = new Uint8Array(80 * 18432);

floppy_image = window.atob(floppy_image);

for (c = 0; c < floppy_image.length; c++)
    floppy[c] = floppy_image.charCodeAt(c);
for (; c < 80 * 18432; c++)
    floppy[c] = 0xfc;

/*
 ** Initial RAM disk for operating system
 */
var ram_disk = new Uint8Array(524288);

for (c = 0; c < ram_disk_boot.length; c++)
    ram_disk[c] = ram_disk_boot[c];
for (; c < 4096; c++)
    ram_disk[c] = 0x00;
for (; c < 524288; c++)
    ram_disk[c] = 0xfc;

/*
 ** Initial harddisk for the operating system
 */
var harddisk = new Uint8Array(40 * 1048576);

harddisk_image = window.atob(harddisk_image);

for (c = 0; c < harddisk_image.length; c++)
    harddisk[c] = harddisk_image.charCodeAt(c);
for (; c < 40 * 1048576; c++)
    harddisk[c] = 0xfc;

bytes_channel0 = maestro.length;
for (c = 0; c < bytes_channel0; c++) {
    channel0[c] = maestro[c];
}

boot_image[61] |= (bytes_channel0 >> 12) & 0x0f;
boot_image[62] |= (bytes_channel0 >>  8) & 0x0f;
boot_image[63] |= (bytes_channel0 >>  4) & 0x0f;
boot_image[64] |= (bytes_channel0      ) & 0x0f;
for (c = 0; c < boot_image.length; c++)
    memory[112 + c] = boot_image[c];

var Areg = 0;
var Breg = 0;
var Creg = 0;
var Oreg = 0;
var Iptr = 0;
var Wptr = 0;
var fp = [0,0,0,0];
var fp_save = [0,0,0,0];
var completed = 0;
var local_count = 0;
var transputer_error = 0;
var transputer_halterr = 0;
var transputer_fperr = 0;
var transputer_priority = 0;
var transputer_FptrReg0 = 0x80000000;
var transputer_BptrReg0 = 0x80000000;
var transputer_CptrReg0 = 0x80000000;
var transputer_FptrReg1 = 0x80000000;
var transputer_BptrReg1 = 0x80000000;
var transputer_CptrReg1 = 0x80000000;
var transputer_WdescReg = 0;
var transputer_StatusReg = 0;
var transputer_ClockReg0 = 0;
var transputer_ClockReg1 = 0;
var roundMode = ROUND_NEAREST;

function memory_dump(addr) {
    var c;
    var str;
    var d;
    
    addr &= 0xffffff00;
    for (c = 0; c < 16; c++) {
        str = (addr >>> 0).toString(16) + " ";
        for (d = 0; d < 16; d++) {
            str += ("0" + read_memory(addr).toString(16)).substr(-2) + " ";
            addr++;
        }
        str += "\n";
        console.log(str);
    }
}

function transputer() {
    Areg = 0;
    Breg = 0;
    Creg = 0;
    Oreg = 0;
    Iptr = 0;
    fp[0] = 0;
    fp[1] = 0;
    fp[2] = 0;
    transputer_error = 0;
    transputer_halterr = 0;
    transputer_fperr = 0;
    transputer_priority = 0;
    transputer_FptrReg0 = 0x80000000;
    transputer_BptrReg0 = 0x80000000;
    transputer_CptrReg0 = 0x80000000;
    transputer_FptrReg1 = 0x80000000;
    transputer_BptrReg1 = 0x80000000;
    transputer_CptrReg1 = 0x80000000;
    transputer_WdescReg = 0;
    transputer_StatusReg = 0;
    transputer_ClockReg0 = 0;
    transputer_ClockReg1 = 0;
    roundMode = ROUND_NEAREST;
    local_count = 0;
    Wptr = 0;
    write_memory_32(0x80000024, 0x80000000);
    write_memory_32(0x80000028, 0x80000000);
};

transputer.prototype.count_timers = function () {
    transputer_ClockReg0 = (transputer_ClockReg0 + 1) >>> 0;
    if ((transputer_ClockReg0 & 0x3f) == 0) {
        transputer_ClockReg1 = (transputer_ClockReg1 + 1) >>> 0; /* One tick each 64 microseconds */
    }
}

transputer.prototype.UpdateWdescReg = function (NewWdescReg) {
    transputer_WdescReg = NewWdescReg >>> 0;
    Wptr = (transputer_WdescReg & ~3) >>> 0;
    transputer_priority = transputer_WdescReg & 1;
};

transputer.prototype.int32_subtract = function (value) {
    if (value >>> 31)
        return -1;
    if ((value >>> 0) == 0)
        return 0;
    return 1;
};

transputer.prototype.SaveRegisters = function () {
    write_memory_32(0x8000002c, transputer_WdescReg);
    if (transputer_WdescReg != (NotProcess_p + 1)) {
        write_memory_32(0x80000030, Iptr);
        write_memory_32(0x80000034, Areg);
        write_memory_32(0x80000038, Breg);
        write_memory_32(0x8000003c, Creg);
        write_memory_32(0x80000040, transputer_StatusReg);
        fp_save[0] = fp[0];
        fp_save[1] = fp[1];
        fp_save[2] = fp[2];
    }
};

transputer.prototype.RestoreRegisters = function () {
    var temp = read_memory_32(0x8000002c);
    this.UpdateWdescReg(temp);
    if (transputer_WdescReg != (NotProcess_p + 1)) {
        Iptr = read_memory_32(0x80000030);
        Areg = read_memory_32(0x80000034);
        Breg = read_memory_32(0x80000038);
        Creg = read_memory_32(0x8000003c);
        transputer_StatusReg = read_memory_32(0x80000040);
        fp[0] = fp_save[0];
        fp[1] = fp_save[1];
        fp[2] = fp_save[2];
    }
};

transputer.prototype.Dequeue0 = function () {
    this.UpdateWdescReg(transputer_FPtrReg0 | 0);
    if (transputer_FPtrReg0 == transputer_BPtrReg0)
        transputer_FPtrReg0 = NotProcess_p;
    else
        transputer_FPtrReg0 = read_memory_32(transputer_FPtrReg0 - 8);
};

transputer.prototype.Dequeue1 = function () {
    this.UpdateWdescReg(transputer_FPtrReg1 | 1);
    if (transputer_FPtrReg1 == transputer_BPtrReg1)
        transputer_FPtrReg1 = NotProcess_p;
    else
        transputer_FPtrReg1 = read_memory_32(transputer_FPtrReg1 - 8);
};

transputer.prototype.ActivateProcess = function () {
    Oreg = 0;
    Iptr = read_memory_32(Wptr - 4);
};

transputer.prototype.Run = function (ProcDesc) {
    if (transputer_priority == 0) {
        if (ProcDesc & 1) {
            if (transputer_FPtrReg1 == NotProcess_p) {
                transputer_FPtrReg1 = (ProcDesc & ~3) >>> 0;
            } else {
                write_memory_32(transputer_BPtrReg1 - 8, ProcDesc & ~3);
            }
            transputer_BPtrReg1 = (ProcDesc & ~3) >>> 0;
        } else {
            if (transputer_FPtrReg0 == NotProcess_p) {
                transputer_FPtrReg0 = (ProcDesc & ~3) >>> 0;
            } else {
                write_memory_32(transputer_BPtrReg0 - 8, ProcDesc & ~3);
            }
            transputer_BPtrReg0 = (ProcDesc & ~3) >>> 0;
        }
    } else {
        if ((ProcDesc & 1) == 0) {
            this.SaveRegisters();
            this.UpdateWdescReg(ProcDesc);
            transputer_StatusReg &= ErrorFlag | HaltOnErrorBit;
            this.ActivateProcess();
        } else if (Wptr == NotProcess_p) {
            this.UpdateWdescReg(ProcDesc);
            this.ActivateProcess();
        } else {
            if (transputer_FPtrReg1 == NotProcess_p) {
                transputer_FPtrReg1 = (ProcDesc & ~3) >>> 0;
            } else {
                write_memory_32(transputer_BPtrReg1 - 8, ProcDesc & ~3);
            }
            transputer_BPtrReg1 = (ProcDesc & ~3) >>> 0;
        }
    }
};

transputer.prototype.start_emulation = function () {
    var temp;
    var temp2;
    var byte;
    var c;
    var loop;

//    console.log(Iptr.toString(16));
//    console.log(Wptr.toString(16));
//    console.log(Oreg.toString(16));
    loop = 0;
    do {
        local_count += 30; /* 30 ns per byte */
        if (local_count >= 1000) {   /* 1 microsecond */
            local_count -= 1000;
            this.count_timers();
        }
        if (completed) {
            if ((transputer_StatusReg & GotoSNPBit) != 0) {
                transputer_StatusReg &= ~GotoSNPBit;
                
                if (transputer_priority == 0) {
                    if (transputer_FPtrReg0 != NotProcess_p) {
                        this.Dequeue0();
                        this.ActivateProcess();
                    } else {
                        this.RestoreRegisters();
                        if (Wptr == (NotProcess_p + 0) && transputer_FPtrReg1 != NotProcess_p) {
                            this.Dequeue1();
                            this.ActivateProcess();
                        } else {
                        }
                    }
                } else if (transputer_FPtrReg1 != NotProcess_p) {
                    this.Dequeue1();
                    this.ActivateProcess();
                } else {
                    while (1) {
                        if ((temp = read_memory_32(0x80000024)) != NotProcess_p && (read_memory_32(temp - 20) - transputer_ClockReg0) >>> 31) {
                            write_memory_32(0x80000024, read_memory_32(temp - 16));  /* Remove timer from list */
                            this.Run(temp);
                            break;
                        }
                        if ((temp = read_memory_32(0x80000028)) != NotProcess_p && (read_memory_32(temp - 20) - transputer_ClockReg1) >>> 31) {
                            write_memory_32(0x80000028, read_memory_32(temp - 16));  /* Remove timer from list */
                            this.Run(temp | 1);
                            break;
                        }
                        if (read_memory_32(0x80000024) == 0x80000000 && read_memory_32(0x80000028) == 0x80000000) {
                            /*                            {
                             FILE *dump;
                             
                             fprintf(stderr, "Iptr=%08x A=%08x B=%08x C=%08x Wptr=%08x\n", Iptr, Areg, Breg, Creg, Wptr);
                             fprintf(stderr, "FPtrReg0=%08x\n", transputer_FPtrReg0);
                             fprintf(stderr, "FPtrReg1=%08x\n", transputer_FPtrReg1);
                             dump = fopen("memory.bin", "wb");
                             fwrite(memory, 1, sizeof(memory), dump);
                             fclose(dump);
                             }*/
                            return;
                        }
                        local_count += 30; /* 30 ns per byte */
                        if (local_count >= 1000) {   /* 1 microsecond */
                            local_count -= 1000;
                            this.count_timers();
                        }
                    }
                }
            } else {

                /*
                 ** Handle high-priority timer
                 ** The transputer has a long microcode here to handle timers.
                 ** This is oversimplified.
                 */
                if ((temp = read_memory_32(0x80000024)) != NotProcess_p && (read_memory_32(temp - 20) - transputer_ClockReg0) >>> 31) {
                    write_memory_32(0x80000024, read_memory_32(temp - 16));  /* Remove timer from list */
                    this.Run(temp);
                } else if ((temp = read_memory_32(0x80000028)) != NotProcess_p && (read_memory_32(temp - 20) - transputer_ClockReg1) >>> 31) {
                    write_memory_32(0x80000028, read_memory_32(temp - 16));  /* Remove timer from list */
                    this.Run(temp | 1);
                }
            }
        }
        
        byte = memory[Iptr & MEMORY_MASK];
        Iptr++;
        
        Oreg += byte & 0x0f;
        completed = 1;
        switch (byte & 0xf0) {
            case 0x00:  /* j */
                Iptr = (Iptr + Oreg) >>> 0;
                Oreg = 0;
                
                /*
                 ** j is one of the scheduling points for the transputer
                 ** Other interruptable instructions (not implemented):
                 ** + move
                 ** + talt
                 ** + tin
                 ** + in
                 ** + out
                 ** + dist
                 */
                if (transputer_priority == 1) {
                    write_memory_32(Wptr - 4, Iptr);
                    if (transputer_FPtrReg1 == NotProcess_p)
                        transputer_FPtrReg1 = Wptr;
                    else
                        write_memory_32(transputer_BPtrReg1 - 8, Wptr);
                    transputer_BPtrReg1 = Wptr;
                    transputer_StatusReg |= GotoSNPBit;
                }
                break;
            case 0x10:  /* ldlp */
                Creg = Breg;
                Breg = Areg;
                Areg = (Wptr + (Oreg << 2)) >>> 0;
                Oreg = 0;
                break;
            case 0x20:  /* pfix */
                Oreg = (Oreg << 4) >>> 0;
                completed = 0;
                break;
            case 0x30:  /* ldnl */
                Areg = read_memory_32(Areg + (Oreg << 2));
                Oreg = 0;
                break;
            case 0x40:  /* ldc */
                Creg = Breg;
                Breg = Areg;
                Areg = Oreg;
                Oreg = 0;
                break;
            case 0x50:  /* ldnlp */
                /* Oreg ~= 3; */    /* Inmos says this happens, it isn't true */
                Areg = (Areg + (Oreg << 2)) >>> 0;
                Oreg = 0;
                break;
            case 0x60:  /* nfix */
                Oreg = (~Oreg << 4) >>> 0;
                completed = 0;
                break;
            case 0x70:  /* ldl */
                Creg = Breg;
                Breg = Areg;
                Areg = read_memory_32(Wptr + (Oreg << 2));
                Oreg = 0;
                break;
            case 0x80:  /* adc */
                temp = (Areg + Oreg) >>> 0;
                /* If operands are same sign but result is different sign */
                if (((Areg ^ Oreg) & 0x80000000) == 0 && ((Areg ^ temp) & 0x80000000) != 0)
                    transputer_error = 1;
                Areg = temp;
                Oreg = 0;
                break;
            case 0x90:  /* call */
                Wptr -= 4;
                write_memory_32(Wptr, Creg);
                Wptr -= 4;
                write_memory_32(Wptr, Breg);
                Wptr -= 4;
                write_memory_32(Wptr, Areg);
                Wptr -= 4;
                write_memory_32(Wptr, Iptr);
                Areg = Iptr;
                this.UpdateWdescReg(Wptr | transputer_priority);
                Iptr = (Iptr + Oreg) >>> 0;
                Oreg = 0;
                break;
            case 0xa0:  /* cj */
                if (Areg == 0) {
                    Iptr = (Iptr + Oreg) >>> 0;
                } else {
                    Areg = Breg;
                    Breg = Creg;
                }
                Oreg = 0;
                break;
            case 0xb0:  /* ajw */
                Wptr = (Wptr + (Oreg << 2)) >>> 0;
                this.UpdateWdescReg(Wptr | transputer_priority);
                Oreg = 0;
                break;
            case 0xc0:  /* eqc */
                if (Areg == Oreg)
                    Areg = 1;
                else
                    Areg = 0;
                Oreg = 0;
                break;
            case 0xd0:  /* stl */
                write_memory_32(Wptr + (Oreg << 2), Areg);
                Areg = Breg;
                Breg = Creg;
                Oreg = 0;
                break;
            case 0xe0:  /* stnl */
                write_memory_32(Areg + (Oreg << 2), Breg);
                Areg = Creg;
                Oreg = 0;
                break;
            case 0xf0:  /* opr */
                switch (Oreg) {
                        
                        /*
                         ** Instructions for handling processes
                         */
                    case 0x2b:  /* tin */
                        if (transputer_priority == 0)   /* High-priority */
                            temp = transputer_ClockReg0;
                        else    /* Low-priority */
                            temp = transputer_ClockReg1;
                        if (this.int32_subtract(Areg - temp) > 0) {  /* Should wait this time */
                            write_memory_32(Wptr - 4, Iptr);    /* Current instruction ptr. */
                            write_memory_32(Wptr - 20, Areg);   /* Time for awakening */
                            if (transputer_priority == 0) { /* Link timer */
                                temp2 = 0x80000024;
                            } else {
                                temp2 = 0x80000028;
                            }
                            while (1) {
                                temp = read_memory_32(temp2);
                                if (temp == 0x80000000)
                                    break;
                                if ((Areg - read_memory_32(temp - 20)) >>> 31)
                                    break;
                                temp2 = temp - 16;
                            }
                            write_memory_32(Wptr - 16, read_memory_32(temp2));
                            write_memory_32(temp2, Wptr);
                            transputer_StatusReg |= GotoSNPBit;
                        }
                        break;
                    case 0x0d:  /* startp */
                        write_memory_32(Areg - 4, Iptr + Breg);
                        temp = Areg | transputer_priority;
                        this.Run(temp);
                        break;
                    case 0x15:  /* stopp */
                        write_memory_32(Wptr - 4, Iptr);
                        transputer_StatusReg |= GotoSNPBit;
                        break;
                    case 0x03:  /* endp */
                        temp = read_memory_32(Areg + 4);
                        if (temp == 1) {
                            Iptr = read_memory_32(Areg);
                            Wptr = Areg;
                        } else {
                            write_memory_32(Areg + 4, temp - 1);
                            transputer_StatusReg |= GotoSNPBit;
                        }
                        break;
                    case 0x39:  /* runp */
                        temp = Areg;
                        this.Run(temp);
                        break;
                    case 0x3d:  /* savel */
                        write_memory_32(Areg, transputer_FPtrReg1);
                        write_memory_32(Areg + 4, transputer_BPtrReg1);
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x3e:  /* saveh */
                        write_memory_32(Areg, transputer_FPtrReg0);
                        write_memory_32(Areg + 4, transputer_BPtrReg0);
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x50:  /* sthb */
                        transputer_BPtrReg0 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x17:  /* stlb */
                        transputer_BPtrReg1 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x18:  /* sthf */
                        transputer_FPtrReg0 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x1c:  /* stlf */
                        transputer_FPtrReg1 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                        
                        /*
                         ** Instructions for handling channels
                         */
                    case 0x07:  /* in */
                        if ((Breg & ~0x1f) == (0x80000000 & ~0x1f)) {
                            handle_input(Creg, Breg, Areg);
                        } else {
                            var procDesc = read_memory_32(Breg);
                            if (procDesc == NotProcess_p) {
                                write_memory_32(Breg, transputer_WdescReg);
                                write_memory_32(Wptr - 4, Iptr);
                                write_memory_32(Wptr - 12, Creg);
                                transputer_StatusReg |= GotoSNPBit;
                            } else {
                                var procPtr = procDesc & ~3;
                                var source_address = read_memory_32(procPtr - 12);
                                while(Areg--) {
                                    write_memory(Creg, read_memory(source_address));
                                    Creg++;
                                    source_address++;
                                }
                                write_memory_32(Breg, NotProcess_p);
                                this.Run(procDesc);
                            }
                        }
                        break;
                    case 0x0e:  /* outbyte */
                    case 0x0f:  /* outword */
                    case 0x0b:  /* out */
                        if (Oreg == 0x0e) {
                            write_memory(Wptr, Areg & 0xff);
                            Creg = Wptr;
                            Areg = 1;
                        } else if (Oreg == 0x0f) {
                            write_memory_32(Wptr, Areg);
                            Creg = Wptr;
                            Areg = 4;
                        }
                        if ((Breg & ~0x1f) == (0x80000000 & ~0x1f)) {
                            handle_output(Creg, Breg, Areg);
                        } else {
                            var procDesc = read_memory_32(Breg);
                            if (procDesc == NotProcess_p) {
                                write_memory_32(Breg, transputer_WdescReg);
                                write_memory_32(Wptr - 4, Iptr);
                                write_memory_32(Wptr - 12, Creg);
                                transputer_StatusReg |= GotoSNPBit;
                            } else {
                                var procPtr = procDesc & ~3;
                                var target_address = read_memory_32(procPtr - 12);
                                while (Areg--) {
                                    write_memory(target_address, read_memory(Creg));
                                    Creg++;
                                    target_address++;
                                }
                                write_memory_32(Breg, NotProcess_p);
                                this.Run(procDesc);
                            }
                        }
                        break;
                    case 0x00:  /* rev */
                        temp = Areg;
                        Areg = Breg;
                        Breg = temp;
                        break;
                    case 0x01:  /* lb */
                        Areg = read_memory(Areg);
                        break;
                    case 0x02:  /* bsub */
                        Areg = (Areg + Breg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x04:  /* diff */
                        Areg = (Breg - Areg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x05:  /* add */
                        temp = (Breg + Areg) >>> 0;
                        /* If operands are same sign but result is different sign */
                        if (((Breg ^ Areg) & 0x80000000) == 0 && ((Breg ^ temp) & 0x80000000) != 0)
                            transputer_error = 1;
                        Areg = temp;
                        Breg = Creg;
                        break;
                    case 0x06:  /* gcall */
                        temp = Iptr;
                        Iptr = Areg;
                        Areg = temp;
                        break;
                    case 0x08:  /* prod */
                        Areg = (Breg * Areg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x09:  /* gt */
                        /*
                         ** The book Transputer Assembly Language Programming
                         ** says this is unsigned. It isn't.
                         */
                        if ((Breg | 0) > (Areg | 0))    /* Signed comparison */
                            Areg = 1;
                        else
                            Areg = 0;
                        Breg = Creg;
                        break;
                    case 0x0a:  /* wsub */
                        Areg = (Areg + Breg * 4) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x0c:  /* sub */
                        temp = (Breg - Areg) >>> 0;
                        /* If operands are different sign but result is different sign than first operand */
                        if (((Breg ^ Areg) & 0x80000000) != 0 && ((Breg ^ temp) & 0x80000000) != 0)
                            transputer_error = 1;
                        Areg = temp;
                        Breg = Creg;
                        break;
                    case 0x12:  /* resetch */
                        temp = read_memory_32(Areg);
                        write_memory_32(Areg, 0x80000000);
                        Areg = temp;
                        break;
                    case 0x10:  /* seterr */
                        transputer_error = 1;
                        break;
                    case 0x13:  /* csub0 */
                        if (Breg >= Areg)
                            transputer_error = 1;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x16:  /* ladd */
                        temp = (Breg + Areg + (Creg & 1)) >>> 0;
                        /* If operands are same sign but result is different sign */
                        if (((Breg ^ Areg) & 0x80000000) == 0 && ((Breg ^ temp) & 0x80000000) != 0)
                            transputer_error = 1;
                        Areg = temp;
                        break;
                    case 0x19:  /* norm */
                        Creg = 0;
                        while (Creg < 64 && (Breg & 0x80000000) == 0) {
                            Breg = (Breg << 1) | (Areg >>> 31);
                            Areg = (Areg << 1);
                            Creg++;
                        }
                        break;
                    case 0x1b:  /* ldpi */
                        Areg += Iptr;
                        break;
                    case 0x1d:  /* xdble */
                        Creg = Breg;
                        Breg = Areg >> 31;
                        break;
                    case 0x1e:  /* ldpri */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_priority;
                        break;
                    case 0x1f:  /* rem */
                        if (Areg == 0 || Areg == 0xffffffff && Breg == 0x80000000) {
                            transputer_error = 1;
                        } else {
                            /* Important: Operation is signed */
                            Areg = (Breg | 0) % (Areg | 0);
                        }
                        /* Sign is the same as dividend, Javascript is the same */
                        Breg = Creg;
                        break;
                    case 0x20:  /* ret */
                        Iptr = read_memory_32(Wptr);
                        Wptr += 16;
                        this.UpdateWdescReg(Wptr | transputer_priority);
                        break;
                    case 0x21:  /* lend */
                        Breg &= ~3;
                        temp = (read_memory_32(Breg + 4) - 1) >>> 0;
                        write_memory_32(Breg + 4, temp);
                        if (temp == 0)
                            break;
                        temp = (read_memory_32(Breg) + 1) >>> 0;
                        write_memory_32(Breg, temp);
                        Iptr = (Iptr - Areg) >>> 0;
                        if (transputer_priority == 1) {
                            write_memory_32(Wptr - 4, Iptr);
                            if (transputer_FPtrReg1 == NotProcess_p)
                                transputer_FPtrReg1 = Wptr;
                            else
                                write_memory_32(transputer_BPtrReg1 - 8, Wptr);
                            transputer_BPtrReg1 = Wptr;
                            transputer_StatusReg |= GotoSNPBit;
                        }
                        break;
                    case 0x22:  /* ldtimer */
                        Creg = Breg;
                        Breg = Areg;
                        if (transputer_priority == 0)
                            Areg = transputer_ClockReg0;
                        else
                            Areg = transputer_ClockReg1;
                        break;
                    case 0x29:  /* testerr */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_error;
                        transputer_error = 0;
                        break;
                    case 0x2a:  /* testpranal */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = 0;   /* Analyse pin not asserted */
                        break;
                    case 0x2c:  /* div */
                        if (Areg == 0 || Areg == 0xffffffff && Breg == 0x80000000) {
                            /* Force emulator exit */
                            return;
                            transputer_error = 1;
                        } else {
                            /* Important: Operation is signed */
                            Areg = Math.floor((Breg | 0) / (Areg | 0));
                        }
                        Breg = Creg;
                        break;
                    case 0x2e:  /* dist */
                        not_handled();
                        break;
                    case 0x2f:  /* disc */
                        not_handled();
                        break;
                    case 0x30:  /* diss */
                        not_handled();
                        break;
                    case 0x32:  /* not */
                        Areg = ~Areg >>> 0;
                        break;
                    case 0x33:  /* xor */
                        Areg = (Breg ^ Areg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x34:  /* bcnt */
                        Areg = (Areg << 2) >>> 0;
                        break;
                    case 0x38:  /* lsub */
                        temp = (Breg - Areg - (Creg & 1)) >>> 0;
                        /* If operands are different sign but result is different sign than first operand */
                        if (((Breg ^ Areg) & 0x80000000) != 0 && ((Breg ^ temp) & 0x80000000) != 0)
                            transputer_error = 1;
                        Areg = temp;
                        break;
                    case 0x3a:  /* xword */
                        if (Breg < Areg) {
                            Areg = Breg;
                        } else {
                            Areg = (Breg - Areg * 2) >>> 0;
                        }
                        Breg = Creg;
                        break;
                    case 0x3b:  /* sb */
                        write_memory(Areg, Breg & 0xff);
                        Areg = Creg;
                        break;
                    case 0x3c:  /* gajw */
                        Areg &= ~3;
                        Wptr = Areg;
                        this.UpdateWdescReg(Wptr | transputer_priority);
                        break;
                    case 0x3f:  /* wcnt */
                        Creg = Breg;
                        Breg = Areg & 3;
                        Areg = Areg >>> 2;
                        break;
                    case 0x40:  /* shr */
                        Areg = Breg >>> Areg;
                        Breg = Creg;
                        break;
                    case 0x41:  /* shl */
                        Areg = (Breg << Areg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x42:  /* mint */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = 0x80000000;
                        break;
                    case 0x43:  /* alt */
                        not_handled();
                        break;
                    case 0x44:  /* altwt */
                        not_handled();
                        break;
                    case 0x45:  /* altend */
                        not_handled();
                        break;
                    case 0x46:  /* and */
                        Areg = (Breg & Areg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x47:  /* enbt */
                        not_handled();
                        break;
                    case 0x48:  /* enbc */
                        not_handled();
                        break;
                    case 0x49:  /* enbs */
                        not_handled();
                        break;
                    case 0x4a:  /* move */
                        while (Areg) {
                            write_memory(Breg, read_memory(Creg));
                            Creg = (Creg + 1) >>> 0;
                            Breg = (Breg + 1) >>> 0;
                            Areg--;
                        }
                        break;
                    case 0x4b:  /* or */
                        Areg = (Breg | Areg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x4c:  /* csngl */
                        if ((Areg & 0x80000000) != 0) {
                            if (Breg != 0xffffffff)
                                transputer_error = 1;
                        } else {
                            if (Breg != 0x00000000)
                                transputer_error = 1;
                        }
                        Breg = Creg;
                        break;
                    case 0x4d:  /* ccnt1 */
                        if (Breg == 0 || Breg > Areg)
                            transputer_error = 1;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x4e:  /* talt */
                        not_handled();
                        break;
                    case 0x51:  /* taltwt */
                        not_handled();
                        break;
                    case 0x52:  /* sum */
                        Areg = (Breg + Areg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x53:  /* mul */
                        Areg = (Breg * Areg) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x54:  /* sttimer */
                        /* !!! This instruction should start timers */
                        transputer_ClockReg0 = Areg;
                        transputer_ClockReg1 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x55:  /* stoperr */
                        /* !!! if transputer_error then stop current process */
                        not_handled();
                        break;
                    case 0x56:  /* cword */
                    case 0x57:  /* clrhalterr */
                        transputer_halterr = 0;
                        break;
                    case 0x58:  /* sethalterr */
                        transputer_halterr = 1;
                        break;
                    case 0x59:  /* testhalterr */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_halterr;
                        break;
                    case 0x63:  /* unpacksn (only T414) */
                        not_handled();
                        break;
                    case 0x6c:  /* postnormsn (only T414) */
                        not_handled();
                        break;
                    case 0x6d:  /* roundsn (only T414) */
                        not_handled();
                        break;
                    case 0x71:  /* ldinf (only T414) */
                        not_handled();
                        break;
                    case 0x72:  /* fmul */
                        not_handled();
                        break;
                    case 0x73:  /* cflerr (only T414) */
                        not_handled();
                        break;
                        
                        /*
                         ** Start of T800 instructions
                         */
                    case 0x5a:  /* dup */
                        Creg = Breg;
                        Breg = Areg;
                        break;
                    case 0x5b:  /* move2dinit */
                        not_handled();
                        break;
                    case 0x5c:  /* move2dall */
                        not_handled();
                        break;
                    case 0x5d:  /* move2dnonzero */
                        not_handled();
                        break;
                    case 0x5e:  /* move2dzero */
                        not_handled();
                        break;
                    case 0x74:  /* crcword */
                        c = 32;
                        do {
                            temp = Breg & 0x80000000;
                            Breg = ((Breg << 1) | (Areg >>> 31)) >>> 0;
                            if (temp)
                                Breg = (Breg ^ Creg) >>> 0;
                            Areg <<= 1;
                        } while (--c) ;
                        Areg = Breg;
                        break;
                    case 0x75:  /* crcbyte */
                        c = 8;
                        do {
                            temp = Breg & 0x80000000;
                            Breg = ((Breg << 1) | (Areg >>> 31)) >>> 0;
                            if (temp)
                                Breg = (Breg ^ Creg) >>> 0;
                            Areg <<= 1;
                        } while (--c) ;
                        Areg = Breg;
                        break;
                    case 0x76:  /* bitcnt */
                        not_handled();
                        break;
                    case 0x77:  /* bitrevword */
                        not_handled();
                        break;
                    case 0x78:  /* bitrevnbits */
                        not_handled();
                        break;
                    case 0x79:  /* pop */
                        temp = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        Creg = temp;
                        break;
                    case 0x7a:  /* timerdisableh */
                        /* !!! Do something (^^)! */
                        break;
                    case 0x7b:  /* timerdisablel */
                        /* !!! Do something (^^)! */
                        break;
                    case 0x7c:  /* timerenableh */
                        /* !!! Do something (^^)! */
                        break;
                    case 0x7d:  /* timerenablel */
                        /* !!! Do something (^^)! */
                        break;
                    case 0x81:  /* wsubdb */
                        Areg = (Areg + Breg * 8) >>> 0;
                        Breg = Creg;
                        break;
                    case 0x82:  /* fpldnldbi */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = read_memory_fp64(Areg + Breg * 8);
                        Areg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x83:  /* fpchkerr */
                        transputer_error |= transputer_fperr;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x84:  /* fpstnldb */
                        write_memory_fp64(Areg, fp[0]);
                        fp[0] = fp[1];
                        fp[1] = fp[2];
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x86:  /* fpldnlsni */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = read_memory_fp32(Areg + Breg * 4);
                        Areg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x87:  /* fpadd */
                        fp[0] = fp[1] + fp[0];
                        fp[1] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x88:  /* fpstnlsn */
                        write_memory_fp32(Areg, fp[0]);
                        fp[0] = fp[1];
                        fp[1] = fp[2];
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x89:  /* fpsub */
                        fp[0] = fp[1] - fp[0];
                        fp[1] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8a:  /* fpldnldb */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = read_memory_fp64(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8b:  /* fpmul */
                        fp[0] = fp[1] * fp[0];
                        fp[1] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8c:  /* fpdiv */
                        fp[0] = fp[1] / fp[0];
                        fp[1] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8e:  /* fpldnlsn */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = read_memory_fp32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8f:  /* fpremfirst */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x90:  /* fpremstep */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x91:  /* fpnan */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x92:  /* fpordered */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x93:  /* fpnotfinite */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x94:  /* fpgt */
                        /* !!! Set transputer_fperr if operands are NaN or Inf */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = (fp[1] > fp[0]) ? 1 : 0;
                        fp[0] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x95:  /* fpeq */
                        /* !!! Set transputer_fperr if operands are NaN or Inf */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = (fp[1] == fp[0]) ? 1 : 0;
                        fp[0] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x96:  /* fpi32tor32 */
                        temp = read_memory_32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = temp | 0;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x98:  /* fpi32tor64 */
                        temp = read_memory_32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = temp | 0;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9a:  /* fpb32tor64 */
                        temp = read_memory_32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = temp;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9c:  /* fptesterr */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_fperr;
                        transputer_fperr = 0;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9d:  /* fprtoi32 */
                        /* Does nothing, does process in high-precision */
                        /* !!! Validate or set transputer_fperr */
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9e:  /* fpstnli32 */
                        temp = fp[0] | 0;
                        write_memory_32(Areg, temp);
                        fp[0] = fp[1];
                        fp[1] = fp[2];
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9f:  /* fpldzerosn */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = 0;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa0:  /* fpldzerodb */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0] = 0;
                        RoundMode = ROUND_NEAREST;
                        break;

                        /*
                         ** Start of T805-only instructions.
                         */
                    case 0x7e:  /* ldmemstartval */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = 0x80000070;
                        break;
                    case 0x9c:  /* fptesterr */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_fperr;
                        transputer_fperr = 0;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa1:  /* fpint */
                        switch (RoundMode) {
                            case ROUND_MINUS:
                                fp[0] = Math.floor(fp[0]);
                                break;
                            case ROUND_POSITIVE:
                                fp[0] = Math.ceil(fp[0]);
                                break;
                            case ROUND_ZERO:
                                if (fp[0] < 0) {
                                    fp[0] = -Math.floor(-fp[0]);
                                } else {
                                    fp[0] = Math.floor(fp[0]);
                                }
                                break;
                            case ROUND_NEAREST:
                                if (fp[0] < 0) {
                                    fp[0] = Math.ceil(fp[0] - 0.5);
                                } else {
                                    fp[0] = Math.floor(fp[0] + 0.5);
                                }
                                break;
                        }
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa3:  /* fpdup */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa4:  /* fprev */
                        fp[3] = fp[0];
                        fp[0] = fp[1];
                        fp[1] = fp[3];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa6:  /* fpldnladddb */
                        fp[0] += read_memory_fp64(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa8:  /* fpldnlmuldb */
                        fp[0] *= read_memory_fp64(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xaa:  /* fpldnladdsn */
                        fp[0] += read_memory_fp32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xac:  /* fpldnlmulsn */
                        fp[0] *= read_memory_fp32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xab:  /* fpentry */
                        temp = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        switch (temp) {
                            case 0x01:  /* fpusqrtfirst */
                                /* Ignored */
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x02:  /* fpusqrtstep */
                                /* Ignored */
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x03:  /* fpusqrtlast */
                                fp[0] = Math.sqrt(fp[0]);
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x06:  /* fpurz */
                                RoundMode = ROUND_ZERO;
                                break;
                            case 0x22:  /* fpurn */
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x04:  /* fpurp */
                                RoundMode = ROUND_POSITIVE;
                                break;
                            case 0x05:  /* fpurm */
                                RoundMode = ROUND_MINUS;
                                break;
                            case 0x11:  /* fpudivby2 */
                                fp[0] /= 2.0;
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x12:  /* fpumulby2 */
                                fp[0] *= 2.0;
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x0b:  /* fpuabs */
                                fp[0] = Math.abs(fp[0]);
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x23:  /* fpuseterr */
                                transputer_fperr = 1;
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x9c:  /* fpuclearerr */
                                transputer_fperr = 0;
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x07:  /* fpur32tor64 */
                                /* Nothing to do, all process in high-precision */
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x08:  /* fpur64tor32 */
                                /* Nothing to do, all process in high-precision */
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x0e:  /* fpuchki32 */
                            case 0x0f:  /* fpuchki64 */
                            case 0x0d:  /* fpunoround */
                            case 0x0a:  /* fpuexpinc32 */
                            case 0x09:  /* fpuexpdec32 */
                                memory_dump(Iptr);
                                throw "Unhandled fpu instruction " + temp.toString(16) + " at " + Iptr.toString(16);
                                RoundMode = ROUND_NEAREST;
                                return;
                            default:
                                memory_dump(Iptr);
                                throw "Undocumented fpu instruction " + temp.toString(16) + " at " + Iptr.toString(16);
                                return;
                        }
                        break;
                    case 0xb1:  /* break */
                        /* !!! Should be same as j 0 when j0_break flag is enabled */
                        not_handled();
                        break;
                    case 0xb2:  /* clrj0break */
                        not_handled();
                        break;
                    case 0xb3:  /* setj0break */
                        not_handled();
                        break;
                    case 0xb4:  /* testj0break */
                        not_handled();
                        break;
                    case 0x17c: /* lddevid */
                        not_handled();
                        break;
                        
                        /*
                         ** Others.
                         */
                    case 0xff:  /* Exit emulator :) */
                        return;
                        
                    default:
                        memory_dump(Iptr);
                        window.alert("Instruction 0x" + Oreg.toString(16) + " undocumented at " + Iptr.toString(16) + " (loop " + loop + ")");
                        return;
                }
                Oreg = 0;
                break;
        }
    } while (++loop < 100000) ;
};
