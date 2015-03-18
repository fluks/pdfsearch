#ifndef CONFIG_H
    #define CONFIG_H

namespace Pdfsearch {
    /** Program and system specific constants.
     * This class defines constants for the program, such as file paths to
     * resources. make can change these on install.
     * @note An object of this class cannot be instantiated.
     */
    struct Config {
        Config() = delete;

        constexpr static const char* VERSION = "0.01";
        constexpr static const char* PROGRAM_NAME = "pdfsearch";
        constexpr static const char* CONFIG_FILE = "src/dummy.conf";
        constexpr static const char* DATABASE_FILE = "pdfsearch.sqlite";
    };
}

#endif // CONFIG_H
