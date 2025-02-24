## Transputer T805 emulator
### (also including assembler, Pascal compiler, Small-C compiler and OS)
#### by Oscar Toledo G. https://nanochess.org/

Once upon a time when I was a teen (1993), I wrote an almost full Pascal compiler for a transputer processor (the FILE type wasn't never completed)

It was the convergence of several things I had been learning that year: Pascal (The BYTE Book of Pascal), code generation (Compilers: Principles, Techniques and Tools), and transputer programming.

It was a time when the INMOS transputer promised parallel computing for everyone, but it was too expensive. They did a few good things, like a very fast 32-bit T805 transputer with 64-bit floating-point before the Intel 486DX2 was a thing.

In case you want to read the complete article: [https://nanochess.org/pascal.html](https://nanochess.org/pascal.html)

I've added also my early operating system complete with simple command-line interface, text editor, C compiler, and assembler: [https://nanochess.org/bootstrapping_c_os_transputer.html](https://nanochess.org/bootstrapping_c_os_transputer.html)

### What we have here

In order for you to experience my Pascal compiler, I needed to write two tools in modern C. Namely, the emulator for the transputer from the ground up, and port the assembler that ran on my Z280 host machine (the transputer was a board for the Z280 computer)

And nope, it wasn't a standard INMOS board, it was a board specifically designed for the Z280 computer.

The emulator supports _only_ the instructions used by my bootstrap code, my Pascal compiler, and a Ray Tracer program I ported from C to Pascal. Anyway this is a fairly big set of instructions, as floating-point is supported, but there's no support to compile the emulator on big-endian architectures.

The assembler on the other side is based on more modern code used for my later C compiler for transputer (I'll write also something about this compiler), and supports the full instruction set for an Inmos T805 transputer.

Compilation instructions:

    cc tem.c -o tem
    
    cc tasm.c -o tasm

### Pascal compiler

The Pascal compiler follows the Niklaus Wirth's 1971 specification, and it is composed of the following files:

    pascal/variable.pas
    pascal/errores.pas
    pascal/analexic.pas
    pascal/gencodig.pas
    pascal/expresio.pas
    pascal/sentenci.pas
    pascal/declarac.pas
    pascal/principa.pas
    
A transputer executable is provided so you can compile programs immediately:

    pascal/pascal.cmg
    
An older version of the executable is provided for historical purposes (it has a bug handling NIL):

    pascal/pascal0.cmg
    
Two example programs are provided:

    pascal/Hanoi.pas       Hanoi tower solver (based on a book, but I forgot which one)
    pascal/animales.pas    The animals question game.

To compile a Pascal program use this (also in *compile.sh*):

    ./tem pascal/pascal.cmg animales.pas >animales.len
    ./tasm animales.len animales.cmg library.len
    
To execute the compiled result:

    ./tem animales.cmg
    
Also there is longer command-line for compiling the Pascal compiler. Because it is somewhat cumbersome, I put it on *make_compiler.sh*

The file *library.len* contains the support library for the Pascal compiler.

The len extension means Listado ENsamblador (Assembler listing), while the CMG extension means (Codigo Maquina G10, or G10 machine code, where G10 was the name given to the transputer board)
 
 
## Ray tracer

Once my Pascal compiler was working, I ported the Ray Tracer from the book "Programming in 3 Dimensions: 3-D Graphics, Ray Tracing, and Animation" by Watkins & Sharp.

You can compile it doing this:

    ./tem pascal/pascal.cmg pascal/m3d.pas >m3d.len
    ./tasm m3d.len m3d.cmg library.len
    
To execute it:

    ./tem m3d.cmg m3d/BOLACRIS.M3D
    
Although originally the image was displayed directly on the screen using a different "driver" program, I considered the complications of adding the libSDL library to handle display weren't worth it, and instead I've adapted the code as necessary to avoid making yet another emulator executable, so a BMP image file named image001.bmp will appear on your directory.

I did a few demos and animations. I still haven't found the animations.

You can also find a Julia demo as pascal/julia.pas ported from the same book.

![image](README.png)

## Small-C compiler

The Small-C compiler is based on the Ron Cain's public domain Small-C compiler. I've ported it to transputer, and made a very much enhanced version that generates pretty small code using my tree generator evaluator (the compiler sizes up at 16 kb of code).

To execute it:

    ./tem -cc os/tc.cmg
    
The first two questions can be answered N (stop on errors and show C language source code). It will then ask for the input file name, and the output file name.

The resulting assembly file can be passed through tasm, and added STDIO2.LEN for executing it this way, or STDIO3.LEN for executing it inside the operating system SOM32.

## Operating system

This is my early version of first operating system (Jun/1995). It is composed of several files:

    os/arranque.len    Boot sector.
    os/editor.c        Visual text editor for running it inside the OS.
    os/ensg10.c        The transputer assembler for running it inside the OS.
    os/interfaz.c      The command-line interpreter for the OS.
    os/monitor.c       Debugging monitor.
    os/som32.c         The operating system (SOM32 stands for Sistema Operativo Mexicano 32 bits)
    os/tc.c            The Small-C compiler.
    os/tc2.c           The Small-C compiler with optimized code generator.
    os/mensajes.len    Library for assembling som32.c
    os/stdio.len       Library for the tc.c compiler (running in host)
    os/stdio2.len      Library for the tc2.c compiler (running in host)
    os/stdio3.len      Library for the tc2.c compiler (running inside the OS)
    os/buildboot.c     Program to build a 1.44 mb disk image file.

To run the operating system (using the prebuilt disk image):

    ./tem -os os/maestro.cmg os/disk.img
    
The disk image is built with os/build_disk.sh

Each compiled C file generates a LEN file. There are so many LEN files, that I've provided os/assemble_os.sh for assembling all in one pass.

It requires the host system to provide an ANSI escape terminal, because it refreshes it like a text framebuffer. It works just fine in macOS (including mapping the function and arrows keys for the visual text editor), I haven't tested Windows nor Linux.

This environment is pretty powerful, as I evolved the operating system starting from this. I'll talk later more about this.


## Further notes

The original programs are under pascal/original because I translated Animales.pas to English. I intend to translate also the compiler error messages, but in the meanwhile it isn't urgent.

The tasm (transputer assembler) program is still in Spanish. It should be translated to English.

I'm afraid the whole of the Pascal files are commented in Spanish and even the variable names are Spanish. But given it is a ton of code, I preferred to leave it as such.
