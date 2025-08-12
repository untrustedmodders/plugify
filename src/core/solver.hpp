// libsolv C++ Wrappers - Modern C++20 RAII Interface
// ============================================================================

#pragma once

// libsolv C headers
extern "C" {
#include <pool.h>
#include <repo.h>
#include <solver.h>
#include <transaction.h>
#include <selection.h>
#include <solverdebug.h>
#include <repo_rpmdb.h>
#include <repo_solv.h>
#include <repo_write.h>
#include <repo_repomdxml.h>
#include <repo_rpmmd.h>
#include <repo_susetags.h>
#include <repo_comps.h>
#include <repo_appdata.h>
#include <repo_updateinfoxml.h>
#include <repo_deltainfoxml.h>
#include <repo_products.h>
#include <repo_autopattern.h>
#include <solv_xfopen.h>
#include <testcase.h>
#include <chksum.h>
}

namespace solv {

// ============================================================================
// Forward Declarations
// ============================================================================

class Pool;
class Repo;
class Solvable;
class Solver;
class Transaction;
class Queue;
class Selection;
class Dataiterator;
class Chksum;

// ============================================================================
// Exception Types
// ============================================================================

class SolvException : public std::runtime_error {
public:
    explicit SolvException(const std::string& what) : std::runtime_error(what) {}
};

class PoolException : public SolvException {
public:
    explicit PoolException(const std::string& what) : SolvException("Pool: " + what) {}
};

class SolverException : public SolvException {
public:
    explicit SolverException(const std::string& what) : SolvException("Solver: " + what) {}
};

class RepoException : public SolvException {
public:
    explicit RepoException(const std::string& what) : SolvException("Repo: " + what) {}
};

// ============================================================================
// Enums and Types
// ============================================================================

enum class DistType {
    Unknown,
    RPM,
    DEB,
    ARCH,
    HAIKU
};

enum class SelectionFlags {
    None = 0,
    Name = SELECTION_NAME,
    Provides = SELECTION_PROVIDES,
    FileList = SELECTION_FILELIST,
    Canon = SELECTION_CANON,
    DotArch = SELECTION_DOTARCH,
    Rel = SELECTION_REL,
    Installed = SELECTION_INSTALLED_ONLY,
    Glob = SELECTION_GLOB,
    Flat = SELECTION_FLAT,
    NoCase = SELECTION_NOCASE,
    SourceOnly = SELECTION_SOURCE_ONLY,
    WithSource = SELECTION_WITH_SOURCE
};

inline SelectionFlags operator|(SelectionFlags a, SelectionFlags b) {
    return static_cast<SelectionFlags>(static_cast<int>(a) | static_cast<int>(b));
}

enum class SolverFlag {
    AllowDowngrade = SOLVER_FLAG_ALLOW_DOWNGRADE,
    AllowArchChange = SOLVER_FLAG_ALLOW_ARCHCHANGE,
    AllowVendorChange = SOLVER_FLAG_ALLOW_VENDORCHANGE,
    AllowNameChange = SOLVER_FLAG_ALLOW_NAMECHANGE,
    AllowUninstall = SOLVER_FLAG_ALLOW_UNINSTALL,
    NoUpdateProvide = SOLVER_FLAG_NO_UPDATEPROVIDE,
    SplitProvides = SOLVER_FLAG_SPLITPROVIDES,
    IgnoreRecommended = SOLVER_FLAG_IGNORE_RECOMMENDED,
    AddAlreadyRecommended = SOLVER_FLAG_ADD_ALREADY_RECOMMENDED,
    NoInferarch = SOLVER_FLAG_NO_INFARCHCHECK,
    KeepExplicitObsoletes = SOLVER_FLAG_KEEP_EXPLICIT_OBSOLETES,
    BestObeyPolicy = SOLVER_FLAG_BEST_OBEY_POLICY,
    NoAutoTarget = SOLVER_FLAG_NO_AUTOTARGET,
    DupAllowDowngrade = SOLVER_FLAG_DUP_ALLOW_DOWNGRADE,
    DupAllowArchChange = SOLVER_FLAG_DUP_ALLOW_ARCHCHANGE,
    DupAllowVendorChange = SOLVER_FLAG_DUP_ALLOW_VENDORCHANGE,
    DupAllowNameChange = SOLVER_FLAG_DUP_ALLOW_NAMECHANGE,
    KeepOrphans = SOLVER_FLAG_KEEP_ORPHANS,
    BreakOrphans = SOLVER_FLAG_BREAK_ORPHANS,
    FocusInstalled = SOLVER_FLAG_FOCUS_INSTALLED,
    YumObsoletes = SOLVER_FLAG_YUM_OBSOLETES
};

/*enum class TransactionMode {
    Nothing = 0,
    All = SOLVER_TRANSACTION_IGNORE,
    Obsoletes = SOLVER_TRANSACTION_SHOW_OBSOLETES,
    Multiversion = SOLVER_TRANSACTION_SHOW_MULTIINSTALL,
    Active = SOLVER_TRANSACTION_SHOW_ACTIVE
};*/

enum class JobCommand {
    Noop = SOLVER_NOOP,
    Install = SOLVER_INSTALL,
    Erase = SOLVER_ERASE,
    Update = SOLVER_UPDATE,
    WeakenDeps = SOLVER_WEAKENDEPS,
    MultiVersion = SOLVER_MULTIVERSION,
    Lock = SOLVER_LOCK,
    DistUpgrade = SOLVER_DISTUPGRADE,
    Verify = SOLVER_VERIFY,
    DropOrphaned = SOLVER_DROP_ORPHANED,
    UserInstalled = SOLVER_USERINSTALLED,
    AllowUninstall = SOLVER_ALLOWUNINSTALL,
    FavorInstalled = SOLVER_FAVOR,
    DisfavorInstalled = SOLVER_DISFAVOR
};

/*enum class ProblemRule {
    Distupgrade = SOLVER_RULE_DISTUPGRADE,
    Infarch = SOLVER_RULE_INFARCH,
    Update = SOLVER_RULE_UPDATE,
    Job = SOLVER_RULE_JOB,
    JobNothing = SOLVER_RULE_JOB_NOTHING_PROVIDES_DEP,
    JobProvidedBySystem = SOLVER_RULE_JOB_PROVIDED_BY_SYSTEM,
    RPM = SOLVER_RULE_RPM,
    RPMNotInstallable = SOLVER_RULE_RPM_NOT_INSTALLABLE,
    RPMNothingProvidesRequired = SOLVER_RULE_RPM_NOTHING_PROVIDES_DEP,
    RPMPackageConflict = SOLVER_RULE_RPM_PACKAGE_CONFLICT,
    RPMPackageObsoletes = SOLVER_RULE_RPM_PACKAGE_OBSOLETES
};*/

// ============================================================================
// RAII Queue Wrapper
// ============================================================================

class Queue {
public:
    Queue() : m_queue(std::make_unique<::Queue>()) {
        queue_init(m_queue.get());
    }

