#ifndef SOLVERSTRINGS_H_INCLUDED
#define SOLVERSTRINGS_H_INCLUDED

namespace FlagAlgebra {
	// This enum enumerates all solvers supported
	enum Solvers {
		CSDP,
		SDPA,
		/* Dummy item */ SolversSize
	};

#define SOLVERRETURNMAXSIZE 12
	// This array holds what each Solver error return value means.
	static const char *const SolverReturnStrings[][SOLVERRETURNMAXSIZE] = {
		/* CSDP */ {
			/*  1 */ "Success. The problem is primal infeasible.",
			/*  2 */ "Success. The problem is dual infeasible.",
			/*  3 */ "Partial Success. A solution has been found, but full accuracy"
			" was not achieved.\n"
			"One or more of primal infeasibility, dual infeasibility,"
			" or relative duality gap are larger than their tolerances,"
			" but by a factor of less than 1000.",
			/*  4 */ "Failure. Maximum iterations reached.",
			/*  5 */ "Failure. Stuck at edge of primal feasibility.",
			/*  6 */ "Failure. Stuck at edge of dual feasibility.",
			/*  7 */ "Failure. Lack of progress.",
			/*  8 */ "Failure. X, Z, or O was sigular.",
			/*  9 */ "Failure. Detected NaN or Inf values.",
			/* 10 */ "Error. There was an input problem.",
			/* 11 */ "Unknown error.",
			/* 12 */ "Failure. The gap became infinite or NaN."
		},
		/* SDPA */ {
			/*  1 */ "Unknown error."
		}
	};

	static const int SolverReturnSize[] = {
		/* CSDP */ 12,
		/* SDPA */ 1
	};
	static const int SolverUnknown[] = {
		/* CSDP */ 11,
		/* SDPA */ 1
	};

	// These strings hold parts of the names of files and names of programs to be used
	static const std::string extension(".txt"),
		problem_suffix("_prob"),
		tfused_suffix("_used"),
		solver_solution_suffix("_sol"),
		reductor_suffix("_red"),
		final_solution_base_suffix("_base"),
		final_solution_suffix("_solved"),
		final_solution_squares_suffix("_solved_squares"),
		copier("cp"),
		remover("rm");
	static const std::string solver[] = {
		"csdp",
		"sdpa -ds "
	};
	static const std::string outputOption[] = {
		"",
		"-o "
	};

	static const char *const SolversHelp =
		"          Solvers available:\n"
		"          0 = CSDP\n"
		"          1 = SDPA\n";
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
