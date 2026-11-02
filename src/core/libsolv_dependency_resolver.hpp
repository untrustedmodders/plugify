#pragma once

#include "plugify/dependency_resolver.hpp"
#include "plugify/logger.hpp"

extern "C" {
#include <solv/pool.h>
#include <solv/queue.h>
#include <solv/repo.h>
#include <solv/solvable.h>
#include <solv/solver.h>
#include <solv/solverdebug.h>
#include <solv/transaction.h>
}

namespace plugify {
	/**
	 * @brief Libsolv-based implementation of the IDependencyResolver interface
	 *
	 * This class uses the libsolv library to perform dependency resolution,
	 * leveraging its capabilities to handle complex dependency graphs, version
	 * constraints, and conflict detection.
	 */
	class LibsolvDependencyResolver final : public IDependencyResolver {
		struct PoolDeleter {
			void operator()(Pool* p) const {
				if (p) {
					pool_free(p);
				}
			}
		};

		struct SolverDeleter {
			void operator()(Solver* s) const {
				if (s) {
					solver_free(s);
				}
			}
		};

		struct TransactionDeleter {
			void operator()(Transaction* t) const {
				if (t) {
					transaction_free(t);
				}
			}
		};

		struct QueueDeleter {
			void operator()(Queue* q) const {
				if (q) {
					queue_free(q);
				}
			}
		};

		// libsolv's Queue owns a heap buffer but is a plain struct itself, so it is
		// initialized and freed rather than constructed and destroyed. Wrapping it in
		// a unique_ptr would claim ownership of a stack address; this says what is
		// actually going on, once.
		struct ScopedQueue {
			Queue queue;

			ScopedQueue() {
				queue_init(&queue);
			}

			~ScopedQueue() {
				queue_free(&queue);
			}

			ScopedQueue(const ScopedQueue&) = delete;
			ScopedQueue& operator=(const ScopedQueue&) = delete;

			Queue* get() {
				return &queue;
			}

			Queue* operator->() {
				return &queue;
			}
		};

	public:
		LibsolvDependencyResolver(std::shared_ptr<ILogger> logger);
		~LibsolvDependencyResolver() override = default;

		/**
		 * Resolves dependencies for all extensions, producing a report with resolutions,
		 * conflicts, and load order.
		 * @return DependencyReport containing the resolution results
		 */
		ResolutionReport Resolve(std::span<const Extension> extensions) override;

	private:
		// Setup functions
		void InitializePool();
		void AddExtensionsToPool(std::span<const Extension> extensions);
		Id AddSolvable(const Manifest& manifest);
		void SetupDependencies(Id solvableId, const Manifest& manifest);
		void SetupConflicts(Id solvableId, const Manifest& manifest);
		void SetupObsoletes(Id solvableId, const Manifest& manifest);

		// Constraint conversion
		Id MakeDepConstraint(const std::string& name, const std::optional<Constraint>& constraints);
		std::vector<const Extension*> Matches(
			std::span<const Extension> extensions,
			const std::string& name,
			const std::optional<Constraint>& constraints,
			Id self = 0  // a solvable to leave out, none by default
		);

		// Resolution
		void Exclude(ResolutionReport& resolution, DependencyIssue issue);
		void ExcludeSupersededCopies(std::span<const Extension> extensions, ResolutionReport& resolution);
		void ExcludeConflictDeclarers(std::span<const Extension> extensions, ResolutionReport& resolution);
		void ExcludeObsoletedExtensions(std::span<const Extension> extensions, ResolutionReport& resolution);
		void ReportUnmetDependencies(std::span<const Extension> extensions, ResolutionReport& resolution);
		void RunSolver(ResolutionReport& resolution);
		void ProcessSolverProblems(Solver* solver, ResolutionReport& resolution);
		std::vector<std::string> ExtractSolutions(Solver* solver, Id problemId);
		void ComputeInstallationOrder(Transaction* trans, ResolutionReport& resolution);
		void ReportOrderCycles(Transaction* trans, ResolutionReport& resolution);

	private:
		std::shared_ptr<ILogger> _logger;
		std::unique_ptr<Pool, PoolDeleter> _pool;
		Repo* _repo = nullptr;
		std::unordered_map<UniqueId, Id> _extensionToSolvableId;
		std::unordered_map<Id, UniqueId> _solvableIdToExtension;
		std::unordered_set<UniqueId> _locked;
	};
}  // namespace plugify
