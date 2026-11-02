# libjpeg in Sheep, Dog 'n' Wolf

The game decodes its JPEG images with the Independent JPEG Group's JPEG library, **release 6 of 2-Aug-95**. The `.c` and `.h` files in this folder are IJG's release 6 files **unmodified** (the decompression half of the library), and IJG's `README` is included unaltered as their licence requires. This software is based in part on the work of the Independent JPEG Group.

## Where the files come from

`jpegsrc.v6.tar.gz`, 531,703 bytes, SHA-256 `6311cbbb24b57bc7fb68bd788a9876bb63673302fded219070b39147fb124de7`, from FUNET's archive (`ftp.funet.fi/pub/graphics/packages/jpeg/`); GWDG's copy (`ftp.gwdg.de/pub/misc/ghostscript/3rdparty/`) is byte-identical. IJG's own site keeps only 6a and later. Release 6, not 6a: the game calls `jpeg_create_decompress` with one argument (6a turned it into a macro over `jpeg_CreateDecompress(cinfo, version, size)`).

## Additions (not IJG files)

- `jconfig.h`: release 6 ships no configuration for these compilers; this one is the game's.
- `README-SDW.md`: this file.
