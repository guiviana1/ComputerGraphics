#pragma once

#include <string>

struct RunningOptions {
    std::string input_file;
    bool quick_render = false;
};

class API {
public:
    static void init_engine(const RunningOptions& opt);
    static void run();
    static void clean_up();

private:
    static RunningOptions options;
};