    explicit Queue(::Queue* q, bool owned = false)
        : m_queue(owned ? q : nullptr), m_external(owned ? nullptr : q) {
        if (!owned && q) {
            m_queue = std::make_unique<::Queue>();
            queue_init_clone(m_queue.get(), q);
        }
    }

    ~Queue() {
        if (m_queue) {
            queue_free(m_queue.get());
        }
    }

    // Disable copy, enable move
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    Queue(Queue&& other) noexcept
        : m_queue(std::move(other.m_queue)), m_external(other.m_external) {
        other.m_external = nullptr;
    }

    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            if (m_queue) {
                queue_free(m_queue.get());
            }
            m_queue = std::move(other.m_queue);
            m_external = other.m_external;
            other.m_external = nullptr;
        }
        return *this;
    }

    void Push(Id id) {
        queue_push(GetRaw(), id);
    }

    void Push2(Id id1, Id id2) {
        queue_push2(GetRaw(), id1, id2);
    }

    void Clear() {
        queue_empty(GetRaw());
    }

    [[nodiscard]] size_t Size() const {
        return GetRaw()->count;
    }

    [[nodiscard]] bool Empty() const {
        return Size() == 0;
    }

    [[nodiscard]] Id operator[](size_t index) const {
        if (index >= Size()) {
            throw std::out_of_range("Queue index out of range");
        }
        return GetRaw()->elements[index];
    }

    [[nodiscard]] std::vector<Id> ToVector() const {
        const auto* q = GetRaw();
        return std::vector<Id>(q->elements, q->elements + q->count);
    }

    [[nodiscard]] ::Queue* GetRaw() {
        return m_external ? m_external : m_queue.get();
    }

    [[nodiscard]] const ::Queue* GetRaw() const {
        return m_external ? m_external : m_queue.get();
    }

