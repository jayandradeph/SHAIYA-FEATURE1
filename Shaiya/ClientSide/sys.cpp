#define _CRT_SECURE_NO_WARNINGS
#include <util/util.h>
#include "include/main.h"
#include "include/sys.h"
#include <windows.h>
#include <io.h>
#include <cstdio>
#include <vector>

enum PanelType {
    PANEL_HIDE,
    PANEL_FEED,
    PANEL_BALANCE,
    PANEL_ONLINE
};

enum ProgressType {
    PROGRESS_ONLINE,
    PROGRESS_BALANCE
};

struct PanelUIState {
    int offsetX, offsetY, width, height;
    void* background = nullptr;
    void* hoverBackground = nullptr;
    PanelType targetPanel = PANEL_HIDE;
    POINT lastMousePos{};
    bool dragging = false;
    HWND gameHwnd = nullptr;
    int baseX = 0, baseY = 0;
    const wchar_t* configFile = nullptr;
    const wchar_t* sectionName = nullptr;
    int defaultOffsetX = 0, defaultOffsetY = 0;
};

struct StatusText {
    char buffer[128];
};

struct OnlineStatus {
    StatusText total, light, fury;
    StatusText percentLight, percentFury;
    StatusText fighter, defender, ranger, archer, mage, priest;
    StatusText warrior, guardian, assassin, hunter, pagan, oracle;
    bool isAoLLeading = false;
    bool isUoFLeading = false;
    int lightPercent = 0;
    int furyPercent = 0;
};

struct KillStatus {
    StatusText percentLight, percentFury;
    bool isAoLLeading = false;
    bool isUoFLeading = false;
    int lightPercent = 0;
    int furyPercent = 0;
};

struct TextEntry {
    int offsetX;
    int offsetY;
    const char* text;
    int r, g, b, a;
};

StatusText feed_texts[5];
OnlineStatus g_onlineStatus;
KillStatus g_killStatus;
PanelType g_activePanel = PANEL_HIDE;
PanelUIState* g_activeDraggingPanel = nullptr;
PanelUIState background_toolbar{ 200, -300, 250, 140, &toolbar_background, nullptr };

std::map<PanelType, PanelUIState> panels = {
    { PANEL_HIDE,    {200, -300, 250, 140, nullptr, nullptr, PANEL_HIDE, {}, false, nullptr, 0, 0, L".\\panel.ini", L"BUTTON_NOTICE_UI", 200, -300 } },
    { PANEL_FEED,    {199, -227, 250, 140, nullptr, nullptr, PANEL_FEED, {}, false, nullptr, 0, 0, L".\\panel.ini", L"KILL_NOTICE_UI", 199, -227 } },
    { PANEL_BALANCE, {199, -227, 250, 140, nullptr, nullptr, PANEL_BALANCE, {}, false, nullptr, 0, 0, L".\\panel.ini", L"BALANCE_NOTICE_UI", 199, -227 } },
    { PANEL_ONLINE,  {199, -227, 250, 140, nullptr, nullptr, PANEL_ONLINE, {}, false, nullptr, 0, 0, L".\\panel.ini", L"ONLINE_NOTICE_UI", 199, -227 } }
};

std::map<PanelType, PanelUIState> toggleButtons = {
    { PANEL_HIDE,    {205, 1, 32, 32, &hide_button,    &hide_button_hover,    PANEL_HIDE } },
    { PANEL_FEED,    {16, 32, 32, 32, &feed_button,    &feed_button_hover,    PANEL_FEED } },
    { PANEL_BALANCE, {85, 32, 32, 32, &balance_button, &balance_button_hover, PANEL_BALANCE } },
    { PANEL_ONLINE,  {154,32, 32, 32, &online_button,  &online_button_hover,  PANEL_ONLINE } }
};

int g_lightPercentIntOnline = 0;
int g_furyPercentIntOnline = 0;
int g_lightPercentIntBalance = 0;
int g_furyPercentIntBalance = 0;

inline void createDefaultConfig(PanelUIState& ui) {
    if (!ui.configFile || !ui.sectionName) return;
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", ui.defaultOffsetX);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetX", buf, ui.configFile);
    swprintf(buf, 32, L"%d", ui.defaultOffsetY);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetY", buf, ui.configFile);
}

