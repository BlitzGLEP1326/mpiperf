/*
 * iallgather.c: Benchmark functions for Iallgather.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "iallgather.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize, rbufsize;

int bench_iallgather_init(nbctest_params_t *params)
{
    sbufpool = mempool_create(params->count, mpiperf_isflushcache);
    rbufpool = mempool_create(params->count * params->nprocs, mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }
    sbufsize = params->count * sizeof(char);
    rbufsize = params->count * params->nprocs;
    return MPIPERF_SUCCESS;
}

int bench_iallgather_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

int bench_iallgather_printinfo()
{
    printf("* Iallgather\n"
           "  proto: MPI_Iallgather(sbuf, count, MPI_BYTE, \n"
           "                        rbuf, count, MPI_BYTE, comm)\n");
    return MPIPERF_SUCCESS;
}

int measure_iallgather_blocking(nbctest_params_t *params,
                                nbctest_result_t *result)
{
#ifdef HAVE_NBC
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPI_Iallgather(mempool_alloc(sbufpool, sbufsize), params->count,
                        MPI_BYTE, mempool_alloc(rbufpool, rbufsize),
                        params->count, MPI_BYTE, params->comm, &req);
    result->inittime = hpctimer_wtime() - result->inittime;
    result->waittime = hpctimer_wtime();
    rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
    result->waittime = hpctimer_wtime() - result->waittime;
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
        result->totaltime = endtime - starttime;
        return MEASURE_SUCCESS;
    }
#endif
    return MEASURE_FAILURE;
}

int measure_iallgather_overlap(nbctest_params_t *params,
                               nbctest_result_t *result)

{
#ifdef HAVE_NBC
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPI_Iallgather(mempool_alloc(sbufpool, sbufsize), params->count,
                        MPI_BYTE, mempool_alloc(rbufpool, rbufsize),
                        params->count, MPI_BYTE, params->comm, &req);
    result->inittime = hpctimer_wtime() - result->inittime;
    nbcbench_simulate_computing(params, &req, result);
    result->waittime = hpctimer_wtime();
    rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
    result->waittime = hpctimer_wtime() - result->waittime;
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
        result->totaltime = endtime - starttime;
        return MEASURE_SUCCESS;
    }
#endif
    return MEASURE_FAILURE;
}
