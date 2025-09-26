// Battleship.cpp : This file contains the 'main' function. Program execution begins and ends there.
// by mason


#include <iostream>
#include <vector>
#include <windows.h>
#include <map>
#include <conio.h>  // for _kbhit
#include <random>
#include <string>



enum class Scenes {
    Intro,
    GameLoop,
    GameOver
};
Scenes current_state = Scenes::Intro;


std::vector<std::vector<std::pair<std::string, std::string>>> generateBattlefield();
void generateShips();
void drawBattlefield();
void shoot();
std::string numToString(int num);
void receive();
int stringToNum(std::string letter);
bool didTheShotSinkTheShip(std::string shipType);
bool isClearToFire(int indexXCoord, int indexYCoord);
void drawEnemyBattlefield();
void updateEnemyMap();


//globals
std::string goodMapOrNot;
std::string order;
std::vector<std::pair<int, int>> shotsMissed; // computer value
std::vector<std::pair<int, int>> shotsLanded; // computer value
std::vector<std::vector<std::pair<std::string, std::string>>> map = generateBattlefield();
std::vector<std::vector<std::pair<std::string, std::string>>> enemyMap = generateBattlefield();

// global - shooting info
struct HitShipInfo {
    std::pair<int, int> compCoord = {}; // index version not display version
    int circleState = 1; // 1 starts above then goes clockwise 1-4
    std::string vertOrHoriz = "idk";
    std::pair<int, int > coordFromCircle = {};
};
std::vector<HitShipInfo> getTarget = {};

enum class ShootAlgSteps {
    Random,
    Circle,
    CheckInDirection,
    DoubleBack
};
ShootAlgSteps currentShootStep = ShootAlgSteps::Random;

int main()
{
    std::cout << "Hi, this is my Battleship program";
    
    while (true) {
        updateEnemyMap();
        drawEnemyBattlefield();
        drawBattlefield();

        //load intro or game
        switch (current_state) {
            case Scenes::Intro:
                std::cout << "Press any button to generate the ships.\n";
                while (true) {
                    if (_kbhit()) break;
                }
                map = generateBattlefield();
                generateShips();
                system("cls");
                drawBattlefield();
                std::cout << "good or redo('g' or 'r'): ";
                std::cin >> goodMapOrNot;
                
                if (goodMapOrNot == "g" || goodMapOrNot == "good" || goodMapOrNot == "G" || goodMapOrNot == "Good" || goodMapOrNot == "'g'") {
                    std::cout << "Are we going first or second('1' or '2'): ";
                    std::cin >> order;
                    current_state = Scenes::GameLoop;
                }

                system("cls");
                break;

            case Scenes::GameLoop:
                if (order == "1") {
                    shoot();
                    system("cls");
                    updateEnemyMap();
                    drawEnemyBattlefield();
                    drawBattlefield();
                    receive();
                }
                else {
                    receive();
                    system("cls");
                    updateEnemyMap();
                    drawEnemyBattlefield();
                    drawBattlefield();
                    shoot();
                }


                break;

            case Scenes::GameOver:
                system("cls");
                drawEnemyBattlefield();
                drawBattlefield();
                std::cout << "All the ships are sunk. you lost\n";
                return 0;
                break;
                
        }
        
        system("cls"); // clears console
    }
    
}

std::vector<std::vector<std::pair<std::string, std::string>>> generateBattlefield() {
    
    std::vector<std::vector<std::pair<std::string, std::string>>> tempMap = {};

    for (int o = 0; o < 10; o++) {
        std::vector<std::pair<std::string, std::string>> temp;
        for (int k = 0; k < 10; k++) {
            temp.push_back({ "  X  ", ""});
        }
        tempMap.push_back(temp);
    }

    return tempMap;
}

