#include "core/libsolv_dependency_resolver.hpp"

#include "core/conflict_impl.hpp"
#include "core/dependency_impl.hpp"

using namespace plugify;

// ============================================================================
// Main Resolution Function
// ============================================================================

LibsolvDependencyResolver::LibsolvDependencyResolver(std::shared_ptr<ILogger> logger)
	: _logger(std::move(logger)) {
}

ResolutionReport LibsolvDependencyResolver::Resolve(std::span<const Extension> extensions) {
	ResolutionReport resolution;

	// Step 1: Initialize libsolv pool
	InitializePool();

	// Step 2: Add all extensions to the pool
	AddExtensionsToPool(extensions);

	// Step 3: Settle what the manifests decide between themselves
	ExcludeConflictDeclarers(extensions, resolution);
	ExcludeObsoletedExtensions(extensions, resolution);

	// Step 4: Run the solver
	RunSolver(resolution);

	return resolution;
}

// ============================================================================
// Pool Setup
// ============================================================================

static void debug_callback([[maybe_unused]] Pool* pool, void* data, int type, const char* str) {
	auto* logger = reinterpret_cast<ILogger*>(data);
	if (logger == nullptr) {
		return;
	}

	std::string_view sv(str);
	if (sv.ends_with('\n')) {
		sv.remove_suffix(1);
	}

	if (type & SOLV_FATAL) {
		logger->Log(sv, Severity::Fatal);
	} else if (type & SOLV_ERROR) {
		logger->Log(sv, Severity::Error);
	} else if (type & SOLV_WARN) {
		logger->Log(sv, Severity::Warning);
	} else {
		logger->Log(sv, Severity::Debug);
	}
}

void LibsolvDependencyResolver::InitializePool() {
	// Resolve can run again on the same resolver - the manager re-initializes on
	// reload - and everything below is keyed by ids that belong to the pool being
	// replaced. Carrying them over hands the new solver a solvable id from a freed
	// pool as soon as the extension count drops, which reads off the end of the
	// solvables array.
	_extensionToSolvableId.clear();
	_solvableIdToExtension.clear();
	_excluded.clear();

	_pool.reset(pool_create());

	pool_setdebugcallback(_pool.get(), debug_callback, _logger.get());

	constexpr int level = PLUGIFY_IS_DEBUG ? 3 : 1;
	pool_setdebuglevel(_pool.get(), level);

	// Create a repository for our extensions
	_repo = repo_create(_pool.get(), "installed");
}

void LibsolvDependencyResolver::AddExtensionsToPool(std::span<const Extension> extensions) {
	_extensionToSolvableId.reserve(extensions.size());
	_solvableIdToExtension.reserve(extensions.size());

	for (const auto& extension : extensions) {
		const auto& id = extension.GetId();
		const auto& manifest = extension.GetManifest();

		Id solvableId = AddSolvable(manifest);
		_extensionToSolvableId[id] = solvableId;
		_solvableIdToExtension[solvableId] = id;

		// Setup dependencies and conflicts
		SetupDependencies(solvableId, manifest);
		SetupConflicts(solvableId, manifest);
		SetupObsoletes(solvableId, manifest);
	}

	// Create provides for all extensions (self-provides)
	pool_createwhatprovides(_pool.get());
}

Id LibsolvDependencyResolver::AddSolvable(const Manifest& manifest) {
	Id solvableId = repo_add_solvable(_repo);
	Solvable* s = pool_id2solvable(_pool.get(), solvableId);

	// Set extension name
	s->name = pool_str2id(_pool.get(), manifest.name.c_str(), 1);

	// Set version
	auto versionStr = manifest.version.to_string();
	s->evr = pool_str2id(_pool.get(), versionStr.c_str(), 1);

	// Set vendor
	if (manifest.author) {
		s->vendor = pool_str2id(_pool.get(), manifest.author->c_str(), 1);
	}

	// Add self-provides
	Id provideDep = pool_rel2id(_pool.get(), s->name, s->evr, REL_EQ, 1);
	solvable_add_deparray(s, SOLVABLE_PROVIDES, provideDep, 0);

	return solvableId;
}

// ============================================================================
// Dependencies and Conflicts Setup
// ============================================================================

