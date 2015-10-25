/*****************************************************************************
 * freetype.c : Put text on the video, using freetype2
 *****************************************************************************
 * Copyright (C) 2002 - 2015 VLC authors and VideoLAN
 * $Id$
 *
 * Authors: Sigmund Augdal Helberg <dnumgis@videolan.org>
 *          Gildas Bazin <gbazin@videolan.org>
 *          Bernie Purcell <bitmap@videolan.org>
 *          Jean-Baptiste Kempf <jb@videolan.org>
 *          Felix Paul Kühne <fkuehne@videolan.org>
 *          Salah-Eddin Shaban <salshaaban@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#ifndef PLATFORM_FONTS_H
#define PLATFORM_FONTS_H

/** \defgroup freetype_fonts Freetype Fonts management 
 * \ingroup freetype
 * Freetype text rendering cross platform
 * @{
 * \file
 * Freetype module
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "freetype.h"

/* Default fonts */
#ifdef __APPLE__
# define SYSTEM_DEFAULT_FONT_FILE "/Library/Fonts/Arial Unicode.ttf"
# define SYSTEM_DEFAULT_FAMILY "Arial Unicode MS"
# define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "/System/Library/Fonts/Monaco.dfont"
# define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Monaco"
#elif defined( _WIN32 )
# define SYSTEM_DEFAULT_FONT_FILE "arial.ttf" /* Default path font found at run-time */
# define SYSTEM_DEFAULT_FAMILY "Arial"
# define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "cour.ttf"
# define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Courier New"
#elif defined( __OS2__ )
# define SYSTEM_DEFAULT_FONT_FILE "/psfonts/tnrwt_k.ttf"
# define SYSTEM_DEFAULT_FAMILY "Times New Roman WT K"
# define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "/psfonts/mtsansdk.ttf"
# define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Monotype Sans Duospace WT K"
#elif defined( __ANDROID__ )
# define SYSTEM_DEFAULT_FONT_FILE "/system/fonts/DroidSans-Bold.ttf"
# define SYSTEM_DEFAULT_FAMILY "Droid Sans"
# define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "/system/fonts/DroidSansMono.ttf"
# define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Monospace"
#else
# define SYSTEM_DEFAULT_FONT_FILE "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf"
# define SYSTEM_DEFAULT_FAMILY "Serif Bold"
# define SYSTEM_DEFAULT_MONOSPACE_FONT_FILE "/usr/share/fonts/truetype/freefont/FreeMono.ttf"
# define SYSTEM_DEFAULT_MONOSPACE_FAMILY "Monospace"
#endif

#ifndef DEFAULT_FONT_FILE
# define DEFAULT_FONT_FILE SYSTEM_DEFAULT_FONT_FILE
#endif

#ifndef DEFAULT_FAMILY
# define DEFAULT_FAMILY SYSTEM_DEFAULT_FAMILY
#endif

#ifndef DEFAULT_MONOSPACE_FONT_FILE
# define DEFAULT_MONOSPACE_FONT_FILE SYSTEM_DEFAULT_MONOSPACE_FONT_FILE
#endif

#ifndef DEFAULT_MONOSPACE_FAMILY
# define DEFAULT_MONOSPACE_FAMILY SYSTEM_DEFAULT_MONOSPACE_FAMILY
#endif

/**
 * Representation of the fonts (linked-list)
 */
typedef struct vlc_font_t vlc_font_t;
struct vlc_font_t
{
    vlc_font_t *p_next; /**< next font in the chain */
    char       *psz_fontfile; /**< path to the file on the disk */
    int         i_index; /**< index of the font in the font file, starts at 0 */
    bool        b_bold; /**< if the font is a bold version */
    bool        b_italic; /**< if the font is an italic version */
    FT_Face     p_face; /**< the freetype structure for the font */
};

/**
 * Representation of font families (linked-list)
 */
struct vlc_family_t
{
    vlc_family_t *p_next; /**< next family in the chain */
    char         *psz_name; /**< Human-reable name, usually requested */
    vlc_font_t   *p_fonts; /**< fonts matching this family */
};

#define FB_LIST_ATTACHMENTS "attachments"
#define FB_LIST_DEFAULT     "default"
#define FB_NAME             "fallback"

