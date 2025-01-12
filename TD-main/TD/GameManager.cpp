#include "GameManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <thread>
#include <chrono>
#include <conio.h>
#include <Windows.h>

GameManager::GameManager() : playerLife(10), gold(0), isPreparation(true) // 게임 매니저 초기화
{
    erasecursol(); 
}

void GameManager::run()
{
    loadMap("Map1.csv");
    parsePath();
    loadUnitTypes("UnitTypes.csv");
    loadWaves("Stage1.csv");

    for (const auto& wave : waves) 
    {
        int waveID = wave.getWaveID();

        // 웨이브 시작 전 준비 단계 시작
        startPreparationPhase();

        // 1, 3, 5 웨이브만 수비 라운드로 처리
        if (waveID == 1 || waveID == 3 || waveID == 5)
        {
            std::cout << "\n=== 웨이브 " << waveID << " 시작 ===\n";

            if (wave.getLife() > 0)
            {
                playerLife = wave.getLife();
            }
            gold = wave.getGold();

            if (wave.getIsDefence())
            {
                // 수비 웨이브 처리
                std::cout << "수비 웨이브입니다. 골드: " << gold << ", 라이프: " << playerLife << "\n";

                // 유닛 대기열 생성
                std::queue<Unit> unitQueue;
                for (int unitID : wave.getUnits()) 
                {
                    // unitID에 해당하는 UnitType 찾기
                    auto it = std::find_if(unitTypes.begin(), unitTypes.end(),
                        [unitID](const UnitType& ut) { return ut.getId() == unitID; });
                    if (it != unitTypes.end())
                    {
                        UnitType& unitType = *it;
                        Unit unit(unitType.getId(), unitType.getUnitName(), unitType.getHp(),
                            unitType.getTimePerMove(), path);
                        unitQueue.push(unit);
                    }
                }

                std::vector<Unit> activeUnits;

                const int maxTicks = 100;
                for (int tick = 0; tick < maxTicks; ++tick)
                {
                    // S가 비어있으면 다음 유닛 생성
                    bool sOccupied = false;
                    for (const auto& unit : activeUnits)
                    {
                        if (unit.getX() == path[0].first && unit.getY() == path[0].second)
                        {
                            sOccupied = true;
                            break;
                        }
                    }
                    if (!sOccupied && !unitQueue.empty())
                    {
                        Unit unit = unitQueue.front();
                        unitQueue.pop();
                        activeUnits.push_back(unit);
                    }

                    // 유닛 업데이트
                    for (auto it = activeUnits.begin(); it != activeUnits.end();) 
                    {
                        bool arrived = it->update();
                        if (arrived) 
                        {
                            // 유닛이 목적지에 도달
                            std::cout << it->getName() << " 유닛이 목적지에 도달했습니다!\n";
                            playerLife -= 1;
                            it = activeUnits.erase(it);
                        }
                        else {
                            ++it;
                        }
                    }

                    // 맵 업데이트 및 출력
                    updateAndPrintMap(activeUnits);

                    // 플레이어 라이프 출력
                    //std::cout << "Player Life: " << playerLife << "\n";

                    if (playerLife <= 0) 
                    {
                        std::cout << "Game Over!\n";
                        return;
                    }

                    if (activeUnits.empty() && unitQueue.empty()) 
                    {
                        std::cout << "웨이브 " << waveID << " 클리어!\n";
                        break;
                    }

                    // 틱 사이에 시간 지연 추가 (예: 500밀리초)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }
        }
        else {
            std::cout << "=== 웨이브 " << waveID << "는 수비 웨이브가 아닙니다. 건너뜁니다. ===\n";
        }
    }

    std::cout << "프로그램을 종료합니다.\n";
}


void GameManager::updateAndPrintMap(const std::vector<Unit>& activeUnits)
{
    // 화면을 지우고 맵을 다시 출력합니다.
    // Windows와 Linux에서 화면을 지우는 명령이 다르므로, 시스템에 맞게 선택합니다.
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    // 맵을 복사하여 유닛을 표시할 수 있도록 합니다.
    std::vector<std::vector<std::string>> mapWithUnits = map;

    // 유닛들의 위치에 유닛의 이름을 표시합니다.
    for (const auto& unit : activeUnits) 
    {
        int x = unit.getX();
        int y = unit.getY();
        if (y >= 0 && y < (int)mapWithUnits.size() && x >= 0 && x < (int)mapWithUnits[0].size())
        {
            mapWithUnits[y][x] = unit.getName();
        }
    }

    // 맵 출력
    for (const auto& row : mapWithUnits) 
    {
        for (const auto& cell : row) 
        {
            std::cout << cell << ' ';
        }
        std::cout << std::endl;
    }

    std::cout << "Player Life: " << playerLife << std::endl;
}

void GameManager::loadMap(const std::string& filename) 
{
    // 기존 코드와 동일
    std::ifstream file(filename);

    if (!file.is_open()) 
    {
        std::cout << "파일을 열 수 없습니다: " << filename << std::endl;
        return;
    }

    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) 
    {
        if (firstLine) {
            // UTF-8 BOM 제거
            const std::string utf8_bom = "\xEF\xBB\xBF";
            if (line.compare(0, utf8_bom.size(), utf8_bom) == 0) 
            {
                line.erase(0, utf8_bom.size());
            }
            firstLine = false;
        }

        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string item;

        while (std::getline(ss, item, ',')) 
        {
            if (!item.empty()) 
            {
                row.push_back(item);
            }
        }
        map.push_back(row);
    }
    file.close();
}

void GameManager::printMap()
{
    std::cout << "=== 맵 출력 ===" << std::endl;
    for (const auto& row : map) 
    {

        for (const auto& cell : row) 
        {
            std::cout << cell << ' ';
        }
        std::cout << std::endl;
    }
}

void GameManager::parsePath() {
    // BFS 알고리즘을 사용하여 S에서 D까지의 경로를 찾습니다.
    size_t rows = map.size();
    size_t cols = map[0].size();
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::queue<std::pair<size_t, size_t>> q;
    std::vector<std::vector<std::pair<int, int>>> parent(rows, std::vector<std::pair<int, int>>(cols, { -1, -1 }));

    // 시작 지점 찾기
    size_t startX = 0, startY = 0;
    bool foundStart = false;
    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            if (map[y][x] == "S") {
                startX = x;
                startY = y;
                foundStart = true;
                break;
            }
        }
        if (foundStart) break;
    }

    q.push({ startY, startX });
    visited[startY][startX] = true;

    std::vector<int> dx = { 0, 1, 0, -1 };
    std::vector<int> dy = { -1, 0, 1, 0 };

    bool foundDest = false;
    size_t destX = 0, destY = 0;

    while (!q.empty()) {
        auto [y, x] = q.front();
        q.pop();

        if (map[y][x] == "D") {
            destX = x;
            destY = y;
            foundDest = true;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            int ny = y + dy[i];
            int nx = x + dx[i];

            if (ny >= 0 && ny < (int)rows && nx >= 0 && nx < (int)cols) {
                if (!visited[ny][nx] && (map[ny][nx] == "P" || map[ny][nx] == "D")) {
                    visited[ny][nx] = true;
                    q.push({ ny, nx });
                    parent[ny][nx] = { y, x };
                }
            }
        }
    }

    if (foundDest) {
        // 경로 추적
        path.clear();
        int y = destY;
        int x = destX;
        while (!(x == (int)startX && y == (int)startY)) {
            path.push_back({ x, y });
            auto [py, px] = parent[y][x];
            y = py;
            x = px;
        }
        path.push_back({ (int)startX, (int)startY });
        std::reverse(path.begin(), path.end());
    }
    else {
        std::cout << "경로를 찾을 수 없습니다." << std::endl;
    }
}

