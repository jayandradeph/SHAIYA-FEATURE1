#define _CRT_SECURE_NO_WARNINGS
#include <util/util.h>
#include "include/main.h"
#include <windows.h>
#include <io.h>
#include <cstdio>

struct PanelUIState {
    int offsetX;
    int offsetY;
    POINT lastMousePos{};
    bool dragging = false;
    HWND gameHwnd = nullptr;
    int baseX = 0;
    int baseY = 0;
    const wchar_t* configFile;
    const wchar_t* sectionName;
    int defaultOffsetX;
    int defaultOffsetY;
    int width;
    int height;
};

inline void createDefaultConfig(PanelUIState& ui) {
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", ui.defaultOffsetX);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetX", buf, ui.configFile);
    swprintf(buf, 32, L"%d", ui.defaultOffsetY);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetY", buf, ui.configFile);
}

inline void loadConfig(PanelUIState& ui) {
    if (_access((const char*)ui.configFile, 0) != 0) {
        createDefaultConfig(ui);
    }
    ui.offsetX = GetPrivateProfileIntW(ui.sectionName, L"OffsetX", ui.defaultOffsetX, ui.configFile);
    ui.offsetY = GetPrivateProfileIntW(ui.sectionName, L"OffsetY", ui.defaultOffsetY, ui.configFile);
    ui.gameHwnd = FindWindowW(L"GAME", nullptr);
}

inline void saveConfig(PanelUIState& ui) {
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", ui.offsetX);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetX", buf, ui.configFile);
    swprintf(buf, 32, L"%d", ui.offsetY);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetY", buf, ui.configFile);
}

constexpr int BUTTON_PANEL_WIDTH = 250;
constexpr int BUTTON_PANEL_HEIGHT = 140;
constexpr int BUTTON_DEFAULT_OFFSET_X = 200;
constexpr int BUTTON_DEFAULT_OFFSET_Y = -300;

constexpr int FEED_PANEL_WIDTH = 250;
constexpr int FEED_PANEL_HEIGHT = 140;
constexpr int FEED_DEFAULT_OFFSET_X = 600;
constexpr int FEED_DEFAULT_OFFSET_Y = -300;

constexpr int KILL_PANEL_WIDTH = 250;
constexpr int KILL_PANEL_HEIGHT = 140;
constexpr int KILL_DEFAULT_OFFSET_X = 900;
constexpr int KILL_DEFAULT_OFFSET_Y = -300;

constexpr int ONLINE_PANEL_WIDTH = 250;
constexpr int ONLINE_PANEL_HEIGHT = 140;
constexpr int ONLINE_DEFAULT_OFFSET_X = 1000;
constexpr int ONLINE_DEFAULT_OFFSET_Y = -300;

PanelUIState buttonUi{
    BUTTON_DEFAULT_OFFSET_X, BUTTON_DEFAULT_OFFSET_Y,
    {}, false, nullptr, 0, 0,
    L".\\panel.ini", L"BUTTON_UI",
    BUTTON_DEFAULT_OFFSET_X, BUTTON_DEFAULT_OFFSET_Y,
    BUTTON_PANEL_WIDTH, BUTTON_PANEL_HEIGHT
};

PanelUIState feedUi{
    FEED_DEFAULT_OFFSET_X, FEED_DEFAULT_OFFSET_Y,
    {}, false, nullptr, 0, 0,
    L".\\panel.ini", L"FEED_UI",
    FEED_DEFAULT_OFFSET_X, FEED_DEFAULT_OFFSET_Y,
    FEED_PANEL_WIDTH, FEED_PANEL_HEIGHT
};

PanelUIState killUi{
    KILL_DEFAULT_OFFSET_X, KILL_DEFAULT_OFFSET_Y,
    {}, false, nullptr, 0, 0,
    L".\\panel.ini", L"KILL_UI",
    KILL_DEFAULT_OFFSET_X, KILL_DEFAULT_OFFSET_Y,
    KILL_PANEL_WIDTH, KILL_PANEL_HEIGHT
};

PanelUIState onlineUi{
    ONLINE_DEFAULT_OFFSET_X, ONLINE_DEFAULT_OFFSET_Y,
    {}, false, nullptr, 0, 0,
    L".\\panel.ini", L"ONLINE_UI",
    ONLINE_DEFAULT_OFFSET_X, ONLINE_DEFAULT_OFFSET_Y,
    ONLINE_PANEL_WIDTH, ONLINE_PANEL_HEIGHT
};

