
#include <string>
#include "platform/platform.h"
#include "synio.h"


int main(int argc, char *argv[])
{
    std::string filename = "";
    // filename = "test.cpp";
    // filename = "short.cpp";
    // filename = "scroll.txt";
    // filename = "tabbed_file.txt";

    if (argc > 1)
        filename = std::string(argv[1]);

    //
    #ifdef DEBUG
    Log::open("/home/iomanip/source/synio/build/log.txt");
    #endif

    set_backend();
    api->initialize();

    {
        Synio synio(filename);
    }

    api->shutdown();

    Log::close();

    return 0;

}
