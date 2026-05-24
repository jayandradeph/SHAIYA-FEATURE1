#define _CRT_SECURE_NO_WARNINGS
#include <util/util.h>
#include "include/main.h"
#include <windows.h>
#include <io.h>
#include <cstdio>
#include <vector>
#include <map>
#include <cstring>

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
    int offsetX, offsetY;
    int width, height;
    void* background = nullptr;
    void* hoverBackground = nullptr;
    PanelType targetPanel = PANEL_HIDE;
    POINT lastMousePos{};
    bool dragging = false;
    HWND gameHwnd = nullptr;
    int baseX = 0, baseY = 0;
    const wchar_t* configFile = nullptr;
    const wchar_t* sectionName = nullptr;
};

struct StatusText {
    char buffer[128];
};

struct OnlineStatus {
    StatusText total, light, fury;
    StatusText percentLight, percentFury;
    StatusText fighter, defender, ranger, archer, mage, priest;
    StatusText warrior, guardian, assassin, hunter, pagan, oracle;
    int lightPercentIntOnline = 0;
    int furyPercentIntOnline = 0;
};

struct KillStatus {
    StatusText percentLight, percentFury;
    int lightPercentIntBalance = 0;
    int furyPercentIntBalance = 0;
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

std::map<PanelType, PanelUIState> panels = {
    { PANEL_HIDE,    {200,-300,250,140,&toolbar_background,nullptr,PANEL_HIDE,{},false,nullptr,0,0,L".\\panel.ini",L"BUTTON_NOTICE_UI"} },
    { PANEL_FEED,    {199,-227,250,140,&killfeed_background,nullptr,PANEL_FEED,{},false,nullptr,0,0,L".\\panel.ini",L"KILL_NOTICE_UI"} },
    { PANEL_BALANCE, {199,-227,250,140,&balance_background,nullptr,PANEL_BALANCE,{},false,nullptr,0,0,L".\\panel.ini",L"BALANCE_NOTICE_UI"} },
    { PANEL_ONLINE,  {199,-227,250,140,&online_background,nullptr,PANEL_ONLINE,{},false,nullptr,0,0,L".\\panel.ini",L"ONLINE_NOTICE_UI"} }
};

std::map<PanelType, PanelUIState> toggleButtons = {
    { PANEL_HIDE,    {205,1,32,32,&hide_button,&hide_button_hover,PANEL_HIDE } },
    { PANEL_FEED,    {16,32,32,32,&feed_button,&feed_button_hover,PANEL_FEED } },
    { PANEL_BALANCE, {85,32,32,32,&balance_button,&balance_button_hover,PANEL_BALANCE } },
    { PANEL_ONLINE,  {154,32,32,32,&online_button,&online_button_hover,PANEL_ONLINE } }
};

inline void createDefaultConfig(PanelUIState& ui) {
    if (!ui.configFile || !ui.sectionName) return;
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", ui.offsetX);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetX", buf, ui.configFile);
    swprintf(buf, 32, L"%d", ui.offsetY);
    WritePrivateProfileStringW(ui.sectionName, L"OffsetY", buf, ui.configFile);
}

inline void loadConfig(PanelUIState& ui) {
    if (!ui.configFile || !ui.sectionName) return;
    if (_waccess(ui.configFile, 0) != 0) {
        createDefaultConfig(ui);
    }
    ui.offsetX = GetPrivateProfileIntW(ui.sectionName, L"OffsetX", ui.offsetX, ui.configFile);
    ui.offsetY = GetPrivateProfileIntW(ui.sectionName, L"OffsetY", ui.offsetY, ui.configFile);
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

void updateStatusKill(const char* newNotice) {
    for (int i = 4; i > 0; --i) {
        strcpy(feed_texts[i].buffer, feed_texts[i - 1].buffer);
    }
    strncpy(feed_texts[0].buffer, newNotice, sizeof(feed_texts[0].buffer) - 1);
    feed_texts[0].buffer[sizeof(feed_texts[0].buffer) - 1] = '\0';
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
        g_onlineStatus.lightPercentIntOnline = lightPercent;
        g_onlineStatus.furyPercentIntOnline = furyPercent;
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
    }
}

