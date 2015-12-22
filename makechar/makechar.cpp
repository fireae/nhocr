/*--------------------------------------------------------------
  Make character images     makevec  v1.1
  (C) 2009-2013  Hideaki Goto  (see accompanying LICENSE file)
    Written by H.Goto,  July 2009
    Written by H.Goto,  Oct. 2009
    Written by H.Goto,  Feb. 2010
    Written by H.Goto,  Aug. 2013
--------------------------------------------------------------*/

#define		IMGSIZE		128
#define		CHARSIZE	128
#define		CHARSIZE_Norm	56

#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#include	<ft2build.h>
#include	FT_FREETYPE_H

#define WIDTH   IMGSIZE
#define HEIGHT  IMGSIZE

#include	"siplib.h"
#include	"codelist.h"

static int	sw_verbose;
static int	sw_help;

/* origin is on the upper left corner */
unsigned char image[HEIGHT][WIDTH];




/*----------------------------------------------
  UTF-8 string to UTF-16 array conversion
----------------------------------------------*/

static int utf8_to_utf16(char *utf8str, int *utf16array){
	uchar	*p = (uchar*)utf8str;
	int	n = 0;
	int	len;
	uint	C;
	int	rcode = 0;
	while ( *p != 0 ){
		len = 1;
		C = (uint)*p;
		if ( (C & 0x80) != 0 ){
			if ( (C & 0xe0) == 0xc0 ){
				len = 2; C &= 0x1f; }
			else if ( (C & 0xf0) == 0xe0 ){
				len = 3; C &= 0x0f; }
			else if ( (C & 0xf8) == 0xf0 ){
				len = 4; C &= 0x07; }
			else{	rcode = -1;
				break;
			}
		}
		for ( len-- ; len > 0 ; len-- ){
			p++;
			if ( (*p & 0xc0) != 0x80 ){
				rcode = -1;
				break;
			}
			C <<= 6;
			C |= ((uint)*p & 0x3f);
		}
		if ( rcode )  break;
		utf16array[n++] = C;
		p++;
	}
	utf16array[n] = 0;
	if ( rcode )  return(rcode);
	return(n);
}




/*----------------------------------------------
  Character image rendering
----------------------------------------------*/

