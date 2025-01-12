#ifndef UNIT_H
#define UNIT_H

#include <string>
#include <vector>

class Unit {
private:
    int id;                   // 유닛의 id (UnitType의 id)
    std::string name;         // 유닛 이름
    int hp;                   // 현재 체력
    int timePerMove;          // 몇 틱에 한 칸 움직이는지
    int moveCounter;          // 이동을 위한 틱 카운터
    int x, y;                 // 현재 위치
    size_t pathIndex;         // 경로에서의 현재 위치 인덱스
    std::vector<std::pair<int, int>> path; // 이동 경로

public:
    Unit(int id, const std::string& name, int hp, int timePerMove, const std::vector<std::pair<int, int>>& path);

    // 행동 함수
    bool update(); // 이동 및 상태 업데이트, 목적지에 도달하면 true 반환

    // 위치 정보
    int getX() const;
    int getY() const;

    // 이름 가져오기
    std::string getName() const;

    // 출력
    void printStatus() const;
};

#endif // UNIT_H
