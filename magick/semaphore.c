/*
% Copyright (C) 2003 GraphicsMagick Group
% Copyright (C) 2002 ImageMagick Studio
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%        SSSSS  EEEEE  M   M   AAA   PPPP   H   H   OOO   RRRR   EEEEE        %
%        SS     E      MM MM  A   A  P   P  H   H  O   O  R   R  E            %
%         SSS   EEE    M M M  AAAAA  PPPP   HHHHH  O   O  RRRR   EEE          %
%           SS  E      M   M  A   A  P      H   H  O   O  R R    E            %
%        SSSSS  EEEEE  M   M  A   A  P      H   H   OOO   R  R   EEEEE        %
%                                                                             %
%                                                                             %
%                     GraphicsMagick Semaphore Methods                        %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                             William Radcliffe                               %
%                                John Cristy                                  %
%                                 June 2000                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/utility.h"
#if defined(HAVE_PTHREAD)
#include <pthread.h>
#endif
#if defined(WIN32)
#include <windows.h>
#define USE_SPINLOCKS
#define SPINLOCK_DELAY_MILLI_SECS 10
#endif
#include "magick/semaphore.h"

/*
  Struct declaractions.
*/
struct SemaphoreInfo
{
#if defined(HAVE_PTHREAD)
  pthread_mutex_t
    mutex;		/* POSIX thread mutex */

  pthread_t
    thread_id;		/* ID of thread which holds the lock */
#endif
#if defined(WIN32)
  CRITICAL_SECTION
    mutex;		/* Windows critical section */

  DWORD
    thread_id;		/* ID of thread which holds the lock */
#endif

  unsigned int
    locked;		/* True if semaphore is locked */

  unsigned long
    signature;		/* Used to validate structure */
};

/*
  Static declaractions.
*/
#if defined(HAVE_PTHREAD)
static pthread_mutex_t
  semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#if defined(WIN32)
#if !defined(USE_SPINLOCKS)
static CRITICAL_SECTION
  semaphore_mutex;

static unsigned int
  active_semaphore = False;
#else
static int
  semaphore_mutex = 0;
/* Wait for spin lock */
static void spinlock_wait (int *sl)
{
  while (InterlockedCompareExchange (sl, 1, 0) != 0)
  {
    /* slight delay - just in case OS does not giveup CPU */
    Sleep (SPINLOCK_DELAY_MILLI_SECS);
  }
}
/* Release spin lock */
static void spinlock_release (int *sl)
{
  InterlockedExchange (sl, 0);
}
#endif
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S e m a p h o r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireSemaphoreInfo() acquires a semaphore.
%
%  The format of the AcquireSemaphoreInfo method is:
%
%      AcquireSemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
%
*/
MagickExport void AcquireSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
#if defined(HAVE_PTHREAD)
  (void) pthread_mutex_lock(&semaphore_mutex);
#endif
#if defined(WIN32)
#if !defined(USE_SPINLOCKS)
  if (!active_semaphore)
    InitializeCriticalSection(&semaphore_mutex);
  active_semaphore=True;
  EnterCriticalSection(&semaphore_mutex);
#else
  spinlock_wait(&semaphore_mutex);
#endif
#endif
  if (*semaphore_info == (SemaphoreInfo *) NULL)
    *semaphore_info=AllocateSemaphoreInfo();
#if defined(HAVE_PTHREAD)
  (void) pthread_mutex_unlock(&semaphore_mutex);
#endif
#if defined(WIN32)
#if !defined(USE_SPINLOCKS)
  LeaveCriticalSection(&semaphore_mutex);
#else
  spinlock_release(&semaphore_mutex);