void updateStatusBalance(const char* val) {
    double lightPercent = 0.0, furyPercent = 0.0;
    int matched = sscanf(val, "%lf %lf", &lightPercent, &furyPercent);
    if (matched == 2) {
        if (lightPercent < 0.0) lightPercent = 0.0;
        if (lightPercent > 100.0) lightPercent = 100.0;
        if (furyPercent < 0.0) furyPercent = 0.0;
        if (furyPercent > 100.0) furyPercent = 100.0;
        g_killStatus.lightPercentIntBalance = (int)(lightPercent + 0.5);
        g_killStatus.furyPercentIntBalance = (int)(furyPercent + 0.5);
        snprintf(g_killStatus.percentLight.buffer, 128, "AoL: %.2f%%%%", lightPercent);
        snprintf(g_killStatus.percentFury.buffer, 128, "UoF: %.2f%%%%", furyPercent);
    }
    else {
        strcpy(g_killStatus.percentLight.buffer, "AoL: 0%");
        strcpy(g_killStatus.percentFury.buffer, "UoF: 0%");
        g_killStatus.lightPercentIntBalance = 0;
        g_killStatus.furyPercentIntBalance = 0;
    }
}

inline void DrawTexture(void* background, int x, int y,
    const char* text = nullptr,
    int r = 255, int g = 255, int b = 255, int a = 0) {
    if (background) {
        __asm {
            push y
            push x
            mov ecx, background
            call DrawObjectTexture
        }
    }
    if (text) {
        __asm { push text }
        __asm { push a }
        __asm { push r }
        __asm { push g }
        __asm { push b }
        __asm {
            push y
            push x
            push 0x22B69B0
            call DrawTextStroke
            add esp, 32
        }
    }
}

inline void ProgressBar(int x, int y, int percent, void* barTexture, bool fromRight, int maxWidth) {
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
            call DrawObjectTexture
        }
    }
}

inline void PanelDisplay(PanelType type) {
    auto it = panels.find(type);
    if (it == panels.end()) return;
    PanelUIState& ui = it->second;
    int panelX = ui.baseX + ui.offsetX;
    int panelY = ui.baseY + ui.offsetY;
    DrawTexture(ui.background, panelX, panelY);
    if (type == PANEL_FEED) {
        TextEntry feedTexts[] = {
            {20,32,feed_texts[0].buffer,255,255,255,0},
            {20,52,feed_texts[1].buffer,255,255,255,0},
            {20,72,feed_texts[2].buffer,255,255,255,0},
            {20,92,feed_texts[3].buffer,255,255,255,0},
            {20,112,feed_texts[4].buffer,255,255,255,0}
        };
        for (auto& t : feedTexts) {
            DrawTexture(nullptr, panelX + t.offsetX, panelY + t.offsetY, t.text, t.r, t.g, t.b, t.a);
        }
    }
    else if (type == PANEL_BALANCE) {
        ProgressBar(panelX + 11, panelY + 32, g_killStatus.lightPercentIntBalance, (void*)loadbar_AoL, false, 230);
        ProgressBar(panelX + 241, panelY + 32, g_killStatus.furyPercentIntBalance, (void*)loadbar_UoF, true, 230);
        TextEntry killTexts[] = {
            {36,32,g_killStatus.percentLight.buffer,255,255,255,0},
            {150,32,g_killStatus.percentFury.buffer,255,255,255,0}
        };
        for (auto& t : killTexts) {
            DrawTexture(nullptr, panelX + t.offsetX, panelY + t.offsetY, t.text, t.r, t.g, t.b, t.a);
        }
    }
    else if (type == PANEL_ONLINE) {
        ProgressBar(panelX + 11, panelY + 32, g_onlineStatus.lightPercentIntOnline, (void*)loadbar_AoL, false, 230);
        ProgressBar(panelX + 241, panelY + 32, g_onlineStatus.furyPercentIntOnline, (void*)loadbar_UoF, true, 230);
        TextEntry onlineTexts[] = {
            {105,52,g_onlineStatus.total.buffer,0,255,0,0},
            {10,52,g_onlineStatus.light.buffer,0,255,0,0},
            {205,52,g_onlineStatus.fury.buffer,0,255,0,0},
            {16,32,g_onlineStatus.percentLight.buffer,255,255,255,0},
            {170,32,g_onlineStatus.percentFury.buffer,255,255,255,0},
            {16,70,g_onlineStatus.fighter.buffer,255,255,255,0},
            {16,85,g_onlineStatus.defender.buffer,255,255,255,0},
            {16,100,g_onlineStatus.ranger.buffer,255,255,255,0},
            {16,115,g_onlineStatus.archer.buffer,255,255,255,0},
            {16,130,g_onlineStatus.mage.buffer,255,255,255,0},
            {16,145,g_onlineStatus.priest.buffer,255,255,255,0},
            {170,70,g_onlineStatus.warrior.buffer,255,255,255,0},
            {170,85,g_onlineStatus.guardian.buffer,255,255,255,0},
            {170,100,g_onlineStatus.assassin.buffer,255,255,255,0},
            {170,115,g_onlineStatus.hunter.buffer,255,255,255,0},
            {170,130,g_onlineStatus.pagan.buffer,255,255,255,0},
            {170,145,g_onlineStatus.oracle.buffer,255,255,255,0}
        };
        for (auto& t : onlineTexts) {
            DrawTexture(nullptr, panelX + t.offsetX, panelY + t.offsetY, t.text, t.r, t.g, t.b, t.a);
        }
    }
}

