#include <iostream>
#include "mappercore.h"

#include <functional>
static bool console_handler(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char *msg, size_t len){
    std::cout.write(msg, len);
    std::cout << std::endl;
    return true;
}

static bool event_handler(MapperHandle mapper, MAPPER_EVENT ev, int64_t data){
    return true;
}

int main(int argc, char* argv[]){
    if (argc < 2){
        std::cerr << "usage: " << argv[0] << "script-path" << std::endl;
        return 1;
    }

    MapperHandle mapper = mapper_init(event_handler, console_handler, nullptr);
    mapper_run(mapper, argv[1]);

    return 0;
}