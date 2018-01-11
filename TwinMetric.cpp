// This file is public domain software (PDS).
#include <ft2build.h>
#include FT_FREETYPE_H

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#define FONT_NAME "MS Gothic"
#define FONT_FILE "msgothic.ttc"

SIZE TestWin(const char *text, INT nPointSize)
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
        }
        SelectObject(hDC, hFontOld);
        DeleteObject(hFont);
    }
    DeleteDC(hDC);

    return siz;
}

SIZE TestFT(const char *text, INT nPointSize)
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

    FT_Set_Char_Size(face, 0, nPointSize * 64, 96, 96);

    FT_GlyphSlot slot = face->glyph;
    assert(slot);
    for (size_t i = 0; text[i]; ++i)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(face, text[i]);
        FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);

        siz.cx += slot->advance.x;
    }

    siz.cx >>= 6;
    siz.cy = face->size->metrics.height >> 6;

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return siz;
}

int main(void)
{
    const char *text = "This is a sample text.";

    SIZE sizWin = TestWin(text, 200);
    printf("sizWin: %ld, %ld\n", sizWin.cx, sizWin.cy);

    SIZE sizFT = TestFT(text, 200);
    printf("sizFT: %ld, %ld\n", sizFT.cx, sizFT.cy);

    assert(sizWin.cx == sizFT.cx);
    assert(sizWin.cy == sizFT.cy);

    return 0;
}
