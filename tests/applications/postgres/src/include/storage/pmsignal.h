/*-------------------------------------------------------------------------
 *
 * pmsignal.h
 *	  routines for signaling the postmaster from its child processes
 *
 *
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/storage/pmsignal.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PMSIGNAL_H
#define PMSIGNAL_H

#include <signal.h>

#ifdef HAVE_SYS_PRCTL_H
#include "sys/prctl.h"
#endif

#ifdef HAVE_SYS_PROCCTL_H
#include "sys/procctl.h"
#endif

/*
 * Reasons for signaling the postmaster.  We can cope with simultaneous
 * signals for different reasons.  If the same reason is signaled multiple
 * times in quick succession, however, the postmaster is likely to observe
 * only one notification of it.  This is okay for the present uses.
 */
typedef enum
{
	PMSIGNAL_RECOVERY_STARTED,	/* recovery has started */
	PMSIGNAL_BEGIN_HOT_STANDBY, /* begin Hot Standby */
	PMSIGNAL_WAKEN_ARCHIVER,	/* send a NOTIFY signal to xlog archiver */
	PMSIGNAL_ROTATE_LOGFILE,	/* send SIGUSR1 to syslogger to rotate logfile */
	PMSIGNAL_START_AUTOVAC_LAUNCHER,	/* start an autovacuum launcher */
	PMSIGNAL_START_AUTOVAC_WORKER,	/* start an autovacuum worker */
	PMSIGNAL_BACKGROUND_WORKER_CHANGE,	/* background worker state change */
	PMSIGNAL_START_WALRECEIVER, /* start a walreceiver */
	PMSIGNAL_ADVANCE_STATE_MACHINE, /* advance postmaster's state machine */

	NUM_PMSIGNALS				/* Must be last value of enum! */
} PMSignalReason;

/* PMSignalData is an opaque struct, details known only within pmsignal.c */
typedef struct PMSignalData PMSignalData;

/*
 * prototypes for functions in pmsignal.c
 */
extern Size PMSignalShmemSize(void);
extern void PMSignalShmemInit(void);
extern void SendPostmasterSignal(PMSignalReason reason);
extern bool CheckPostmasterSignal(PMSignalReason reason);
extern int	AssignPostmasterChildSlot(void);
extern bool ReleasePostmasterChildSlot(int slot);
extern bool IsPostmasterChildWalSender(int slot);
extern void MarkPostmasterChildActive(void);
extern void MarkPostmasterChildInactive(void);
extern void MarkPostmasterChildWalSender(void);
extern bool PostmasterIsAliveInternal(void);
extern void PostmasterDeathSignalInit(void);


/*
 * Do we have a way to ask for a signal on parent death?
 *
 * If we do, pmsignal.c will set up a signal handler, that sets a flag when
 * the parent dies.  Checking the flag first makes PostmasterIsAlive() a lot
 * cheaper in usual case that the postmaster is alive.
 */
// #if (defined(HAVE_SYS_PRCTL_H) && defined(PR_SET_PDEATHSIG)) || \
// 	(defined(HAVE_SYS_PROCCTL_H) && defined(PROC_PDEATHSIG_CTL))
// #define USE_POSTMASTER_DEATH_SIGNAL
// #endif
// LIND: we don't have any of this, lets not define it

#ifdef USE_POSTMASTER_DEATH_SIGNAL
extern volatile sig_atomic_t postmaster_possibly_dead;

static inline bool
PostmasterIsAlive(void)
{
	if (likely(!postmaster_possibly_dead))
		return true;
	return PostmasterIsAliveInternal();
}
#else
#define PostmasterIsAlive() PostmasterIsAliveInternal()
#endif

#endif							/* PMSIGNAL_H */
