#include <climits>
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

    void save_status(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Nie można otworzyć pliku statusu do zapisu." << std::endl;
            return;
        }

        file << gold << std::endl;
        for (int unitIndex : jednostki) {
            file << unitIndex << std::endl;
        }

        file.close();

        std::cout << "Status został zapisany pomyślnie." << std::endl;
    }
};

void tworzJednostke(Map& map, Status& status, int baseIdGracza) {
    std::vector<Unit> units = getUnits();

    // Znajdź bazę gracza na mapie
    std::pair<int, int> basePosition;
    for (const auto& base : map.bazy) {
        if (baseIdGracza == map.grid[base.second][base.first]) {
            basePosition = base;
            break;
        }
    }
    // Wybierz losową jednostkę z dostępnych
    int randomIndex = std::rand() % units.size();
    Unit unit = units[randomIndex];

    // Wygeneruj unikalny identyfikator jednostki
    unit.id = generateUniqueId();

    // Ustal pozycję jednostki jako pozycję bazy
    unit.x = basePosition.first;
    unit.y = basePosition.second;

    // Dodaj jednostkę do statusu
    status.jednostki.push_back(unit.id);

    std::cout << "Utworzono jednostkę o identyfikatorze: " << unit.id << std::endl;
}

int main() {
    std::srand(std::time(0)); // Inicjalizacja generatora liczb losowych

    Map map("mapa.txt");
    Status status("status.txt");

    int baseIdGracza = 1; // Identyfikator bazy gracza
    tworzJednostke(map, status, baseIdGracza);

    status.save_status("status.txt");

    return 0;
}