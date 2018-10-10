/*
 * Only give sleepers 50% of their service deficit. This allows
 * them to run sooner, but does not allow tons of sleepers to
 * rip the spread apart.
 */
SCHED_FEAT(GENTLE_FAIR_SLEEPERS, true)

/*
 * Place new tasks ahead so that they do not starve already running
 * tasks
 */
SCHED_FEAT(START_DEBIT, true)

/*
 * Prefer to schedule the task we woke last (assuming it failed
 * wakeup-preemption), since its likely going to consume data we
 * touched, increases cache locality.
 */
SCHED_FEAT(NEXT_BUDDY, true)

/*
 * Prefer to schedule the task that ran last (when we did
 * wake-preempt) as that likely will touch the same data, increases
 * cache locality.
 */
SCHED_FEAT(LAST_BUDDY, true)

/*
 * Consider buddies to be cache hot, decreases the likelyness of a
 * cache buddy being migrated away, increases cache locality.
 */
SCHED_FEAT(CACHE_HOT_BUDDY, true)

/*
 * Allow wakeup-time preemption of the current task:
 */
SCHED_FEAT(WAKEUP_PREEMPTION, true)

SCHED_FEAT(HRTICK, false)
SCHED_FEAT(DOUBLE_TICK, false)

/*
 * Decrement CPU capacity based on time not spent running tasks
 */
SCHED_FEAT(NONTASK_CAPACITY, true)

/*
 * Queue remote wakeups on the target CPU and process them
 * using the scheduler IPI. Reduces rq->lock contention/bounces.
 */
SCHED_FEAT(TTWU_QUEUE, true)

/*
 * When doing wakeups, attempt to limit superfluous scans of the LLC domain.
 */
SCHED_FEAT(SIS_AVG_CPU, false)
SCHED_FEAT(SIS_PROP, true)

#ifdef HAVE_RT_PUSH_IPI
/*
 * In order to avoid a thundering herd attack of CPUs that are
 * lowering their priorities at the same time, and there being
 * a single CPU that has an RT task that can migrate and is waiting
 * to run, where the other CPUs will try to take that CPUs
 * rq lock and possibly create a large contention, sending an
 * IPI to that CPU and let that CPU push the RT task to where
 * it should go may be a better scenario.
 */
SCHED_FEAT(RT_PUSH_IPI, true)
#endif

SCHED_FEAT(FORCE_SD_OVERLAP, false)
SCHED_FEAT(RT_RUNTIME_SHARE, true)
SCHED_FEAT(LB_MIN, false)
SCHED_FEAT(ATTACH_AGE_LOAD, true)

SCHED_FEAT(WA_IDLE, true)
SCHED_FEAT(WA_WEIGHT, true)
SCHED_FEAT(WA_BIAS, true)

/*
 * UtilEstimation. Use estimated CPU utilization.
 */
SCHED_FEAT(UTIL_EST, true)
SCHED_FEAT(UTIL_EST_FASTUP, true)

/*
 * Energy aware scheduling. Use platform energy model to guide scheduling
 * decisions optimizing for energy efficiency.
 */
#ifdef CONFIG_DEFAULT_USE_ENERGY_AWARE
SCHED_FEAT(ENERGY_AWARE, true)
#else
SCHED_FEAT(ENERGY_AWARE, false)
#endif

/*
 * SchedTune. Use Performance/Energy filtering function to evaluate the trade
 * off between energy consumption and performance impact when comparing
 * previous and next candidate CPUs.
 */
SCHED_FEAT(ENERGY_FILTER, false)


/*
 * Energy aware scheduling algorithm choices:
 * EAS_PREFER_IDLE
 *   Direct tasks in a schedtune.prefer_idle=1 group through
 *   the EAS path for wakeup task placement. Otherwise, put
 *   those tasks through the mainline slow path.
 */
SCHED_FEAT(EAS_PREFER_IDLE, true)

/*
 * Enforce the priority of candidates selected by find_best_target()
 * ON: If the target CPU saves any energy, use that.
 * OFF: Use whichever of target or backup saves most.
 */
SCHED_FEAT(FBT_STRICT_ORDER, true)
SCHED_FEAT(HISI_FILTER, true)

#ifdef CONFIG_SCHED_EHMP
SCHED_FEAT(EXYNOS_HMP, true)
SCHED_FEAT(EXYNOS_HMP_OM, false)
#else
SCHED_FEAT(EXYNOS_HMP, false)
SCHED_FEAT(EXYNOS_HMP_OM, false)
#endif

/*
 * Apply schedtune boost hold to tasks of all sched classes.
 * If enabled, schedtune will hold the boost applied to a CPU
 * for 50ms regardless of task activation - if the task is
 * still running 50ms later, the boost hold expires and schedtune
 * boost will expire immediately the task stops.
 * If disabled, this behaviour will only apply to tasks of the
 * RT class.
 */
SCHED_FEAT(SCHEDTUNE_BOOST_HOLD_ALL, false)

/*
 * Minimum capacity capping. Keep track of minimum capacity factor when
 * minimum frequency available to a policy is modified.
 * If enabled, this can be used to inform the scheduler about capacity
 * restrictions.
 */
SCHED_FEAT(MIN_CAPACITY_CAPPING, true)

/*
 * Use the Simplified Energy Model for EAS accounting only for
 * active costs of CPUs.
 */
SCHED_FEAT(EAS_SIMPLIFIED_EM, true)