auto ONLINE_format = "[ONLINE]";
char ON[128] = "Total: 0";
char LI[256] = "Light: 0";
char FU[256] = "Fury: 0";
char PERCENT_LIGHT[256] = "AoL: 0.00%";
char PERCENT_FURY[256] = "UoF: 0.00%";
char FIGHTER[64] = "Fighter: 0";
char DEFENDER[64] = "Defender: 0";
char RANGER[64] = "Ranger: 0";
char ARCHER[64] = "Archer: 0";
char MAGE[64] = "Mage: 0";
char PRIEST[64] = "Priest: 0";
char WARRIOR[64] = "Warrior: 0";
char GUARDIAN[64] = "Guardian: 0";
char ASSASSIN[64] = "Assassin: 0";
char HUNTER[64] = "Hunter: 0";
char PAGAN[64] = "Pagan: 0";
char ORACLE[64] = "Oracle: 0";
bool isAoLLeading2 = false;
bool isUoFLeading2 = false;
int g_lightPercentInt2 = 0;
int g_furyPercentInt2 = 0;

void updateStatusOnline(const char* val) {
    int total = 0, lightCount = 0, lightPercent = 0, furyCount = 0, furyPercent = 0;
    int fighter = 0, defender = 0, ranger = 0, archer = 0, mage = 0, priest = 0;
    int warrior = 0, guardian = 0, assassin = 0, hunter = 0, pagan = 0, oracle = 0;
    int matched = sscanf(val, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        &total, &lightCount, &lightPercent,
        &furyCount, &furyPercent,
        &fighter, &defender, &ranger, &archer, &mage, &priest,
        &warrior, &guardian, &assassin, &hunter, &pagan, &oracle);
    if (matched >= 5 && total > 0) {
        g_lightPercentInt2 = lightPercent;
        g_furyPercentInt2 = furyPercent;
        double lightPercentCalc = static_cast<double>(lightPercent);
        double furyPercentCalc = static_cast<double>(furyPercent);
        snprintf(ON, sizeof(ON), "Total: %d", total);
        snprintf(LI, sizeof(LI), "Light: %d", lightCount);
        snprintf(FU, sizeof(FU), "Fury: %d", furyCount);
        snprintf(PERCENT_LIGHT, sizeof(PERCENT_LIGHT), "AoL: %.2f%%%%", lightPercentCalc);
        snprintf(PERCENT_FURY, sizeof(PERCENT_FURY), "UoF: %.2f%%%%", furyPercentCalc);
        snprintf(FIGHTER, sizeof(FIGHTER), "Fighter: %d", fighter);
        snprintf(DEFENDER, sizeof(DEFENDER), "Defender: %d", defender);
        snprintf(RANGER, sizeof(RANGER), "Ranger: %d", ranger);
        snprintf(ARCHER, sizeof(ARCHER), "Archer: %d", archer);
        snprintf(MAGE, sizeof(MAGE), "Mage: %d", mage);
        snprintf(PRIEST, sizeof(PRIEST), "Priest: %d", priest);
        snprintf(WARRIOR, sizeof(WARRIOR), "Warrior: %d", warrior);
        snprintf(GUARDIAN, sizeof(GUARDIAN), "Guardian: %d", guardian);
        snprintf(ASSASSIN, sizeof(ASSASSIN), "Assassin: %d", assassin);
        snprintf(HUNTER, sizeof(HUNTER), "Hunter: %d", hunter);
        snprintf(PAGAN, sizeof(PAGAN), "Pagan: %d", pagan);
        snprintf(ORACLE, sizeof(ORACLE), "Oracle: %d", oracle);
        isAoLLeading2 = (g_lightPercentInt2 >= g_furyPercentInt2);
        isUoFLeading2 = !isAoLLeading2;
    }
    else {
        strcpy(ON, "Total: 0");
        strcpy(LI, "Light: 0");
        strcpy(FU, "Fury: 0");
        strcpy(PERCENT_LIGHT, "AoL: 0.00%");
        strcpy(PERCENT_FURY, "UoF: 0.00%");
        strcpy(FIGHTER, "Fighter: 0");
        strcpy(DEFENDER, "Defender: 0");
        strcpy(RANGER, "Ranger: 0");
        strcpy(ARCHER, "Archer: 0");
        strcpy(MAGE, "Mage: 0");
        strcpy(PRIEST, "Priest: 0");
        strcpy(WARRIOR, "Warrior: 0");
        strcpy(GUARDIAN, "Guardian: 0");
        strcpy(ASSASSIN, "Assassin: 0");
        strcpy(HUNTER, "Hunter: 0");
        strcpy(PAGAN, "Pagan: 0");
        strcpy(ORACLE, "Oracle: 0");
        g_lightPercentInt2 = 0;
        g_furyPercentInt2 = 0;
        isAoLLeading2 = false;
        isUoFLeading2 = false;
    }
}