void LibsolvDependencyResolver::SetupDependencies(Id solvableId, const Manifest& manifest) {
	if (!manifest.dependencies || manifest.dependencies->empty()) {
		return;
	}

	Solvable* s = pool_id2solvable(_pool.get(), solvableId);

	for (const auto& dependency : *manifest.dependencies) {
		const auto& [depName, constraints, optional] = *dependency._impl;

		Id depId = MakeDepConstraint(depName, constraints);

		if (optional.value_or(false)) {
			// Optional dependencies go to RECOMMENDS
			solvable_add_deparray(s, SOLVABLE_RECOMMENDS, depId, 0);
		} else {
			// Required dependencies go to REQUIRES
			solvable_add_deparray(s, SOLVABLE_REQUIRES, depId, 0);
		}
	}
}

void LibsolvDependencyResolver::SetupConflicts(Id solvableId, const Manifest& manifest) {
	if (!manifest.conflicts || manifest.conflicts->empty()) {
		return;
	}

	Solvable* s = pool_id2solvable(_pool.get(), solvableId);

	for (const auto& conflict : *manifest.conflicts) {
		const auto& [conflictName, constraints, reason] = *conflict._impl;

		Id conflictId = MakeDepConstraint(conflictName, constraints);

		// Add as CONFLICTS
		solvable_add_deparray(s, SOLVABLE_CONFLICTS, conflictId, 0);
	}
}

void LibsolvDependencyResolver::SetupObsoletes(Id solvableId, const Manifest& manifest) {
	if (!manifest.obsoletes || manifest.obsoletes->empty()) {
		return;
	}

	Solvable* s = pool_id2solvable(_pool.get(), solvableId);

	for (const auto& obsolete : *manifest.obsoletes) {
		const auto& [obsoleteName, constraints, reason] = *obsolete._impl;

		Id obsoleteId = MakeDepConstraint(obsoleteName, constraints);

		// Add as OBSOLETES
		solvable_add_deparray(s, SOLVABLE_OBSOLETES, obsoleteId, 0);
	}
}

Id LibsolvDependencyResolver::MakeDepConstraint(
	const std::string& name,
	const std::optional<Constraint>& constraints
) {
	Id nameId = pool_str2id(_pool.get(), name.c_str(), 1);

	if (!constraints || constraints->empty()) {
		return nameId;	// No version constraint
	}

	// Handle multiple constraints (OR logic)
	Id resultDep = 0;

	for (const auto& constraint : *constraints) {
		// Handle multiple constraints (AND logic)
		Id innerDep = 0;

		for (const auto& comparator : constraint) {
			auto versionStr = comparator.get_version().to_string();
			Id evr = pool_str2id(_pool.get(), versionStr.c_str(), 1);

			int flags = ConvertComparison(comparator.get_operator());
			Id dep = pool_rel2id(_pool.get(), nameId, evr, flags, 1);

			if (innerDep == 0) {
				innerDep = dep;
			} else {
				// Combine with AND for multiple constraints
				innerDep = pool_rel2id(_pool.get(), innerDep, dep, REL_AND, 1);
			}
		}

		if (resultDep == 0) {
			resultDep = innerDep;
		} else {
			// Combine with OR for multiple constraints
			resultDep = pool_rel2id(_pool.get(), resultDep, innerDep, REL_OR, 1);
		}
	}

	return resultDep;
}

int LibsolvDependencyResolver::ConvertComparison(plg::detail::range_operator op) {
	switch (op) {
		case plg::detail::range_operator::less:
			return REL_LT;
		case plg::detail::range_operator::less_or_equal:
			return REL_LT | REL_EQ;
		case plg::detail::range_operator::equal:
			return REL_EQ;
		case plg::detail::range_operator::greater_or_equal:
			return REL_GT | REL_EQ;
		case plg::detail::range_operator::greater:
			return REL_GT;
		default:
			return 0;  // No version requirement
	}
}

// ============================================================================
// Solver Execution
// ============================================================================

