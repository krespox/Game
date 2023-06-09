#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>

struct Unit {
    int id;                // Unikalny identyfikator jednostki
    std::string type;
    int endurance;
    int speed;
    int cost;
    int attackRange;
    int buildTime;
    int baseId; // Identyfikator bazy, do której należy jednostka
    int x;
    int y;
};

// Funkcja pomocnicza do generowania unikalnych identyfikatorów dla jednostek
int generateUniqueId() {
    static int uniqueId = 0;
    return ++uniqueId;
}

std::vector<Unit> getUnits() {
    std::vector<Unit> units;

    // Przykładowe jednostki
    units.push_back({ generateUniqueId(), "Knight", 70, 5, 400, 1, 5 });
    units.push_back({ generateUniqueId(), "Swordsman", 60, 2, 250, 1, 3 });
    units.push_back({ generateUniqueId(), "Archer", 40, 2, 250, 5, 3 });
    units.push_back({ generateUniqueId(), "Pikeman", 50, 2, 200, 2, 3, });
    units.push_back({ generateUniqueId(), "Ram", 90, 2, 500, 1, 4, });
    units.push_back({ generateUniqueId(), "Catapult", 50, 2, 800, 7, 6 });
    units.push_back({ generateUniqueId(), "Worker", 20, 2, 100, 1, 2 });

    return units;
}

std::vector<std::vector<int>> getAttackTable() {
    std::vector<std::vector<int>> attackTable = {
        {35, 35, 35, 35, 35, 50, 35, 35},
        {30, 30, 30, 20, 20, 30, 30, 30},
        {15, 15, 15, 15, 10, 10, 15, 15},
        {35, 15, 15, 15, 15, 10, 15, 10},
        {40, 40, 40, 40, 40, 40, 40, 50},
        {10, 10, 10, 10, 10, 10, 10, 50},
        {5, 5, 5, 5, 5, 5, 5, 1}
    };

    return attackTable;
}

class Map {
public:
    int width;
    int height;
    std::vector<std::vector<int>> grid;
    std::vector<std::pair<int, int>> kopalnie;
    std::vector<std::pair<int, int>> przeszkody;
    std::vector<std::pair<int, int>> bazy;

    Map(const std::string& filename) {
        load_map(filename);
        find_elements();
    }

    void load_map(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Nie można otworzyć pliku mapy." << std::endl;
            return;
        }

        std::string line;
        if (std::getline(file, line)) {
            width = line.length();
            height = 1;
            grid.push_back(parse_line(line));

            while (std::getline(file, line)) {
                grid.push_back(parse_line(line));
                height++;
            }
        }

        file.close();

        std::cout << "Mapa została wczytana pomyślnie." << std::endl;
    }

    std::vector<int> parse_line(const std::string& line) {
        std::vector<int> row;
        for (char c : line) {
            int value = c - '0';
            row.push_back(value);
        }
        return row;
    }

    void find_elements() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int element = grid[i][j];
                if (element == 6) {
                    kopalnie.push_back(std::make_pair(j, i));
                }
                else if (element == 9) {
                    przeszkody.push_back(std::make_pair(j, i));
                }
                else if (element == 1 || element == 2) {
                    int baseId = generateUniqueId();
                    bazy.push_back(std::make_pair(j, i));
                    grid[i][j] = baseId;
                }
            }
        }
    }
};

class Status {
public:
    long gold;
    std::vector<int> jednostki;

    Status(const std::string& filename) {
        load_status(filename);
    }

    void load_status(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Nie można otworzyć pliku statusu." << std::endl;
            return;
        }

        std::string line;
        if (std::getline(file, line)) {
            gold = std::stol(line);
        }

        while (std::getline(file, line)) {
            int unitIndex = std::stoi(line);
            jednostki.push_back(unitIndex);
        }

        file.close();

        std::cout << "Status został wczytany pomyślnie." << std::endl;
    }
};

