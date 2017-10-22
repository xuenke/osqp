#include "util.h"

/***************
 * Versioning  *
 ***************/
const char *osqp_version(void) {
    return OSQP_VERSION;
}

/************************************
 * Printing Constants to set Layout *
 ************************************/
#ifdef PRINTING
#define HEADER_LINE_LEN 65
#endif

/**********************
 * Utility Functions  *
 **********************/


void c_strcpy(char dest[], const char source[]){
    int i = 0;
    while (1) {
        dest[i] = source[i];
        if (dest[i] == '\0') break;
        i++;
    } }


#ifdef PRINTING

static void print_line(void){
    char the_line[HEADER_LINE_LEN+1];
    c_int i;
    for (i = 0; i < HEADER_LINE_LEN; ++i)
        the_line[i] = '-';
    the_line[HEADER_LINE_LEN] = '\0';
    c_print("%s\n", the_line);
}

void print_header(void){
    c_print("iter   objective    pri res    dua res    rho");
#ifdef PROFILING
    c_print("        time");
#endif
    c_print("\n");
}

void print_setup_header(const OSQPWorkspace * work) {
    OSQPData *data;
    OSQPSettings * settings;
    data = work->data;
    settings = work->settings;
    c_int nnz;  // Number of nonzeros in the problem

    // Number of nonzeros
    nnz = data->P->nzmax + data->A->p[data->A->n];

    print_line();
    c_print("           OSQP v%s  -  Operator Splitting QP Solver\n"
            "              (c) Bartolomeo Stellato,  Goran Banjac\n"
            "        University of Oxford  -  Stanford University 2017\n",
            OSQP_VERSION);
    print_line();

    // Print variables and constraints
    c_print("problem:  ");
    c_print("variables n = %i, constraints m = %i\n          ", (int)data->n, (int)data->m);
    c_print("nnz(P) + nnz(A) = %i\n", (int)nnz);

    // Print Settings
    c_print("settings: ");
    c_print("linear system solver = %s",
            LINSYS_SOLVER_NAME[settings->linsys_solver]);
    if (work->linsys_solver->nthreads != 1){
        c_print(" (%d threads)", work->linsys_solver->nthreads);
    }
    c_print(",\n          ");

    c_print("eps_abs = %.1e, eps_rel = %.1e,\n          ",
            settings->eps_abs, settings->eps_rel);
    c_print("eps_prim_inf = %.1e, eps_dual_inf = %.1e,\n          ",
            settings->eps_prim_inf, settings->eps_dual_inf);
    c_print("rho = %.2e ", settings->rho);
    if (settings->adaptive_rho) c_print("(adaptive)");
    c_print("\n          ");
    // c_print("sigma = %.1f, alpha = %.2f, \n          ",
    c_print("sigma = %.2e, alpha = %.2f, ",
            settings->sigma, settings->alpha);
    c_print("max_iter = %i\n", (int)settings->max_iter);

    if (settings->check_termination)
        c_print("          check_termination: on (interval %i)\n",
                (int)settings->check_termination);
    else
        c_print("          check_termination: off \n");
    if (settings->scaling){
        c_print("          scaling: on ");
        if (settings->scaling_norm != -1)
            c_print("(%d-norm), ", (int)settings->scaling_norm);
        else
            c_print("(inf-norm), ");
    }
    else
        c_print("          scaling: off, ");
    if (settings->scaled_termination)
        c_print("scaled_termination: on\n");
    else
        c_print("scaled_termination: off\n");
    if (settings->warm_start)
        c_print("          warm start: on, ");
    else
        c_print("          warm start: off, ");
    if (settings->polish)
        c_print("polish: on\n");
    else
        c_print("polish: off\n");
    c_print("\n");
}


void print_summary(OSQPWorkspace * work){
    OSQPInfo * info;
    info = work->info;

    c_print("%4i", (int)info->iter);
    c_print(" %12.4e", info->obj_val);
    c_print("  %9.2e", info->pri_res);
    c_print("  %9.2e", info->dua_res);
    c_print("  %9.2e", work->settings->rho);
#ifdef PROFILING
    if (work->first_run) {
        // total time: setup + solve
        c_print("  %9.2es", info->setup_time + info->solve_time);
    } else {
        // total time: solve
        c_print("  %9.2es", info->solve_time);
    }
#endif
    c_print("\n");

    work->summary_printed = 1; // Summary has been printed
}


void print_polish(OSQPWorkspace * work) {
    OSQPInfo * info;
    info = work->info;

    c_print("%4s", "plsh");
    c_print(" %12.4e", info->obj_val);
    c_print("  %9.2e", info->pri_res);
    c_print("  %9.2e", info->dua_res);
    c_print("   --------");
#ifdef PROFILING
    c_print("  %9.2es", info->setup_time + info->solve_time +
            info->polish_time);
#endif
    c_print("\n");
}


#endif /* End #ifdef PRINTING */


#ifdef PRINTING