// A `conflicts` entry is one-sided in intent - "do not load me together with X" -
// but libsolv knows only the symmetric kind. With both sides queued as hard
// installs the pair is simply unsolvable, and the partial transaction left behind
// holds whichever side the solver happened to decide on first: the arbitrary half
// of an unordered_map walk, so the extension that declared nothing can be the one
// that is dropped. Settle it before the solver sees the pair - the side that
// declared the conflict is the side that loses it - and the other one still loads.
void LibsolvDependencyResolver::ExcludeConflictDeclarers(
	std::span<const Extension> extensions,
	ResolutionReport& resolution
) {
	for (const auto& extension : extensions) {
		const auto& manifest = extension.GetManifest();
		if (!manifest.conflicts || manifest.conflicts->empty()) {
			continue;
		}

		Id solvableId = _extensionToSolvableId[extension.GetId()];

		for (const auto& conflict : *manifest.conflicts) {
			const auto& [conflictName, constraints, reason] = *conflict._impl;

			// Ids are interned, so the same name and constraint give back exactly
			// what SetupConflicts put into SOLVABLE_CONFLICTS.
			Id dep = MakeDepConstraint(conflictName, constraints);

			Id providers = pool_whatprovides(_pool.get(), dep);
			if (providers == 0) {
				continue;
			}

			for (Id* pp = _pool->whatprovidesdata + providers; *pp; ++pp) {
				// Conflicting with yourself is how an extension rules out a second
				// copy of itself; libsolv permits it and so do we.
				if (*pp == solvableId) {
					continue;
				}

				Solvable* other = pool_id2solvable(_pool.get(), *pp);

				DependencyIssue issue;
				issue.affectedExtension = extension.GetId();
				issue.problem = "Conflict";
				issue.description = std::format(
					"the manifest declares a conflict with '{}', which is installed as {} {}{}",
					pool_dep2str(_pool.get(), dep),
					pool_id2str(_pool.get(), other->name),
					pool_id2str(_pool.get(), other->evr),
					reason ? std::format(" ({})", *reason) : std::string()
				);

				if (auto it = _solvableIdToExtension.find(*pp); it != _solvableIdToExtension.end()) {
					issue.involvedExtension = it->second;
				}

				resolution.issues[extension.GetId()].push_back(std::move(issue));
				_excluded.insert(extension.GetId());
			}
		}
	}
}

// `obsoletes` says "I replace X", so the side that steps aside is X - the mirror
// of a conflict, where the side that declares it is the one that loses. libsolv
// can only act on an obsoletes against a package that is already installed, and
// nothing here ever is: no installed repo is ever set, so the entry degrades into
// a plain conflict and takes out both sides. Settle it here instead.
void LibsolvDependencyResolver::ExcludeObsoletedExtensions(
	std::span<const Extension> extensions,
	ResolutionReport& resolution
) {
	// Collected and applied in one go at the end: while the walk is running
	// `_excluded` holds the conflict losers only, so two manifests obsoleting each
	// other take each other out instead of one of them surviving by being walked
	// first.
	std::vector<UniqueId> obsoleted;

	for (const auto& extension : extensions) {
		const auto& manifest = extension.GetManifest();
		if (!manifest.obsoletes || manifest.obsoletes->empty()) {
			continue;
		}

		// Nothing steps aside for an extension that lost a conflict itself
		if (_excluded.contains(extension.GetId())) {
			continue;
		}

		Id solvableId = _extensionToSolvableId[extension.GetId()];

		for (const auto& obsolete : *manifest.obsoletes) {
			const auto& [obsoleteName, constraints, reason] = *obsolete._impl;

			Id dep = MakeDepConstraint(obsoleteName, constraints);

			Id providers = pool_whatprovides(_pool.get(), dep);
			if (providers == 0) {
				continue;
			}

			for (Id* pp = _pool->whatprovidesdata + providers; *pp; ++pp) {
				// Obsoleting your own name is how a manifest supersedes older copies of
				// itself, not a reason for this one to step aside
				if (*pp == solvableId) {
					continue;
				}

				auto it = _solvableIdToExtension.find(*pp);
				if (it == _solvableIdToExtension.end()) {
					continue;
				}

				DependencyIssue issue;
				issue.affectedExtension = it->second;
				issue.involvedExtension = extension.GetId();
				issue.problem = "Obsolete";
				issue.description = std::format(
					"replaced by {} {}, which obsoletes '{}'{}",
					manifest.name,
					manifest.version.to_string(),
					pool_dep2str(_pool.get(), dep),
					reason ? std::format(" ({})", *reason) : std::string()
				);

				resolution.issues[it->second].push_back(std::move(issue));
				obsoleted.push_back(it->second);
			}
		}
	}

	_excluded.insert(obsoleted.begin(), obsoleted.end());
}

