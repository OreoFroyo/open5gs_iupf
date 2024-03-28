#include "ogs-app.h"

int app_initialize(const char *const argv[])
{
    int rv;

    rv = esac_initialize();
    if (rv != OGS_OK) {
        ogs_error("Failed to intialize EASC");
        return rv;
    }
    ogs_info("EASC initialize...done");

    return OGS_OK;
}

void app_terminate(void)
{
    iupf_terminate();
    ogs_info("ESAC terminate...done");
}