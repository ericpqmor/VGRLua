# Vector-Graphics-Renderer
This is the last assignment implementation code for IMPA's 2D Computer Graphics course. It was made using Shortcut Tree as an acceleration datastructure.

The driver code was made by Diego Nehab (IMPA).

The png.lua (Implementation itself) was made by Eric Moreira, Joaquin del Priore and Leonardo Ferreira.

##Winding logic:

1- Starts with father's winding;
2- Winding increment each child cell using father's segments (Both normal and shortcut);
3- Subdivide segments to their respective child cells;
4- Create the child cells' shortcut
5 - Sample