/***
 * PLATFORM SPECIFIC SELECTORS
 **/
#ifdef HAVE_FONTCONFIG
vlc_family_t *FontConfig_GetFallbacks( filter_t *p_filter, const char *psz_family,
                                       uni_char_t codepoint );
const vlc_family_t *FontConfig_GetFamily( filter_t *p_filter, const char *psz_family );
int FontConfig_Prepare( filter_t *p_filter );
#endif

#if defined( _WIN32 ) && !VLC_WINSTORE_APP
vlc_family_t *Win32_GetFallbacks( filter_t *p_filter, const char *psz_family,
                                  uni_char_t codepoint );

const vlc_family_t *Win32_GetFamily( filter_t *p_filter, const char *psz_family );
#endif /* _WIN32 */

#ifdef __APPLE__
#if !TARGET_OS_IPHONE
char* MacLegacy_Select( filter_t *p_filter, const char* psz_fontname,
                        bool b_bold, bool b_italic,
                        int *i_idx, uni_char_t codepoint );
#endif
#endif

#ifdef __ANDROID__
const vlc_family_t *Android_GetFamily( filter_t *p_filter, const char *psz_family );
vlc_family_t *Android_GetFallbacks( filter_t *p_filter, const char *psz_family,
                                    uni_char_t codepoint );
int Android_Prepare( filter_t *p_filter );
#endif

char* Dummy_Select( filter_t *p_filter, const char* family,
                    bool b_bold, bool b_italic,
                    int *i_idx, uni_char_t codepoint );

#define File_Select(a) Dummy_Select(NULL, a, 0, 0, NULL, 0)

char* Generic_Select( filter_t *p_filter, const char* family,
                      bool b_bold, bool b_italic,
                      int *i_idx, uni_char_t codepoint );


/* ******************
 * Family and fonts *
 ********************/

/**
 * Creates a new family.
 *
 * \param psz_family the usual font family name, human-readable;
 *                   if NULL, will use "fallback-xx"[IN]
 * \param pp_list the family list where to append the font;
 *                can be NULL if not in a list [IN]
 * \param p_dict dictionnary where to insert this family; can be NULL [IN]
 * \param psz_key specific key for the dictionnary [IN]
 *
 * \return the new family representation
 */
vlc_family_t *NewFamily( filter_t *p_filter, const char *psz_family,
                         vlc_family_t **pp_list, vlc_dictionary_t *p_dict,
                         const char *psz_key );

/**
 * Creates a new font.
 *
 * \param psz_fontfile font file [IN]
 * \param i_index index of the font in the font file [IN]
 * \param b_bold is a bold font or not [IN]
 * \param b_bold is an italic or not [IN]
 * \param p_parent parent family.
 *                 If not NULL, the font will be associated to this family, and
 *                 appended to the font list in that family [IN]
 *
 * \remark This function takes ownership of psz_fontfile
 * \return the new font
 */
vlc_font_t *NewFont( char *psz_fontfile, int i_index,
                     bool b_bold, bool b_italic,
                     vlc_family_t *p_parent );

/**
 * Free families and fonts associated.
 *
 * \param p_family the family to free [IN]
 */
void FreeFamiliesAndFonts( vlc_family_t *p_family );

/**
 * Free families, but not the fonts associated.
 *
 * \param p_families the families to free [IN]
 */
void FreeFamilies( void *p_families, void *p_obj );

/**
 * Construct the default family list
 *
 * In some platforms, you might want multiple fonts as default
 *
 * \param p_filter the freetype module object [IN]
 * \param ppsz_default the table default fonts [IN]
 * \param i_size the size of the supplied table [IN]
 *
 * return the default family font
 */
vlc_family_t *InitDefaultList( filter_t *p_filter, const char *const *ppsz_default,
                               int i_size );

/* Debug Helpers */
void DumpFamily( filter_t *p_filter, const vlc_family_t *p_family,
                 bool b_dump_fonts, int i_max_families );

void DumpDictionary( filter_t *p_filter, const vlc_dictionary_t *p_dict,
                     bool b_dump_fonts, int i_max_families );

/* String helpers */
char* ToLower( const char *psz_src );

int ConvertToLiveSize( filter_t *p_filter, const text_style_t *p_style );

#endif //PLATFORM_FONTS_H
