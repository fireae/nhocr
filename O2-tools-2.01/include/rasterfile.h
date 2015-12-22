/*    rasterfile.h   ---  Sun Raster File  Header Definitions     */

#ifndef rasterfile_h
#define rasterfile_h

#define	RAS_MAGIC	0x59a66a95
#define RT_OLD		0
#define RT_STANDARD	1
#define RT_BYTE_ENCODED	2
#define RT_FORMAT_RGB	3
#define RT_FORMAT_TIFF	4
#define RT_FORMAT_IFF	5
#define RT_EXPERIMENTAL 0xffff
#define RMT_RAW		2
#define RMT_NONE	0
#define RMT_EQUAL_RGB	1

struct rasterfile {
	int	ras_magic;
	int	ras_width;
	int	ras_height;
	int	ras_depth;
	int	ras_length;
	int	ras_type;
	int	ras_maptype;
	int	ras_maplength;
};

#endif	/* rasterfile_h */