static void draw_bitmap( FT_Bitmap*  bitmap, FT_Int x, FT_Int y){
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;


	for ( i = x, p = 0; i < x_max; i++, p++ ){
		for ( j = y, q = 0; j < y_max; j++, q++ ){
			if ( i >= WIDTH || j >= HEIGHT )
			continue;

			image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}
}


static int pixelcount(){
	int	x,y;
	int	count = 0;
	for ( y=0 ; y<HEIGHT ; y++ ){
		for ( x=0 ; x<WIDTH ; x++ ){
			if ( image[y][x] )  count++;
		}
	}
	return(count);
}


static int show_image(FILE *fp){
	int	i;
	uchar	buf[WIDTH];

// for test
//	printf("P4\n%d %d\n",WIDTH,HEIGHT);
//	printf("P5\n%d %d\n255\n",WIDTH,HEIGHT);
	for ( i=0 ; i<HEIGHT ; i++ ){
		sip_cvt8to1u((uchar *)image[i], buf, 0, WIDTH, 127);
		if ( 1 != fwrite((void *)buf, (WIDTH + 7)/8, 1, fp) ){
			return(-1);
		}
/*
		for ( int j=0 ; j<WIDTH ; j++ ){
			putchar( image[i][j] < 128 ? 0xff : 0 );
		}
*/
	}
	return(0);
}


int render_character(FT_Face face, char *text){
	FT_GlyphSlot	slot;
//	FT_UInt		glyph_index;
	FT_Vector	pen;
	FT_Error	error;
	int	*text_int;

	int	target_height;
	int	n, num_chars;

	target_height = HEIGHT;

	if ( 1 > (num_chars = strlen( text )) )  return(0);
	if ( 0 == (text_int = new int[num_chars]) ){
		return(-1);
	}
	if ( 0 > (num_chars = utf8_to_utf16(text, text_int)) ){
		delete []text_int;
		return(-1);
	}

	/* use 50pt at 100dpi */
	FT_F26Dot6	cs = (FT_F26Dot6)(CHARSIZE_Norm * (64 * 72.27 / 100));
	error = FT_Set_Char_Size( face, cs, cs, 100, 100 );	/* set character size */
	/* error handling omitted */

	slot = face->glyph;

	pen.x = 8 * 64;
	pen.y = ( target_height /4 ) * 64;

	for ( int y=0 ; y<HEIGHT ; y++ ){
		for ( int x=0 ; x<WIDTH ; x++ ){
			image[y][x] = 0;
		}
	}

	for ( n = 0; n < num_chars; n++ ){
		/* set transformation */
		FT_Set_Transform( face, 0, &pen );

		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Char( face, text_int[n], FT_LOAD_RENDER );
//		error = FT_Load_Char( face, 0x3042, FT_LOAD_RENDER );
		if ( error ) continue;	/* ignore errors */

		/* now, draw to our target surface (convert position) */
		draw_bitmap( &slot->bitmap, slot->bitmap_left, \
			target_height - slot->bitmap_top );

		/* increment pen position */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
		if ( pen.x > 64 * WIDTH ){
			delete []text_int;
			return(1);
		}
	}

	delete []text_int;
	return(0);
}





/*----------------------------------------------
  Main routine
----------------------------------------------*/

int main(int ac, char **av){
	char	*infile = NULL;
	char	*outfile = NULL;
	char	*fontfile = NULL;
	FILE	*fp_out;
	int	k;
	int	rcode;
	int	sw_err = 0;
	CharCode	*cclist;
	int	cid, charcount;

	FT_Library	ftlibrary;
	FT_Face		face;

	for ( k=1 ; k<ac ; k++ ){
		if ( 0 == strcmp(av[k],"-h") ){  sw_help = 1;  continue; }
		if ( 0 == strcmp(av[k],"-v") ){  sw_verbose = 1;  continue; }
		if ( 0 == strcmp(av[k],"-font") ){
			if ( ++k >= ac ){  sw_err = 1;  break; }
			fontfile = av[k];  continue;
		}
		if ( infile == NULL ){  infile = av[k]; continue; }
		else{	outfile = av[k]; continue; }
		sw_err = 1;  break;
	}

	if ( sw_err || sw_help \
	  || infile == NULL || outfile == NULL || fontfile == NULL ){
		fputs("Make character images\n",stderr);
		fputs("makechar v1.1  Copyright (C) 2009,2010 Hideaki Goto\n",stderr);
		fputs("usage: makechar [options] in_cctable out_charfile\n",stderr);
		fputs("         -font file : set TrueType font file\n",stderr);
		fputs("         -v         : verbose mode\n",stderr);
		return(1);
	}

	if ( 0 > (charcount = load_codelist(infile, &cclist, 0)) ){
		// Note: The first code is always SPACE.
		fprintf(stderr, "Can't load character code list \"%s\".\n", infile);
		return(2);
	}

	// Initialize FreeType
	if ( 0 != FT_Init_FreeType( &ftlibrary ) ){
		fputs("FreeType initialization failed.\n",stderr);
		return(3);
	}
	if ( 0 != FT_New_Face(ftlibrary, fontfile, 0, &face ) ){
		fprintf(stderr, "Failed in loading TrueType font file \"%s\".\n",fontfile);
		FT_Done_FreeType(ftlibrary);
		return(3);
	}

	if ( NULL == (fp_out = fopen(outfile,"wb")) ){
		fprintf(stderr,"Failed in opening file \"%s\".\n",outfile);
		return(3);
	}

	for ( cid=0 ; cid < charcount -1 ; cid++ ){
		// Note: The first code is always SPACE.
		if ( sw_verbose ){
			fprintf(stderr,"%06d: %s\n", cid, cclist[cid+1].ccode_dic);
		}

		if ( 0 > (rcode = render_character(face, cclist[cid+1].ccode_dic)) ){
			fprintf(stderr,"Character %d rendering failed.\n",cid);
			break;
		}
		else if ( rcode == 1 ){
			fprintf(stderr,"Warning: Width overflow at character %d.\n",cid);
		}

		if ( pixelcount() == 0 ){
			fprintf(stderr,"Warning: Character %d is blank.\n",cid);
		}
		if ( show_image(fp_out) ){
			fprintf(stderr,"Write error on \"%s\".\n",outfile);
			fclose(fp_out);
			remove(outfile);
			return(3);
		}
	}

	if ( fclose(fp_out) ){
		fprintf(stderr,"Write error on \"%s\".\n",outfile);
		remove(outfile);
		return(3);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ftlibrary);
	return(0);
}


