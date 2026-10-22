Welcome to mfind!

This document is mfind’s user manual. It explains how to compile and run mfind, how to find oscillators, mfind’s history, and how it works.
## Downloading mfind

This manual assumes you have mfind installed. If not, you can install mfind by running this command:

`git clone https://github.com/HavenMDA/mfind.git`

wherever you want mfind to be installed.

### Updating mfind

If you want to update mfind to the most recent version, enter mfind’s directory by running

`cd <path to mfind>`

and run

`git pull`

##  Compiling mfind

### Precompilation

Before compiling mfind, open `mfind.c` in a text editor and find the line

`results = fopen("/path/to/your/results/file", "w");`

near the bottom of the file.

Change “/path/to/your/results/file” to the location where you want oscillators to be stored.

### Compilaton

To run mfind, first you need to compile it. Run

`gcc -O3 -march=native -mtune=native -o mfind mfind.c`

to compile it or

`clang -O3 -march=native -mtune=native -o mfind mfind.c`

if the previous command fails.

## Running mfind

Assuming you’ve compiled mfind, the next step is to run it.

To run mfind, enter mfind’s directory (if you have left it) and run

`./mfind`

The output should be

```
Usage: ./mfind <rule> <minpop> <period>
       ./mfind <minpop> <period>
```

### Conway’s Game of Life

If you want to search for oscillators in Conway’s Game of Life (B3/S23), run

`./mfind <minpop> <period>`

where `<minpop>` and `<period>` are the minimum population and period of the oscillators you want to find.

#### Example

To find the blinker (a 3-cell period-2 oscillator) in Conway’s Game of Life, run:

`./mfind 3 2`

and find the text file you defined in **Precompilation**.

Upon opening the file, you should see:

```
x = 0, y = 0, rule = B3/S23
bbbbbbb$
bbbbbbb$
bbbbbbb$
bbbbbbb$
bbooobb$
bbbbbbb$
bbbbbbb$
```

which is the RLE (Run Length Encoding) of a blinker.

Pasting the RLE into Golly will reveal a blinker, which is what we wanted to find.

### OCA (Other Cellular Automata)

If you want to search for oscillators in OCA, repeat the procedure in **Conway’s Game of Life**, except with a specific rule (in B/S notation) in mfind’s input:

`mfind B34/S34 4 2`

will attempt to find all 4-cell period-2 oscillators in B34/S34 (3-4 Life).

To see the oscillators you found, paste the RLE into Golly, as before.

## mfind’s history

I started developing mfind around April 15th, 2026. I originally intended to use it to generate pattern databases for my Conway’s Game of Life website, but I never finished a fully-working version until October 2026.

Unlike the early 2026 versions, which were written in Python, the latest version is written in C, a language I had started learning to write a faster version of mfind.

The C version also has several performance improvements that I didn’t bother implementing when I wrote mfind in Python.

## How it works

Note: In subsequent paragraphs, `m` will refer to the search’s population and `p` to the search’s period.

mfind treats oscillators as two-dimensional grids of cells, with a diagonal slice running down the middle, dividing the grid in half. The main grid’s dimensions are `m`-by-`m`, but there is also a layer of padding around the main grid, `p` cells thick.

For example, an `m = 3, p = 2` search would look like this:

```
P P P P P P P
P P P P P P P
P P   0 0 P P
P P     0 P P
P P       P P
P P P P P P P
P P P P P P P
```

The `P` cells are dead padding cells, the ` ` cells are variable cells, and the `0` cells represent unused areas of the grid.

The program searches by dividing the variable area into diagonal strips that go from the bottom-left to the top-right. As a diagram:

```
1
2 3
3 4 5
```

The numbers used represent the layers. The two `3` cells are a single layer.

To search this area, the program starts by checking which layer configurations can be extended as partials, extending them if they can.

Once the initial partial can no longer be extended, the program increments the layer-based representation, taking the variable grid limits into account. For example, the stack

`0 0 0 0 1`

would be incremented as

`0 0 0 1 0`

rather than

`0 0 0 0 2`

This is because the equivalent grid for `0 0 0 0 2` would have a living cell in its padding area.

Once all possible branches have been exhausted, mfind returns, outputting the number of results it has found in that search.

### Caveats

Though this enumeration style is exhaustive for most rules, it can occasionally skip oscillators in other rules. For example, in “Just Friends” (B2-a/S12), the following 8-cell period-2 oscillator gets skipped because it cannot fit in the current search space:

```
 0   
 0 00
  A  
  A  
00 0 
   0
```

Search space (excluding padding):

```
 0000000
  000000
   00000
    0000
     000
      00
       0

```