auto KILL_format = "[KILL]";
char PERCENT_LIGHT_KILL[256] = "AoL: 0%";
char PERCENT_FURY_KILL[256] = "UoF: 0%";
bool isAoLLeadingKill = false;
bool isUoFLeadingKill = false;
int g_lightPercentIntKill = 0;
int g_furyPercentIntKill = 0;
void updateStatusKill(const char* val) {
    double lightPercent = 0.0;
    double furyPercent = 0.0;
    int matched = sscanf(val, "%lf %lf", &lightPercent, &furyPercent);
    if (matched == 2) {
        if (lightPercent < 0.0) lightPercent = 0.0;
        if (lightPercent > 100.0) lightPercent = 100.0;
        if (furyPercent < 0.0) furyPercent = 0.0;
        if (furyPercent > 100.0) furyPercent = 100.0;
        g_lightPercentIntKill = static_cast<int>(lightPercent + 0.5);
        g_furyPercentIntKill = static_cast<int>(furyPercent + 0.5);
        snprintf(PERCENT_LIGHT_KILL, sizeof(PERCENT_LIGHT_KILL), "AoL: %.2f%%%%", lightPercent);
        snprintf(PERCENT_FURY_KILL, sizeof(PERCENT_FURY_KILL), "UoF: %.2f%%%%", furyPercent);
        isAoLLeadingKill = (g_lightPercentIntKill >= g_furyPercentIntKill);
        isUoFLeadingKill = !isAoLLeadingKill;
    }
    else {
        strcpy(PERCENT_LIGHT_KILL, "AoL: 0%");
        strcpy(PERCENT_FURY_KILL, "UoF: 0%");
        g_lightPercentIntKill = 0;
        g_furyPercentIntKill = 0;
        isAoLLeadingKill = false;
        isUoFLeadingKill = false;
    }
}

inline constexpr int KILL_PROGRESS_WIDTH = 230;
inline constexpr int PROGRESS_WIDTH = 230;
inline void __stdcall renderProgressBarGeneric(int x, int y, int percent,
    void* barTexture, bool fromRight,
    int maxWidth) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    int width = (percent * maxWidth) / 100;
    for (int i = 0; i < width; i++) {
        int drawX = fromRight ? (x - i) : (x + i);
        int drawY = y;
        __asm {
            push drawY
            push drawX
            mov ecx, barTexture
            call render_tga
        }
    }
}

inline void renderProgressBarKill(int x, int y, int percent, void* barTexture, bool fromRight) {
    renderProgressBarGeneric(x, y, percent, barTexture, fromRight, KILL_PROGRESS_WIDTH);
}

inline void renderProgressBar(int x, int y, int percent, void* barTexture, bool fromRight) {
    renderProgressBarGeneric(x, y, percent, barTexture, fromRight, PROGRESS_WIDTH);
}

const char* AoLFrames[] = {
    loadbar_AoL, loadbar_AoL_alt1, loadbar_AoL_alt2, loadbar_AoL_alt3, loadbar_AoL_alt4, loadbar_AoL_alt5 //ANIMATION IMAGE
};

const char* UoFFrames[] = {
    loadbar_UoF, loadbar_UoF_alt1, loadbar_UoF_alt2, loadbar_UoF_alt3, loadbar_UoF_alt4, loadbar_UoF_alt5 //ANIMATION IMAGE
};

