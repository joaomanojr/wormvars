# wormvars

  Wormvars is allows smart use of Flash sectors to hold non-volatile variables on small microcontrolers systems.

  Basically what it does is to control a set of Flash sectors allowing user to save variables setting its ID - a name composed of 16 bits and an extension composed of 8 bits. If a request to save a new instance with same name and extension occurs the older one is safely replaced - new value is saved and then older one is deemed invalid.

  As times passes and new data is saved sectors become having less useful variables and so a garbage collector like thread moves valid data into another sector and erases original one, implementing a circular buffer approach. As threads are somewhat heavy for smaller systems as of today code uses Alan Dunkel's Protothreads (http://dunkels.com/adam/pt/). Code from protothreads v1.4 is just uncompressed on root dir under pt-1.4 subdir.
  
  On boottime all Flash sectors are accounted so an index is built allowing fast access to variables only when information is actually needed saving RAM space.