void generateShips() {
    //<size, direction, type>
    struct Ship {
        int size;
        int direction;
        std::string type;
    };
    std::vector<Ship> ships;

    
    // distrib(gen) - https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution.html
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, 1);
    


    //correct: checked
    ships.push_back({ 2, distrib(gen), "2-point ship" });
    ships.push_back({ 3, distrib(gen), "first 3-point ship" });
    ships.push_back({ 3, distrib(gen), "second 3-point ship" });
    ships.push_back({ 4, distrib(gen), "4-point ship"});
    ships.push_back({ 5, distrib(gen), "5-point ship" });


    for (auto& ship : ships) {
        Ship currentShip = ship;
        while (true) {
            // gen ship
            std::uniform_int_distribution<> distrib(0, 9 - currentShip.size);
            int startingYPos = distrib(gen);
            int endingYPos = currentShip.size + startingYPos;

            std::uniform_int_distribution<> distribSecond(0, 9); // or should I limit it to the edges
            int column = distribSecond(gen);

            // check for overlap
            bool regenerate = false;
            for (int row = startingYPos; row < endingYPos; row++) {
                if (currentShip.direction == 0) {
                    if (map[row][column].first != "  X  ") {
                        regenerate = true;
                    }
                }
                else {
                    if (map[column][row].first != "  X  ") {
                        regenerate = true;
                    }
                }
            }
            if (!regenerate) {
                // add ship to map
                if (currentShip.direction == 0) {
                    for (int row = startingYPos; row < endingYPos; row++) {
                        map[row][column] = { " ( ) " , currentShip.type};
                    }
                }
                else {
                    for (int row = startingYPos; row < endingYPos; row++) {
                        map[column][row] = { " ||| " , currentShip.type };
                    }
                }
                    
                break;
            }
        }
        
      
    }

}

void drawBattlefield() {
    std::cout << "My Ships-";
    std::vector<std::string> letters = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J" };
    std::cout << "\n";
    std::cout << "       1    2    3    4    5    6    7    8    9   10 \n";
    std::cout << "      ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n";
    for (int o = 0; o < 10; o++) {
        std::cout << " " << letters[o] << "  |";
        for (int k = 0; k < 10; k++) {
            std::cout << map[o][k].first;
        }
        std::cout << "\n\n";
    }
}

void drawEnemyBattlefield() {
    std::cout << "\nEnemy Ships-";
    std::vector<std::string> letters = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J" };
    std::cout << "\n\n";
    std::cout << "       1    2    3    4    5    6    7    8    9   10 \n";
    std::cout << "      ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n";
    for (int o = 0; o < 10; o++) {
        std::cout << " " << letters[o] << "  |";
        for (int k = 0; k < 10; k++) {
            std::cout << enemyMap[o][k].first;
        }
        std::cout << "\n\n";
    }
}

