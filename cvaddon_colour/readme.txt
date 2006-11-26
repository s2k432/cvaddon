Colour histogram and backproject filters using OpenCV, using HSV, normalized RGB, etc...
Fast colour conversion functions are also included.

Notes:
---
OpenCV's histogram implementation is very fast (~2ms for 2D 640x480 image plane input)
- it uses LUT for indices (n*256 in size, where n is the number of histogram dimensions)
- I can almost beat this with my own LUT, at around 2.3ms
- Using LUT also means that one can have non-linear bin sizes, which OpenCV supports