#endif
#endif
  (void) LockSemaphoreInfo(*semaphore_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e S e m a p h o r e I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method AllocateSemaphoreInfo initializes the SemaphoreInfo structure.
%
%  The format of the AllocateSemaphoreInfo method is:
%
%      SemaphoreInfo *AllocateSemaphoreInfo(void)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Method AllocateSemaphoreInfo returns a pointer to an
%      initialized SemaphoreInfo structure.
%
%
*/
MagickExport SemaphoreInfo *AllocateSemaphoreInfo(void)
{
  SemaphoreInfo
    *semaphore_info;

  /*
    Allocate semaphore.
  */
  semaphore_info=MagickAllocateMemory(SemaphoreInfo *,sizeof(SemaphoreInfo));
  if (semaphore_info == (SemaphoreInfo *) NULL)
    MagickFatalError3(ResourceLimitFatalError,MemoryAllocationFailed,
      UnableToAllocateSemaphoreInfo);
  memset(semaphore_info,0,sizeof(SemaphoreInfo));
  /*
    Initialize the semaphore.
  */
#if defined(HAVE_PTHREAD)
  {
    int
      status;

    status=pthread_mutex_init(&semaphore_info->mutex,
      (const pthread_mutexattr_t *) NULL);
    if (status != 0)
      {
        MagickFreeMemory(semaphore_info);
        return((SemaphoreInfo *) NULL);
      }
  }
#endif
#if defined(WIN32)
  InitializeCriticalSection(&semaphore_info->mutex);
#endif
  semaphore_info->locked=False;
  semaphore_info->signature=MagickSignature;
  return(semaphore_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S e m a p h o r e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySemaphore() destroys the semaphore environment.
%
%  The format of the DestroySemaphore method is:
%
%      DestroySemaphore(void)
%
%
*/
MagickExport void DestroySemaphore(void)
{
#if defined(HAVE_PTHREAD)
  (void) pthread_mutex_destroy(&semaphore_mutex);
#endif
#if defined(WIN32)
#if !defined(USE_SPINLOCKS)
  if (active_semaphore)
    DeleteCriticalSection(&semaphore_mutex);
  active_semaphore=False;
#endif
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S e m a p h o r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DestroySemaphoreInfo destroys a semaphore.
%
%  The format of the DestroySemaphoreInfo method is:
%
%      DestroySemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
%
*/
MagickExport void DestroySemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
  if (*semaphore_info == (SemaphoreInfo *) NULL)
    return;
  assert((*semaphore_info)->signature == MagickSignature);
#if defined(HAVE_PTHREAD)
  (void) pthread_mutex_lock(&semaphore_mutex);
#endif
#if defined(WIN32)
#if !defined(USE_SPINLOCKS)
  if (!active_semaphore)
    InitializeCriticalSection(&semaphore_mutex);
  active_semaphore=True;
  EnterCriticalSection(&semaphore_mutex);
#else
  spinlock_wait(&semaphore_mutex);
#endif
#endif
#if defined(HAVE_PTHREAD)
  (void) pthread_mutex_destroy(&(*semaphore_info)->mutex);
#endif
#if defined(WIN32)
  DeleteCriticalSection(&(*semaphore_info)->mutex);
#endif
  MagickFreeMemory((*semaphore_info));
#if defined(HAVE_PTHREAD)
  (void) pthread_mutex_unlock(&semaphore_mutex);
#endif
#if defined(WIN32)
#if !defined(USE_SPINLOCKS)
  LeaveCriticalSection(&semaphore_mutex);
#else
  spinlock_release(&semaphore_mutex);
#endif
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e S e m a p h o r e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method InitializeSemaphore initializes the semaphore environment.
%
%  The format of the InitializeSemaphore method is:
%
%      InitializeSemaphore(void)
%
%
*/
MagickExport void InitializeSemaphore(void)
{
#if defined(HAVE_PTHREAD)
  (void) pthread_mutex_init(&semaphore_mutex,
    (const pthread_mutexattr_t *) NULL);
#endif
#if defined(WIN32)
#if !defined(USE_SPINLOCKS)
  if (!active_semaphore)
    InitializeCriticalSection(&semaphore_mutex);
  active_semaphore=True;
#endif
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i b e r a t e S e m a p h o r e I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method LiberateSemaphoreInfo liberates a semaphore.
%
%  The format of the LiberateSemaphoreInfo method is:
%
%      LiberateSemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
%
*/
MagickExport void LiberateSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
  if (*semaphore_info == (SemaphoreInfo *) NULL)
    return;
  assert((*semaphore_info)->signature == MagickSignature);
  (void) UnlockSemaphoreInfo(*semaphore_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c k S e m a p h o r e I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method LockSemaphoreInfo locks a semaphore.
%
%  The format of the LockSemaphoreInfo method is:
%
%      unsigned int LockSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o status:  Method LockSemaphoreInfo returns True on success otherwise
%      False.
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
%
*/
MagickExport unsigned int LockSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == MagickSignature);
#if defined(HAVE_PTHREAD)
  if (pthread_mutex_lock(&semaphore_info->mutex))
    return(False);
  /* Record the thread ID of the locking thread */
  semaphore_info->thread_id=pthread_self();
#endif
#if defined(WIN32)
  EnterCriticalSection(&semaphore_info->mutex);
  /* Record the thread ID of the locking thread */
  semaphore_info->thread_id=GetCurrentThreadId();
#endif
  semaphore_info->locked=True;
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n l o c k S e m a p h o r e I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnlockSemaphoreInfo unlocks a semaphore.
%
%  The format of the LockSemaphoreInfo method is:
%
%      unsigned int UnlockSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o status:  Method UnlockSemaphoreInfo returns True on success otherwise
%      False.
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
%
*/
MagickExport unsigned int UnlockSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == MagickSignature);
  if (semaphore_info->locked != True)
    return (False);
  semaphore_info->locked=False;
#if defined(HAVE_PTHREAD)
  /* Enforce that unlocking thread is the same as the locking thread */
  assert(pthread_equal(semaphore_info->thread_id,pthread_self()));
  if (pthread_mutex_unlock(&semaphore_info->mutex))
    return(False);
#endif
#if defined(WIN32)
  /* Enforce that unlocking thread is the same as the locking thread */
  assert(GetCurrentThreadId() == semaphore_info->thread_id);
  LeaveCriticalSection(&semaphore_info->mutex);
#endif
  return(True);
}