void print_footer(OSQPInfo * info, c_int polish){

#ifdef PRINTING
    c_print("\n"); // Add space after iterations
#endif

    c_print("status:               %s\n", info->status);

    if (polish && info->status_val == OSQP_SOLVED) {
        if (info->status_polish == 1){
            c_print("solution polish:      successful\n");
        } else if (info->status_polish < 0){
            c_print("solution polish:      unsuccessful\n");
        }
    }

    c_print("number of iterations: %i\n", (int)info->iter);
    if (info->status_val == OSQP_SOLVED ||
            info->status_val == OSQP_SOLVED_INACCURATE) {
        c_print("optimal objective:    %.4f\n", info->obj_val);
    }

#ifdef PROFILING
    c_print("run time:             %.2es\n", info->run_time);
#endif
    c_print("\n");

}

#endif


void set_default_settings(OSQPSettings * settings) {
    settings->scaling = SCALING; /* heuristic problem scaling */

#if EMBEDDED != 1
    settings->scaling_norm = SCALING_NORM;
    settings->adaptive_rho = ADAPTIVE_RHO;
    settings->adaptive_rho_interval = ADAPTIVE_RHO_INTERVAL;
#endif

    settings->rho = (c_float) RHO; /* ADMM step */
    settings->sigma = (c_float) SIGMA; /* ADMM step */
    settings->max_iter = MAX_ITER; /* maximum iterations to take */
    settings->eps_abs = (c_float) EPS_ABS;         /* absolute convergence tolerance */
    settings->eps_rel = (c_float) EPS_REL;         /* relative convergence tolerance */
    settings->eps_prim_inf = (c_float) EPS_PRIM_INF;         /* primal infeasibility tolerance */
    settings->eps_dual_inf = (c_float) EPS_DUAL_INF;         /* dual infeasibility tolerance */
    settings->alpha = (c_float) ALPHA;     /* relaxation parameter */
    settings->linsys_solver = LINSYS_SOLVER;     /* relaxation parameter */

#ifndef EMBEDDED
    settings->delta = DELTA;    /* regularization parameter for polish */
    settings->polish = POLISH;     /* ADMM solution polish: 1 */
    settings->polish_refine_iter = POLISH_REFINE_ITER; /* iterative refinement
                                                    steps in polish */
    settings->verbose = VERBOSE;     /* print output */
#endif

    settings->scaled_termination = SCALED_TERMINATION;     /* Evaluate scaled termination criteria*/
    settings->check_termination = CHECK_TERMINATION;     /* Interval for evaluating termination criteria */
    settings->warm_start = WARM_START;     /* warm starting */

}

#ifndef EMBEDDED

OSQPSettings * copy_settings(OSQPSettings * settings){
    OSQPSettings * new = c_malloc(sizeof(OSQPSettings));

    // Copy settings
    new->scaling = settings->scaling;
    new->scaling_norm = settings->scaling_norm;
    new->adaptive_rho = settings->adaptive_rho;
    new->adaptive_rho_interval = settings->adaptive_rho_interval;
    new->rho = settings->rho;
    new->sigma = settings->sigma;
    new->max_iter = settings->max_iter;
    new->eps_abs = settings->eps_abs;
    new->eps_rel = settings->eps_rel;
    new->eps_prim_inf = settings->eps_prim_inf;
    new->eps_dual_inf = settings->eps_dual_inf;
    new->alpha = settings->alpha;
    new->linsys_solver = settings->linsys_solver;
    new->delta = settings->delta;
    new->polish = settings->polish;
    new->polish_refine_iter = settings->polish_refine_iter;
    new->verbose = settings->verbose;
    new->scaled_termination = settings->scaled_termination;
    new->check_termination = settings->check_termination;
    new->warm_start = settings->warm_start;

    return new;
}

#endif  // #ifndef EMBEDDED



/*******************
 * Timer Functions *
 *******************/

#ifdef PROFILING

// Windows
#ifdef IS_WINDOWS

void tic(OSQPTimer* t)
{
    QueryPerformanceFrequency(&t->freq);
    QueryPerformanceCounter(&t->tic);
}

c_float toc(OSQPTimer* t)
{
    QueryPerformanceCounter(&t->toc);
    return ((t->toc.QuadPart - t->tic.QuadPart) / (c_float)t->freq.QuadPart);
}

// Mac
#elif defined IS_MAC

void tic(OSQPTimer* t)
{
    /* read current clock cycles */
    t->tic = mach_absolute_time();
}

c_float toc(OSQPTimer* t)
{

    uint64_t duration; /* elapsed time in clock cycles*/

    t->toc = mach_absolute_time();
    duration = t->toc - t->tic;

    /*conversion from clock cycles to nanoseconds*/
    mach_timebase_info(&(t->tinfo));
    duration *= t->tinfo.numer;
    duration /= t->tinfo.denom;

    return (c_float)duration / 1e9;
}


// Linux
#else

/* read current time */
void tic(OSQPTimer* t)
{
    clock_gettime(CLOCK_MONOTONIC, &t->tic);
}


