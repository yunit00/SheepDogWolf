#ifndef SDW_GLOBAL_VIEWS_H
#define SDW_GLOBAL_VIEWS_H

struct CamScriptState {
    Vec3s eye;          /* g_camScriptEye */
    Vec3s rot;          /* g_camScriptRot */
    s16 focal;          /* g_camScriptFocal */
    Vec3s prevEye;      /* g_camScriptPrevEye */
    u8 returnMode;      /* g_camScriptReturnMode */
    Vec3s prevRot;      /* g_camScriptPrevRot */
    s16 prevFocal;      /* g_camScriptPrevFocal */
    Vec3s preEye;       /* g_camPreScriptEye */
    s16 preFocal;       /* g_camPreScriptFocal */
    Vec3s preRot;       /* g_camPreScriptRot */
};

struct TextPort {
    u32 *layer;                 /* g_textLayer: the 2D layer handle Draw2D_LayerToZ turns into a depth */
    s16 winX, winY, winW, winH; /* g_textWinX/Y/W/H */
    s16 cursorX, cursorY;       /* g_textCursorX/Y */
    s16 clipOffX, clipOffY;     /* g_textClipOffX/Y */
    u32 noClip;                 /* g_textNoClip */
};

struct TextScroll {
    char *begin;    /* g_scrollTextBegin: first page marker of the text being scrolled */
    char *source;   /* g_scrollTextSource: the text ScrollText_Run was last given */
    char *page;     /* g_scrollTextPage: start of the page on screen */
    char *nextPage; /* g_scrollTextNextPage: where the page on screen ends */
    union {
        u32 flags;                   /* g_scrollTextFlags */
        ScrollTextFlagBits flagBits; /* the same word as ScrollTextFlagBits */
    };
    s32 startMs;   /* g_scrollTextStartMs: g_rawTimeMs when the text started; written only */
    s32 pageMs;    /* g_scrollTextPageMs: display time of the page, ms; 0 = not parsed yet */
    s32 remainMs;  /* g_scrollTextRemainMs: display time left, ms */
    s32 inputTime; /* g_scrollTextInputTime: g_rawTime of the last scroll input (auto-repeat) */
};

#endif
