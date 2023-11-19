#pragma once
#include "ScoreProcessor.h"
#include "Linq.h"
#include "Defines.h"
#include "TaikoObject.h"

class TaikoScoreProcessor : public ScoreProcessor<TaikoObject> {
	bool wt_mode = false;
	std::map<HitResult, double> hit_ranges;
	double reference_rating = 1;
	double pp_m = 1;
	double score_m = 1;
	int bonus_score = 0;

public:
	virtual void ApplyBeatmap(double ref_rating) override {
		reference_rating = pow(ref_rating, 3);
	}
	virtual void SetMods(OsuMods mods) override {
		score_m = GetModScale(mods);
		mods = RemoveFlag(mods, OsuMods::HalfTime);
		mods = RemoveFlag(mods, OsuMods::Nightcore);
		pp_m = GetModScale(mods);
	}
	void SetWtMode(bool enable) {
		wt_mode = enable;
	}
	virtual void SetDifficulty(double od) override {
		hit_ranges = GetHitRanges(od);
	}
	TaikoScoreProcessor() {
		ResultCounter[HitResult::Great];
		ResultCounter[HitResult::Good];
		ResultCounter[HitResult::Miss];
		hit_ranges = GetHitRanges(0);
	}
	// ͨ�� ScoreProcessor �̳�
	virtual HitResult ApplyHit(TaikoObject& to, double err) override {
		auto large = HasFlag(to.ObjectType, TaikoObject::Large);
		auto tick = HasFlag(to.ObjectType, TaikoObject::SliderTick);
		auto is_hold = to.EndTime != 0;
		if (to.HasHit && !is_hold) {
			// need more code there.

			return HitResult::None;
		}
		HitResult res = HitResult::None;
		if (std::isnan(err)) {
			res = HitResult::Miss;
		}
		else {
			ResultCounter > ForEach([&](const auto& item) {
				if (std::abs(err) < hit_ranges[item.first]) {
					res = item.first;
				}
			});
		}
		if (tick) {
			to.HasHit = true;
			bonus_score += 300;
			UpdateScore();
			return HitResult::None;
		}
		if (res > HitResult::None) {
			if (!is_hold && to.RemainsHits == 0 && large)
			{
				to.RemainsHits--;
			}
			else
			{
				to.HasHit = true;
			}

			Combo++;
			if (res == HitResult::Miss)
				Combo = 0;
			AppliedHit++;
			ResultCounter[res]++;
			RawScore += GetBaseScore(res);
			RawAccuracy += std::min(GetBaseScore(res), GetBaseScore(HitResult::Great));
			if (res != HitResult::Miss) {
				RawError += err;
				Errors.push_back(err);
			}
			UpdateScore();
		}
		return res;
	}
	void UpdateScore() {

		MaxCombo = std::max(Combo, MaxCombo);

		Accuracy = (double)RawAccuracy / AppliedHit / GetBaseScore(HitResult::Great);

		Rating = reference_rating * std::pow(((double)MaxCombo / BeatmapMaxCombo), 0.3) * pow(Accuracy, 1.3) * pow(pp_m, 1.2) * pow(0.95, ResultCounter[HitResult::Miss]);
		Mean = (double)RawError / Errors.size();
		Error = variance(Mean, Errors);
		Score = (((double)RawScore / BeatmapMaxCombo / GetBaseScore(HitResult::Perfect)) * 0.7 + ((double)MaxCombo / BeatmapMaxCombo) * 0.3) * score_m + bonus_score / 1000000.0;
	}
	virtual double GetHealthIncreaseFor(HitResult res) override {
		return 0.0;
	}

	// ͨ�� ScoreProcessor �̳�
	virtual void SaveRecord() override {
		RulesetRecord->Rating = Rating;
		RulesetRecord->Mean = Mean;
		RulesetRecord->Error = Error;
		RulesetRecord->Score = Score;
		RulesetRecord->Accuracy = Accuracy;
		RulesetRecord->ResultCounter = ResultCounter;
		RulesetRecord->MaxCombo = MaxCombo;
		RulesetRecord->BeatmapMaxCombo = BeatmapMaxCombo;
	}
};