/* return time passed since last call to tic on this timer */
c_float toc(OSQPTimer* t)
{
    struct timespec temp;

    clock_gettime(CLOCK_MONOTONIC, &t->toc);

    if ((t->toc.tv_nsec - t->tic.tv_nsec)<0) {
        temp.tv_sec = t->toc.tv_sec - t->tic.tv_sec-1;
        temp.tv_nsec = 1e9+t->toc.tv_nsec - t->tic.tv_nsec;
    } else {
        temp.tv_sec = t->toc.tv_sec - t->tic.tv_sec;
        temp.tv_nsec = t->toc.tv_nsec - t->tic.tv_nsec;
    }
    return (c_float)temp.tv_sec + (c_float)temp.tv_nsec / 1e9;
}

#endif

#endif // If Profiling end






/* ==================== DEBUG FUNCTIONS ======================= */


#ifndef EMBEDDED

c_float * csc_to_dns(csc * M)
{
    c_int i, j=0; // Predefine row index and column index
    c_int idx;

    // Initialize matrix of zeros
    c_float * A = (c_float *)c_calloc(M->m * M->n, sizeof(c_float));

    // Allocate elements
    for (idx = 0; idx < M->p[M->n]; idx++)
    {
        // Get row index i (starting from 1)
        i = M->i[idx];

        // Get column index j (increase if necessary) (starting from 1)
        while (M->p[j+1] <= idx) {
            j++;
        }

        // Assign values to A
        A[j*(M->m)+i] = M->x[idx];

    }
    return A;
}


c_int is_eq_csc(csc *A, csc *B, c_float tol){
    c_int j, i;
    // If number of columns does not coincide, they are not equal.
    if (A->n != B->n) return 0;

    for (j=0; j<A->n; j++) { // Cycle over columns j

        // if column pointer does not coincide, they are not equal
        if (A->p[j] != B->p[j]) return 0;

        for (i=A->p[j]; i<A->p[j+1]; i++) { // Cycle rows i in column j
            if (A->i[i] != B->i[i] || // Different row indices
                    c_absval(A->x[i] - B->x[i]) > tol) {
                return 0;
            }
        }
    }
    return(1);
}

#endif  // #ifndef EMBEDDED


#ifdef PRINTING

void print_csc_matrix(csc* M, const char * name)
{
    c_int j, i, row_start, row_stop;
    c_int k=0;

    // Print name
    c_print("%s :\n", name);

    for(j=0; j<M->n; j++) {
        row_start = M->p[j];
        row_stop = M->p[j+1];
        if (row_start == row_stop)
            continue;
        else {
            for(i=row_start; i<row_stop; i++ ) {
                c_print("\t[%3u,%3u] = %g\n", (int)M->i[i], (int)j, M->x[k++]);
            }
        }
    }
}

void dump_csc_matrix(csc * M, const char * file_name){
    c_int j, i, row_strt, row_stop;
    c_int k = 0;
    FILE *f = fopen(file_name,"w");
    if( f != NULL ){
        for(j=0; j<M->n; j++){
            row_strt = M->p[j];
            row_stop = M->p[j+1];
            if (row_strt == row_stop)
                continue;
            else {
                for(i = row_strt; i < row_stop; i++ ){
                    fprintf(f,"%d\t%d\t%20.18e\n",
                            (int)M->i[i]+1, (int)j+1, M->x[k++]);
                }
            }
        }
        fprintf(f,"%d\t%d\t%20.18e\n", (int)M->m, (int)M->n, 0.0);
        fclose(f);
        c_print("File %s successfully written.\n", file_name);
    } else {
        c_print("Error during writing file %s.\n", file_name);
    }
}

void print_trip_matrix(csc* M, const char * name)
{
    c_int k = 0;

    // Print name
    c_print("%s :\n", name);

    for (k=0; k<M->nz; k++){
        c_print("\t[%3u, %3u] = %g\n", (int)M->i[k], (int)M->p[k], M->x[k]);
    }
}



void print_dns_matrix(c_float * M, c_int m, c_int n, const char *name)
{
    c_int i, j;
    c_print("%s : \n\t", name);
    for(i=0; i<m; i++) { // Cycle over rows
        for(j=0; j<n; j++) { // Cycle over columns
            if (j < n - 1)
                // c_print("% 14.12e,  ", M[j*m+i]);
                c_print("% .8f,  ", M[j*m+i]);

            else
                // c_print("% 14.12e;  ", M[j*m+i]);
                c_print("% .8f;  ", M[j*m+i]);
        }
        if (i < m - 1) {
            c_print("\n\t");
        }
    }
    c_print("\n");
}


void print_vec(c_float * v, c_int n, const char *name){
    print_dns_matrix(v, 1, n, name);
}

void dump_vec(c_float * v, c_int len, const char * file_name){
    c_int i;
    FILE *f = fopen(file_name,"w");
    if( f != NULL ){
        for (i = 0; i < len; i++){
            fprintf(f,"%20.18e\n", v[i]);
        }
        fclose(f);
        c_print("File %s successfully written.\n", file_name);
    } else {
        c_print("Error during writing file %s.\n", file_name);
    }
}

void print_vec_int(c_int * x, c_int n, const char *name) {
    c_int i;
    c_print("%s = [", name);
    for(i=0; i<n; i++) {
        c_print(" %d ", (int)x[i]);
    }
    c_print("]\n");
}


#endif  // PRINTING
