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

	// Base interface
	template <typename T>
	class IStage {
	public:
		virtual ~IStage() = default;

		virtual std::string_view GetName() const = 0;

		virtual StageType GetType() const = 0;

		// Optional: setup/teardown
		virtual void Setup([[maybe_unused]] std::span<T> items) {
		}

		virtual void Teardown([[maybe_unused]] std::span<T> items) {
		}
	};

	// Transform stage - processes items in place
	template <typename T>
	class ITransformStage : public IStage<T> {
	public:
		StageType GetType() const override {
			return StageType::Transform;
		}

		// Process single item
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

		// Process all items
		// This allows the stage to reorder, filter, or add/remove items
		virtual Result<void> ProcessAll(std::vector<T>& items) = 0;
	};

	// Sequential stage - processes in container order
	template <typename T>
	class ISequentialStage : public IStage<T> {
	public:
		StageType GetType() const override {
			return StageType::Sequential;
		}

		// Process item with knowledge of its position
		virtual Result<void> ProcessItem(T& item, size_t position, size_t total) = 0;

		// Should we continue after an error?
		virtual bool ContinueOnError() const {
			return true;
		}

		// Optional: filter
		virtual bool ShouldProcess([[maybe_unused]] const T& item) const {
			return true;
		}
	};

}