void GameManager::loadUnitTypes(const std::string& filename) 
{
    // 기존 코드와 동일
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cout << "유닛 타입 파일을 열 수 없습니다: " << filename << std::endl;
        return;
    }

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) 
    {
        if (isHeader) 
        {
            isHeader = false; // 헤더는 건너뜁니다.
            continue;
        }

        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> tokens;

        while (std::getline(ss, item, ','))
        {
            tokens.push_back(item);
        }

        if (tokens.size() == 8) 
        {
            int id = std::stoi(tokens[0]);
            std::string unitName = tokens[1];
            int hp = std::stoi(tokens[2]);
            int timePerMove = std::stoi(tokens[3]);
            int armor = std::stoi(tokens[4]);
            int resist = std::stoi(tokens[5]);
            int productionCost = std::stoi(tokens[6]);
            int killReward = std::stoi(tokens[7]);

            UnitType unitType(id, unitName, hp, timePerMove, armor, resist, productionCost, killReward);
            unitTypes.push_back(unitType);
        }
        else
        {
            std::cout << "유닛 타입 데이터를 파싱하는 중 오류 발생: " << line << std::endl;
        }
    }
    file.close();
}

void GameManager::printUnitTypes() 
{
    std::cout << "=== 유닛 타입 목록 ===" << std::endl;
    for (const auto& unitType : unitTypes)
    {
        unitType.printInfo();
    }
}