std::vector<std::string> plan_actions(const Map& map, const Status& status) {
    std::vector<std::string> actions;

    std::vector<Unit> units = getUnits();
    std::vector<std::vector<int>> attackTable = getAttackTable();

    // Przykładowa logika planowania akcji
    for (int jednostkaIndex : status.jednostki) {
        // Sprawdź, czy jednostka o danym indeksie istnieje
        if (jednostkaIndex < 1 || jednostkaIndex > units.size()) {
            std::cout << "Nieprawidłowy indeks jednostki: " << jednostkaIndex << std::endl;
            continue;
        }

        const auto& jednostka = units[jednostkaIndex - 1];

        // Znajdź najbliższą bazę
        int minBaseDistance = INT_MAX;
        std::pair<int, int> targetBase;

        for (const auto& baza : map.bazy) {
            int distance = abs(baza.first - map.przeszkody[jednostkaIndex - 1].first) + abs(baza.second - map.przeszkody[jednostkaIndex - 1].second);
            if (distance < minBaseDistance) {
                minBaseDistance = distance;
                targetBase = baza;
            }
        }

        // Sprawdź najbliższego przeciwnika
        int minEnemyDistance = INT_MAX;
        std::pair<int, int> targetEnemy;

        for (const auto& przeciwnik : map.przeszkody) {
            int distance = abs(przeciwnik.first - map.przeszkody[jednostkaIndex - 1].first) + abs(przeciwnik.second - map.przeszkody[jednostkaIndex - 1].second);
            if (distance < minEnemyDistance && map.grid[przeciwnik.second][przeciwnik.first] != map.grid[map.przeszkody[jednostkaIndex - 1].second][map.przeszkody[jednostkaIndex - 1].first]) {
                minEnemyDistance = distance;
                targetEnemy = przeciwnik;
            }
        }

        // Wykonaj ruch w kierunku przeciwnika lub bazy
        std::string moveAction;
        if (minEnemyDistance < minBaseDistance) {
            // Wykonaj ruch w kierunku przeciwnika
            moveAction = "MOVE " + std::to_string(jednostka.id) + " TO " + std::to_string(targetEnemy.first) + "," + std::to_string(targetEnemy.second);
        }
        else {
            // Wykonaj ruch w kierunku bazy
            moveAction = "MOVE " + std::to_string(jednostka.id) + " TO " + std::to_string(targetBase.first) + "," + std::to_string(targetBase.second);
        }
        actions.push_back(moveAction);

        // Sprawdź czy atak jest możliwy
        int attackRange = jednostka.attackRange;
        if (minEnemyDistance <= attackRange) {
            // Wykonaj atak na przeciwnika
            std::string attackAction = "ATTACK " + std::to_string(jednostka.id) + " AT " + std::to_string(targetEnemy.first) + "," + std::to_string(targetEnemy.second);
            actions.push_back(attackAction);
        }
    }

    return actions;
}

void save_actions(const std::vector<std::string>& actions) {
    std::ofstream file("rozkazy.txt");
    if (!file.is_open()) {
        std::cout << "Nie można otworzyć pliku do zapisu akcji." << std::endl;
        return;
    }

    for (const auto& action : actions) {
        file << action << std::endl;
    }

    file.close();

    std::cout << "Akcje zostały zapisane w pliku 'rozkazy.txt'." << std::endl;
}
void save_status(const Status& status) {
    std::ofstream file("status.txt");
    if (!file.is_open()) {
        std::cout << "Nie można otworzyć pliku do zapisu statusu." << std::endl;
        return;
    }

    // Zapisz ilość złota
    file << status.gold << std::endl;

    // Zapisz jednostki na planszy
    for (const auto& jednostkaIndex : status.jednostki) {
        // Sprawdź, czy indeks jednostki jest poprawny
        if (jednostkaIndex < 1 || jednostkaIndex > getUnits().size()) {
            std::cout << "Nieprawidłowy indeks jednostki: " << jednostkaIndex << std::endl;
            continue;
        }
        const auto& jednostka = getUnits()[jednostkaIndex - 1];
        // Przykład: 1 Knight 70 5 400 1 5
        file << jednostka.id << " " << jednostka.type << " " << jednostka.endurance << " " << jednostka.speed << " " << jednostka.cost << " " << jednostka.attackRange << " " << jednostka.buildTime << " " << jednostka.baseId << std::endl;
    }
    file.close();
    std::cout << "Status został zapisany w pliku 'status.txt'." << std::endl;
}

