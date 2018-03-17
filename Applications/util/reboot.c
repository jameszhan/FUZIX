/*
 *	A simple reboot and halt
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  int pv = 0;
  char *p = strchr(argv[0], '/');
  if (p)
    p++;
  else
    p = argv[0];
  if (argc == 2 && strcmp(argv[1], "-f") == 0) {
    argc--;
    pv = AD_NOSYNC;
  }
  if (argc != 1) {
    write(2, "unexpected argument.\n", 21);
    exit(1);
  }
    
  if (strcmp(p, "halt") == 0)
    uadmin(A_SHUTDOWN, pv, 0);
  else
    uadmin(A_REBOOT, pv, 0);
  /* If we get here there was an error! */
  perror(argv[0]);
  return 1;
}