void shoot() {
    // thiis the hard part
    if (shotsLanded.empty() || shotsMissed.empty()) currentShootStep = ShootAlgSteps::Random;

    // switch vars
    // random
    
    //circle
    
    //CheckInDirection
    
    //DoubleBack
   
    bool notShot = true;
    while (notShot) {
        

        switch (currentShootStep) { // dont forget breaks
            case ShootAlgSteps::Random: {
                std::string hitOrMiss;
                std::random_device rd;  // a seed source for the random number engine
                std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
                std::uniform_int_distribution<> distrib(0, 9);
                int rRow = distrib(gen);
                int rColumn = distrib(gen);
        
                if (!isClearToFire(rRow, rColumn)) break;
                std::cout << ">>>> " << numToString(rRow) << rColumn + 1 << "\n";
                

                std::cout << "Hit or Miss('h' or 'm'): ";
                std::cin >> hitOrMiss;

                if (hitOrMiss == "h") {
                    shotsLanded.push_back({ rRow, rColumn });

                    getTarget.push_back({ { rRow, rColumn } });

                }
                else if (hitOrMiss == "m") {
                    shotsMissed.push_back({ rRow, rColumn });
                }

                std::cout << "\ndone?";
                while (true) {
                    if (_kbhit()) break;
                }

                if (hitOrMiss == "h") currentShootStep = ShootAlgSteps::Circle;
                notShot = false;
                break;
            }
            case ShootAlgSteps::Circle: {
      
                if (getTarget.empty()) { currentShootStep = ShootAlgSteps::Random; break; }
                HitShipInfo& lockedShip = getTarget[0];

                int baseRow = lockedShip.compCoord.first;   
                int baseCol = lockedShip.compCoord.second;  

                int testRow = baseRow;
                int testCol = baseCol;
                if (lockedShip.circleState == 1) { 
                    testRow = baseRow - 1; 
                    testCol = baseCol; 
                } else if (lockedShip.circleState == 2) { 
                    testRow = baseRow;     
                    testCol = baseCol + 1; 
                } else if (lockedShip.circleState == 3) { 
                    testRow = baseRow + 1; 
                    testCol = baseCol; 
                } else { 
                    testRow = baseRow;     
                    testCol = baseCol - 1; } 

                if (!isClearToFire(testRow, testCol)) {
                    if (lockedShip.circleState == 4) {
                        getTarget.erase(getTarget.begin());
                        currentShootStep = ShootAlgSteps::Random;
                    }
                    else {
                        lockedShip.circleState++;
                    }
                    break;
                }
                notShot = false;

                std::cout << ">>>> " << numToString(testRow) << (testCol + 1) << "\n";

                std::string hitOrMissOrSunk;
                std::cout << "Hit or Miss or Sunk('h' or 'm' or 's'): ";
                std::cin >> hitOrMissOrSunk;

                if (hitOrMissOrSunk == "h") {
                    shotsLanded.push_back({ testRow, testCol });
                    lockedShip.coordFromCircle = { testRow, testCol };
                    lockedShip.vertOrHoriz = (lockedShip.circleState % 2 == 0) ? "horiz" : "vert";
                }
                else if (hitOrMissOrSunk == "m") {
                    shotsMissed.push_back({ testRow, testCol });
                    if (lockedShip.circleState == 4) {
                        getTarget.erase(getTarget.begin());
                        currentShootStep = ShootAlgSteps::Random;
                        break;
                    }
                    lockedShip.circleState++;
                }
                else if (hitOrMissOrSunk == "s") {
                    shotsLanded.push_back({ testRow, testCol });
                    getTarget.erase(getTarget.begin());
                    currentShootStep = ShootAlgSteps::Random;
                    break;
                }

                std::cout << "\ndone?";
                while (true) { 
                    if (_kbhit()) break; 
                }

                if (hitOrMissOrSunk == "h") currentShootStep = ShootAlgSteps::CheckInDirection;
                break;
            }


            case ShootAlgSteps::CheckInDirection: {
        
                if (getTarget.empty()) { currentShootStep = ShootAlgSteps::Random; break; }
                HitShipInfo& lockedShip = getTarget[0];

                int row = lockedShip.coordFromCircle.first;
                int col = lockedShip.coordFromCircle.second;
                std::string dir = lockedShip.vertOrHoriz;

                if (dir == "horiz") col++; else row++;  // forward along axis

                if (!isClearToFire(row, col)) {
                    currentShootStep = ShootAlgSteps::DoubleBack;
                    break;
                }
                notShot = false;

                std::cout << ">>>> " << numToString(row) << (col + 1) << "\n";
                lockedShip.coordFromCircle = { row, col };

                std::string hitOrMissOrSunk;
                std::cout << "Hit or Miss or Sunk('h' or 'm' or 's'): ";
                std::cin >> hitOrMissOrSunk;

                if (hitOrMissOrSunk == "h") {
                    shotsLanded.push_back({ row, col });
                }
                else if (hitOrMissOrSunk == "m") {
                    shotsMissed.push_back({ row, col });
                }
                else if (hitOrMissOrSunk == "s") {
                    shotsLanded.push_back({ row, col });
                    getTarget.erase(getTarget.begin());
                    currentShootStep = ShootAlgSteps::Random;
                    break;
                }

                std::cout << "\ndone?";
                while (true) { if (_kbhit()) break; }

                if (hitOrMissOrSunk == "m") currentShootStep = ShootAlgSteps::DoubleBack;
                break;
            }


            case ShootAlgSteps::DoubleBack: {
        
                if (getTarget.empty()) { currentShootStep = ShootAlgSteps::Random; break; }
                HitShipInfo& lockedShip = getTarget[0];

                int row = lockedShip.compCoord.first;    // start from original hit
                int col = lockedShip.compCoord.second;
                std::string dirDB = lockedShip.vertOrHoriz;

                if (dirDB == "horiz") col--; else row--; // go opposite direction

                if (!isClearToFire(row, col)) {
                    currentShootStep = ShootAlgSteps::Random;
                    break;
                }
                notShot = false;

                std::cout << ">>>> " << numToString(row) << (col + 1) << "\n";
                lockedShip.compCoord = { row, col };

                std::string hitOrMissOrSunk;
                std::cout << "Hit or Miss or Sunk('h' or 'm' or 's'): ";
                std::cin >> hitOrMissOrSunk;

                if (hitOrMissOrSunk == "h") {
                    shotsLanded.push_back({ row, col });
                }
                else if (hitOrMissOrSunk == "m") {
                    shotsMissed.push_back({ row, col });
                }
                else if (hitOrMissOrSunk == "s") {
                    shotsLanded.push_back({ row, col });
                    getTarget.erase(getTarget.begin());
                    currentShootStep = ShootAlgSteps::Random;
                    break;
                }

                std::cout << "\ndone?";
                while (true) { if (_kbhit()) break; }

                if (hitOrMissOrSunk == "m") {
                    currentShootStep = ShootAlgSteps::Random;
                    getTarget.erase(getTarget.begin());
                }
                break;
            }

            }

    }
    
    


}

