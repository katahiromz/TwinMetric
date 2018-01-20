// This file is public domain software (PDS).
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#include <windows.h>
#include <stdio.h>
#include <assert.h>


struct NAME_AND_FILE
{
    const char *name;
    const char *file;
};

NAME_AND_FILE g_pairs[] =
{
    { "MS Gothic", "msgothic.ttc" }, 
    { "FreeMono", "FreeMono.ttf" },
    { "DejaVu Serif", "DejaVuSerif.ttf" },
    { "DejaVu Sans", "DejaVuSans.ttf" },
    { "Ubuntu Mono", "UbuntuMono-R.ttf" },
};
size_t g_pair_count = sizeof(g_pairs) / sizeof(g_pairs[0]);

HDC g_hDC;
FT_Library g_library;

SIZE TestWin(const char *font_name, const char *text, LONG nHeight, TEXTMETRIC& tm)
{
    SIZE siz = { 0, 0 };
    LOGFONTA lf;

    ZeroMemory(&lf, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = nHeight;
    lstrcpyA(lf.lfFaceName, font_name);
    HFONT hFont = CreateFontIndirectA(&lf);

    HGDIOBJ hFontOld = SelectObject(g_hDC, hFont);
    {
        GetTextExtentPoint32A(g_hDC, text, lstrlenA(text), &siz);

        GetTextMetrics(g_hDC, &tm);
    }
    SelectObject(g_hDC, hFontOld);
    DeleteObject(hFont);

    assert(siz.cy == tm.tmHeight);

    return siz;
}

SIZE TestFT(FT_Face face, const char *text, INT nHeight, TEXTMETRIC& tm)
{
    SIZE siz = { 0, 0 };

    // internal leading in FUnits
    FT_Long internal_leading = (face->ascender - face->descender) - face->units_per_EM;
    printf("%ld\n", internal_leading);

    LONG lfHeight = nHeight;
    INT n64thPoints = labs(lfHeight) * 72 * 64 / GetDeviceCaps(g_hDC, LOGPIXELSY);

    FT_Size_RequestRec req;
    //req.type = (lfHeight < 0) ? FT_SIZE_REQUEST_TYPE_NOMINAL : FT_SIZE_REQUEST_TYPE_REAL_DIM;
    req.type = FT_SIZE_REQUEST_TYPE_SCALES;
    req.width = 0;
    req.height = 1 << 16;
    req.horiResolution = 96;
    req.vertResolution = 96;
    bool flag = false;
    for (;;)
    {
        FT_Request_Size(face, &req);

        LONG pixel_height = MulDiv(face->height - internal_leading, face->size->metrics.y_ppem, face->units_per_EM);

        if (lfHeight < 0)
        {
            if (pixel_height > labs(lfHeight))
            {
                if (!flag)
                {
                    req.height -= 1 << 16;
                    flag = true;
                }
                else
                {
                    for (;;)
                    {
                        FT_Request_Size(face, &req);
                        pixel_height = MulDiv(face->height - internal_leading, face->size->metrics.y_ppem, face->units_per_EM);
                        if (pixel_height <= labs(lfHeight))
                            break;
                        req.height -= 1;
                    }
                    printf("*req.height: %d, pixel_height: %d\n", req.height, pixel_height);
                    break;
                }
            }
        }
        else
        {
            LONG pixel_height = face->height * face->size->metrics.y_ppem / face->units_per_EM;
            if (pixel_height > nHeight)
            {
                --req.height;
                printf("req.height: %d\n", req.height);
                break;
            }
        }

        if (flag)
            req.height += 1 << 6;
        else
            req.height += 1 << 16;
    }

    FT_GlyphSlot slot = face->glyph;
    assert(slot);
    FT_Bool useKerning = FT_HAS_KERNING(face);
    FT_UInt previous = 0;
    for (size_t i = 0; text[i]; ++i)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(face, text[i]);

        if (useKerning && previous && glyph_index)
        {
            FT_Vector delta;
            FT_Get_Kerning(face, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
            siz.cx += (delta.x + 32) >> 6;
        }

        FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);

        siz.cx += (slot->advance.x + 32) >> 6;

        previous = glyph_index;
    }

    //siz.cx >>= 6;
    siz.cy = face->size->metrics.height;
    siz.cy >>= 6;

    tm.tmHeight = (face->size->metrics.height) >> 6;
    tm.tmAscent = (face->size->metrics.ascender) >> 6;
    tm.tmDescent = tm.tmHeight - tm.tmAscent;
    tm.tmInternalLeading = (face->size->metrics.height >> 6) - face->size->metrics.y_ppem;
    tm.tmExternalLeading = 0;
    {
        FT_UInt glyph_index = FT_Get_Char_Index(face, 'x');
        FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        tm.tmAveCharWidth = slot->advance.x >> 6;
    }

    return siz;
}

int main(void)
{
    const char *text = "This is a sample text.";

    g_hDC = CreateCompatibleDC(NULL);
    FT_Init_FreeType(&g_library);

    TEXTMETRIC tm1, tm2;

    FILE *fp = fopen("metrics-minus.csv", "w");
    fprintf(fp, "i,tmHeight,tmAscent,tmDescent,tmInternalLeading,tmExternalLeading,units_per_EM,ascender,descender,height,usWinAscent,usWinDescent\n");

    for (size_t k = 0; k < g_pair_count; ++k)
    {
        char szPath[MAX_PATH];
        GetWindowsDirectoryA(szPath, MAX_PATH);
        lstrcatA(szPath, "\\Fonts\\");
        lstrcatA(szPath, g_pairs[k].file);
        if (GetFileAttributesA(szPath) == 0xFFFFFFFF)
        {
            printf("%s: skipped\n", g_pairs[k].file);
            continue;
        }

        for (int i = -500; i <= 0; ++i)
        {
            if (abs(i) < 4)
                continue;

            TestWin(g_pairs[k].name, text, i, tm1);

            FT_Face face = NULL;
            FT_Error err = FT_New_Face(g_library, szPath, 0, &face);

            TestFT(face, text, tm1.tmHeight, tm2);

            TT_OS2 *pOS2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);

            fprintf(fp, "%d,%ld,%ld,%ld,%ld,%ld,%d,%d,%d,%d,%d,%d\n",
                i, tm1.tmHeight, tm1.tmAscent, tm1.tmDescent, tm1.tmInternalLeading, tm1.tmExternalLeading,
                face->units_per_EM, face->ascender, face->descender, face->height, pOS2->usWinAscent, pOS2->usWinDescent);

            FT_Done_Face(face);
        }
    }

    fclose(fp);

    DeleteDC(g_hDC);
    FT_Done_FreeType(g_library);

    return 0;
}