inline void loadConfig(PanelUIState& ui) {
    if (!ui.configFile || !ui.sectionName) return;
    if (_access((const char*)ui.configFile, 0) != 0) {
        createDefaultConfig(ui);
    }
    ui.offsetX = GetPrivateProfileIntW(ui.sectionName, L"OffsetX", ui.defaultOffsetX, ui.configFile);
    ui.offsetY = GetPrivateProfileIntW(ui.sectionName, L"OffsetY", ui.defaultOffsetY, ui.configFile);
    ui.gameHwnd = FindWindowW(L"GAME", nullptr);
}

inline void saveConfig(PanelUIState& ui) {
    if (!ui.configFile || !ui.sectionName) return;
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", ui.offsetX);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetX", buf, ui.configFile);
    swprintf(buf, 32, L"%d", ui.offsetY);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetY", buf, ui.configFile);
}

void updateStatusOnline(const char* val) {
    int total = 0, lightCount = 0, lightPercent = 0, furyCount = 0, furyPercent = 0;
    int fighter = 0, defender = 0, ranger = 0, archer = 0, mage = 0, priest = 0;
    int warrior = 0, guardian = 0, assassin = 0, hunter = 0, pagan = 0, oracle = 0;
    int matched = sscanf(val, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        &total, &lightCount, &lightPercent, &furyCount, &furyPercent,
        &fighter, &defender, &ranger, &archer, &mage, &priest,
        &warrior, &guardian, &assassin, &hunter, &pagan, &oracle);

    if (matched >= 5 && total > 0) {
        g_lightPercentIntOnline = lightPercent;
        g_furyPercentIntOnline = furyPercent;
        snprintf(g_onlineStatus.total.buffer, 128, "Total: %d", total);
        snprintf(g_onlineStatus.light.buffer, 128, "Light: %d", lightCount);
        snprintf(g_onlineStatus.fury.buffer, 128, "Fury: %d", furyCount);
        snprintf(g_onlineStatus.percentLight.buffer, 128, "AoL: %.2f%%%%", (double)lightPercent);
        snprintf(g_onlineStatus.percentFury.buffer, 128, "UoF: %.2f%%%%", (double)furyPercent);
        snprintf(g_onlineStatus.fighter.buffer, 128, "Fighter: %d", fighter);
        snprintf(g_onlineStatus.defender.buffer, 128, "Defender: %d", defender);
        snprintf(g_onlineStatus.ranger.buffer, 128, "Ranger: %d", ranger);
        snprintf(g_onlineStatus.archer.buffer, 128, "Archer: %d", archer);
        snprintf(g_onlineStatus.mage.buffer, 128, "Mage: %d", mage);
        snprintf(g_onlineStatus.priest.buffer, 128, "Priest: %d", priest);
        snprintf(g_onlineStatus.warrior.buffer, 128, "Warrior: %d", warrior);
        snprintf(g_onlineStatus.guardian.buffer, 128, "Guardian: %d", guardian);
        snprintf(g_onlineStatus.assassin.buffer, 128, "Assassin: %d", assassin);
        snprintf(g_onlineStatus.hunter.buffer, 128, "Hunter: %d", hunter);
        snprintf(g_onlineStatus.pagan.buffer, 128, "Pagan: %d", pagan);
        snprintf(g_onlineStatus.oracle.buffer, 128, "Oracle: %d", oracle);
        g_onlineStatus.isAoLLeading = (lightPercent >= furyPercent);
        g_onlineStatus.isUoFLeading = !g_onlineStatus.isAoLLeading;
    }
    else {
        strcpy(g_onlineStatus.total.buffer, "Total: 0");
        strcpy(g_onlineStatus.light.buffer, "Light: 0");
        strcpy(g_onlineStatus.fury.buffer, "Fury: 0");
        strcpy(g_onlineStatus.percentLight.buffer, "AoL: 0.00%");
        strcpy(g_onlineStatus.percentFury.buffer, "UoF: 0.00%");
        strcpy(g_onlineStatus.fighter.buffer, "Fighter: 0");
        strcpy(g_onlineStatus.defender.buffer, "Defender: 0");
        strcpy(g_onlineStatus.ranger.buffer, "Ranger: 0");
        strcpy(g_onlineStatus.archer.buffer, "Archer: 0");
        strcpy(g_onlineStatus.mage.buffer, "Mage: 0");
        strcpy(g_onlineStatus.priest.buffer, "Priest: 0");
        strcpy(g_onlineStatus.warrior.buffer, "Warrior: 0");
        strcpy(g_onlineStatus.guardian.buffer, "Guardian: 0");
        strcpy(g_onlineStatus.assassin.buffer, "Assassin: 0");
        strcpy(g_onlineStatus.hunter.buffer, "Hunter: 0");
        strcpy(g_onlineStatus.pagan.buffer, "Pagan: 0");
        strcpy(g_onlineStatus.oracle.buffer, "Oracle: 0");
        g_lightPercentIntOnline = 0;
        g_furyPercentIntOnline = 0;
        g_onlineStatus.isAoLLeading = false;
        g_onlineStatus.isUoFLeading = false;
    }
}