void GameManager::loadWaves(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "웨이브 파일을 열 수 없습니다: " << filename << std::endl;
        return;
    }

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        // UTF-8 BOM 제거
        if (!line.empty() && (unsigned char)line[0] == 0xEF) {
            line.erase(0, 3);
        }

        if (isHeader) {
            isHeader = false; // 헤더는 건너뜁니다.
            continue;
        }

        // 라인이 비어있으면 건너뜁니다.
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> tokens;

        while (std::getline(ss, item, ',')) {
            tokens.push_back(item);
        }

        if (tokens.size() >= 5) {
            int waveID = std::stoi(tokens[0]);
            int life = std::stoi(tokens[1]);
            bool isDefence = (tokens[2] == "1");
            int gold = std::stoi(tokens[3]);

            // units는 tokens[4]부터 시작
            std::vector<int> units;
            for (size_t i = 4; i < tokens.size(); ++i) {
                if (tokens[i].empty()) continue;
                units.push_back(std::stoi(tokens[i]));
            }

            Wave wave(waveID, life, isDefence, gold, units);
            waves.push_back(wave);
        }
        else {
            std::cout << "웨이브 데이터를 파싱하는 중 오류 발생: " << line << std::endl;
        }
    }
    file.close();
}

void GameManager::printWaves() {
    std::cout << "=== 웨이브 목록 ===" << std::endl;
    for (const auto& wave : waves) {
        wave.printInfo();
    }
}

void GameManager::startPreparationPhase()
{
    std::vector<std::vector<std::string>> mapWithUnits = map;
    isPreparation = true;
    bool isTowerPlacementMode = false;  // 타워 설치 모드 상태 변수

    // 커서 초기 위치를 맵 중앙으로 설정
    int selectedX = mapWithUnits[0].size() / 2;
    int selectedY = mapWithUnits.size() / 2;

    // 타워 설치 모드 이전 위치를 저장할 변수
    int originalX = selectedX;
    int originalY = selectedY;

    while (isPreparation)
    {
        // 화면을 지우고 맵과 커서 위치를 다시 출력
        updateAndPrintMap({});

        if (isTowerPlacementMode)
        {
            COORD cursorPos;
            cursorPos.X = static_cast<SHORT>(selectedX * 2); // 콘솔의 가로 간격 고려 (2배 설정)
            cursorPos.Y = static_cast<SHORT>(selectedY);
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
            std::cout << " ";                                //현재 커서 위치를 빈칸으로 표시
        }
        
        

        // 준비 단계 안내 메시지 출력
        COORD messagePos = { 0, static_cast<SHORT>(mapWithUnits.size() + 2) };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), messagePos);
        std::cout << "준비 단계입니다. 'S'를 눌러 시작하거나 'B'를 눌러 타워를 설치하십시오.\n";

        char input = _getch(); // 키 입력 대기
        if (input == 's' || input == 'S')
        {
            isPreparation = false;
            break; // 준비 단계를 종료
        }
        else if (input == 'b' || input == 'B')     // 타워 설치 모드 토글
        {
           
            if (isTowerPlacementMode)
            {
                // 타워 설치 모드 비활성화 - 커서를 원래 위치로 되돌림
                selectedX = originalX;
                selectedY = originalY;
                isTowerPlacementMode = false;
                
            }
            else
            {
                // 타워 설치 모드 활성화 - 현재 위치 저장
                originalX = selectedX;
                originalY = selectedY;
                isTowerPlacementMode = true;
                
            }
        }
        else if (isTowerPlacementMode)      // 방향키 입력으로 커서 이동 (타워 설치 모드일 때만)
        {
            
            if (input == 224 || input == -32)
            {
                input = _getch();
                if (input == 72 && selectedY > 0)                  // 위쪽 방향키
                    selectedY--;
                else if (input == 80 && selectedY < mapWithUnits.size() - 1) // 아래쪽 방향키
                    selectedY++;
                else if (input == 75 && selectedX > 0)             // 왼쪽 방향키
                    selectedX--;
                else if (input == 77 && selectedX < mapWithUnits[0].size() - 1) // 오른쪽 방향키
                    selectedX++;

                
                updateAndPrintMap({});
            }
            else if (input == 13)           // 엔터로 타워 생산(미구현)
            {

            }
        }
    }
}


void GameManager::erasecursol()
{
    HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO curConsoleinfo;

    GetConsoleCursorInfo(hConsoleOut, &curConsoleinfo);
    curConsoleinfo.bVisible = 0;

    SetConsoleCursorInfo(hConsoleOut, &curConsoleinfo);
}