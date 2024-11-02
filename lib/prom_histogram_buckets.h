/**
 * @file prom_histogram_buckets.h
 * @brief Histogram buckets for Prometheus.
 *
 * See https://prometheus.io/docs/concepts/metric_types/#histogram for more details on histograms.
 */

#include "stdlib.h"

#ifndef PROM_HISTOGRAM_BUCKETS_H
#define PROM_HISTOGRAM_BUCKETS_H

/**
 * @brief Structure representing histogram buckets.
 *
 * This structure stores the number of buckets and the upper bounds
 * for each bucket in the histogram.
 */
typedef struct prom_histogram_buckets
{
    int count;                  /**< Number of buckets */
    const double* upper_bounds; /**< The upper bound values for the buckets */
} prom_histogram_buckets_t;

/**
 * @brief Construct a new prom_histogram_buckets_t*.
 *
 * @param count The number of buckets.
 * @param bucket The first bucket value. Multiple bucket values can be passed as additional arguments.
 *               The number of bucket values must equal the value passed as `count`.
 * @return A pointer to the constructed prom_histogram_buckets_t*.
 */
prom_histogram_buckets_t* prom_histogram_buckets_new(size_t count, double bucket, ...);

/**
 * @brief The default histogram buckets: .005, .01, .025, .05, .1, .25, .5, 1, 2.5, 5, 10.
 */
extern prom_histogram_buckets_t* prom_histogram_default_buckets;

/**
 * @brief Construct linearly spaced histogram buckets.
 *
 * @param start The first inclusive upper bound for the buckets.
 * @param width The spacing between consecutive upper bounds.
 * @param count The total number of buckets (excluding the final +Inf bucket).
 * @return A pointer to the constructed prom_histogram_buckets_t*.
 */
prom_histogram_buckets_t* prom_histogram_buckets_linear(double start, double width, size_t count);

/**
 * @brief Construct exponentially spaced histogram buckets.
 *
 * @param start The first inclusive upper bound (must be greater than 0).
 * @param factor The factor to multiply the previous upper bound by to generate the next one (must be greater than 1).
 * @param count The total number of buckets (excluding the final +Inf bucket, must be >= 1).
 * @return A pointer to the constructed prom_histogram_buckets_t*.
 */
prom_histogram_buckets_t* prom_histogram_buckets_exponential(double start, double factor, size_t count);

/**
 * @brief Destroy a prom_histogram_buckets_t* and free its memory.
 *
 * The pointer `self` must be set to NULL after destruction.
 * @param self The target prom_histogram_buckets_t* to destroy.
 * @return Non-zero integer value on failure, 0 on success.
 */
int prom_histogram_buckets_destroy(prom_histogram_buckets_t* self);

/**
 * @brief Get the number of buckets.
 *
 * @param self The target prom_histogram_buckets_t*.
 * @return The number of buckets in the structure.
 */
size_t prom_histogram_buckets_count(prom_histogram_buckets_t* self);

#endif // PROM_HISTOGRAM_BUCKETS_H