inline const char* SelectKillTexture(const char** frames, unsigned int frameCount) {
    unsigned int tick = GetTickCount();
    unsigned int speed = 120;
    unsigned int cycle = (tick / speed) % (frameCount * 2 - 2);
    unsigned int frameIndex;
    if (cycle < frameCount) {
        frameIndex = cycle;
    }
    else {
        frameIndex = (frameCount * 2 - 2) - cycle;
    }
    return frames[frameIndex];
}

DWORD aolStartTick = 0;
DWORD uofStartTick = 0;
int lastAoLPercent = -1;
int lastUoFPercent = -1;
const DWORD ANIMATION_DURATION = 5000; // 5 SECONDS TO STOP ANIMATION
const char* GetAoLTextureWithStop(int percent) {
    DWORD now = GetTickCount();
    if (percent > lastAoLPercent && percent >= g_furyPercentIntKill) {
        lastAoLPercent = percent;
        aolStartTick = now;
    }
    else {
        lastAoLPercent = percent;
    }
    if (percent > 0 && (now - aolStartTick) < ANIMATION_DURATION) {
        return SelectKillTexture(AoLFrames, _countof(AoLFrames));
    }
    return loadbar_AoL;
}

const char* GetUoFTextureWithStop(int percent) {
    DWORD now = GetTickCount();
    if (percent > lastUoFPercent && percent >= g_lightPercentIntKill) {
        lastUoFPercent = percent;
        uofStartTick = now;
    }
    else {
        lastUoFPercent = percent;
    }
    if (percent > 0 && (now - uofStartTick) < ANIMATION_DURATION) {
        return SelectKillTexture(UoFFrames, _countof(UoFFrames));
    }
    return loadbar_UoF;
}

void renderAoLBar(int x, int y, int percent) {
    const char* tex = GetAoLTextureWithStop(percent);
    renderProgressBarKill(x, y, percent, (void*)tex, false);
}

void renderUoFBar(int x, int y, int percent) {
    const char* tex = GetUoFTextureWithStop(percent);
    renderProgressBarKill(x, y, percent, (void*)tex, true);
}

auto FEED_format = "[FEED]";
char feed_text_1[128];
char feed_text_2[128];
char feed_text_3[128];
char feed_text_4[128];
char feed_text_5[128];

void shiftFeedTexts(const char* newNotice) {
    strcpy(feed_text_5, feed_text_4);
    strcpy(feed_text_4, feed_text_3);
    strcpy(feed_text_3, feed_text_2);
    strcpy(feed_text_2, feed_text_1);
    strncpy(feed_text_1, newNotice, sizeof(feed_text_1));
    feed_text_1[sizeof(feed_text_1) - 1] = '\0';
}

DWORD render_notice = 0x5E5C10;
inline void parseAndHandle(void* espBase) {
    void* arg = *(void**)((BYTE*)espBase + 0x54);

    if (arg == *(void**)KILL_format) {
        updateStatusKill((const char*)((BYTE*)espBase + 0x54 + 6));
        return;
    }
    if (arg == *(void**)ONLINE_format) {
        updateStatusOnline((const char*)((BYTE*)espBase + 0x54 + 8));
        return;
    }
    if (arg == *(void**)FEED_format) {
        shiftFeedTexts((const char*)((BYTE*)espBase + 0x54 + 6));
        return;
    }
    reinterpret_cast<void(__stdcall*)(DWORD)>(render_notice)((DWORD)arg);
}

enum PanelType {
    PANEL_HIDE,
    PANEL_FEED,
    PANEL_KILL,
    PANEL_ONLINE
};

struct TextEntry {
    int offsetX;
    int offsetY;
    const char* text;
    int r, g, b, a;
};

static PanelUIState* panelUi[] = {
    nullptr,   // PANEL_HIDE
    &feedUi,   // PANEL_FEED
    &killUi,   // PANEL_KILL
    &onlineUi  // PANEL_ONLINE
};

static void* panelBackgrounds[] = {
    nullptr,                // PANEL_HIDE
    &globalkill_background, // PANEL_FEED
    &killfeed_background,   // PANEL_KILL
    &online_background      // PANEL_ONLINE
};

inline void renderBackground(void* background, int x, int y) {
    __asm {
        push y
        push x
        mov ecx, background
        call render_tga
    }
}

