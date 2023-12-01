#pragma once
#include "AudioManager.h"
#include "HitObject.h"
struct TaikoObject : public HitObject {
	enum {
		Don = 0,
		Kat = 1,// 0λ
		Large = 2,// 1λ
		Spinner = 4,// 2λ
		Barline = 8,
		SliderTick = 16,
		Slider = 32,
	} ObjectType;
	double TickTime;
	double EndTime;
	int RemainsHits;
	int TotalHits;
	// �� 1 Ϊ��λ(
	double Velocity;
};