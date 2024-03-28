
#include <string>
#include "platform/platform.h"
#include "synio.h"

//
int main(int argc, char *argv[])
{
    // std::string filename = "short.cpp";
    std::string filename = "test.cpp";
    // std::string filename = "tabbed_file.txt";

    if (argc > 1)
        filename = std::string(argv[1]);

    //
    Log::open();

    set_backend();
    api->initialize();

    {
        Synio synio(filename);
    }

    api->shutdown();

    Log::close();

    return 0;

}