inline void renderPercentTextUnified(int x, int y, const char* text,
    int r, int g, int b, int a) {
    __asm { push text }
    __asm { push a }
    __asm { push r }
    __asm { push g }
    __asm { push b }
    __asm {
        push y
        push x
        push 0x22B69B0
        call render_text_with_stroke
        add esp, 32
    }
}

inline void renderPanel(PanelType type) {
    PanelUIState* ui = panelUi[type];
    if (!ui) return;
    int panelX = ui->baseX + ui->offsetX;
    int panelY = ui->baseY + ui->offsetY;
    if (panelBackgrounds[type]) {
        renderBackground(panelBackgrounds[type], panelX, panelY);
    }
    if (type == PANEL_FEED) {
        TextEntry feedTexts[] = {
            {20, 32, feed_text_1, 255,255,255,0},
            {20, 52, feed_text_2, 255,255,255,0},
            {20, 72, feed_text_3, 255,255,255,0},
            {20, 92, feed_text_4, 255,255,255,0},
            {20,112, feed_text_5, 255,255,255,0}
        };
        for (auto& t : feedTexts) {
            renderPercentTextUnified(panelX + t.offsetX, panelY + t.offsetY,
                t.text, t.r, t.g, t.b, t.a);
        }
    }
    else if (type == PANEL_KILL) {
        renderAoLBar(panelX + 11, panelY + 56, g_lightPercentIntKill);
        renderUoFBar(panelX + 241, panelY + 56, g_furyPercentIntKill);

        TextEntry killTexts[] = {
            {36, 32, PERCENT_LIGHT_KILL, 255,255,255,0},
            {150,32, PERCENT_FURY_KILL,  255,255,255,0}
        };
        for (auto& t : killTexts) {
            renderPercentTextUnified(panelX + t.offsetX, panelY + t.offsetY,
                t.text, t.r, t.g, t.b, t.a);
        }
    }
    else if (type == PANEL_ONLINE) {
        renderProgressBar(panelX + 11, panelY + 32, g_lightPercentInt2, (void*)loadbar_AoL, false);
        renderProgressBar(panelX + 241, panelY + 32, g_furyPercentInt2, (void*)loadbar_UoF, true);

        TextEntry onlineTexts[] = {
            {105,52, ON, 0,255,0,0},
            {10, 52, LI, 0,255,0,0},
            {205,52, FU, 0,255,0,0},
            {16, 32, PERCENT_LIGHT, 255,255,255,0},
            {170,32, PERCENT_FURY,  255,255,255,0},
            {16, 70, FIGHTER, 255,255,255,0},
            {16, 85, DEFENDER,255,255,255,0},
            {16,100, RANGER,  255,255,255,0},
            {16,115, ARCHER,  255,255,255,0},
            {16,130, MAGE,    255,255,255,0},
            {16,145, PRIEST,  255,255,255,0},
            {170,70, WARRIOR, 255,255,255,0},
            {170,85, GUARDIAN,255,255,255,0},
            {170,100,ASSASSIN,255,255,255,0},
            {170,115,HUNTER,  255,255,255,0},
            {170,130,PAGAN,   255,255,255,0},
            {170,145,ORACLE,  255,255,255,0}
        };
        for (auto& t : onlineTexts) {
            renderPercentTextUnified(panelX + t.offsetX, panelY + t.offsetY,
                t.text, t.r, t.g, t.b, t.a);
        }
    }
}

PanelType g_activePanel = PANEL_HIDE;
inline void setActivePanel(PanelType type) {
    g_activePanel = type;
}

struct ButtonUIState {
    int offsetX, offsetY;
    int width, height;
    PanelType targetPanel;
    void* background;
};

static void* hideBtnBackground = &hide_button;     // HIDE button image
static void* feedBtnBackground = &feed_button;   // FEED button image
static void* killBtnBackground = &kill_button;     // KILL button image
static void* onlineBtnBackground = &online_button;   // ONLINE button image
static void* buttonBackground = &toolbar_background;    // BACKGROUND button image

