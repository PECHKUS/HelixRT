/*
 * HelixRT - System Call Numbers
 *
 * SVC immediate values used by user wrappers and SVC dispatcher.
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#define SVC_START_FIRST_TASK  0

#define SVC_TASK_CREATE       1
#define SVC_TASK_DELETE       2
#define SVC_TASK_SUSPEND      3
#define SVC_TASK_RESUME       4
#define SVC_TASK_YIELD        5
#define SVC_TASK_DELAY        6
#define SVC_TASK_PRIORITY     7

#endif // SYSCALL_H 
