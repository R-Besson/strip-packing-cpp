# Strip Packing

This repository contains implementations for the strip packing problem that can be found here: https://en.wikipedia.org/wiki/Strip_packing_problem

The strip packing problem attempts to optimize the placing of rectangles in a strip of fixed width and variable height, such that the overall height of the strip is the smallest possible.

In this algorithm, I use an ingenious method of placing rectangles inside of Holes.
The first hole starts as the entirety of the strip (with a Height that is negligeable but higher than the height of any rectangle & the theoretical min height of the strip).
As we place the rectangles inside the holes, we break the antecedent hole into new holes using 15 different positional cases, check for overlap, etc ...
We can choose to rotate rectangles efficiently, or to not do so. We can specify the width of the canvas (strip), specify the sorting strategies, etc.

<br><h3>Compilation</h3>

```
g++ packer.cpp Draw\ez-draw.cpp -O3 -o packer.exe -lgdi32
```

```
packer.exe list.txt ???? ? ?
```

<br>
<h3>Complexity</h3>
<kbd><img src="https://drive.google.com/uc?id=1mji6LxmmXzAA6rv9C1sTSRNm7m6cAUd1"></kbd>
