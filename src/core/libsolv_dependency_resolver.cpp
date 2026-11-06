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
	ExcludeSupersededCopies(extensions, resolution);
	ExcludeConflictDeclarers(extensions, resolution);
	ExcludeObsoletedExtensions(extensions, resolution);
	ReportUnmetDependencies(extensions, resolution);

	// Step 4: Run the solver for what is left
	RunSolver(resolution);

	return resolution;
}

// ============================================================================
// Helpers
// ============================================================================

// The version in the form libsolv has to compare, as opposed to the form it has
// to print. Semver and rpm read the same string differently in two places, and
// both are fixable in the string itself:
//  - `-` is rpm's release separator, and a version that carries a release outranks
//    one that does not, so `1.0.0-beta` would come out newer than `1.0.0`. rpm's
//    `~` goes the other way - it sorts before anything at all, the end of the
//    string included - which is what a prerelease means in semver.
//  - build metadata takes no part in semver precedence, so it is dropped instead
//    of being compared: `1.0.0+build1` has to satisfy `=1.0.0`.
// What is left over is rpm's reading of a prerelease identifier: it splits
// `beta2` into `beta` and `2`, and ranks a numeric identifier above an
// alphabetic one where semver ranks it below.
static std::string MatchEvr(const Version& version) {
	auto evr = std::format("{}.{}.{}", version.major(), version.minor(), version.patch());

	const auto& prerelease = version.prerelease_tag();
	if (!prerelease.empty()) {
		evr += '~';
		evr += prerelease;
	}

	return evr;
}

static int ConvertComparison(plg::detail::range_operator op) {
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
	}

	// Every operator range_operator has is covered above, and there is no sane
	// fallback: 0 would not read as "no version requirement" here, it would read
	// as "matches nothing", so a new one must not slip through silently.
	PLUGIFY_UNREACHABLE();
}

// A dependency as its manifest spells it. The pool renders it from the rewritten
// form that version comparison needs, which is not what the author wrote.
static std::string DescribeDependency(
	const std::string& name,
	const std::optional<Constraint>& constraints
) {
	std::string text = name;

	if (constraints && !constraints->empty()) {
		text += ' ';
		text += constraints->to_string();
	}

	return text;
}

// "version 2.0.0", or "versions 2.0.0, 1.5.0", for everything installed under
// this name - empty when nothing answers to it at all, which is a different
// message. Read from the manifests, since the pool holds the rewritten form that
// version comparison needs rather than the version as written. Two extensions
// under one name is a broken install rather than a thing to support, but nothing
// upstream of here rejects it, so say what is actually there.
static std::string DescribeInstalledVersions(
	std::span<const Extension> extensions,
	const std::string& name
) {
	std::vector<std::string_view> versions;

	for (const auto& extension : extensions) {
		if (extension.GetName() == name) {
			versions.emplace_back(extension.GetVersionString());
		}
	}

	if (versions.empty()) {
		return {};
	}

	return std::format(
		"{} {}",
		versions.size() == 1 ? "version" : "versions",
		plg::join(versions, ", ")
	);
}