inline void MovePanel(PanelUIState& ui) {
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

inline void UIHandler(void* ebxPtr) {
    int baseX = *(int*)((uintptr_t)ebxPtr + 4);
    int baseY = *(int*)((uintptr_t)ebxPtr + 8);
    for (auto& [type, ui] : panels) {
        ui.baseX = baseX;
        ui.baseY = baseY;
        MovePanel(ui);
    }
    if (g_activePanel != PANEL_HIDE) {
        PanelDisplay(g_activePanel);
    }
    auto it = panels.find(PANEL_HIDE);
    if (it != panels.end()) {
        PanelUIState& toolbar = it->second;
        int bx = toolbar.baseX + toolbar.offsetX;
        int by = toolbar.baseY + toolbar.offsetY;
        DrawTexture(toolbar.background, bx, by);
        bool draggingOtherPanel = (g_activeDraggingPanel != nullptr && g_activeDraggingPanel->targetPanel != PANEL_HIDE);
        for (auto& [type, btn] : toggleButtons) {
            int btnX = bx + btn.offsetX;
            int btnY = by + btn.offsetY;
            POINT curPos;
            GetCursorPos(&curPos);
            if (btn.gameHwnd) ScreenToClient(btn.gameHwnd, &curPos);
            bool isHover = (curPos.x >= btnX && curPos.x <= btnX + btn.width && curPos.y >= btnY && curPos.y <= btnY + btn.height);
            void* tex = (isHover && btn.hoverBackground) ? btn.hoverBackground : btn.background;
            DrawTexture(tex, btnX, btnY);
            if (!draggingOtherPanel) {
                if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && isHover) {
                    g_activePanel = btn.targetPanel;
                }
            }
        }
    }
}

inline void EventHandle(void* espBase) {
    const int baseOffset = 84;
    const char* msg = (const char*)((BYTE*)espBase + baseOffset);
    struct Trigger {
        const char* prefix;
        void (*handler)(const char*);
    };
    static const Trigger triggers[] = {
        { "[KILL_NOTICE]",    updateStatusKill },
        { "[BALANCE_NOTICE]", updateStatusBalance },
        { "[ONLINE_NOTICE]",  updateStatusOnline }
    };
    for (const auto& t : triggers) {
        size_t len = strlen(t.prefix);
        if (strncmp(msg, t.prefix, len) == 0) {
            t.handler(msg + len);
            return;
        }
    }
}

auto f0x551B40 = 0x551B40;
auto u0x47DD8F = 0x47DD8F;
__declspec(naked) void naked_0x47DD8A() {
    __asm {
        pushad
        push ebx
        call UIHandler
        add esp, 4
        popad
        call f0x551B40
        jmp u0x47DD8F
    }
}

__declspec(naked) void naked_0x5F3740() {
    __asm {
        pushad
        mov eax, esp
        push eax
        call EventHandle
        add esp, 4
        popad
        ret
    }
}

void hook::online() {
    for (auto& [type, ui] : panels) {loadConfig(ui);}
    util::detour((void*)0x47DD8A, naked_0x47DD8A, 5);
    util::detour((void*)0x5F3740, naked_0x5F3740, 5);
}
