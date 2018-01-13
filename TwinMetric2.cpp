// This file is public domain software (PDS).
#include <ft2build.h>
#include FT_FREETYPE_H

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

int main(void)
{
    const char *text = "This is a sample text.";

    g_hDC = CreateCompatibleDC(NULL);
    FT_Init_FreeType(&g_library);

    TEXTMETRIC tm1, tm2;

    FILE *fp = fopen("metrics.csv", "w");
    fprintf(fp, "i,tmHeight,tmAscent,tmDescent,tmInternalLeading,tmExternalLeading,units_per_EM,ascender,descender,height\n");

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

        for (int i = -120; i < 120; ++i)
        {
            if (abs(i) < 4)
                continue;

            TestWin(g_pairs[k].name, text, i, tm1);

            FT_Face face = NULL;
            FT_Error err = FT_New_Face(g_library, szPath, 0, &face);

            fprintf(fp, "%d,%ld,%ld,%ld,%ld,%ld,%d,%d,%d,%d\n",
                i, tm1.tmHeight, tm1.tmAscent, tm1.tmDescent, tm1.tmInternalLeading, tm1.tmExternalLeading,
                face->units_per_EM, face->ascender, face->descender, face->height);

            FT_Done_Face(face);

        }
    }

    fclose(fp);

    DeleteDC(g_hDC);
    FT_Done_FreeType(g_library);

    return 0;
}
