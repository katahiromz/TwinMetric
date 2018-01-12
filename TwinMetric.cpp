// This file is public domain software (PDS).
#include <ft2build.h>
#include FT_FREETYPE_H

#include <windows.h>
#include <stdio.h>
#include <assert.h>

//#define FONT_NAME "MS Gothic"
//#define FONT_FILE "msgothic.ttc"
#define FONT_NAME "DejaVu Sans"
#define FONT_FILE "DejaVuSans.ttf"
//#define FONT_NAME "DejaVu Serif"
//#define FONT_FILE "DejaVuSerif.ttf"
//#define FONT_NAME "FreeMono"
//#define FONT_FILE "FreeMono.ttf"

SIZE TestWin(const char *text, INT nPointSize, TEXTMETRIC& tm)
{
    SIZE siz = { 0, 0 };
    LOGFONT lf;

    HDC hDC = CreateCompatibleDC(NULL);
    {
        ZeroMemory(&lf, sizeof(lf));
        lf.lfHeight = -MulDiv(nPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
        lstrcpy(lf.lfFaceName, TEXT(FONT_NAME));
        HFONT hFont = CreateFontIndirect(&lf);

        HGDIOBJ hFontOld = SelectObject(hDC, hFont);
        {
            GetTextExtentPoint32A(hDC, text, lstrlenA(text), &siz);

            GetTextMetrics(hDC, &tm);
            printf("tmHeight: %ld\n", tm.tmHeight);
            printf("tmAscent: %ld\n", tm.tmAscent);
            printf("tmDescent: %ld\n", tm.tmDescent);
            printf("tmInternalLeading: %ld\n", tm.tmInternalLeading);
            printf("tmExternalLeading: %ld\n", tm.tmExternalLeading);
        }
        SelectObject(hDC, hFontOld);
        DeleteObject(hFont);
    }
    DeleteDC(hDC);

    return siz;
}

SIZE TestFT(const char *text, INT nPointSize, TEXTMETRIC& tm)
{
    SIZE siz = { 0, 0 };
    FT_Library library;
    FT_Init_FreeType(&library);

    char szPath[MAX_PATH];
    GetWindowsDirectoryA(szPath, MAX_PATH);
    lstrcatA(szPath, "\\Fonts\\");
    lstrcatA(szPath, FONT_FILE);

    FT_Face face = NULL;
    FT_Error err = FT_New_Face(library, szPath, 0, &face);

    LONG lfHeight = -MulDiv(nPointSize, 96, 72);
    INT nHeight = MulDiv(-lfHeight, 72, 96);

    FT_Set_Char_Size(face, 0, nHeight * 64, 96, 96);

    FT_GlyphSlot slot = face->glyph;
    assert(slot);
    for (size_t i = 0; text[i]; ++i)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(face, text[i]);
        FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);

        siz.cx += slot->advance.x;
    }

    siz.cx >>= 6;
    siz.cy = face->size->metrics.height;
    siz.cy >>= 6;

    tm.tmHeight = face->size->metrics.height >> 6;
    tm.tmAscent = face->size->metrics.ascender >> 6;
    tm.tmDescent = tm.tmHeight - tm.tmAscent;
    tm.tmInternalLeading = (face->size->metrics.height >> 6) - face->size->metrics.y_ppem;
    tm.tmExternalLeading = 0;

    printf("tmHeight: %ld\n", tm.tmHeight);
    printf("tmAscent: %ld\n", tm.tmAscent);
    printf("tmDescent: %ld\n", tm.tmDescent);
    printf("tmInternalLeading: %ld\n", tm.tmInternalLeading);
    printf("tmExternalLeading: %ld\n", tm.tmExternalLeading);

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return siz;
}

int main(void)
{
    const char *text = "This is a sample text.";

    INT nPenalty = 0;
    for (int i = 20; i < 750; i += 21)
    {
        printf("---\n");
        TEXTMETRIC tm1;
        SIZE sizWin = TestWin(text, i, tm1);
        printf("sizWin: %ld, %ld\n", sizWin.cx, sizWin.cy);

        printf("---\n");
        TEXTMETRIC tm2;
        SIZE sizFT = TestFT(text, i, tm2);
        printf("sizFT: %ld, %ld\n", sizFT.cx, sizFT.cy);

        assert(labs(tm1.tmHeight - tm2.tmHeight) <= 1);
        assert(labs(tm1.tmAscent - tm2.tmAscent) <= 1);
        assert(labs(tm1.tmDescent - tm2.tmDescent) <= 1);
        assert(labs(tm1.tmInternalLeading - tm2.tmInternalLeading) <= 1);
        assert(labs(tm1.tmExternalLeading - tm2.tmExternalLeading) <= 1);
        assert(labs(sizWin.cx - sizFT.cx) <= 1);
        assert(labs(sizWin.cy - sizFT.cy) <= 1);

        nPenalty += (tm1.tmHeight == tm2.tmHeight) * 2;
        nPenalty += tm1.tmAscent == tm2.tmAscent;
        nPenalty += tm1.tmDescent == tm2.tmDescent;
        nPenalty += tm1.tmInternalLeading == tm2.tmInternalLeading;
        nPenalty += tm1.tmExternalLeading == tm2.tmExternalLeading;
        nPenalty += (sizWin.cx == sizFT.cx) * 2;
        nPenalty += (sizWin.cy == sizFT.cy) * 2;
    }
    printf("---\n");
    printf("nPenalty: %d\n", nPenalty);

    return 0;
}
