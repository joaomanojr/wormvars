# wormvars

  Wormvars is allows smart use of Flash sectors to hold non-volatile variables on small microcontrolers systems.

  Basically what it does is to control a set of Flash sectors allowing user to save variables setting its ID - a name composed of 16 bits and an extension composed of 3 bits. If a request to save a new instance with same name and extension occurs the older one is safely replaced - new value is saved and then older one is deemed invalid.

  As times passes and new data is saved sectors become having less useful variables and so a garbage collector like thread moves valid data into another sector and erases original one, implementing a circular buffer approach. As threads are somewhat heavy for smaller systems as of today code uses Alan Dunkel's Protothreads (http://dunkels.com/adam/pt/). Code from protothreads v1.4 is just uncompressed on root dir under pt-1.4 subdir.
  
  On boottime all Flash sectors are accounted so an index is built allowing fast access to variables only when information is actually needed saving RAM space.
  
# Further work:

- Actually populate tests showing a functional structure.
  - must use protothreads infrastructure.
  - mock actual HW accesses, fix static sources included on include dir
- Fix tab identation and camelCase vars on wormvars.c
- Check wrote info reading again before invalidate former fd. (May be implemented on upper levels but still
  dangerous since it is up to RAM to keep info if write fails...). Better to create a wrapper to check newer info prior
  to make older invalid.
- Give up after a while (time TBD) on writing flash sectors.
- Multi-block variable support.
- (???) Locked sector concept: contains vital info (environment variables) and should be unlocked prior to relocation.
