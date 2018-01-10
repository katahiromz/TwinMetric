#include <ft2build.h>

#include FT_FREETYPE_H

#include <windows.h>
#include <stdio.h>

#define FONT_NAME "DejaVu Sans"
#define FONT_FILE "DejaVuSans.ttf"

SIZE TestWin(INT nHeight)
{
    SIZE siz = { 0, 0 };
    LOGFONT lf;

    HDC hDC = CreateCompatibleDC(NULL);
    {
        ZeroMemory(&lf, sizeof(lf));
        lf.lfHeight = -MulDiv(nHeight, GetDeviceCaps(hDC, LOGPIXELSY), 72);
        lstrcpy(lf.lfFaceName, TEXT(FONT_NAME));
        HFONT hFont = CreateFontIndirect(&lf);

        HGDIOBJ hFontOld = SelectObject(hDC, hFont);
        {
            GetTextExtentPoint32A(hDC, "W", 1, &siz);
        }
        SelectObject(hDC, hFontOld);
    }
    DeleteDC(hDC);

    return siz;
}

SIZE TestFT(INT nHeight)
{
    SIZE siz = { 0, 0 };
    FT_Library library;
    FT_Init_FreeType(&library);

    char szPath[MAX_PATH];
    GetWindowsDirectoryA(szPath, MAX_PATH);
    lstrcatA(szPath, "\\Fonts\\");
    lstrcatA(szPath, FONT_FILE);

    FT_Face face;
    FT_New_Face(library, szPath, 0, &face);

    FT_Size_RequestRec req;
    req.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
    req.width = 0;
    req.height = nHeight;
    req.horiResolution = 96;
    req.vertResolution = 96;
    FT_Request_Size(face, &req);

    FT_Load_Char(face, 'W', FT_LOAD_RENDER);
    FT_GlyphSlot slot = face->glyph;

    INT cx = slot->advance.x;
    INT cy = slot->metrics.height;
    siz.cx = cx;
    siz.cy = cy * 96 / 72;

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return siz;
}

int main(void)
{
    SIZE sizWin = TestWin(100);
    SIZE sizFT = TestFT(100);
    printf("sizWin: %ld, %ld\n", sizWin.cx, sizWin.cy);
    printf("sizFT: %ld, %ld\n", sizFT.cx, sizFT.cy);
    return 0;
}
