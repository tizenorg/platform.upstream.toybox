/* mknod.c - make block or character special file
 *
 * Copyright 2012 Elie De Brauwer <eliedebrauwer@gmail.com>
 *
 * http://refspecs.linuxfoundation.org/LSB_4.1.0/LSB-Core-generic/LSB-Core-generic/mknod.html

USE_MKNOD(NEWTOY(mknod, "<2>4Z:", TOYFLAG_BIN))

config MKNOD
  bool "mknod"
  default y
  help
    usage: mknod [-Z CONTEXT] NAME TYPE [MAJOR MINOR]

    Create a special file NAME with a given type, possible types are
    b	block device
    c or u	character device
    p	named pipe (ignores MAJOR/MINOR)

    -Z	Set security context to created file
*/

#define FOR_mknod
#include "toys.h"

GLOBALS(
  char *arg_context;
)

#ifdef USE_SMACK
#include <sys/smack.h>
#include <linux/xattr.h>
#endif //USE_SMACK

void mknod_main(void)
{
  mode_t modes[] = {S_IFIFO, S_IFCHR, S_IFCHR, S_IFBLK};
  int major=0, minor=0, type;
  int mode = 0660;
#ifdef USE_SMACK
  char *label;
#endif

  if (toys.optflags & FLAG_Z) {
#ifdef USE_SMACK
	  /* That is usage of side effect. This changes current process smack label.
	   * All nodes created later by this process will get access label
	   * equal to process label that they were created by.
	   * TODO Maybe it would be more clean to use smack_label_length for label
	   * validation and then smack_set_label_for_path for setting labels for
	   * nodes, but those functions are only available on libsmack 1.1.
	   */
	  if(smack_set_label_for_self (TT.arg_context) < 0)
		  perror_exit("Failed to set context %s to %s\n", TT.arg_context,
				  toys.optargs[0]);

#else
	  printf("mknod: -Z works only with smack enabled toybox");
	  xputc('\n');
#endif
  }

  type = stridx("pcub", *toys.optargs[1]);
  if (type == -1) perror_exit("bad type '%c'", *toys.optargs[1]);
  if (type) {
    if (toys.optc != 4) perror_exit("need major/minor");

    major = atoi(toys.optargs[2]);
    minor = atoi(toys.optargs[3]);
  }

  if (mknod(toys.optargs[0], mode | modes[type], makedev(major, minor))) {
    perror_exit("mknod %s failed", toys.optargs[0]);
  }
#ifdef USE_SMACK
  else {
    if(toys.optflags & FLAG_Z) {
      smack_new_label_from_path(toys.optargs[0], XATTR_NAME_SMACK, 0, &label);
      if (strcmp(label, TT.arg_context) != 0)
        fprintf(stderr, "Warning: SMACK label of %s set to '%s' and not '%s' due "
          "to label transmutation\n", toys.optargs[0], label, TT.arg_context);
      free(label);
    }
  }

#endif
}
