#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "disassemble.h"
#include "vm.h"


void test_output() {
    printf("Hello from C!\n");
}

// In the brower you can use the following to run the source code
//
// run_source = Module.cwrap('run_source', 'void', ['string'])
// run_source('print "Hello from C!";')

void run_source(char* source) {
    InterpretResult result = interpret(source);
    free(source); // [owner]

    switch (result) {
        case INTERPRET_COMPILE_ERROR:
            exit(65);
        case INTERPRET_RUNTIME_ERROR:
            exit(70);
        default:
            break;
    }
}