/* mkfifo.c - Create FIFOs (named pipes)
 *
 * Copyright 2012 Georgi Chorbadzhiyski <georgi@unixsol.org>
 *
 * See http://opengroup.org/onlinepubs/9699919799/utilities/mkfifo.html

USE_MKFIFO(NEWTOY(mkfifo, "<1m:Z:", TOYFLAG_BIN))

config MKFIFO
  bool "mkfifo"
  default y
  help
    usage: mkfifo [-Z context] [fifo_name...]

    Create FIFOs (named pipes).

    -Z	Set security context to created file
*/

#define FOR_mkfifo
#include "toys.h"

#ifdef USE_SMACK
#include <sys/smack.h>
#include <linux/xattr.h>
#endif //USE_SMACK

GLOBALS(
  char *arg_context;
  char *m_string;
  mode_t mode;
)

void mkfifo_main(void)
{
  char **s;
#ifdef USE_SMACK
  char *label;
#endif

  if (toys.optflags & FLAG_Z) {
#ifdef USE_SMACK
  /* That is usage of side effect. This changes current process smack label.
   * All FIFO special files created later by this process will get access label
   * equal to process label that they were created by.
   * TODO Maybe it would be more clean to use smack_label_length for label
   * validation and then smack_set_label_for_path for setting labels for
   * FIFO files, but those functions are only available on libsmack 1.1.
   */
    if(smack_set_label_for_self (TT.arg_context) < 0)
      perror_exit("Failed to set context %s to %s\n", TT.arg_context,
        toys.optargs[0]);

#else
  printf("mkfifo: -Z works only with smack enabled toybox");
  xputc('\n');
#endif
  }

  TT.mode = 0666;
  if (toys.optflags & FLAG_m) TT.mode = string_to_mode(TT.m_string, 0);

  for (s = toys.optargs; *s; s++) {
    if (mknod(*s, S_IFIFO | TT.mode, 0) < 0) {
      perror_msg("%s", *s);
    }
#ifdef USE_SMACK
    else {
      if(toys.optflags & FLAG_Z) {
        smack_new_label_from_path(*s, XATTR_NAME_SMACK, 0, &label);
          if (strcmp(label, TT.arg_context) != 0)
            fprintf(stderr, "Warning: SMACK label of %s set to '%s' and not '%s' due "
              "to label transmutation\n", *s, label, TT.arg_context);
          free(label);
      }
    }
#endif
  }
}
