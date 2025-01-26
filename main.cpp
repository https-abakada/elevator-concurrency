#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <chrono>
#include <cmath>
#include <cctype>
#include <algorithm>

std::mutex mtx;

int numFloors;
int numElevators;
std::vector<int> elevators;
std::vector<std::string> elevatorStates;

void displayElevators() {
    std::lock_guard<std::mutex> lock(mtx);
    system("cls");
    for (int i = numFloors; i >= 1; --i) {
        for (int j = 0; j < numElevators; ++j) {
            if (i == elevators[j]) {
                std::cout << "|   E" << (j + 1) << "    ";
            } else {
                std::cout << "|         ";
            }
        }
        std::cout << "| Floor " << i << "\n";
    }
    std::cout << "\n\n";
}

void moveElevator(int elevatorIndex, int targetFloor) {
    while (elevators[elevatorIndex] != targetFloor) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (elevators[elevatorIndex] < targetFloor) {
            elevators[elevatorIndex]++;
            elevatorStates[elevatorIndex] = "up";
        } else {
            elevators[elevatorIndex]--;
            elevatorStates[elevatorIndex] = "down";
        }
        displayElevators();
    }
    elevatorStates[elevatorIndex] = "idle";
}

void handleRequest(int currentFloor, int destination) {
    int bestElevator = -1;
    int minDistance = numFloors + 1;

    for (int i = 0; i < numElevators; ++i) {
        if (elevatorStates[i] == "idle" || 
           (elevatorStates[i] == "up" && destination > elevators[i] && currentFloor >= elevators[i]) || 
           (elevatorStates[i] == "down" && destination < elevators[i] && currentFloor <= elevators[i])) {
            int distance = std::abs(elevators[i] - currentFloor);
            if (distance < minDistance) {
                minDistance = distance;
                bestElevator = i;
            }
        }
    }

    if (bestElevator == -1) {
        for (int i = 0; i < numElevators; ++i) {
            if (elevatorStates[i] == "idle") {
                int distance = std::abs(elevators[i] - currentFloor);
                if (distance < minDistance) {
                    minDistance = distance;
                    bestElevator = i;
                }
            }
        }
    }

    if (bestElevator != -1) {
        std::cout << "Elevator " << (bestElevator + 1) << " is coming to pick you up!\n";
        moveElevator(bestElevator, currentFloor);
        moveElevator(bestElevator, destination);
    } else {
        std::cout << "No elevators available to handle the request.\n";
    }
}

int main() {
    do {
        std::cout << "Enter the number of floors (9-15): ";
        std::cin >> numFloors;
        if (std::cin.fail() || numFloors < 9 || numFloors > 15) {
            std::cout << "Invalid input. Please enter a number between 9 and 15.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        } else {
            break;
        }
    } while (true);

    do {
        std::cout << "Enter the number of elevators (3-6): ";
        std::cin >> numElevators;
        if (std::cin.fail() || numElevators < 3 || numElevators > 6) {
            std::cout << "Invalid input. Please enter a number between 3 and 6.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        } else {
            break;
        }
    } while (true);

    elevators.resize(numElevators, 1);
    elevatorStates.resize(numElevators, "idle");
    displayElevators();

    while (true) {
        int currentFloor, destination;
        char direction;

        do {
            std::cout << "Enter your current floor (1-" << numFloors << "): ";
            std::cin >> currentFloor;
            if (std::cin.fail() || currentFloor < 1 || currentFloor > numFloors) {
                std::cout << "Invalid floor. Please enter a floor between 1 and " << numFloors << ".\n";
                std::cin.clear();
                std::cin.ignore(10000, '\n');
            } else {
                break;
            }
        } while (true);

        do {
            std::cout << "Going up or down? (u/d): ";
            std::cin >> direction;
            direction = tolower(direction);
            if (direction != 'u' && direction != 'd') {
                std::cout << "Invalid direction. Please enter 'u' for up or 'd' for down.\n";
            } else {
                break;
            }
        } while (true);

        do {
            std::cout << "Enter your destination floor (1-" << numFloors << "): ";
            std::cin >> destination;
            if (std::cin.fail() || destination < 1 || destination > numFloors) {
                std::cout << "Invalid floor. Please enter a floor between 1 and " << numFloors << ".\n";
                std::cin.clear();
                std::cin.ignore(10000, '\n');
            } else if ((direction == 'u' && destination <= currentFloor) || 
                       (direction == 'd' && destination >= currentFloor)) {
                std::cout << "Invalid destination. If going up, enter a higher floor. If going down, enter a lower floor.\n";
            } else {
                break;
            }
        } while (true);

        std::thread requestThread(handleRequest, currentFloor, destination);
        requestThread.join();
        displayElevators();
    }

    return 0;
}