Unit sprawdzNajlepszaJednostke(const std::vector<Unit>& units, const Map& map) {
    Unit najlepszaJednostka;
    double najwyzszaWartosc = 0.0;
    std::vector<std::vector<int>> attackTable = getAttackTable();
    for (const Unit& unit : units) {
        double wartosc = (unit.attackRange / unit.endurance) * (unit.speed / (double)unit.buildTime) * (1.0 / unit.cost);

        // Sprawdź, czy jednostka jest zdolna do ataku na przeciwnika
        if (unit.attackRange > 0) {
            for (const auto& przeciwnik : map.przeszkody) {
                // Sprawdź, czy przeciwnik jest na zasięgu ataku
                int distance = abs(przeciwnik.first - map.przeszkody[unit.id - 1].first) + abs(przeciwnik.second - map.przeszkody[unit.id - 1].second);
                if (distance <= unit.attackRange && map.grid[przeciwnik.second][przeciwnik.first] != map.grid[map.przeszkody[unit.id - 1].second][map.przeszkody[unit.id - 1].first]) {
                    // Jednostka może zaatakować przeciwnika, zwiększ wartość ataku na przeciwnika
                    wartosc *= (attackTable[unit.id - 1][map.grid[przeciwnik.second][przeciwnik.first] - 1] / 100.0);
                }
            }
        }

        if (wartosc > najwyzszaWartosc) {
            najwyzszaWartosc = wartosc;
            najlepszaJednostka = unit;
        }
    }

    return najlepszaJednostka;
}
void attackUnit(Unit& attacker, Unit& target, const std::vector<std::vector<int>>& attackTable) {
    int attackerIndex = -1;
    int targetIndex = -1;

    // Pobranie indeksu jednostki atakującej
    for (size_t i = 0; i < getUnits().size(); ++i) {
        if (getUnits()[i].id == attacker.id) {
            attackerIndex = i;
            break;
        }
    }

    // Pobranie indeksu jednostki atakowanej
    for (size_t i = 0; i < getUnits().size(); ++i) {
        if (getUnits()[i].id == target.id) {
            targetIndex = i;
            break;
        }
    }

    // Sprawdzenie poprawności indeksów jednostek
    if (attackerIndex == -1 || targetIndex == -1) {
        // Nieprawidłowe indeksy jednostek
        return;
    }

    // Pobranie typu jednostki atakującej i atakowanej
    std::string attackerType = getUnits()[attackerIndex].type;
    std::string targetType = getUnits()[targetIndex].type;

    // Konwersja typów jednostek na indeksy w tabeli ataków
    int attackerTypeIndex = -1;
    int targetTypeIndex = -1;

    if (attackerType == "Knight")
        attackerTypeIndex = 0;
    else if (attackerType == "Swordsman")
        attackerTypeIndex = 1;
    else if (attackerType == "Archer")
        attackerTypeIndex = 2;
    else if (attackerType == "Pikeman")
        attackerTypeIndex = 3;
    else if (attackerType == "Ram")
        attackerTypeIndex = 4;
    else if (attackerType == "Catapult")
        attackerTypeIndex = 5;
    else if (attackerType == "Worker")
        attackerTypeIndex = 6;

    if (targetType == "Knight")
        targetTypeIndex = 0;
    else if (targetType == "Swordsman")
        targetTypeIndex = 1;
    else if (targetType == "Archer")
        targetTypeIndex = 2;
    else if (targetType == "Pikeman")
        targetTypeIndex = 3;
    else if (targetType == "Ram")
        targetTypeIndex = 4;
    else if (targetType == "Catapult")
        targetTypeIndex = 5;
    else if (targetType == "Worker")
        targetTypeIndex = 6;

    // Sprawdzenie poprawności indeksów typów jednostek w tabeli ataków
    if (attackerTypeIndex == -1 || targetTypeIndex == -1 ||
        attackerTypeIndex >= attackTable.size() || targetTypeIndex >= attackTable[attackerTypeIndex].size()) {
        // Nieprawidłowe indeksy typów jednostek w tabeli ataków
        return;
    }

    // Pobranie wartości obrażeń z tabeli ataków
    int damage = attackTable[attackerTypeIndex][targetTypeIndex];

    // Odejmowanie obrażeń od wytrzymałości jednostki atakowanej
    getUnits()[targetIndex].endurance -= damage;
    if (getUnits()[targetIndex].endurance <= 0) {
        // Jednostka została zniszczona
        getUnits().erase(getUnits().begin() + targetIndex);
        // Dodaj tutaj inne odpowiednie działania po zniszczeniu jednostki
    }
}
std::string getUnitSymbol(const std::string& unitType) {
    if (unitType == "Knight") {
        return "K";
    }
    if (unitType == "Swordsman") {
        return "S";
    }
    if (unitType == "Archer") {
        return "A";
    }
    if (unitType == "Pikeman") {
        return "P";
    }
    if (unitType == "Ram") {
        return "R";
    }
    if (unitType == "Catapult") {
        return "C";
    }
    if (unitType == "Worker") {
        return "W";
    }
}
std::string getBaseSymbol(int baseId) {
    if (baseId == 1) {
        return "P";
    }
    else if (baseId == 2) {
        return "E";
    }
    else {
        return "?";
    }
}
// Funkcja tworząca jednostkę
void tworzJednostke(const Map& map, Status& status, int baseId) {
    std::vector<Unit> units = getUnits();
    Unit najlepszaJednostka = sprawdzNajlepszaJednostke(units, map);

    // Przypisz unikalny identyfikator jednostce
    najlepszaJednostka.id = generateUniqueId();

    // Przypisz identyfikator bazy
    najlepszaJednostka.baseId = baseId;

    // Dodaj najlepszą jednostkę do statusu
    status.jednostki.emplace_back(najlepszaJednostka);

    // Zmniejsz ilość złota o koszt tworzenia jednostki
    status.gold -= najlepszaJednostka.cost;

    // Dodaj dane o tworzeniu jednostki do pliku status.txt
    std::ofstream file("status.txt", std::ios_base::app);
    if (file.is_open()) {
        std::string baseSymbol = getBaseSymbol(baseId);
        std::string unitSymbol = getUnitSymbol(najlepszaJednostka.type);

        // Przykład: P B 10 0 0 98 A
        file << baseSymbol << " " << unitSymbol << " "
            << najlepszaJednostka.x << " " << najlepszaJednostka.y << " "
            << najlepszaJednostka.endurance << " " << najlepszaJednostka.id << std::endl;
        file.close();

        std::cout << "Jednostka została utworzona i zapisana w pliku 'status.txt'." << std::endl;
    }
    else {
        std::cout << "Nie można otworzyć pliku do zapisu statusu." << std::endl;
    }
}
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Podaj nazwę pliku mapy oraz pliku statusu jako argumenty wywołania programu." << std::endl;
        return 1;
    }

    std::string map_filename = argv[1];
    Map map(map_filename);

    if (map.grid.empty()) {
        return 1;
    }

    std::string status_filename = argv[2];
    Status status(status_filename);

    if (status.jednostki.empty()) {
        return 1;
    }

    std::vector<std::string> actions = plan_actions(map, status);
    save_actions(actions);

    return 0;
}
