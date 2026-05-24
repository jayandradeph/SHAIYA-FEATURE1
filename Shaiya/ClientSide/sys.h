#pragma once

static const unsigned render_address_tga = 0x57B860;
RenderFunc render_tga = reinterpret_cast<RenderFunc>(render_address_tga);
using RenderFunc = void(__cdecl*)(const char*);
extern RenderFunc render_tga;

extern char feed_button[16];
extern char hide_button[16];
extern char balance_button[16];
extern char online_button[16];

extern char feed_button_hover[16];
extern char hide_button_hover[16];
extern char balance_button_hover[16];
extern char online_button_hover[16];

auto icon_loadbar_AoL = "loadbar_AoL.tga";
char loadbar_AoL[16];

auto icon_loadbar_UoF = "loadbar_UoF.tga";
char loadbar_UoF[16];

auto icon_online_background = "online_background.tga";
char online_background[16];

auto icon_killfeed_background = "killfeed_background.tga";
char killfeed_background[16];

auto icon_balance_background = "balance_background.tga";
char balance_background[16];

auto icon_toolbar_background = "toolbar_background.tga";
char toolbar_background[16];

auto icon_hide_button = "hide_button.tga";
char hide_button[16];

auto icon_feed_button = "feed_button.tga";
char feed_button[16];

auto icon_balance_button = "balance_button.tga";
char balance_button[16];

auto icon_online_button = "online_button.tga";
char online_button[16];

auto icon_hide_button_hover = "hide_button_hover.tga";
char hide_button_hover[16];

auto icon_feed_button_hover = "feed_button_hover.tga";
char feed_button_hover[16];

auto icon_balance_button_hover = "balance_button_hover.tga";
char balance_button_hover[16];

auto icon_online_button_hover = "online_button_hover.tga";
char online_button_hover[16];
