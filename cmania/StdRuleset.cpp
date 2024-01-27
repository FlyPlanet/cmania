//#pragma once
//#include <filesystem>
//#include "Ruleset.h"
//#include "StdObject.h"
//#include "StdScoreProcessor.h"
//#include "BassAudioManager.h"
//#include "Animator.h"
//#include "OsuBeatmap.h"
//#include "OsuSample.h"
//#include <set>
//#include "File.h"
//#include "ConsoleInput.h"
//#include "StdRuleset.h"
//
//class StdRuleset : public Ruleset<StdObject> {
//	std::vector<Animator<PowerEasingFunction<1.5>>> KeyHighlight;
//	Animator<PowerEasingFunction<4.0>> LastHitResultAnimator{ 255, 0, 400 };
//	HitResult LastHitResult = HitResult::None;
//	AudioStream bgm;
//	double endtime = 0;
//	std::string skin_path;
//	OsuBeatmap orig_bmp;
//	std::filesystem::path parent_path;
//	double offset = 0;
//	double first_obj = 1e300;
//	double end_obj = -1e300;
//	double resume_time = -1e300;
//	bool no_hs = false;
//	double last_rec = 0;
//	double miss_offset = 200;
//	double cs = 0;
//	double ar = 0;
//
//public:
//	StdRuleset() {
//		RulesetScoreProcessor = new StdScoreProcessor();
//	}
//
//private:
//	// ͨ�� Ruleset �̳�
//	void LoadSettings(BinaryStorage& settings) override {
//		offset = settings["Offset"].Get<double>();
//		no_hs = settings["NoBmpHs"].Get<bool>();
//		const char* skinpath = settings["SkinPath"].GetString();
//		if (skinpath == 0 || !std::filesystem::exists(skinpath)) {
//			skinpath = "Samples/Triangles";
//		}
//		skin_path = skinpath;
//	}
//	void Load(std::filesystem::path beatmap_path) override {
//		auto am = GetBassAudioManager();
//		std::filesystem::path parent = beatmap_path.parent_path();
//		parent_path = parent;
//		{
//			auto ifs = std::ifstream(beatmap_path);
//			if (!ifs.good())
//				throw std::invalid_argument("Cannot open file.");
//			orig_bmp = OsuBeatmap::Parse(ifs);
//			RulesetRecord.BeatmapTitle = orig_bmp.Title;
//			RulesetRecord.BeatmapVersion = orig_bmp.Version;
//			RulesetRecord.BeatmapHash = 0;
//		}
//		cs = orig_bmp.CircleSize;
//		ar = orig_bmp.ApproachRate;
//		std::set<std::string> Samples;
//
//		Samples > AddRangeSet(orig_bmp.HitObjects > Select([](const auto& ho) -> std::string { return ho.CustomSampleFilename; }) > Where([](const auto& str) -> bool { return !str.empty(); }) > Select([&](const auto& str) -> auto { return (parent / str).string(); })) > AddRangeSet(
//																																																																				  orig_bmp.StoryboardSamples > Select([&](const auto& item) -> auto { return (parent / item.path).string(); }));
//
//		auto SampleIndex = BuildSampleIndex(parent, 1);		   // ���������������(sampleset==1Ĭ��)
//		auto SkinSampleIndex = BuildSampleIndex(skin_path, 0); // ����Ƥ����������(sampleset==0)
//
//		auto selector = [](const AudioSampleMetadata& md) -> auto {
//			return md.filename.string();
//		}; // linq ��ѯ
//
//		Samples > AddRangeSet(Select(SampleIndex, selector))  // ����������·��
//			> AddRangeSet(Select(SkinSampleIndex, selector)); // ���Ƥ������·��
//
//		std::map<std::string, AudioSample> SampleCaches; // ��������
//
//		Samples > ForEach([&](const std::string& path) {
//			try {
//				auto dat = ReadAllBytes(path);
//				SampleCaches[path] = AudioSample(am->loadSample(dat.data(), dat.size()));
//			}
//			catch (...) {
//			}
//		});
//		Beatmap > AddRange(Select(
//					  orig_bmp.HitObjects, [&](const OsuBeatmap::HitObject& obj) -> auto {
//						  StdObject so{};
//						  so.StartTime = obj.StartTime;
//						  so.Location = { obj.X, obj.Y };
//						  auto tp = GetTimingPointTiming(orig_bmp, obj.StartTime);
//						  auto tp2 = GetTimingPointNonTiming(orig_bmp, obj.StartTime);
//
//						  auto scoringdist = BASE_SCORING_DISTANCE * orig_bmp.SliderMultiplier * tp2.SpeedMultiplier();
//
//						  auto velocity = scoringdist / tp.BeatLength;
//						  auto tickdist = scoringdist / orig_bmp.SliderTickRate;
//						  if (!obj.PathRecord.empty()) {
//							  SliderPath sp = SliderPath::Parse(obj.PathRecord, { obj.X, obj.Y }, obj.Length);
//							  so.Path = (new SliderPath(sp));
//							  so.EndTime = obj.StartTime + obj.RepeatCount * sp.actualLength / velocity;
//							  so.Velocity = velocity;
//							  bool rev = false;
//							  double time = obj.StartTime;
//							  for (size_t i = 0; i < obj.RepeatCount; i++) {
//								  if (tickdist > 20)
//									  for (double j = tickdist; j < sp.actualLength; j += tickdist) {
//										  Event evt{};
//										  evt.EventType = Event::Tick;
//										  auto prog = j / sp.actualLength;
//										  evt.Location = sp.PositionAt(prog);
//										  evt.StartTime = time + j / velocity;
//										  so.Events.push_back(evt);
//									  }
//								  Event evt2{};
//								  evt2.EventType = Event::Repeat;
//								  time += sp.actualLength / velocity;
//								  evt2.StartTime = time;
//								  evt2.Location = rev ? PointD{ obj.X, obj.Y } : PointD(sp.PositionAt(1));
//								  so.Events.push_back(evt2);
//							  }
//							  so.RepeatCount = obj.RepeatCount;
//						  }
//
//						  endtime = std::max(endtime, std::max(obj.EndTime, obj.StartTime));
//
//						  first_obj = std::min(first_obj, obj.StartTime);
//						  end_obj = std::max(end_obj, obj.StartTime);
//						  if (so.EndTime != 0)
//							  end_obj = std::max(end_obj, so.EndTime);
//						  return so;
//					  }));
//		// ����bgm
//		{
//			auto dat = ReadAllBytes((parent / orig_bmp.AudioFilename).string());
//			bgm = AudioStream(am->load(dat.data(), dat.size()));
//		}
//
//		if (RulesetInputHandler == 0)
//			throw std::invalid_argument("RulesetInputHandler mustn't be nullptr.");
//
//		RulesetInputHandler->SetBinds(std::vector<int>({ (int)ConsoleKey::Z, (int)ConsoleKey::X }));
//
//		RulesetRecord.Mods = Mods;
//		RulesetRecord.RatingGraph.resize(((end_obj - first_obj) + 11000) / 100);
//
//		RulesetScoreProcessor->RulesetRecord = &RulesetRecord;
//		RulesetScoreProcessor->SetDifficulty(orig_bmp.OverallDifficulty);
//		RulesetScoreProcessor->SetMods(Mods);
//
//		RulesetScoreProcessor->ApplyBeatmap(1 * GetPlaybackRate(Mods));
//
//		miss_offset = GetHitRanges(orig_bmp.OverallDifficulty)[HitResult::Meh];
//
//		Clock.SetRate(GetPlaybackRate(Mods));
//		Clock.Offset(std::min(first_obj - 5000, -3000.0)); // �������ʱ��׼������
//		Clock.Start();									   // ��ʼHpet��ʱ��
//
//		RulesetInputHandler->SetClockSource(Clock);
//
//		GameStarted = true;
//	}
//	Record GetAutoplayRecord() override {
//		return {};
//	}
//	struct Rect {
//		int x1, y1, x2, y2;
//	};
//
//	Rect calcViewport(int width, int height, int viewportWidth, int viewportHeight) {
//		float aspectRatio = (float)width / height;
//		float viewportAspectRatio = (float)viewportWidth / viewportHeight;
//
//		Rect rect;
//
//		if (aspectRatio > viewportAspectRatio) {
//			// ��С���
//			rect.x1 = 0;
//			rect.x2 = viewportWidth;
//			rect.y2 = height * (viewportWidth / (float)width);
//			rect.y1 = (viewportHeight - rect.y2) / 2;
//		}
//		else if (aspectRatio < viewportAspectRatio) {
//			// ��С�߶�
//			rect.y1 = 0;
//			rect.y2 = viewportHeight;
//			rect.x2 = width * (viewportHeight / (float)height);
//			rect.x1 = (viewportWidth - rect.x2) / 2;
//		}
//		else {
//			// ����Ҫ����
//			rect.x1 = 0;
//			rect.y1 = 0;
//			rect.x2 = viewportWidth;
//			rect.y2 = viewportHeight;
//		}
//
//		return rect;
//	}
//	struct Trail {
//		PointD loc{};
//		double time = -1e300;
//	};
//	constexpr static auto trail_count = 64;
//	Trail cursortrail[trail_count];
//	double lastrec = -1e300;
//	void Render(GameBuffer& buf) override { // 512 , 384 osu pixel
//		const auto rt = 2;
//		auto vp = calcViewport(512 * rt, 384, buf.Width - 4, buf.Height - 4);
//		vp.x1 += 2;
//		vp.y1 += 2;
//		buf.FillRect(vp.x1, vp.y1, vp.x1 + vp.x2, vp.y1 + vp.y2, { {}, { 60, 0, 0, 0 }, ' ' });
//		auto preempt = DifficultyPreempt(ar);
//		auto fadein = DifficultyFadeIn(preempt);
//		preempt *= 0.8;
//		fadein *= 0.8;
//		auto scale = DifficultyScale(cs) * vp.y2 / 384.0 * 128;
//		auto t = Clock.Elapsed();
//		for (auto& ho : Beatmap) {
//			if (ho.EndTime == 0) {
//				if (t > ho.StartTime + 200)
//					continue;
//			}
//			else {
//				if (t > ho.EndTime + 200)
//					continue;
//			}
//
//			if (ho.EndTime != 0 && ho.Path == 0) {
//				// ת��
//				continue;
//			}
//			Color comboclr{ 255, 120, 160, 240 };
//			if (ho.Path != 0) {
//				comboclr = { 255, 255, 0, 0 };
//			}
//			// Slider
//			if (ho.Path != 0 && t > ho.StartTime - preempt) {
//				// ��Ⱦ������...
//				//auto points = ho.Path->calcedPath;
//				//for (int i = 0; i < points.size() - 1; i++) {
//				//	PointD currentPoint = points[i];
//				//	PointD nextPoint = points[(i + 1) % points.size()];
//
//				//	double dx = nextPoint.X - currentPoint.X;
//				//	double dy = nextPoint.Y - currentPoint.Y;
//				//	double distance = std::sqrt(dx * dx + dy * dy);
//
//				//	float stepX = static_cast<float>(dx) / distance;
//				//	float stepY = static_cast<float>(dy) / distance;
//
//				//	for (int j = 0; j < distance; j++) {
//				//		double x = currentPoint.X + static_cast<int>(stepX * j);
//				//		double y = currentPoint.Y + static_cast<int>(stepY * j);
//
//				//		buf.FillCircle(vp.x1 + x / 512 * vp.x2, vp.y1 + y / 384 * vp.y2,scale,rt+1, { {}, { 10, 255, 255, 255 }, ' ' });
//				//	}
//				//}
//				if (t < ho.EndTime && t > ho.StartTime) {
//					auto duration = ho.Path->actualLength / ho.Velocity;
//					auto progress = std::clamp(fmod(t - ho.StartTime, duration * 2) / duration, 0.0, 2.0);
//					if (progress > 1)
//						progress = 2 - progress;
//					auto sb = ho.Path->PositionAt(progress);
//					buf.FillCircle(vp.x1 + sb.X / 512.0 * vp.x2, vp.y1 + sb.Y / 384.0 * vp.y2, scale, rt, { {}, (comboclr * 0.8), ' ' });
//				}
//			}
//			// HeadCircle
//			if (t > ho.StartTime - preempt) {
//				auto alpha = (t - (ho.StartTime - preempt)) / (preempt - fadein);
//				if (t > ho.StartTime - fadein) {
//					alpha = 1.0;
//				}
//				if (ho.Path != 0) {
//					if (t > ho.StartTime)
//						continue;
//				}
//				buf.FillCircle(vp.x1 + ho.Location.X / 512.0 * vp.x2, vp.y1 + ho.Location.Y / 384.0 * vp.y2, scale, rt, { {}, (comboclr * (alpha * 0.8)), ' ' });
//				// buf.DrawCircle(vp.x1 + ho.Location.X / 512.0 * vp.x2, vp.y1 + ho.Location.Y / 384.0 * vp.y2, scale, 1.5, rt + 1, { {}, (Color{ 255, 255, 255, 255 } * alpha), ' ' });
//				// ��Ȧ
//				auto progress = std::max(1 + (1 - (t - (ho.StartTime - preempt)) / preempt) * 3, 1.0);
//				buf.DrawCircle(vp.x1 + ho.Location.X / 512.0 * vp.x2, vp.y1 + ho.Location.Y / 384.0 * vp.y2, scale * progress, 0.5, rt, { {}, (Color{ 255, 255, 255, 255 } * alpha), ' ' });
//			}
//		}
//		auto evt = RulesetInputHandler->PollEvent();
//		auto mpos = RulesetInputHandler->GetMousePosition();
//		if (evt.has_value()) {
//			// Process it...
//		}
//		auto last = cursortrail[0];
//		//for (size_t i = 1; i < trail_count; i++) {
//		//	auto anim = std::clamp(1 - (t - cursortrail[i].time) / 500.0, 0.0, 1.0);
//		//	if (anim <= 0.01)
//		//		continue;
//		//	if ((cursortrail[i].loc - last.loc).SquaredLength() < 4)
//		//		continue;
//		//	auto clr = Color{ 160, 255, 255, 255 } * anim;
//		//	buf.DrawLine(last.loc.X, cursortrail[i].loc.X, last.loc.Y, cursortrail[i].loc.Y, { {}, clr, ' ' });
//		//	last = cursortrail[i];
//		//}
//		if (t > lastrec + 20) {
//			const auto t2 = trail_count - 1;
//			Trail buff[t2];
//			std::memcpy(buff, cursortrail, t2 * sizeof(Trail));
//			std::memcpy(cursortrail + 1, buff, t2 * sizeof(Trail));
//			cursortrail[0] = { { (double)std::get<0>(mpos), (double)std::get<1>(mpos) }, t };
//			lastrec = t;
//		}
//		if (scale <= 8) {
//			buf.SetPixel(std::get<0>(mpos), std::get<1>(mpos), { {}, { 255, 255, 255, 255 }, ' ' });
//		}
//		else
//		{
//			buf.FillCircle(std::get<0>(mpos), std::get<1>(mpos), scale / 4.0, rt, { {}, { 255, 255, 255, 255 }, ' ' });
//		}
//	}
//	// ͨ�� Ruleset �̳�
//	virtual void Pause() override {
//		for (auto& light : KeyHighlight) {
//			light.Reset();
//		}
//		bgm->pause(true);
//		Clock.Stop();
//		for (auto& ho : Beatmap) {
//			if (ho.ssample_stream != 0)
//				ho.ssample_stream->stop();
//			if (ho.ssamplew_stream != 0)
//				ho.ssamplew_stream->stop();
//		}
//	}
//	virtual void Resume() override {
//		resume_time = Clock.Elapsed();
//		Clock.Offset(-3000);
//		Clock.Start();
//	}
//	virtual void Skip() override {
//		if (first_obj - 3000 > 1000 && Clock.Elapsed() < first_obj - 3000) {
//			Clock.Reset();
//			Clock.Offset(first_obj - 3000);
//		}
//	}
//
//	// ͨ�� Ruleset �̳�
//	virtual double GetCurrentTime() override {
//		return Clock.Elapsed() - first_obj;
//	}
//	virtual double GetDuration() override {
//		return end_obj - first_obj;
//	}
//
//	// ͨ�� Ruleset �̳�
//	virtual std::string GetBgPath() override {
//		return (parent_path / orig_bmp.Background).string();
//	}
//
//	// ͨ�� Ruleset �̳�
//	void Update() override {
//		if (!GameStarted)
//			return;
//		if (GameEnded)
//			return;
//		auto time = Clock.Elapsed();
//
//		if (bgm != 0 && (time > bgm->getDuration() * 1000 + 3000 || time > end_obj + 3000)) {
//			GameEnded = true;
//			Clock.Stop();
//			return;
//		}
//
//		if (time > first_obj) {
//			RulesetRecord.RatingGraph[(time - first_obj) / 100] = RulesetScoreProcessor->Rating;
//		}
//
//		if (time < resume_time || !Clock.Running())
//			return;
//
//		if (bgm != 0 && time > -30 && time < bgm->getDuration() * 1000 - 3000) {
//			if (!bgm->isPlaying()) {
//				if (!bgm->isPaused()) // ��������һЩС��������Ƶ��Clock����ͬ��
//				{
//					Clock.Stop();
//					bgm->setPlaybackRate(Clock.ClockRate());
//					bgm->play();
//					while (bgm->getCurrent() < 0.003) {
//					}
//					Clock.Reset();
//					Clock.Offset(bgm->getCurrent() * 1000);
//					Clock.Start();
//				}
//				else {
//					bgm->pause(false);
//				}
//			}
//			else {
//				auto err = time - bgm->getCurrent() * 1000;
//				if (std::abs(err) > 150) {
//					bgm->setCurrent(time / 1000);
//				}
//			}
//		}
//	}
//};
//
//RulesetBase* MakeStdRuleset() {
//	return new StdRuleset();
//}
