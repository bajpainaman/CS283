#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dshlib.h"

/*
 * DO NOT EDIT:
 * main() is just a wrapper around exec_local_cmd_loop().
 */
int main(void) {
    int rc = exec_local_cmd_loop();
    printf("cmd loop returned %d\n", rc);
    return rc;
}
