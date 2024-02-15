﻿#pragma once
#include "OsuStatic.h"
#include "String.h"
#include <string>
#include <istream>
#include <vector>

class OsuBeatmap
{
public:
	struct TimingPoint {
		double Time = 0;
		double BeatLength = 100;
		double SampleVolume = 0;
		EffectFlags Effects = EffectFlags::None;
		SampleBank SampleBank = SampleBank::Normal;
		int TimeSignature = 4;
		int SampleSet = 0;
		bool TimingChange = 0;

		double BPM() const {
			return 60000 / BeatLength;
		}

		double SpeedMultiplier() const {
			return BeatLength < 0 ? 100.0 / -BeatLength : 1;
		}
	};
	struct HitObject {
		std::string PathRecord;
		std::string CustomSampleFilename;
		std::string CustomSampleBanks;
		double X;
		double Y;
		double StartTime;
		double Length = 1.0 / 0.0 * 0.0;
		double EndTime;
		double CustomSampleVolume;
		int RepeatCount;
		HitObjectType Type;
		HitSoundType SoundType;
		HitObjectType GetHitobjectType()
		{
			return (HitObjectType)((int)Type & ((int)HitObjectType::Circle | (int)HitObjectType::Slider | (int)HitObjectType::Spinner | (int)HitObjectType::Hold));
		}
		void ResolveCustomSampleBanks()
		{
			auto args = split(CustomSampleBanks, ':');
			if (args.size() > 4)
			{
				CustomSampleVolume = std::stod(args[3]);
				if (CustomSampleVolume != 0)
					CustomSampleFilename = args[4];
			}
		}
	};
	struct StoryboardSoundSample {
		std::string path;
		double StartTime = 0;
	};

	std::vector<std::pair<double, double>> BreakPeriods;
	std::vector<TimingPoint> TimingPoints;
	std::vector<HitObject> HitObjects;
	std::vector<StoryboardSoundSample> StoryboardSamples;
	std::vector<std::tuple<std::string, std::string, std::string>> Others;
	std::string Title;
	std::string Artist;
	std::string TitleUnicode;
	std::string ArtistUnicode;
	std::string AudioFilename;
	std::string Version;
	std::string Creator;
	std::string Background;
	std::string Video;
	std::string Source;
	std::string Tags;
	std::string SampleSet;
	double StackLeniency = 0.7;
	double HPDrainRate = 0;
	double CircleSize = 0;
	double OverallDifficulty = 0;
	double ApproachRate = 0;
	double SliderMultiplier = 0;
	double SliderTickRate = 0;
	int VideoOffset = 0;
	int PreviewTime = 0;
	int Countdown = 0;
	GameMode Mode = GameMode::Std;
	static OsuBeatmap Parse(std::istream& sr);
};