void updateStatusKill(const char* val) {
    double lightPercent = 0.0, furyPercent = 0.0;
    int matched = sscanf(val, "%lf %lf", &lightPercent, &furyPercent);

    if (matched == 2) {
        if (lightPercent < 0.0) lightPercent = 0.0;
        if (lightPercent > 100.0) lightPercent = 100.0;
        if (furyPercent < 0.0) furyPercent = 0.0;
        if (furyPercent > 100.0) furyPercent = 100.0;
        g_lightPercentIntBalance = (int)(lightPercent + 0.5);
        g_furyPercentIntBalance = (int)(furyPercent + 0.5);
        snprintf(g_killStatus.percentLight.buffer, 128, "AoL: %.2f%%%%", lightPercent);
        snprintf(g_killStatus.percentFury.buffer, 128, "UoF: %.2f%%%%", furyPercent);
        g_killStatus.isAoLLeading = (g_lightPercentIntBalance >= g_furyPercentIntBalance);
        g_killStatus.isUoFLeading = !g_killStatus.isAoLLeading;
    }
    else {
        strcpy(g_killStatus.percentLight.buffer, "AoL: 0%");
        strcpy(g_killStatus.percentFury.buffer, "UoF: 0%");
        g_lightPercentIntBalance = 0;
        g_furyPercentIntBalance = 0;
        g_killStatus.isAoLLeading = false;
        g_killStatus.isUoFLeading = false;
    }
}

