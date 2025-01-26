#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

std::mutex elevatorMutex; 
std::vector<int> elevators;
std::vector<std::string> elevatorStates;
int numFloors;
int numElevators;

void displayElevators() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
    system("cls");
    for (int floor = numFloors; floor >= 1; --floor) {
        for (int i = 0; i < numElevators; ++i) {
            if (elevators[i] == floor) {
                std::cout << "|   E" << (i + 1) << "    ";
            } else {
                std::cout << "|         ";
            }
        }
        std::cout << "| Floor " << floor << std::endl;
    }
    for (int i = 0; i < numElevators; ++i) {
        std::cout << "==========";
    }
    std::cout << "=" << std::endl;
}

void moveElevator(int elevatorIndex, int targetFloor) {
    while (elevators[elevatorIndex] != targetFloor) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        {
            std::lock_guard<std::mutex> lock(elevatorMutex);
            if (elevators[elevatorIndex] < targetFloor) elevators[elevatorIndex]++;
            else if (elevators[elevatorIndex] > targetFloor) elevators[elevatorIndex]--;
        }

        displayElevators();
    }
}

void handleRequest(int currentFloor, int destination) {
    int bestElevator = -1;
    int minDistance = numFloors + 1;

    for (int i = 0; i < numElevators; ++i) {
        int distance = std::abs(elevators[i] - currentFloor);
        {
            std::lock_guard<std::mutex> lock(elevatorMutex);
            if ((elevatorStates[i] == "idle" || 
                 (elevatorStates[i] == "up" && destination > elevators[i] && currentFloor >= elevators[i]) ||
                 (elevatorStates[i] == "down" && destination < elevators[i] && currentFloor <= elevators[i])) && 
                distance < minDistance) {
                bestElevator = i;
                minDistance = distance;
            }
        }
    }

    if (bestElevator != -1) {
        {
            std::lock_guard<std::mutex> lock(elevatorMutex);
            elevatorStates[bestElevator] = (destination > currentFloor) ? "up" : "down";
        }

        moveElevator(bestElevator, currentFloor);
        moveElevator(bestElevator, destination);

        {
            std::lock_guard<std::mutex> lock(elevatorMutex);
            elevatorStates[bestElevator] = "idle";
        }
    }
}

bool validateInput(int floor, char direction, int destination) {
    if (floor < 1 || floor > numFloors || destination < 1 || destination > numFloors) {
        std::cout << "Invalid floor. Please enter a floor between 1 and " << numFloors << ".\n";
        return false;
    }
    if (direction == 'u' && destination <= floor) {
        std::cout << "You pressed UP but selected a lower floor. Try again.\n";
        return false;
    }
    if (direction == 'd' && destination >= floor) {
        std::cout << "You pressed DOWN but selected a higher floor. Try again.\n";
        return false;
    }
    return true;
}

int main() {
    std::cout << "Enter the number of floors (9-15): ";
    std::cin >> numFloors;
    while (numFloors < 9 || numFloors > 15) {
        std::cout << "Invalid input. Enter a number between 9 and 15: ";
        std::cin >> numFloors;
    }

    std::cout << "Enter the number of elevators (3-6): ";
    std::cin >> numElevators;
    while (numElevators < 3 || numElevators > 6) {
        std::cout << "Invalid input. Enter a number between 3 and 6: ";
        std::cin >> numElevators;
    }

    elevators = std::vector<int>(numElevators, 1);
    elevatorStates = std::vector<std::string>(numElevators, "idle");

    while (true) {
        displayElevators();

        int currentFloor, destination;
        char direction;

        std::cout << "Enter your current floor (1-" << numFloors << "): ";
        std::cin >> currentFloor;
        std::cout << "Going up or down? (u/d): ";
        std::cin >> direction;
        std::cout << "Enter your destination floor (1-" << numFloors << "): ";
        std::cin >> destination;

        if (!validateInput(currentFloor, direction, destination)) {
            continue;
        }

        std::thread requestThread(handleRequest, currentFloor, destination);
        requestThread.detach();
    }

    return 0;
}
