# wormvars

  Wormvars is allows smart use of FLASH sectors to hold non-volatile variables on small microcontrolers systems.

  On boottime all Flash sectors are accounted so an index is built allowing fast access to variables only when information is actually needed saving RAM space.

  Basically what it does is to control a set of FLASH sectors allowing user to save variables setting its ID - a name composed of 16 bits and an extension composed of 3 bits. If a request to save a new instance with same name and extension occurs the older one is safely replaced - new value is saved and then older one is deemed invalid.

  As times passes and new data is saved FLASH sectors become having less useful variables - which coined 'wormed' jargon that gives this project name meaning 'fragmented' sector. So a garbage-collector like thread comes into scene moving valid variables out of wormed FLASH sector and preparing it to be erased. At the end it works as a circular buffer of FLASH sectors were variables are organized to occupy as few sector as possible. The search for a new wormed sector to relocate is therefore based on how many valid variables it contains.
  
  Threads are somewhat heavy for smaller systems. As of today this project uses Alan Dunkel's Protothreads (http://dunkels.com/adam/pt/) to circumvent this but in fact all we need is to keep relocation process going somehow in background. Code from protothreads v1.4 is just uncompressed on root dir under pt-1.4 subdir by now.
  

# Further work:

- Actually populate tests showing a functional structure.
  - must use protothreads infrastructure.
  - mock actual HW accesses as a static lib to be used on tests.
- Improve documentation.
  - add diagrams.
  - put some examples on defragmentation.
- Check wrote info reading again before invalidate former fd. (May be implemented on upper levels but still
  dangerous since it is up to RAM to keep info if write fails...). Better to create a wrapper to check newer info prior
  to make older invalid.
- Give up after a while (time TBD) on writing flash sectors.
- Multi-block variable support.
- (???) Locked sector concept: contains vital info (environment variables) and should be unlocked prior to relocation.
