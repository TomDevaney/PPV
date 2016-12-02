#pragma once

enum AnimType
{
	LOOP = 0,
	RETURN_DEFAULT,
	RETURN_LAST,
	RUN_ONCE,
};

enum BlendType
{
	CUR_GO_NEXT_GO = 0,
	CUR_GO_NEXT_STOP,
	CUR_STOP_NEXT_GO,
	CUR_STOP_NEXT_STOP,
	DEFAULT,
	POP_NEXT,
};

struct BlendInfo
{
	BlendType blendStyle;
	float totalBlendTime;
};