inline void renderProgressBarGeneric(
    int x, int y, int percent,
    void* barTexture, bool fromRight,
    int maxWidth){
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

inline constexpr int BALANCE_PROGRESS_WIDTH = 230;
inline constexpr int ONLINE_PROGRESS_WIDTH = 230;
inline void renderProgressBar(
    int x, int y, int percent,
    void* barTexture, bool fromRight,
    ProgressType type)
{
    int maxWidth = (type == PROGRESS_BALANCE) ? BALANCE_PROGRESS_WIDTH : ONLINE_PROGRESS_WIDTH;
    renderProgressBarGeneric(x, y, percent, barTexture, fromRight, maxWidth);
}

const char* AoLFrames[] = { loadbar_AoL, loadbar_AoL_alt1, loadbar_AoL_alt2, loadbar_AoL_alt3, loadbar_AoL_alt4, loadbar_AoL_alt5 };
const char* UoFFrames[] = { loadbar_UoF, loadbar_UoF_alt1, loadbar_UoF_alt2, loadbar_UoF_alt3, loadbar_UoF_alt4, loadbar_UoF_alt5 };

inline const char* SelectKillTexture(const char** frames, unsigned int frameCount) {
    unsigned int tick = GetTickCount();
    unsigned int speed = 120; // frame change speed in ms
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

DWORD aolStartTick = 0, uofStartTick = 0;
int lastAoLPercent = -1, lastUoFPercent = -1;
const DWORD ANIMATION_DURATION = 5000; // 5 seconds
const char* GetAoLTextureWithStop(int percent) {
    DWORD now = GetTickCount();
    if (percent > lastAoLPercent && percent >= g_furyPercentIntBalance) {
        lastAoLPercent = percent;
        aolStartTick = now;
    }
    else {
        lastAoLPercent = percent;
    }
    if (percent > 0 && (now - aolStartTick) < ANIMATION_DURATION) {
        return SelectKillTexture(AoLFrames, sizeof(AoLFrames) / sizeof(AoLFrames[0]));
    }
    return (const char*)loadbar_AoL;
}

const char* GetUoFTextureWithStop(int percent) {
    DWORD now = GetTickCount();
    if (percent > lastUoFPercent && percent >= g_lightPercentIntBalance) {
        lastUoFPercent = percent;
        uofStartTick = now;
    }
    else {
        lastUoFPercent = percent;
    }
    if (percent > 0 && (now - uofStartTick) < ANIMATION_DURATION) {
        return SelectKillTexture(UoFFrames, sizeof(UoFFrames) / sizeof(UoFFrames[0]));
    }
    return (const char*)loadbar_UoF;
}

void shiftFeedTexts(const char* newNotice) {
    strcpy(feed_texts[4].buffer, feed_texts[3].buffer);
    strcpy(feed_texts[3].buffer, feed_texts[2].buffer);
    strcpy(feed_texts[2].buffer, feed_texts[1].buffer);
    strcpy(feed_texts[1].buffer, feed_texts[0].buffer);
    strncpy(feed_texts[0].buffer, newNotice, sizeof(feed_texts[0].buffer));
    feed_texts[0].buffer[sizeof(feed_texts[0].buffer) - 1] = '\0';
}

inline void parseAndHandle(void* espBase) {
    void* arg = *(void**)((BYTE*)espBase + 0x54);
    if (arg == *(void**)"[BALANCE_NOTICE]") {
        updateStatusKill((const char*)((BYTE*)espBase + 0x54 + 16));
        return;
    }
    if (arg == *(void**)"[ONLINE_NOTICE]") {
        updateStatusOnline((const char*)((BYTE*)espBase + 0x54 + 15));
        return;
    }
    if (arg == *(void**)"[KILL_NOTICE]") {
        shiftFeedTexts((const char*)((BYTE*)espBase + 0x54 + 13));
        return;
    }
    reinterpret_cast<void(__stdcall*)(uintptr_t)>(0x5E5C10);
}

inline void renderPercentText(int x, int y, const char* text,
    int r, int g, int b, int a)
{
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

inline void renderBackground(void* background, int x, int y) {
    __asm {
        push y
        push x
        mov ecx, background
        call render_tga
    }
}

static void* panelBackgrounds[] = {
    nullptr,
    &globalkill_background,
    &killfeed_background,
    &online_background
};

inline void renderPanel(PanelType type) {
    auto it = panels.find(type);
    if (it == panels.end()) return;
    PanelUIState& ui = it->second;
    int panelX = ui.baseX + ui.offsetX;
    int panelY = ui.baseY + ui.offsetY;
    if (panelBackgrounds[type]) {
        renderBackground(panelBackgrounds[type], panelX, panelY);
    }
    if (type == PANEL_FEED) {
        TextEntry feedTexts[] = {
            {20,  32, feed_texts[0].buffer, 255,255,255,0},
            {20,  52, feed_texts[1].buffer, 255,255,255,0},
            {20,  72, feed_texts[2].buffer, 255,255,255,0},
            {20,  92, feed_texts[3].buffer, 255,255,255,0},
            {20, 112, feed_texts[4].buffer, 255,255,255,0}
        };
        for (auto& t : feedTexts) {
            renderPercentText(panelX + t.offsetX, panelY + t.offsetY, t.text, t.r, t.g, t.b, t.a);
        }
    }
    else if (type == PANEL_BALANCE) {
        renderProgressBar(panelX + 11, panelY + 32, g_lightPercentIntBalance,
            (void*)GetAoLTextureWithStop(g_lightPercentIntBalance),
            false, PROGRESS_BALANCE);

        renderProgressBar(panelX + 241, panelY + 32, g_furyPercentIntBalance,
            (void*)GetUoFTextureWithStop(g_furyPercentIntBalance),
            true, PROGRESS_BALANCE);

        TextEntry killTexts[] = {
            {36, 32, g_killStatus.percentLight.buffer, 255,255,255,0},
            {150,32, g_killStatus.percentFury.buffer,  255,255,255,0}
        };
        for (auto& t : killTexts) {
            renderPercentText(panelX + t.offsetX, panelY + t.offsetY, t.text, t.r, t.g, t.b, t.a);
        }
    }
    else if (type == PANEL_ONLINE) {
        renderProgressBar(panelX + 11, panelY + 32, g_lightPercentIntOnline,
            (void*)loadbar_AoL, false, PROGRESS_ONLINE);

        renderProgressBar(panelX + 241, panelY + 32, g_furyPercentIntOnline,
            (void*)loadbar_UoF, true, PROGRESS_ONLINE);

        TextEntry onlineTexts[] = {
            {105,52, g_onlineStatus.total.buffer, 0,255,0,0},
            {10, 52, g_onlineStatus.light.buffer, 0,255,0,0},
            {205,52, g_onlineStatus.fury.buffer, 0,255,0,0},
            {16, 32, g_onlineStatus.percentLight.buffer, 255,255,255,0},
            {170,32, g_onlineStatus.percentFury.buffer,  255,255,255,0},
            {16, 70, g_onlineStatus.fighter.buffer, 255,255,255,0},
            {16, 85, g_onlineStatus.defender.buffer,255,255,255,0},
            {16,100, g_onlineStatus.ranger.buffer,  255,255,255,0},
            {16,115, g_onlineStatus.archer.buffer,  255,255,255,0},
            {16,130, g_onlineStatus.mage.buffer,    255,255,255,0},
            {16,145, g_onlineStatus.priest.buffer,  255,255,255,0},
            {170,70, g_onlineStatus.warrior.buffer, 255,255,255,0},
            {170,85, g_onlineStatus.guardian.buffer,255,255,255,0},
            {170,100,g_onlineStatus.assassin.buffer,255,255,255,0},
            {170,115,g_onlineStatus.hunter.buffer,  255,255,255,0},
            {170,130,g_onlineStatus.pagan.buffer,   255,255,255,0},
            {170,145,g_onlineStatus.oracle.buffer,  255,255,255,0}
        };
        for (auto& t : onlineTexts) {
            renderPercentText(panelX + t.offsetX, panelY + t.offsetY, t.text, t.r, t.g, t.b, t.a);
        }
    }
}

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

inline void renderButton(const PanelUIState& btn, int baseX, int baseY) {
    int bx = baseX + btn.offsetX;
    int by = baseY + btn.offsetY;
    POINT curPos;
    GetCursorPos(&curPos);
    if (btn.gameHwnd) {
        ScreenToClient(btn.gameHwnd, &curPos);
    }
    bool isHover = (curPos.x >= bx && curPos.x <= bx + btn.width && curPos.y >= by && curPos.y <= by + btn.height);
    void* tex = (isHover && btn.hoverBackground) ? btn.hoverBackground : btn.background;
    renderBackground(tex, bx, by);
    if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && isHover) {
        g_activePanel = btn.targetPanel;
    }
}

inline void doAllPanels(int baseX, int baseY) {
    for (auto& [type, ui] : panels) {
        ui.baseX = baseX;
        ui.baseY = baseY;
        handleMovementExclusive(ui);
    }
    if (g_activePanel != PANEL_HIDE) {
        renderPanel(g_activePanel);
    }
    background_toolbar.baseX = baseX;
    background_toolbar.baseY = baseY;
    handleMovementExclusive(background_toolbar);
    int bx = background_toolbar.baseX + background_toolbar.offsetX;
    int by = background_toolbar.baseY + background_toolbar.offsetY;
    renderBackground(background_toolbar.background, bx, by);
    for (auto& [type, btn] : toggleButtons) {
    renderButton(btn, bx, by);
    }
}

auto u0x47DD54 = 0x47DD54;
__declspec(naked) void naked_0x47DD4D() {
    __asm {
        mov eax, [ebx + 4]
        mov ecx, [ebx + 8]
        pushad
        push ecx
        push eax
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
    for (auto& [type, ui] : panels) {loadConfig(ui);}loadConfig(background_toolbar);
    util::detour((void*)0x47DD4D, naked_0x47DD4D, 7);
    util::detour((void*)0x5F3740, naked_0x5F3740, 5);
}
