#pragma once

static const unsigned render_address_tga = 0x57B860;
RenderFunc render_tga = reinterpret_cast<RenderFunc>(render_address_tga);
using RenderFunc = void(__cdecl*)(const char*);
extern RenderFunc render_tga;
