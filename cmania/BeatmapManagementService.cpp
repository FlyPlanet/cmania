#include <thread>
#include <string>
#include <vector>
#include <filesystem>
#include <iosfwd>
#include <fstream>
#include "DifficultyCalculator.h"
#include "BeatmapManagementService.h"
#include "Game.h"
#include "String.h"
#include "OsuBeatmap.h"

class BeatmapManagementService : public GameComponent {
	SongsCache sc;
	static void GetSongsCache(BeatmapManagementService* pthis) {
		auto song_path = pthis->parent->Settings["SongsPath"].GetString();
		if (song_path != 0 && std::filesystem::exists(song_path)) {
			SongsCahceReadyEventArgs screa{};
			auto songs_path = std::filesystem::path(song_path);
			std::fstream fs("Songs.bin", std::ios::in | std::ios::out | std::ios::binary);
			pthis->sc = SongsCache{};
			if (fs.good())
				Binary::Read(fs, pthis->sc);
			auto tm = std::filesystem::last_write_time(songs_path).time_since_epoch();
			if (tm > pthis->sc.lastchanged) {
				std::filesystem::directory_iterator dir(songs_path);
				std::vector<std::filesystem::directory_entry> dirs;
				std::move(dir, {}, std::insert_iterator(dirs, dirs.begin()));
				pthis->sc.caches.reserve(dirs.size() + 10);
				std::mutex lock;
				std::vector<std::thread> threads;
				for (size_t i = 0; i < 16; i++) {
					threads.push_back(std::thread([&]() {
						std::filesystem::directory_entry song;
						while (true) {
							{
								std::lock_guard<std::mutex> l(lock);
								if (dirs.size() == 0) {
									return;
								}
								auto end_iter = dirs.end();
								end_iter--;
								song = *end_iter;
								dirs.resize(dirs.size() - 1);
							}
							if (song.is_directory()) {
								SongsCacheEntry* disk_cache = 0;
								{
									std::lock_guard<std::mutex> l(lock);
									auto iter = std::find_if(pthis->sc.caches.begin(), pthis->sc.caches.end(), [&](SongsCacheEntry& dat) -> bool {
										return song.path() == dat.path;
									});
									if (iter != pthis->sc.caches.end())
										disk_cache = &*iter;
								}
								if (disk_cache != 0) {
									auto& ondisk = *disk_cache;
									auto now = song.last_write_time().time_since_epoch();
									if (now <= ondisk.lastchanged)
										continue;
									ondisk.lastchanged = now;
								}
								SongsCacheEntry cache{};
								cache.path = song.path().string();
								for (auto& diff : std::filesystem::directory_iterator(song)) {
									if (!diff.is_directory()) {
										if (EndsWith(diff.path().string(), ".osu")) {
											std::ifstream stm(diff.path());
											OsuBeatmap bmp = OsuBeatmap::Parse(stm);
											if (bmp.Mode != GameMode::Mania)
												continue;
											if (bmp.CircleSize < 1)
												continue;
											if (bmp.HitObjects.size() == 0)
												continue;
											cache.artist = bmp.Artist;
											cache.title = bmp.Title;
											cache.titleunicode = bmp.TitleUnicode;
											cache.artistunicode = bmp.ArtistUnicode;
											cache.tags = bmp.Tags;
											cache.source = bmp.Source;
											DifficultyCacheEntry dce{};
											dce.audio = diff.path().parent_path().append(bmp.AudioFilename).string();
											dce.background = diff.path().parent_path().append(bmp.Background).string();
											double first = bmp.HitObjects[0].StartTime;
											for (auto ho : bmp.HitObjects) {
												dce.length = std::max(ho.StartTime, dce.length);
											}
											dce.length -= first;
											dce.keys = bmp.CircleSize;
											dce.od = bmp.OverallDifficulty;
											dce.name = bmp.Version;
											dce.path = diff.path().string();
											dce.nps = bmp.HitObjects.size() / dce.length * 1000;
											dce.diff = CalculateDiff(bmp);
											dce.preview = bmp.PreviewTime;
											cache.difficulties.emplace_back(dce);
										}
									}
								}
								std::sort(cache.difficulties.begin(), cache.difficulties.end(), [](const auto& a, const auto& b) { return a.diff < b.diff; });
								if (!cache.difficulties.empty()) {
									std::lock_guard<std::mutex> l(lock);
									if (disk_cache == 0)
										pthis->sc.caches.push_back(cache);
									else
										*disk_cache = cache;
								}
							}
						}
					}));
				}
				for (auto& thd : threads) {
					thd.join();
				}
			}
			if (!fs.good()) {
				fs = std::fstream("Songs.bin", std::ios::out | std::ios::binary);
				if (!fs.good())
					throw std::invalid_argument("Cannot save to the Songs.bin");
			}
			pthis->sc.lastchanged = tm;
			fs.seekp(0);
			Binary::Write(fs, pthis->sc);
			fs.close();
			screa.Songs = &pthis->sc;
			pthis->parent->Raise("songs_cache_ready", screa);
			return;
		}
		pthis->parent->Raise("require", "SongsPath");
	}
	// ͨ�� Component �̳�
	virtual void ProcessEvent(const char* evt, const void* evtargs) {
		if (strcmp(evt, "get_songs_cache") == 0) {
			std::thread(GetSongsCache, this).detach();
		}
		if (strcmp(evt, "save_db") == 0) {
			auto fs = std::ofstream("Songs.bin", std::ios::out | std::ios::binary);
			fs.seekp(0);
			if (!fs.good())
				throw std::invalid_argument("Cannot save to the Songs.bin");
			Binary::Write(fs, sc);
		}
	}
};

GameComponent* MakeBeatmapManagementService() {
	return new BeatmapManagementService();
}
