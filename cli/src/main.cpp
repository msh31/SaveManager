void help() {
    std::println("");
    std::println("");
    std::println("");
    std::println("");
    std::println("");
}

int main(int argc, char* argv[]) {
    if(argc < 2) { 
        SPDLOG_ERROR("Not enough arguments provided!");
        help();
        return 0; 
    }

    std::println("hello world");
    return 0;
}
