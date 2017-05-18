#pragma once

enum Mode
{
	MODE_WIREFRAME = 0x00000001,
	MODE_ORTHO = 0x00000002,
	MODE_LIGHTING = 0x00000004,
	MODE_SHADOW = 0x00000008,
	MODE_BACKGROUND = 0x00000010,

	MODE_DRAWAABBTREE = 0x00000100,
	MODE_DRAWPRIMBOUNDS = 0x00000200,
	MODE_TRANSPARENS = 0x00000400,
	MODE_FPS = 0x00000800,
};