ButtonUIState hideButton{ 205, 1, 32, 32, PANEL_HIDE, hideBtnBackground };
ButtonUIState feedButton{ 16, 32, 32, 32, PANEL_FEED, feedBtnBackground };
ButtonUIState killButton{ 85, 32, 32, 32, PANEL_KILL, killBtnBackground };
ButtonUIState onlineButton{ 154,32, 32, 32, PANEL_ONLINE, onlineBtnBackground };
PanelUIState* g_activeDraggingPanel = nullptr;

inline void handleMovementExclusive(PanelUIState& ui) {
    POINT curPos;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        GetCursorPos(&curPos);
        if (ui.gameHwnd) ScreenToClient(ui.gameHwnd, &curPos);

        int panelX = ui.baseX + ui.offsetX;
        int panelY = ui.baseY + ui.offsetY;

        if (g_activeDraggingPanel && g_activeDraggingPanel != &ui) return;

        if (curPos.x >= panelX && curPos.x <= panelX + ui.width &&
            curPos.y >= panelY && curPos.y <= panelY + ui.height) {
            if (!ui.dragging) {
                ui.dragging = true;
                g_activeDraggingPanel = &ui;
                ui.lastMousePos = curPos;
            }
            else {
                int dx = curPos.x - ui.lastMousePos.x;
                int dy = curPos.y - ui.lastMousePos.y;
                ui.offsetX += dx;
                ui.offsetY += dy;
                ui.lastMousePos = curPos;
                saveConfig(ui);
            }
        }
    }
    else {
        ui.dragging = false;
        if (g_activeDraggingPanel == &ui) g_activeDraggingPanel = nullptr;
    }
}

inline void renderButton(const ButtonUIState& btn, int baseX, int baseY) {
    int bx = baseX + btn.offsetX;
    int by = baseY + btn.offsetY;
    renderBackground(btn.background, bx, by);
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        POINT curPos;
        GetCursorPos(&curPos);
        if (buttonUi.gameHwnd) ScreenToClient(buttonUi.gameHwnd, &curPos);
        if (curPos.x >= bx && curPos.x <= bx + btn.width &&
            curPos.y >= by && curPos.y <= by + btn.height) {
            setActivePanel(btn.targetPanel);
        }
    }
}

inline void doAllPanels(int baseX, int baseY) {
    switch (g_activePanel) {
    case PANEL_FEED:
        feedUi.baseX = baseX;
        feedUi.baseY = baseY;
        handleMovementExclusive(feedUi);
        renderPanel(PANEL_FEED);
        break;
    case PANEL_KILL:
        killUi.baseX = baseX;
        killUi.baseY = baseY;
        handleMovementExclusive(killUi);
        renderPanel(PANEL_KILL);
        break;
    case PANEL_ONLINE:
        onlineUi.baseX = baseX;
        onlineUi.baseY = baseY;
        handleMovementExclusive(onlineUi);
        renderPanel(PANEL_ONLINE);
        break;
    case PANEL_HIDE:
    default:
        break;
    }

    buttonUi.baseX = baseX;
    buttonUi.baseY = baseY + 200;
    handleMovementExclusive(buttonUi);
    int bx = buttonUi.baseX + buttonUi.offsetX;
    int by = buttonUi.baseY + buttonUi.offsetY;
    renderBackground(buttonBackground, bx, by);
    renderButton(hideButton, bx, by);
    renderButton(feedButton, bx, by);
    renderButton(killButton, bx, by);
    renderButton(onlineButton, bx, by);
}

auto u0x47DD54 = 0x47DD54;
__declspec(naked) void naked_0x47DD4D() {
    __asm {
        mov eax, [ebx + 4]   // baseX
        mov ecx, [ebx + 8]   // baseY

        pushad
        push ecx             // baseY
        push eax             // baseX
        call doAllPanels
        add esp, 8
        popad

        movzx eax, byte ptr[ebx + 0x3CC]
        jmp u0x47DD54
    }
}

__declspec(naked) void naked_0x5F3740() {
    __asm {
        pushad
        mov eax, esp
        push eax
        call parseAndHandle
        add esp, 4
        popad
        ret
    }
}

void hook::online() {
    PanelUIState* panels[] = { &onlineUi, &killUi, &feedUi, &buttonUi };
    for (auto* ui : panels) {
        loadConfig(*ui);
    }
    util::detour((void*)0x47DD4D, naked_0x47DD4D, 7);
    util::detour((void*)0x5F3740, naked_0x5F3740, 5);
}