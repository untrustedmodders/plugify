#pragma once

#include <glaze/thread/threadpool.hpp>

#include "core/stages.hpp"

namespace plugify {
	// ============================================================================
	// Pipeline Implementation
	// ============================================================================

	// Stage statistics
	struct StageStatistics {
		size_t itemsIn = 0;
		size_t itemsOut = 0;
		size_t succeeded = 0;
		size_t failed = 0;
		std::chrono::milliseconds elapsed{ 0 };
		bool stopped = false;
		std::vector<std::pair<std::string, std::string>> errors;
	};

	// What one stage did. The name is copied rather than viewed: a report is a
	// value and outlives nothing in particular, while the title it is printed
	// under is only assembled when something actually prints it.
	struct StageReport {
		std::string name;
		StageType type;
		StageStatistics stats;

		std::string Title() const {
			return std::format("{} [{}]", name, plg::enum_to_string(type));
		}
	};

	template <typename T>
	class Pipeline {
		struct StageEntry {
			std::unique_ptr<IStage<T>> stage;
			bool required;
		};

	public:
		struct Report {
			std::vector<StageReport> stages;
			std::chrono::milliseconds totalTime{ 0 };
			size_t initialItems = 0;
			size_t finalItems = 0;

			std::string Summary() const {
				std::string buffer;
				auto it = std::back_inserter(buffer);

				std::format_to(it, "\n=== Pipeline Report ===\n");
				std::format_to(it, "Items: {} -> {} ({} total)\n", initialItems, finalItems, totalTime);

				for (size_t i = 0; i < stages.size(); ++i) {
					const auto& [name, type, stats] = stages[i];
					std::format_to(
						it,
						"  Stage {} ({}): {} -> {} items, {} succeeded, {} failed ({})\n",
						i,
						stages[i].Title(),
						stats.itemsIn,
						stats.itemsOut,
						stats.succeeded,
						stats.failed,
						stats.elapsed
					);

					size_t maxWidth = 0;
					for (const auto& [key, _] : stats.errors) {
						maxWidth = std::max(maxWidth, key.size());
					}

					for (const auto& [key, value] : stats.errors) {
						std::format_to(it, "    {:<{}} : {}\n", key, maxWidth, value);
					}
				}

				return buffer;
			}

			std::string Error() const {
				std::string buffer;
				auto it = std::back_inserter(buffer);

				// A stage that stopped has nothing to say through its failure count: with
				// no items to process that count is zero, so ask it directly
				size_t totalFailed = 0;
				bool anyStopped = false;
				for (const auto& stage : stages) {
					totalFailed += stage.stats.failed;
					anyStopped |= stage.stats.stopped;
				}

				if (totalFailed == 0 && !anyStopped) {
					return "No errors occurred.";
				}

				std::format_to(it, "\n=== Pipeline Failed ===\n");
				if (totalFailed != 0) {
					std::format_to(it, "{} item(s) failed to process:\n\n", totalFailed);
				}

				// Only show stages that had failures
				for (const auto& stage : stages) {
					if (stage.stats.failed > 0 || !stage.stats.errors.empty()) {
						if (stage.stats.failed > 0) {
							std::format_to(it, "Stage '{}': {} failed\n", stage.Title(), stage.stats.failed);
						} else {
							std::format_to(it, "Stage '{}':\n", stage.Title());
						}

						// Show actual errors with cleaner formatting
						for (const auto& [item, error] : stage.stats.errors) {
							std::format_to(it, "  • {}: {}\n", item, error);
						}
						std::format_to(it, "\n");
					}
				}

				return buffer;
			}
		};

		// Builder pattern for pipeline construction
		class Builder {
			friend class Pipeline;

			std::vector<StageEntry> _stages;
			size_t _threadPoolSize = std::thread::hardware_concurrency();

		public:
			template <typename StageType>
			Builder& AddStage(std::unique_ptr<StageType> stage, bool required = true) {
				static_assert(std::is_base_of_v<IStage<T>, StageType>);
				_stages.emplace_back(std::move(stage), required);
				return *this;
			}

			Builder& WithThreadPoolSize(size_t size) {
				_threadPoolSize = size;
				return *this;
			}

			std::unique_ptr<Pipeline> Build() {
				return std::unique_ptr<Pipeline>(new Pipeline(std::move(*this)));
			}
		};

		static Builder Create() {
			return Builder{};
		}

	private:
		Pipeline(Builder&& builder)
			: _stages(std::move(builder._stages))
			, _pool(builder._threadPoolSize) {
		}

	public:
		// Execute pipeline - container is modified in place
		Report Execute(std::vector<T>& items) {
			Report report;
			report.stages.reserve(_stages.size());
			report.initialItems = items.size();

			auto pipelineStart = std::chrono::steady_clock::now();

			// Execute each stage
			for (const auto& [stage, required] : _stages) {
				auto stats = ExecuteStage(stage.get(), items);

				// Items that failed are the normal outcome of a stage doing its work:
				// each one carries its own reason and the stages after this one still
				// have work to do for everything else. Only a stage that could not run
				// at all leaves nothing to continue with.
				bool stopped = stats.stopped && required;
				report.stages.emplace_back(
					stage->GetName(),
					stage->GetType(),
					std::move(stats)
				);

				if (stopped) {
					break;
				}
			}

			report.finalItems = items.size();
			report.totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - pipelineStart
			);

