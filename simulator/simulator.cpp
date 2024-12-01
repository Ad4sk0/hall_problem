#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <functional>
#include <format>
#include <stdexcept>

struct DoorState {
    bool selected = false;
    bool hasCar = false;
    bool open = false;
};

enum class LogLevel {
    DEBUG,
    INFO,
    WARN
};

class Logger {
    LogLevel logLevel;

public:
    explicit Logger(LogLevel level) : logLevel(level) {}

    void log(const std::string& message, LogLevel level) const {
        if (level >= logLevel) {
            std::cout << message << std::endl;
        }
    }

    void debug(const std::string& message) const {
        log(message, LogLevel::DEBUG);
    }

    void info(const std::string& message) const {
        log(message, LogLevel::INFO);
    }
};

class Simulator {
    static constexpr unsigned int DOORS_NUMBER = 3;
    std::mt19937 gen;
    Logger logger;

    void printBoard(const std::vector<DoorState>& board) const {
        std::string displayValue;
        for (const auto& door : board) {
            std::string symbol = door.hasCar ? "C" : (door.open ? " " : "X");
            std::string left = door.selected ? "{" : "[";
            std::string right = door.selected ? "}" : "]";
            displayValue += std::format("{}{}{} ", left, symbol, right);
        }
        logger.debug(displayValue);
    }

    int getRandom(int max) {
        std::uniform_int_distribution<int> dist(0, max);
        return dist(gen);
    }

    bool simulateSingleGame(bool changePlayerDecision) {
        std::vector<DoorState> board(DOORS_NUMBER);

        int carIndex = getRandom(board.size() - 1);
        board[carIndex].hasCar = true;
        logger.debug(std::format("Car is at door {}", carIndex + 1));

        int playerChoiceIndex = getRandom(board.size() - 1);
        board[playerChoiceIndex].selected = true;
        logger.debug(std::format("Player chooses door {}", playerChoiceIndex + 1));

        logger.debug("Initial board:");
        printBoard(board);

        int openedDoorIndex = chooseRandomIndex(board, [](const DoorState& door) {
            return !door.selected && !door.hasCar;
            });
        board[openedDoorIndex].open = true;
		logger.debug(std::format("Doors {} opened", openedDoorIndex + 1));
		logger.debug("Current board:");
        printBoard(board);

        if (changePlayerDecision) {
            int newChoiceIndex = chooseRandomIndex(board, [](const DoorState& door) {
                return !door.selected && !door.open;
                });
            logger.debug(std::format("Player changes choice to door {}", newChoiceIndex + 1));
            board[playerChoiceIndex].selected = false;
            board[newChoiceIndex].selected = true;
            playerChoiceIndex = newChoiceIndex;
            logger.debug("Board after player changes choice:");
            printBoard(board);
        }

        return board[playerChoiceIndex].hasCar;
    }

    void runSimulation(int simulationsNumber, bool changePlayerDecision) {
        int winCount = 0;
        for (int i = 0; i < simulationsNumber; ++i) {
            winCount += simulateSingleGame(changePlayerDecision);
        }

        double winRatio = static_cast<double>(winCount) / simulationsNumber;
        logger.info(std::format("Simulations: {}, Wins: {}, Win Ratio: {:.2f}", simulationsNumber, winCount, winRatio));
    }

    int chooseRandomIndex(const std::vector<DoorState>& board, const std::function<bool(const DoorState&)>& condition) {
        std::vector<int> indices;
        for (size_t i = 0; i < board.size(); ++i) {
            if (condition(board[i])) {
                indices.push_back(i);
            }
        }

        if (indices.empty()) {
            throw std::invalid_argument("No suitable indices available");
        }

        return indices[getRandom(indices.size() - 1)];
    }

public:
    explicit Simulator(Logger logger) : logger(std::move(logger)) {
        std::random_device rd;
        gen.seed(rd());
    }

    void simulate(int simulationsNumber) {
		logger.info("Running simulation where the player does not change the door:");
        runSimulation(simulationsNumber, false);

		logger.info("Running simulation where the player changes the door:");
        runSimulation(simulationsNumber, true);
    }
};

void parseArguments(int argc, char* argv[], int& simulations, LogLevel& logLevel) {
    simulations = 10000;
    logLevel = LogLevel::INFO;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--simulations" && i + 1 < argc) {
            simulations = std::stoi(argv[++i]);
        }
        else if (arg == "--verbose") {
            logLevel = LogLevel::DEBUG;
        }
        else {
            throw std::invalid_argument("Invalid argument: " + arg);
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        int simulations;
        LogLevel logLevel;
        parseArguments(argc, argv, simulations, logLevel);
        
        Logger logger(logLevel);
        Simulator simulator(logger);
        simulator.simulate(simulations);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Usage: " << argv[0] << " [--simulations <number>] [--verbose]" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