std::string numToString(int num) {
    // comp value
    std::vector<std::string> letters = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};
    return letters[num];
}

void receive() {
    std::string xPos;
    int yPos;
    std::cout << "Where did the enemy shot land?\n";
    std::cout << "Row: ";
    std::cin >> xPos; // yPos
    std::cout << "Column: ";
    std::cin >> yPos; // xPos
    
    if (map[stringToNum(xPos)][yPos-1].first != "  X  ") {
        std::cout << ">>>> " << "Hit T^T ";
        if (map[stringToNum(xPos)][yPos-1].first == " ||| ") {
            map[stringToNum(xPos)][yPos-1].first = " |h| ";
        }
        if (map[stringToNum(xPos)][yPos-1].first == " ( ) ") {
            map[stringToNum(xPos)][yPos-1].first = " (h) ";
        }
        if (didTheShotSinkTheShip(map[stringToNum(xPos)][yPos - 1].second)) std::cout << "The " << map[stringToNum(xPos)][yPos - 1].second << " sunk\n";
    }else std::cout << ">>>> " << "Miss!";
    std::cout << "\n";

    


    // check if ships are all gone ~ lose
    int count = 0;
    for (int o = 0; o < 10; o++) {
        for (int k = 0; k < 10; k++) {
            if ((map[o][k].first == " ||| ")||( map[o][k].first == " ( ) ")) {
                count++;
            }
        }
    }
    if (count == 0) {
        current_state = Scenes::GameOver;
    }

    std::cout << "\ndone?";
    while (true) {
        if (_kbhit()) break;
    }
}

int stringToNum(std::string letter) {
    // comp value
    std::vector<std::string> letters = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J" };

    for (int i = 0; i < letters.size(); i++) {
        if (letter == letters[i]) return i;
    }
    return -1;
}

bool didTheShotSinkTheShip(std::string shipType) {
    bool sunk = true;
    for (int o = 0; o < 10; o++) {
        for (int k = 0; k < 10; k++) {
            if ((map[o][k].second == shipType)) {
                if ((map[o][k].first == " ||| ") || (map[o][k].first == " ( ) ")) {
                    sunk = false;
                }
            }
        }
    }
    return sunk;
}

bool isClearToFire(int indexXCoord, int indexYCoord) { //comp
    // checks if on grid 
    if (!((indexXCoord >= 0 && indexXCoord <= 9) && (indexYCoord >= 0 && indexYCoord <= 9))) return false;

    //not landed shot and not missed shot
    for (auto& coords : shotsMissed) {
        if (coords == std::pair<int, int> {indexXCoord, indexYCoord}) return false;
    }
    for (auto& coords : shotsLanded) {
        if (coords == std::pair<int,int> {indexXCoord, indexYCoord}) return false;
    }

    return true;
}

void updateEnemyMap() {
    
    for (auto& coordsM : shotsMissed) {                
        enemyMap[coordsM.first][coordsM.second].first = " (M) ";
        std::cout << coordsM.first  << " " << coordsM.second;
    }
    for (auto& coordsL : shotsLanded) {
        enemyMap[coordsL.first][coordsL.second].first = " {H} ";
    }
}