#pragma once

namespace plugify {
	// ============================================================================
	// Stage Interfaces
	// ============================================================================
	enum class StageType {
		Transform,	 // Parallel processing, no order changes
		Barrier,	 // Can reorder/filter the container
		Sequential,	 // Processes in container order
	};

	template <typename T>
	class IStage {
	public:
		virtual ~IStage() = default;

		virtual std::string GetName() const = 0;

		virtual StageType GetType() const = 0;

		// Optional: setup/teardown. A stage that cannot set itself up cannot do its
		// job either, so this failing stops the pipeline.
		virtual Result<void> Setup([[maybe_unused]] std::span<T> items) {
			return {};
		}

		// Runs whether the stage did its work or not, and a failure here is recorded
		// rather than acted on: by this point the work has already happened.
		virtual Result<void> Teardown([[maybe_unused]] std::span<T> items) {
			return {};
		}
	};

	// Transform stage - processes items in place
	template <typename T>
	class ITransformStage : public IStage<T> {
	public:
		StageType GetType() const override {
			return StageType::Transform;
		}

		virtual Result<void> ProcessItem(T& item) = 0;

		// Optional: filter predicate
		virtual bool ShouldProcess([[maybe_unused]] const T& item) const {
			return true;
		}
	};

	// Barrier stage - can reorder and filter the container
	template <typename T>
	class IBarrierStage : public IStage<T> {
	public:
		StageType GetType() const override {
			return StageType::Barrier;
		}

		// This allows the stage to reorder, filter, or add/remove items.
		// Stages after a barrier may hold spans into the container or pointers at
		// single items, so this is the last point at which it may be reallocated: a
		// second barrier placed after them would invalidate what they kept.
		virtual Result<void> ProcessAll(std::vector<T>& items) = 0;
	};

	// Sequential stage - processes in container order
	template <typename T>
	class ISequentialStage : public IStage<T> {
	public:
		StageType GetType() const override {
			return StageType::Sequential;
		}

		virtual Result<void> ProcessItem(T& item) = 0;

		virtual bool ContinueOnError() const {
			return true;
		}

		// Optional: filter
		virtual bool ShouldProcess([[maybe_unused]] const T& item) const {
			return true;
		}
	};

}