			return report;
		}

	private:
		StageStatistics
		ExecuteStage(IStage<T>* stage, std::vector<T>& items) {
			StageStatistics stats;
			stats.itemsIn = items.size();
			auto startTime = std::chrono::steady_clock::now();

			// Setup. A stage that could not set itself up cannot do its job either,
			// which is what stops the pipeline.
			if (auto result = Guarded([&] { return stage->Setup(items); })) {
				// Execute based on stage type
				switch (stage->GetType()) {
					case StageType::Transform:
						ExecuteTransform(static_cast<ITransformStage<T>*>(stage), items, stats);
						break;

					case StageType::Barrier:
						ExecuteBarrier(static_cast<IBarrierStage<T>*>(stage), items, stats);
						break;

					case StageType::Sequential:
						ExecuteSequential(static_cast<ISequentialStage<T>*>(stage), items, stats);
						break;
				}
			} else {
				stats.failed = items.size();
				stats.stopped = true;
				stats.errors.emplace_back(stage->GetName(), std::move(result.error()));
			}

			// Teardown whatever happened above: a stage may have taken something in
			// Setup that it has to give back.
			if (auto result = Guarded([&] { return stage->Teardown(items); }); !result) {
				stats.errors.emplace_back(stage->GetName(), std::move(result.error()));
			}

			stats.itemsOut = items.size();
			stats.elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - startTime
			);

			return stats;
		}

		void ExecuteTransform(
			ITransformStage<T>* stage,
			std::vector<T>& items,
			StageStatistics& stats
		) {
			// Process items in parallel
			std::atomic<size_t> succeeded{ 0 };
			std::atomic<size_t> failed{ 0 };
			std::mutex errorMutex;

			for (auto& item : items) {
				_pool.emplace_back([&] {
					if (!stage->ShouldProcess(item)) {
						return;
					}

					if (auto result = Guarded([&] { return stage->ProcessItem(item); })) {
						succeeded.fetch_add(1, std::memory_order_relaxed);
					} else {
						failed.fetch_add(1, std::memory_order_relaxed);
						{
							std::lock_guard lock(errorMutex);
							stats.errors.emplace_back(GetItemName(item), std::move(result.error()));
						}
					}
				});
			}

			_pool.wait();

			stats.succeeded = succeeded.load();
			stats.failed = failed.load();
		}

		void ExecuteBarrier(
			IBarrierStage<T>* stage,
			std::vector<T>& items,
			StageStatistics& stats
		) {
			size_t originalSize = items.size();

			// Process all items together
			if (auto result = Guarded([&] { return stage->ProcessAll(items); })) {
				stats.succeeded = originalSize;
				stats.failed = 0;
			} else {
				stats.succeeded = 0;
				stats.failed = originalSize;
				stats.stopped = true;
				stats.errors.emplace_back(stage->GetName(), std::move(result.error()));
			}
		}

		void ExecuteSequential(
			ISequentialStage<T>* stage,
			std::vector<T>& items,
			StageStatistics& stats
		) {
			bool continueOnError = stage->ContinueOnError();

			for (size_t i = 0; i < items.size(); ++i) {
				auto& item = items[i];

				if (!stage->ShouldProcess(item)) {
					continue;
				}

				// Stop if error occurred and stage doesn't continue on error
				if (stats.failed > 0 && !continueOnError) {
					break;
				}

				if (auto result = Guarded([&] { return stage->ProcessItem(item); })) {
					++stats.succeeded;
				} else {
					++stats.failed;
					stats.errors.emplace_back(GetItemName(item), std::move(result.error()));
				}
			}
		}

		// A stage is third-party code as far as the pipeline is concerned: glaze and
		// valijson while parsing, libsolv while resolving, a language module after
		// that. Whatever any of them throws has to come back as a failure, because
		// Result is the only channel the pipeline reports through - and because the
		// one place that would otherwise catch it, the thread pool, puts it in a
		// future that nobody keeps.
		template <typename Fn>
		static Result<void> Guarded(Fn&& fn) noexcept {
			try {
				if constexpr (std::is_void_v<std::invoke_result_t<Fn>>) {
					fn();
					return {};
				} else {
					return fn();
				}
			} catch (const std::bad_alloc&) {
				return MakeError("out of memory");
			} catch (const std::exception& e) {
				return MakeError("unhandled exception: {}", e.what());
			} catch (...) {
				return MakeError("unhandled exception");
			}
		}

		static std::string GetItemName(const T& item) {
			if constexpr (requires { item.GetName(); }) {
				return item.GetName();
			} else if constexpr (requires { item.name; }) {
				return item.name;
			} else {
				return "item";
			}
		}

	private:
		std::vector<StageEntry> _stages;
		glz::pool _pool;
	};
}