void LibsolvDependencyResolver::RunSolver(ResolutionReport& resolution) {
	// Create solver
	std::unique_ptr<Solver, SolverDeleter> solver(solver_create(_pool.get()));

	// Set solver flags
	solver_set_flag(solver.get(), SOLVER_FLAG_ALLOW_DOWNGRADE, 1);
	solver_set_flag(solver.get(), SOLVER_FLAG_SPLITPROVIDES, 1);

	// Create job queue - install all extensions
	Queue queue;
	std::unique_ptr<Queue, QueueDeleter> jobs(&queue);
	queue_init(jobs.get());

	// Add all extensions as install jobs
	for (const auto& [extId, solvId] : _extensionToSolvableId) {
		if (_excluded.contains(extId)) {
			// Locked rather than merely left out of the install jobs: the solvable
			// stays in the pool, so a dependent would otherwise be free to pull the
			// declarer - and the conflict with it - back in, and the solver would go
			// back to dropping the other side.
			queue_push2(jobs.get(), SOLVER_SOLVABLE | SOLVER_LOCK, solvId);
			continue;
		}

		queue_push2(jobs.get(), SOLVER_SOLVABLE | SOLVER_INSTALL, solvId);
	}

	// Run the solver
	int ret = solver_solve(solver.get(), jobs.get());

	// Success - extract the solution
	std::unique_ptr<Transaction, TransactionDeleter> trans(solver_create_transaction(solver.get()));

	if (trans) {
		ComputeInstallationOrder(trans.get(), resolution);
	}

	if (ret != 0) {
		// Problems detected. Name every requirement that nothing provides first,
		// then let the solver speak for whatever is left.
		AddUnsatisfiedRequires(resolution);
		ProcessSolverProblems(solver.get(), resolution);
	}
}

// ============================================================================
// Problem Processing
// ============================================================================

// The solver reports one representative rule per problem, so an extension with
// several unmet requirements only ever names the first one - fix it, restart, and
// the next one shows up. The pool already knows what provides what, so walk the
// requirements against it and name all of them at once.
// "version 2.0.0", or "versions 2.0.0, 1.5.0", for whatever is installed under
// the name `dep` constrains - empty when that name is not installed at all. The
// pool already holds this, so the constraint is stripped off and the bare name
// looked up again rather than searching the extensions a second time.
std::string LibsolvDependencyResolver::DescribeInstalledVersions(Id dep) {
	// Descend to the plain name: a constraint is a relation around it, and an
	// AND/OR of several ranges is a relation around those.
	Id name = dep;
	while (ISRELDEP(name)) {
		name = GETRELDEP(_pool.get(), name)->name;
	}

	if (name == dep) {
		return {};	// no constraint to begin with, so the name itself is missing
	}

	Id providers = pool_whatprovides(_pool.get(), name);
	if (providers == 0 || _pool->whatprovidesdata[providers] == 0) {
		return {};
	}

	std::vector<std::string> versions;
	for (Id* pp = _pool->whatprovidesdata + providers; *pp; ++pp) {
		Solvable* provider = pool_id2solvable(_pool.get(), *pp);
		versions.emplace_back(pool_id2str(_pool.get(), provider->evr));
	}

	return std::format(
		"{} {}",
		versions.size() == 1 ? "version" : "versions",
		plg::join(versions, ", ")
	);
}