// The solver composes its own text out of the comparable version form, where a
// `~` stands for the `-` the manifest wrote. Neither an extension name nor a
// semver version can hold a `~` of its own, so putting it back is unambiguous.
static std::string SpelledAsWritten(const char* text) {
	std::string spelled(text);
	std::ranges::replace(spelled, '~', '-');
	return spelled;
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
	_locked.clear();

	_pool.reset(pool_create());

	pool_setdebugcallback(_pool.get(), debug_callback, _logger.get());

	// Errors and fatals reach the callback whatever the level is, so this only
	// decides how much of the solver's own trace comes with them. Level 3 is a
	// few hundred logged lines per resolve, which is a lot to pay to have it on
	// by default even in a debug build.
	pool_setdebuglevel(_pool.get(), PLUGIFY_IS_DEBUG);

	// Create a repository for our extensions. Not libsolv's "installed" repo: that
	// one is set with pool_set_installed and would turn every job into an upgrade.
	_repo = repo_create(_pool.get(), "extensions");
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

	// Set version. Not as the manifest writes it: with several extensions under one
	// name the solver picks between them by comparing evrs directly, so this has to
	// be the comparable form as well. Every version printed to the user is taken
	// from the manifest instead.
	auto versionStr = MatchEvr(manifest.version);
	s->evr = pool_str2id(_pool.get(), versionStr.c_str(), 1);

	// Set vendor
	if (manifest.author) {
		s->vendor = pool_str2id(_pool.get(), manifest.author->c_str(), 1);
	}

	// Add self-provides: this is what a versioned dependency is matched against,
	// since a solvable that has provides is never matched on its evr
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
			auto versionStr = MatchEvr(comparator.get_version());
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

// Every installed extension that answers to this name and version range, other
// than `self`. Three passes ask the same question of the pool - who is here that
// this entry is about - and this is that question.
std::vector<const Extension*> LibsolvDependencyResolver::Matches(
	std::span<const Extension> extensions,
	const std::string& name,
	const std::optional<Constraint>& constraints,
	Id self
) {
	std::vector<const Extension*> matches;

	// Interned, so this asks the pool exactly what the Setup functions put into the
	// solvable's deparray
	Id dep = MakeDepConstraint(name, constraints);

	// pool_whatprovides answers with an offset that is non-zero even when nothing
	// provides the dependency: the list it points at is what has to be empty.
	Id providers = pool_whatprovides(_pool.get(), dep);
	if (providers == 0) {
		return matches;
	}

	for (Id* pp = _pool->whatprovidesdata + providers; *pp; ++pp) {
		// A manifest naming its own name means other copies of itself, not this one;
		// libsolv permits that and so do we
		if (*pp == self) {
			continue;
		}

		auto found = _solvableIdToExtension.find(*pp);
		if (found == _solvableIdToExtension.end()) {
			continue;
		}

		auto it = std::ranges::find(extensions, found->second, &Extension::GetId);
		if (it != extensions.end()) {
			matches.push_back(&(*it));
		}
	}

	return matches;
}

// ============================================================================
// Settling What the Manifests Decide
// ============================================================================

// An extension that is out and the reason it is out always travel together
void LibsolvDependencyResolver::Exclude(ResolutionReport& resolution, DependencyIssue issue) {
	_locked.insert(issue.affectedExtension);
	resolution.issues[issue.affectedExtension].push_back(std::move(issue));
}

// Several copies of one extension in the extensions directory is a thing that
// happens by itself - the old folder left behind by an update - and the solver
// answers it with "cannot install both", which is true and of no use. One name is
// one extension, so the newest version of it loads and the older copies step
// aside. The comparison is plg::version's own: this choice is ours to make rather
// than the pool's, so it follows semver instead of rpm, and a release outranks
// its own prereleases.
void LibsolvDependencyResolver::ExcludeSupersededCopies(
	std::span<const Extension> extensions,
	ResolutionReport& resolution
) {
	std::unordered_map<std::string_view, const Extension*> newest;
	newest.reserve(extensions.size());

	for (const auto& extension : extensions) {
		// Equal versions leave nothing to choose by, so the first one discovered
		// keeps the name and the outcome is at least stable
		auto [it, inserted] = newest.try_emplace(extension.GetName(), &extension);
		if (!inserted && extension.GetVersion() > it->second->GetVersion()) {
			it->second = &extension;
		}
	}

	// A second walk, so that every copy is told about the one that actually won
	// rather than about whichever was ahead when it was reached
	for (const auto& extension : extensions) {
		const Extension* kept = newest[extension.GetName()];
		if (kept == &extension) {
			continue;
		}

		DependencyIssue issue;
		issue.affectedExtension = extension.GetId();
		issue.involvedExtension = kept->GetId();
		issue.problem = "Superseded";
		issue.description = std::format(
			"another copy of '{}' is installed as version {}, and that is the one that loads",
			extension.GetName(),
			kept->GetVersionString()
		);

		Exclude(resolution, std::move(issue));
	}
}

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

			for (const Extension* other : Matches(extensions, conflictName, constraints, solvableId)) {
				DependencyIssue issue;
				issue.affectedExtension = extension.GetId();
				issue.involvedExtension = other->GetId();
				issue.problem = "Conflict";
				issue.description = std::format(
					"the manifest declares a conflict with '{}', which is installed as {} {}{}",
					DescribeDependency(conflictName, constraints),
					other->GetName(),
					other->GetVersionString(),
					reason ? std::format(" ({})", *reason) : ""
				);

				Exclude(resolution, std::move(issue));
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
	// Taken before the walk: only the conflict losers count as already out, so that
	// two manifests obsoleting each other take each other out instead of one of
	// them surviving by being walked first.
	const auto lostAConflict = _locked;

	for (const auto& extension : extensions) {
		const auto& manifest = extension.GetManifest();
		if (!manifest.obsoletes || manifest.obsoletes->empty()) {
			continue;
		}

		// Nothing steps aside for an extension that lost a conflict itself
		if (lostAConflict.contains(extension.GetId())) {
			continue;
		}

		Id solvableId = _extensionToSolvableId[extension.GetId()];

		for (const auto& obsolete : *manifest.obsoletes) {
			const auto& [obsoleteName, constraints, reason] = *obsolete._impl;

			for (const Extension* other : Matches(extensions, obsoleteName, constraints, solvableId)) {
				DependencyIssue issue;
				issue.affectedExtension = other->GetId();
				issue.involvedExtension = extension.GetId();
				issue.problem = "Obsolete";
				issue.description = std::format(
					"replaced by {} {}, which obsoletes '{}'{}",
					extension.GetName(),
					extension.GetVersionString(),
					DescribeDependency(obsoleteName, constraints),
					reason ? std::format(" ({})", *reason) : ""
				);

				Exclude(resolution, std::move(issue));
			}
		}
	}
}

// The solver reports one representative rule per problem, so an extension with
// several unmet requirements would only ever name the first one - fix it, restart,
// and the next one shows up. A requirement is a plain question to the pool though,
// and the answer does not depend on the solve: a requirement nothing provides
// cannot be met, and an extension that has one cannot reach the load order. So
// walk the manifests and name all of them at once, before solving.
void LibsolvDependencyResolver::ReportUnmetDependencies(
	std::span<const Extension> extensions,
	ResolutionReport& resolution
) {
	for (const auto& extension : extensions) {
		const auto& manifest = extension.GetManifest();
		if (!manifest.dependencies || manifest.dependencies->empty()) {
			continue;
		}

		// An extension that is already out carries the reason it is out; its
		// optional dependencies are still worth a word.
		const auto extId = extension.GetId();
		const bool settled = _locked.contains(extId);

		for (const auto& dependency : *manifest.dependencies) {
			const auto& [depName, constraints, optional] = *dependency._impl;

			// An unmet optional dependency keeps nothing from starting, so it is a
			// note on what will be missing rather than a reason
			const bool isOptional = optional.value_or(false);
			if (settled && !isOptional) {
				continue;
			}

			auto providers = Matches(extensions, depName, constraints);

			// A provider that is here but not going to load - a superseded copy, or
			// one that lost a conflict - does not answer the requirement
			const bool satisfied = std::ranges::any_of(providers, [this](const Extension* provider) {
				return !_locked.contains(provider->GetId());
			});
			if (satisfied) {
				continue;
			}

			DependencyIssue issue;
			issue.affectedExtension = extId;
			issue.problem = isOptional ? "Optional dependency" : "Unmet dependency";
			issue.isBlocking = !isOptional;

			// Three different stories, and the one about a version is the easiest to
			// tell wrongly: saying nothing provides 'foo >=2.1.2' reads as if foo
			// were missing altogether.
			if (auto wanted = DescribeDependency(depName, constraints); !providers.empty()) {
				issue.description = std::format(
					"'{}' is satisfied only by a copy that is not loading",
					wanted
				);
			} else if (auto installed = DescribeInstalledVersions(extensions, depName); !installed.empty()) {
				issue.description = std::format(
					"'{}' is not satisfied by the installed {}",
					wanted,
					installed
				);
			} else {
				issue.description = std::format("nothing installed provides '{}'", wanted);
			}

			resolution.issues[issue.affectedExtension].push_back(std::move(issue));
		}
	}
}

// ============================================================================
// Solver Execution
// ============================================================================

void LibsolvDependencyResolver::RunSolver(ResolutionReport& resolution) {
	// Create solver
	std::unique_ptr<Solver, SolverDeleter> solver(solver_create(_pool.get()));

	// Create job queue - install all extensions
	ScopedQueue jobs;

	// Add all extensions as install jobs
	for (const auto& [extId, solvId] : _extensionToSolvableId) {
		if (_locked.contains(extId)) {
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
		// Unmet requirements are already named one by one; what the solver adds on
		// top of them is about conflicts, cycles and competing copies.
		ProcessSolverProblems(solver.get(), resolution);
	}
}

// ============================================================================
// Problem Processing
// ============================================================================

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

		if (target) {
			auto it = _solvableIdToExtension.find(target);
			if (it != _solvableIdToExtension.end()) {
				issue.involvedExtension = it->second;
			}
		}

		// A problem has two sides, and the representative rule puts one of them in
		// `source` - not the one that was left out, necessarily. Two copies of the
		// same extension are reported against the copy that won. An issue on an
		// extension that did reach the load order is never read back, so the side
		// that needs the reason would be left holding nothing but "ruled out by
		// something else".
		if (issue.involvedExtension
			&& (!issue.affectedExtension
				|| (std::ranges::contains(resolution.loadOrder, issue.affectedExtension)
					&& !std::ranges::contains(resolution.loadOrder, issue.involvedExtension)))) {
			std::swap(issue.affectedExtension, issue.involvedExtension);
		}

		if (!issue.affectedExtension) {
			continue;
		}

		// Unmet requirements are already spelled out one by one; what the solver
		// adds on top of them is about conflicts, cycles and version constraints.
		// A note about an optional dependency is not a reason and does not count.
		if (auto it = resolution.issues.find(issue.affectedExtension);
			it != resolution.issues.end()
			&& std::ranges::any_of(it->second, &DependencyIssue::isBlocking)) {
			continue;
		}

		// Get all problem rules for more detailed analysis
		issue.description = SpelledAsWritten(solver_problemruleinfo2str(solver, type, source, target, dep));

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
			solutionSteps.emplace_back(SpelledAsWritten(solver_solutionelement2str(solver, p, rp)));
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

									// Two requirements can name the same extension, and
									// both graphs have to stay in step, so the edge goes
									// in once or not at all.
									auto& deps = resolution.dependencyGraph[extId];
									if (!std::ranges::contains(deps, depId)) {
										deps.push_back(depId);
										resolution.reverseDependencyGraph[depId].push_back(extId);
									}
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
	ScopedQueue cycles, members;

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
			resolution.issues[issue.affectedExtension].push_back(std::move(issue));
		}
	}
}
