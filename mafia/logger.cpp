// Based on https://www.geeksforgeeks.org/logging-system-in-cpp/
// C++ program to implement a basic logging system.

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

// Enum to represent log levels
enum class Loglevel { INFO };

class Logger {
public:
    // Constructor: Opens the log file in append mode
    Logger(const std::string& filename)
    {
        logsDir = std::filesystem::current_path() / "logs";
        std::filesystem::create_directory(logsDir);
        logFile.open(logsDir / filename, std::ios::app);
    }

    // Destructor: Closes the log file
    ~Logger() { logFile.close(); }

    // Logs a message with a given log level
    void log(Loglevel level, const std::string& message)
    {

        // Create log entry
        std::ostringstream logEntry;
        // logEntry << "[" << timestamp << "] "
        logEntry << levelToString(level) << ": " << message
                 << std::endl;

        // Output to log file
        if (logFile.is_open()) {
            logFile << logEntry.str();
            logFile.flush(); // Ensure immediate write to file
        }
    }

private:
    std::filesystem::path logsDir;
    std::ofstream logFile; // File stream for the log file

    // Converts log level to a string for output
    std::string levelToString(Loglevel level)
    {
        switch (level) {
            case Loglevel::INFO:
                return "INFO";
            default:
                return "UNKNOWN";
        }
    }
};