private:
    std::unique_ptr<::Queue> m_queue;
    ::Queue* m_external{nullptr};
};

// ============================================================================
// Checksum Wrapper
// ============================================================================

class Chksum {
public:
    enum class Type {
        MD5 = REPOKEY_TYPE_MD5,
        SHA1 = REPOKEY_TYPE_SHA1,
        SHA256 = REPOKEY_TYPE_SHA256,
        SHA512 = REPOKEY_TYPE_SHA512
    };

    explicit Chksum(Type type)
        : m_handle(solv_chksum_create(static_cast<Id>(type))) {
        if (!m_handle) {
            throw SolvException("Failed to create checksum");
        }
    }

    ~Chksum() {
        if (m_handle) {
            solv_chksum_free(m_handle, nullptr);
        }
    }

    // Disable copy, enable move
    Chksum(const Chksum&) = delete;
    Chksum& operator=(const Chksum&) = delete;

    Chksum(Chksum&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    Chksum& operator=(Chksum&& other) noexcept {
        if (this != &other) {
            if (m_handle) {
                solv_chksum_free(m_handle, nullptr);
            }
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    void Add(const void* data, size_t len) {
        solv_chksum_add(m_handle, data, len);
    }

    void Add(const std::string& str) {
        Add(str.data(), str.size());
    }

    [[nodiscard]] std::vector<unsigned char> GetRaw() {
        int len;
        unsigned char* raw = solv_chksum_get(m_handle, &len);
        return std::vector<unsigned char>(raw, raw + len);
    }

    [[nodiscard]] std::string GetHex() {
        return solv_chksum_str2type(solv_chksum_get_type(m_handle));
    }

private:
    ::Chksum* m_handle;
};

// ============================================================================
// Solvable Wrapper
// ============================================================================

class Solvable {
public:
    Solvable(::Pool* pool, Id id) : m_pool(pool), m_id(id) {}

    [[nodiscard]] Id GetId() const { return m_id; }

    [[nodiscard]] std::string GetName() const {
        return pool_id2str(m_pool, GetSolvable()->name);
    }

    [[nodiscard]] std::string GetEvr() const {
        return pool_id2str(m_pool, GetSolvable()->evr);
    }

    [[nodiscard]] std::string GetArch() const {
        return pool_id2str(m_pool, GetSolvable()->arch);
    }

    [[nodiscard]] std::string GetVendor() const {
        return pool_id2str(m_pool, GetSolvable()->vendor);
    }

    [[nodiscard]] std::string GetSummary() const {
        return GetLookupStr(SOLVABLE_SUMMARY);
    }

    [[nodiscard]] std::string GetDescription() const {
        return GetLookupStr(SOLVABLE_DESCRIPTION);
    }

    [[nodiscard]] std::string GetUrl() const {
        return GetLookupStr(SOLVABLE_URL);
    }

    [[nodiscard]] uint64_t GetInstallSize() const {
        return solvable_lookup_num(GetSolvable(), SOLVABLE_INSTALLSIZE, 0);
    }

    [[nodiscard]] uint64_t GetDownloadSize() const {
        return solvable_lookup_num(GetSolvable(), SOLVABLE_DOWNLOADSIZE, 0);
    }

    [[nodiscard]] std::string GetLocation() const {
        return GetLookupStr(SOLVABLE_MEDIAFILE);
    }

    [[nodiscard]] bool IsInstalled() const {
        return m_pool->installed && GetSolvable()->repo == m_pool->installed;
    }

    [[nodiscard]] std::string AsString() const {
        return pool_solvable2str(m_pool, GetSolvable());
    }

    [[nodiscard]] std::vector<std::string> GetRequires() const {
        return GetDependencies(SOLVABLE_REQUIRES);
    }

    [[nodiscard]] std::vector<std::string> GetProvides() const {
        return GetDependencies(SOLVABLE_PROVIDES);
    }

    [[nodiscard]] std::vector<std::string> GetConflicts() const {
        return GetDependencies(SOLVABLE_CONFLICTS);
    }

    [[nodiscard]] std::vector<std::string> GetObsoletes() const {
        return GetDependencies(SOLVABLE_OBSOLETES);
    }

    [[nodiscard]] std::vector<std::string> GetRecommends() const {
        return GetDependencies(SOLVABLE_RECOMMENDS);
    }

    [[nodiscard]] std::vector<std::string> GetSuggests() const {
        return GetDependencies(SOLVABLE_SUGGESTS);
    }

    [[nodiscard]] std::vector<std::string> GetSupplements() const {
        return GetDependencies(SOLVABLE_SUPPLEMENTS);
    }

    [[nodiscard]] std::vector<std::string> GetEnhances() const {
        return GetDependencies(SOLVABLE_ENHANCES);
    }

    bool operator==(const Solvable& other) const {
        return m_pool == other.m_pool && m_id == other.m_id;
    }

private:
    [[nodiscard]] ::Solvable* GetSolvable() const {
        return pool_id2solvable(m_pool, m_id);
    }

    [[nodiscard]] std::string GetLookupStr(Id keyname) const {
        const char* str = solvable_lookup_str(GetSolvable(), keyname);
        return str ? str : "";
    }

    [[nodiscard]] std::vector<std::string> GetDependencies(Id keyname) const {
        std::vector<std::string> deps;
        ::Queue q;
        queue_init(&q);
        solvable_lookup_deparray(GetSolvable(), keyname, &q, -1);

        for (int i = 0; i < q.count; ++i) {
            deps.emplace_back(pool_dep2str(m_pool, q.elements[i]));
        }

        queue_free(&q);
        return deps;
    }

    ::Pool* m_pool;
    Id m_id;
};

// ============================================================================
// Repository Wrapper
// ============================================================================

class Repo {
public:
    Repo(::Pool* pool, const std::string& name)
        : m_pool(pool), m_repo(repo_create(pool, name.c_str())) {
        if (!m_repo) {
            throw RepoException("Failed to create repository: " + name);
        }
    }

    explicit Repo(::Repo* repo) : m_pool(repo->pool), m_repo(repo), m_owned(false) {}

    ~Repo() {
        if (m_owned && m_repo) {
            repo_free(m_repo, 1);
        }
    }

    // Disable copy, enable move
    Repo(const Repo&) = delete;
    Repo& operator=(const Repo&) = delete;

    Repo(Repo&& other) noexcept
        : m_pool(other.m_pool), m_repo(other.m_repo), m_owned(other.m_owned) {
        other.m_repo = nullptr;
        other.m_owned = false;
    }

    Repo& operator=(Repo&& other) noexcept {
        if (this != &other) {
            if (m_owned && m_repo) {
                repo_free(m_repo, 1);
            }
            m_pool = other.m_pool;
            m_repo = other.m_repo;
            m_owned = other.m_owned;
            other.m_repo = nullptr;
            other.m_owned = false;
        }
        return *this;
    }

    [[nodiscard]] std::string GetName() const {
        return m_repo->name ? m_repo->name : "";
    }

    [[nodiscard]] int GetPriority() const {
        return m_repo->priority;
    }

    void SetPriority(int priority) {
        m_repo->priority = priority;
    }

    [[nodiscard]] size_t Size() const {
        return m_repo->nsolvables;
    }

    [[nodiscard]] bool IsEmpty() const {
        return Size() == 0;
    }

    void AddSolv(const std::filesystem::path& solvFile) {
        FILE* fp = fopen(solvFile.c_str(), "r");
        if (!fp) {
            throw RepoException("Cannot open solv file: " + solvFile.string());
        }

        if (repo_add_solv(m_repo, fp, 0) != 0) {
            fclose(fp);
            throw RepoException("Failed to add solv file: " + solvFile.string());
        }
        fclose(fp);
    }

    void WriteSolv(const std::filesystem::path& solvFile) {
        FILE* fp = fopen(solvFile.c_str(), "w");
        if (!fp) {
            throw RepoException("Cannot create solv file: " + solvFile.string());
        }

        if (repo_write(m_repo, fp) != 0) {
            fclose(fp);
            throw RepoException("Failed to write solv file: " + solvFile.string());
        }
        fclose(fp);
    }

    void AddRpmDb() {
        if (repo_add_rpmdb(m_repo, nullptr, 0) != 0) {
            throw RepoException("Failed to add RPM database");
        }
    }

    void AddRepomdXml(const std::filesystem::path& repomdFile) {
        FILE* fp = fopen(repomdFile.c_str(), "r");
        if (!fp) {
            throw RepoException("Cannot open repomd.xml: " + repomdFile.string());
        }

        if (repo_add_repomdxml(m_repo, fp, 0) != 0) {
            fclose(fp);
            throw RepoException("Failed to add repomd.xml");
        }
        fclose(fp);
    }

    void AddRpmMd(const std::filesystem::path& primaryFile, const std::filesystem::path& baseDir) {
        FILE* fp = fopen(primaryFile.c_str(), "r");
        if (!fp) {
            throw RepoException("Cannot open primary.xml: " + primaryFile.string());
        }

        if (repo_add_rpmmd(m_repo, fp, nullptr, 0) != 0) {
            fclose(fp);
            throw RepoException("Failed to add RPM metadata");
        }
        fclose(fp);
    }

    void InternalizeRepository() {
        repo_internalize(m_repo);
    }

    void CreateRepoData(int flags = 0) {
        repo_add_repodata(m_repo, flags);
    }

    [[nodiscard]] std::vector<Solvable> GetSolvables() const {
        std::vector<Solvable> solvables;

        for (Id p = m_repo->start; p < m_repo->end; ++p) {
            ::Solvable* s = pool_id2solvable(m_pool, p);
            if (s->repo == m_repo) {
                solvables.emplace_back(m_pool, p);
            }
        }

        return solvables;
    }

    [[nodiscard]] ::Repo* GetRaw() { return m_repo; }
    [[nodiscard]] const ::Repo* GetRaw() const { return m_repo; }

private:
    ::Pool* m_pool;
    ::Repo* m_repo;
    bool m_owned{true};
};

// ============================================================================
// Selection Wrapper
// ============================================================================

class Selection {
public:
    Selection(::Pool* pool) : m_pool(pool) {
        queue_init(&m_selection);
    }

    ~Selection() {
        queue_free(&m_selection);
    }

    // Disable copy, enable move
    Selection(const Selection&) = delete;
    Selection& operator=(const Selection&) = delete;

    Selection(Selection&& other) noexcept
        : m_pool(other.m_pool), m_selection(other.m_selection) {
        other.m_pool = nullptr;
        queue_init(&other.m_selection);
    }

    bool Select(const std::string& name, SelectionFlags flags = SelectionFlags::Name) {
        queue_empty(&m_selection);
        int ret = selection_make(m_pool, &m_selection, name.c_str(), static_cast<int>(flags));
        return ret > 0;
    }

    [[nodiscard]] std::vector<Solvable> GetSolvables() const {
        std::vector<Solvable> solvables;
        ::Queue q;
        queue_init(&q);
        selection_solvables(m_pool, &m_selection, &q);

        for (int i = 0; i < q.count; ++i) {
            solvables.emplace_back(m_pool, q.elements[i]);
        }

        queue_free(&q);
        return solvables;
    }

    [[nodiscard]] Queue GetJobs(JobCommand cmd) const {
        Queue jobs;

        for (int i = 0; i < m_selection.count; i += 2) {
            jobs.Push2(m_selection.elements[i] | static_cast<Id>(cmd), m_selection.elements[i + 1]);
        }

        return jobs;
    }

    [[nodiscard]] bool IsEmpty() const {
        return m_selection.count == 0;
    }

    [[nodiscard]] size_t Size() const {
        return m_selection.count / 2;
    }

private:
    ::Pool* m_pool;
    ::Queue m_selection;
};

// ============================================================================
// Transaction Wrapper
// ============================================================================

class Transaction {
public:
    Transaction(::Pool* pool, ::Queue* decisionq, const std::unordered_map<Id, Id>& obsoletesmap)
        : m_pool(pool) {
        // Create transaction from solver decisions
        ::Queue obsoletes;
        queue_init(&obsoletes);

        for (const auto& [key, value] : obsoletesmap) {
            queue_push2(&obsoletes, key, value);
        }

        m_trans = solver_create_transaction(nullptr, decisionq, &obsoletes);
        queue_free(&obsoletes);

        if (!m_trans) {
            throw SolvException("Failed to create transaction");
        }
    }

    explicit Transaction(::Transaction* trans) : m_trans(trans), m_owned(false) {}

    ~Transaction() {
        if (m_owned && m_trans) {
            transaction_free(m_trans);
        }
    }

    // Disable copy, enable move
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    Transaction(Transaction&& other) noexcept
        : m_pool(other.m_pool), m_trans(other.m_trans), m_owned(other.m_owned) {
        other.m_trans = nullptr;
        other.m_owned = false;
    }

    [[nodiscard]] std::vector<Solvable> GetSteps() const {
        std::vector<Solvable> steps;
        ::Queue q;
        queue_init(&q);

        transaction_calc_installsizechange(m_trans);
        transaction_order(m_trans, 0);

        for (int i = 0; i < m_trans->steps.count; ++i) {
            Id p = m_trans->steps.elements[i];
            steps.emplace_back(m_pool, p);
        }

        queue_free(&q);
        return steps;
    }

    [[nodiscard]] int64_t GetSizeChange() const {
        return transaction_calc_installsizechange(m_trans);
    }

    void Order(int flags = 0) {
        transaction_order(m_trans, flags);
    }

    [[nodiscard]] std::string GetClassification(int mode = SOLVER_TRANSACTION_SHOW_ALL) const {
        ::Queue classes;
        queue_init(&classes);

        transaction_classify(m_trans, mode, &classes);

        std::string result;
        for (int i = 0; i < classes.count; i += 4) {
            Id cls = classes.elements[i];
            Id cnt = classes.elements[i + 1];

            const char* clsname = pool_id2str(m_pool, cls);
            result += std::string(clsname) + ": " + std::to_string(cnt) + "\n";
        }

        queue_free(&classes);
        return result;
    }

private:
    ::Pool* m_pool{nullptr};
    ::Transaction* m_trans;
    bool m_owned{true};
};

// ============================================================================
// Solver Wrapper
// ============================================================================

class Solver {
public:
    explicit Solver(::Pool* pool)
        : m_pool(pool), m_solver(solver_create(pool)) {
        if (!m_solver) {
            throw SolverException("Failed to create solver");
        }
    }

    ~Solver() {
        if (m_solver) {
            solver_free(m_solver);
        }
    }

    // Disable copy, enable move
    Solver(const Solver&) = delete;
    Solver& operator=(const Solver&) = delete;

    Solver(Solver&& other) noexcept
        : m_pool(other.m_pool), m_solver(other.m_solver) {
        other.m_solver = nullptr;
    }

    Solver& operator=(Solver&& other) noexcept {
        if (this != &other) {
            if (m_solver) {
                solver_free(m_solver);
            }
            m_pool = other.m_pool;
            m_solver = other.m_solver;
            other.m_solver = nullptr;
        }
        return *this;
    }

    void SetFlag(SolverFlag flag, bool value) {
        solver_set_flag(m_solver, static_cast<int>(flag), value ? 1 : 0);
    }

    [[nodiscard]] bool GetFlag(SolverFlag flag) const {
        return solver_get_flag(m_solver, static_cast<int>(flag)) != 0;
    }

    [[nodiscard]] bool Solve(Queue& jobs) {
        int ret = solver_solve(m_solver, jobs.GetRaw());
        return ret == 0;
    }

    [[nodiscard]] std::vector<std::string> GetProblems() const {
        std::vector<std::string> problems;

        int problemCount = solver_problem_count(m_solver);
        for (int i = 1; i <= problemCount; ++i) {
            Id problem = solver_findproblemrule(m_solver, i);
            const char* str = solver_problemruleinfo2str(
                m_solver,
                solver_ruleinfo(m_solver, problem, nullptr, nullptr, nullptr)
            );
            if (str) {
                problems.emplace_back(str);
            }
        }

        return problems;
    }

    [[nodiscard]] std::vector<std::vector<std::string>> GetSolutions(int problemNum) const {
        std::vector<std::vector<std::string>> solutions;

        int solutionCount = solver_solution_count(m_solver, problemNum);
        for (int i = 1; i <= solutionCount; ++i) {
            std::vector<std::string> solution;

            Id solutionId = solver_next_solution(m_solver, problemNum, i);
            while (solutionId) {
                const char* str = solver_solutionelement2str(
                    m_solver, problemNum, solutionId, nullptr
                );
                if (str) {
                    solution.emplace_back(str);
                }
                solutionId = solver_next_solutionelement(m_solver, problemNum, solutionId, nullptr);
            }

            if (!solution.empty()) {
                solutions.emplace_back(solution);
            }
        }

        return solutions;
    }

    [[nodiscard]] Transaction CreateTransaction() {
        ::Queue decisionq;
        queue_init(&decisionq);
        solver_get_decisionqueue(m_solver, &decisionq);

        std::unordered_map<Id, Id> obsoletesmap;
        Transaction trans(m_pool, &decisionq, obsoletesmap);

        queue_free(&decisionq);
        return trans;
    }

    [[nodiscard]] std::vector<Solvable> GetDecisions() const {
        std::vector<Solvable> decisions;
        ::Queue decisionq;
        queue_init(&decisionq);

        solver_get_decisionqueue(m_solver, &decisionq);

        for (int i = 0; i < decisionq.count; ++i) {
            Id p = decisionq.elements[i];
            if (p > 0) {
                decisions.emplace_back(m_pool, p);
            }
        }

        queue_free(&decisionq);
        return decisions;
    }

    void PrintDebugInfo() const {
        solver_printdecisions(m_solver);
    }

    [[nodiscard]] ::Solver* GetRaw() { return m_solver; }
    [[nodiscard]] const ::Solver* GetRaw() const { return m_solver; }

private:
    ::Pool* m_pool;
    ::Solver* m_solver;
};

// ============================================================================
// Pool Wrapper (Main Entry Point)
// ============================================================================

class Pool {
public:
    Pool() : m_pool(pool_create()) {
        if (!m_pool) {
            throw PoolException("Failed to create pool");
        }
        pool_setdisttype(m_pool, DISTTYPE_RPM);
    }

    ~Pool() {
        if (m_pool) {
            pool_free(m_pool);
        }
    }

    // Disable copy, enable move
    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;

    Pool(Pool&& other) noexcept : m_pool(other.m_pool) {
        other.m_pool = nullptr;
    }

    Pool& operator=(Pool&& other) noexcept {
        if (this != &other) {
            if (m_pool) {
                pool_free(m_pool);
            }
            m_pool = other.m_pool;
            other.m_pool = nullptr;
        }
        return *this;
    }

    void SetArch(const std::string& arch = "") {
        if (arch.empty()) {
            pool_setarch(m_pool, nullptr);
        } else {
            pool_setarch(m_pool, arch.c_str());
        }
    }

    [[nodiscard]] std::string GetArch() const {
        const char* arch = pool_get_rootdir(m_pool);
        return arch ? arch : "";
    }

    void SetDistType(DistType type) {
        int dtype;
        switch (type) {
            case DistType::RPM: dtype = DISTTYPE_RPM; break;
            case DistType::DEB: dtype = DISTTYPE_DEB; break;
            case DistType::ARCH: dtype = DISTTYPE_ARCH; break;
            case DistType::HAIKU: dtype = DISTTYPE_HAIKU; break;
            default: dtype = DISTTYPE_RPM;
        }
        pool_setdisttype(m_pool, dtype);
    }

    void SetRootDir(const std::filesystem::path& root) {
        pool_set_rootdir(m_pool, root.c_str());
    }

    [[nodiscard]] std::filesystem::path GetRootDir() const {
        const char* root = pool_get_rootdir(m_pool);
        return root ? std::filesystem::path(root) : std::filesystem::path();
    }

    void AddConsideredPackage(const std::string& name) {
        Id id = pool_str2id(m_pool, name.c_str(), 1);
        ::Queue q;
        queue_init(&q);

        pool_get_considered(m_pool, &q);
        queue_push(&q, id);
        pool_set_considered(m_pool, &q);

        queue_free(&q);
    }

    Repo CreateRepo(const std::string& name) {
        return Repo(m_pool, name);
    }

    [[nodiscard]] std::optional<Repo> GetRepo(const std::string& name) {
		int i;
    	::Repo* pool;
		// Search for the repo by name
    	FOR_REPOS(i, m_pool) {
            ::Repo* repo = pool_id2repo(pool, i);
            if (repo && repo->name && std::string_view(repo->name) == name) {
                return Repo(repo);
            }
        }
        return std::nullopt;
    }

	[[nodiscard]] std::vector<Repo> GetRepos() {
    	int i;
    	::Repo* pool;
        std::vector<Repo> repos;
        FOR_REPOS(i, m_pool) {
            ::Repo* repo = pool_id2repo(m_pool, i);
            if (repo) {
                repos.emplace_back(repo);
            }
        }
        return repos;
    }

    void SetInstalledRepo(Repo& repo) {
        pool_set_installed(m_pool, repo.GetRaw());
    }

    [[nodiscard]] std::optional<Repo> GetInstalledRepo() {
        if (m_pool->installed) {
            return Repo(m_pool->installed);
        }
        return std::nullopt;
    }

    void CreateWhatProvides() {
        pool_createwhatprovides(m_pool);
    }

    [[nodiscard]] std::vector<Solvable> WhatProvides(const std::string& dep) {
        std::vector<Solvable> providers;
        Id depId = pool_str2id(m_pool, dep.c_str(), 0);

        if (depId) {
            ::Queue q;
            queue_init(&q);
            pool_whatprovides_queue(m_pool, depId, &q);

            for (int i = 0; i < q.count; ++i) {
                providers.emplace_back(m_pool, q.elements[i]);
            }

            queue_free(&q);
        }

        return providers;
    }

    [[nodiscard]] std::vector<Solvable> WhatMatchesDep(const std::string& dep) {
        std::vector<Solvable> matches;
        Id depId = pool_str2id(m_pool, dep.c_str(), 0);

        if (depId) {
            ::Queue q;
            queue_init(&q);
            pool_whatmatchesdep(m_pool, REL_EQ, depId, &q, -1);

            for (int i = 0; i < q.count; ++i) {
                matches.emplace_back(m_pool, q.elements[i]);
            }

            queue_free(&q);
        }

        return matches;
    }

    [[nodiscard]] Solvable GetSolvable(Id id) const {
        return Solvable(m_pool, id);
    }

    [[nodiscard]] Selection CreateSelection() const {
        return Selection(m_pool);
    }

    [[nodiscard]] Solver CreateSolver() const {
        return Solver(m_pool);
    }

    [[nodiscard]] std::string IdToString(Id id) const {
        return pool_id2str(m_pool, id);
    }

    [[nodiscard]] Id StringToId(const std::string& str, bool create = false) {
        return pool_str2id(m_pool, str.c_str(), create ? 1 : 0);
    }

    [[nodiscard]] std::string DepToString(Id dep) const {
        return pool_dep2str(m_pool, dep);
    }

    void PrepareTransaction() const {
        pool_createwhatprovides(m_pool);
    }

    [[nodiscard]] ::Pool* GetRaw() { return m_pool; }
    [[nodiscard]] const ::Pool* GetRaw() const { return m_pool; }

private:
    ::Pool* m_pool;
};

} // namespace solv