void LibsolvDependencyResolver::AddUnsatisfiedRequires(ResolutionReport& resolution) {
	for (const auto& [extId, solvId] : _extensionToSolvableId) {
		// Whatever made it into the load order is satisfied by definition, and a
		// conflict declarer already carries the reason it was left out
		if (_excluded.contains(extId)
			|| std::ranges::find(resolution.loadOrder, extId) != resolution.loadOrder.end()) {
			continue;
		}

		Solvable* s = pool_id2solvable(_pool.get(), solvId);
		if (!s->dep_requires) {
			continue;
		}

		Id req, *reqp = s->repo->idarraydata + s->dep_requires;
		while ((req = *reqp++) != 0) {
			// pool_whatprovides returns an offset into whatprovidesdata, which is
			// non-zero even when nothing provides the dependency: the list it
			// points at is what has to be empty.
			Id providers = pool_whatprovides(_pool.get(), req);
			if (providers != 0 && _pool->whatprovidesdata[providers] != 0) {
				continue;
			}

			DependencyIssue issue;
			issue.affectedExtension = extId;
			issue.problem = "Unmet dependency";

			// Tell "not installed" from "installed, wrong version": saying nothing
			// provides 'foo >= 2.1.2' reads as if foo were missing altogether.
			auto installed = DescribeInstalledVersions(req);
			if (installed.empty()) {
				issue.description = std::format(
					"nothing installed provides '{}'",
					pool_dep2str(_pool.get(), req)
				);
			} else {
				issue.description = std::format(
					"'{}' is not satisfied by the installed {}",
					pool_dep2str(_pool.get(), req),
					installed
				);
			}

			resolution.issues[extId].push_back(std::move(issue));
		}
	}
}

void LibsolvDependencyResolver::ProcessSolverProblems(Solver* solver, ResolutionReport& resolution) {
	Id problemCount = static_cast<Id>(solver_problem_count(solver));

	for (Id problemId = 1; problemId <= problemCount; problemId++) {
		Id probr = solver_findproblemrule(solver, problemId);

		Id source, target, dep;
		SolverRuleinfo type = solver_ruleinfo(solver, probr, &source, &target, &dep);

		// Create issue for this problem
		DependencyIssue issue;
		issue.problem = "Dependency problem";
		//issue.isBlocking = true; // All problems from solver are blocking

		// Determine affected extensions
		if (source) {
			auto it = _solvableIdToExtension.find(source);
			if (it != _solvableIdToExtension.end()) {
				issue.affectedExtension = it->second;
			}
		}

		if (issue.affectedExtension == -1) {
			continue;
		}

		// Unmet requirements are already spelled out one by one; what the solver
		// adds on top of them is about conflicts, cycles and version constraints.
		if (auto it = resolution.issues.find(issue.affectedExtension);
			it != resolution.issues.end() && !it->second.empty()) {
			continue;
		}

		if (target) {
			auto it = _solvableIdToExtension.find(target);
			if (it != _solvableIdToExtension.end()) {
				issue.involvedExtension = it->second;
			}
		}

		// Get all problem rules for more detailed analysis
		issue.description = solver_problemruleinfo2str(solver, type, source, target, dep);

		// Extract solutions for this problem
		issue.suggestedFixes = ExtractSolutions(solver, problemId);

		// Add issue to report
		resolution.issues[issue.affectedExtension].push_back(std::move(issue));
	}
}

std::vector<std::string> LibsolvDependencyResolver::ExtractSolutions(Solver* solver, Id problemId) {
	// Get solution count for this problem
	Id solutionCount = static_cast<Id>(solver_solution_count(solver, problemId));

	std::vector<std::string> fixes;
	fixes.reserve(static_cast<size_t>(solutionCount));

	for (Id solutionId = 1; solutionId <= solutionCount; solutionId++) {
		std::vector<std::string> solutionSteps;

		// Iterate through solution elements
		Id element = 0;
		Id p, rp;

		while ((element = solver_next_solutionelement(solver, problemId, solutionId, element, &p, &rp)) != 0) {
			solutionSteps.emplace_back(solver_solutionelement2str(solver, p, rp));
		}

		// Combine solution steps
		if (!solutionSteps.empty()) {
			fixes.emplace_back(std::format("Solution {}: {}", solutionId, plg::join(solutionSteps, ", ")));
		} else {
			fixes.emplace_back(std::format("Solution {}: Adjust installation requirements", solutionId));
		}
	}

	// If no solutions found, add generic suggestions
	if (fixes.empty()) {
		fixes.emplace_back("Check extension availability");
		fixes.emplace_back("Review version constraints");
		fixes.emplace_back("Consider removing conflicting extensions");
	}

	// Add fixes to the last issue in report
	return fixes;
}

// ============================================================================
// Installation Order (Topological Sort)
// ============================================================================

