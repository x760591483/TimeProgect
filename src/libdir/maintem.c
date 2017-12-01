#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>


int main(int argc, char **argv)
{
    void *handle;
    int (*init)(void*);
    char *error;
    handle = dlopen("./libtypelib.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);

    }
    dlerror();    /* Clear any existing error */
    /* Writing: cosine = (double (*)(double)) dlsym(handle, "cos");
     *               would seem more natural, but the C99 standard leaves
     *                             casting from "void *" to a function pointer undefined.
     *                                           The assignment used below is the POSIX.1-2003 (Technical
     *                                                         Corrigendum 1) workaround; see the Rationale for the
     *                                                                       POSIX specification of dlsym(). */
    *(void **) (&init) = dlsym(handle, "sysinit");
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);

    }
    init(NULL);
    
    dlclose(handle);
    exit(EXIT_SUCCESS);

}