void LibsolvDependencyResolver::ComputeInstallationOrder(Transaction* trans, ResolutionReport& resolution) {
	// Get installation order from transaction
	transaction_order(trans, SOLVER_TRANSACTION_KEEP_ORDERCYCLES);

	size_t count = static_cast<size_t>(trans->steps.count);
	resolution.loadOrder.reserve(count);
	resolution.dependencyGraph.reserve(count);
	resolution.reverseDependencyGraph.reserve(count);

	// Get installed extensions from transaction
	for (size_t i = 0; i < count; i++) {
		Id p = trans->steps.elements[i];

		// Check if this is an installation step
		/*Id type = transaction_type(trans, p, SOLVER_TRANSACTION_SHOW_ALL);
		std::cout << "Transaction step: " << i << " - Solvable ID: " << p << " - Type: " << type <<
		std::endl; if (type == SOLVER_TRANSACTION_INSTALL || type == SOLVER_TRANSACTION_REINSTALL ||
			type == SOLVER_TRANSACTION_DOWNGRADE ||
			type == SOLVER_TRANSACTION_UPGRADE)*/
		{
			auto extIt = _solvableIdToExtension.find(p);
			if (extIt != _solvableIdToExtension.end()) {
				const UniqueId& extId = extIt->second;
				resolution.loadOrder.push_back(extId);

				// Build dependency graph
				Solvable* s = pool_id2solvable(_pool.get(), p);

				// Process requires
				if (s->dep_requires) {
					Id req, *reqp;
					reqp = s->repo->idarraydata + s->dep_requires;
					while ((req = *reqp++) != 0) {
						// Find what provides this requirement using pool_whatprovides
						Id providers = pool_whatprovides(_pool.get(), req);
						if (providers) {
							Id* pp;
							for (pp = _pool->whatprovidesdata + providers; *pp; pp++) {
								Id providerId = *pp;
								auto depIt = _solvableIdToExtension.find(providerId);
								if (depIt != _solvableIdToExtension.end() && providerId != p) {
									const UniqueId& depId = depIt->second;
									resolution.dependencyGraph[extId].push_back(depId);
									resolution.reverseDependencyGraph[depId].push_back(extId);
								}
							}
						}
					}
				}
			}
		}
	}

	resolution.isLoadOrderValid = true;	 // Since libsolv provides a valid transaction order

	ReportOrderCycles(trans, resolution);
}

// libsolv breaks an order cycle on its own and then keeps quiet about it, so one
// member ends up started before the dependency it asked for and nobody is told.
// Every member is installed - only the order between them cannot be honoured - so
// this is a warning rather than a reason to keep them out, and the chain is what
// the manifest author needs in order to fix it.
void LibsolvDependencyResolver::ReportOrderCycles(Transaction* trans, ResolutionReport& resolution) {
	Queue cycleQueue, memberQueue;
	std::unique_ptr<Queue, QueueDeleter> cycles(&cycleQueue), members(&memberQueue);
	queue_init(cycles.get());
	queue_init(members.get());

	// Below NORMAL the edge that was broken is not a requirement in the first place
	transaction_order_get_cycleids(trans, cycles.get(), SOLVER_ORDERCYCLE_NORMAL);

	for (int i = 0; i < cycles->count; ++i) {
		transaction_order_get_cycle(trans, cycles->elements[i], members.get());
		if (members->count == 0) {
			continue;
		}

		std::vector<std::string_view> names;
		names.reserve(static_cast<size_t>(members->count));
		for (int j = 0; j < members->count; ++j) {
			Solvable* s = pool_id2solvable(_pool.get(), members->elements[j]);
			names.emplace_back(pool_id2str(_pool.get(), s->name));
		}

		// Closed back on the first name, so that the chain reads as a cycle
		auto chain = std::format("{} -> {}", plg::join(names, " -> "), names.front());

		_logger->Log(
			std::format("Dependency cycle: {}; the order between them is arbitrary", chain),
			Severity::Warning
		);

		for (int j = 0; j < members->count; ++j) {
			auto it = _solvableIdToExtension.find(members->elements[j]);
			if (it == _solvableIdToExtension.end()) {
				continue;
			}

			DependencyIssue issue;
			issue.affectedExtension = it->second;
			issue.problem = "Dependency cycle";
			issue.description = std::format("{}; the order between them is arbitrary", chain);
			issue.isBlocking = false;
			resolution.issues[it->second].push_back(std::move(issue));
		}
	}
}
