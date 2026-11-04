#include <cstdlib>
#include <iostream>
#include <clocale> 
#include <fstream>
#include <time.h>

class Place
{
private:
    int row;
    int col;
    char** data;
    char monster[6] = { 'A','B','C','a','b','c' };

    int getId(char c) const
    {
        for (int i = 0; i < 6; ++i) {
            if (monster[i] == c) return i;
        }
        return -1;
    }
public:
    Place(const char* FN)
    {
        std::ifstream F(FN);
        F >> row;
        F >> col;

        row += 2; col += 2;

        data = new char* [row];
        for (int i = 0; i < row; ++i) {
            data[i] = new char[col];
        }

        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j)
            {
                if (j > col - 2 || i == 0 || i == row - 1 || j < 1) {
                    data[i][j] = '*';
                }
                else { data[i][j] = '.'; }
            }
        }
        F.close();
    }

    ~Place() {
        for (int i = 0; i < row; ++i) delete[] data[i];
        delete[] data;
    }

    void getMonsterPos(int index, int& x, int& y)
    {
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                if (data[i][j] == monster[index]) {
                    x = i; y = j;
                    return;
                }
            }
        }
    }

    void spawn(int& x, int& y, int index)
    {
        x = 1 + rand() % (row - 2);

        if (index <= 2)
        {
            while (data[x][1] != '.') { x = 1 + rand() % (row - 2); }
            data[x][1] = monster[index];
            y = 1;
        }
        else
        {
            while (data[x][col - 2] != '.') { x = 1 + rand() % (row - 2); }
            data[x][col - 2] = monster[index];
            y = col - 2;
        }
    }

    void print() const
    {
        for (int i = 0; i < row; ++i) {
            for (int j = 0; j < col; ++j) {
                std::cout << data[i][j] << ' ';
            }
            std::cout << "\n";
        }
    }

    bool isWall(int r, int c) const {
        if (r < 1 || r >= row - 1 || c < 1 || c >= col - 1) return true;
        return false;
    }

    int whoHere(int s, int& r, int& c, int& dir)
    {
        char symbol = data[r][c];
        int id = -1;
        data[r][c] = '.';

        if (isWall(r, c + s * dir)) {
            dir *= -1;

            int nextR = r + (rand() % 2 == 0 ? 1 : -1);
            if (nextR < 1) nextR = 1;
            if (nextR >= row - 1) nextR = row - 2;

            r = nextR;

            if (data[r][c] != '.') {
                r = (nextR == r + 1) ? r - 1 : r + 1;
            }

            for (int i = 1; i <= s; ++i) {
                if (isWall(r, c + dir)) break;
                if (data[r][c + dir] != '.') {
                    id = getId(data[r][c + dir]);
                    break;
                }
                c += dir;
            }
            data[r][c] = symbol;
            return id;
        }

        for (int i = 1; i <= s; ++i) {
            if (data[r][c + i * dir] != '.') {
                id = getId(data[r][c + i * dir]);
                c += (i - 1) * dir;
                data[r][c] = symbol;
                return id;
            }
        }

        c += s * dir;
        data[r][c] = symbol;
        return id;
    }

    void removeMonster(int index, int r, int c) {
        if (data[r][c] == monster[index]) {
            data[r][c] = '.';
        }
    }
};

class BaseMonster {
public:
    Place* p;
    int x, y;
    int index;
    int health;
    int damage;
    int dir;
    bool isAlive;

    BaseMonster(int ind, int hp, int dmg, int d)
        : index(ind), p(nullptr), x(0), y(0), health(hp), damage(dmg), dir(d), isAlive(true) {
    }

    virtual ~BaseMonster() {}

    void set(Place& p1) {
        p = &p1;
        p->spawn(x, y, index);
        p->getMonsterPos(index, x, y);
    }

    void takeDamage(int dmg) {
        if (!isAlive) return;
        health -= dmg;
        if (health <= 0) {
            health = 0;
            isAlive = false;
            p->removeMonster(index, x, y);
            std::cout << ">>> Monster " << index << " DIED! <<<\n";
        }
    }

    virtual void walk(BaseMonster** allMonsters) = 0;
    virtual void attack(BaseMonster* target) = 0;
    virtual void info() = 0;
};

class Dragon : public BaseMonster
{
private:
    int speed = 1;
public:
    Dragon(int ind) : BaseMonster(ind, 100, 20, (ind <= 2 ? 1 : -1)) {}

    void attack(BaseMonster* target) override {
        std::cout << "Dragon attack" << std::endl;
        target->takeDamage(damage);
    }

    void walk(BaseMonster** allMonsters) override {
        if (!isAlive) return;
        std::cout << "Dragon walk" << std::endl;
        int targetId = p->whoHere(speed, x, y, dir);
        if (targetId != -1) {
            attack(allMonsters[targetId]);
        }
    }

    void info() override {
        std::cout << "Dragon health: " << health << std::endl;
        std::cout << "Dragon x: " << x << std::endl;
        std::cout << "Dragon y: " << y << std::endl;
    }
};

class Zombie : public BaseMonster
{
private:
    int speed = 2;
public:
    Zombie(int ind) : BaseMonster(ind, 80, 10, (ind <= 2 ? 1 : -1)) {}

    void attack(BaseMonster* target) override {
        std::cout << "Zombie attack" << std::endl;
        target->takeDamage(damage);
    }

    void walk(BaseMonster** allMonsters) override {
        if (!isAlive) return;
        std::cout << "Zombie walk" << std::endl;
        int targetId = p->whoHere(speed, x, y, dir);
        if (targetId != -1) {
            attack(allMonsters[targetId]);
        }
    }

    void info() override {
        std::cout << "Zombie health: " << health << std::endl;
        std::cout << "Zombie x: " << x << std::endl;
        std::cout << "Zombie y: " << y << std::endl;
    }
};

class Goblin : public BaseMonster
{
private:
    int speed = 3;
public:
    Goblin(int ind) : BaseMonster(ind, 50, 10, (ind <= 2 ? 1 : -1)) {}

    void attack(BaseMonster* target) override {
        std::cout << "Goblin attack" << std::endl;
        target->takeDamage(damage);
    }

    void walk(BaseMonster** allMonsters) override {
        if (!isAlive) return;
        std::cout << "Goblin walk" << std::endl;
        int targetId = p->whoHere(speed, x, y, dir);
        if (targetId != -1) {
            attack(allMonsters[targetId]);
        }
    }

    void info() override {
        std::cout << "Goblin health: " << health << std::endl;
        std::cout << "Goblin x: " << x << std::endl;
        std::cout << "Goblin y: " << y << std::endl;
    }
};

int main() {
    srand(time(0));
    setlocale(LC_ALL, "Russian");

    Place p("Data.txt");

    BaseMonster* m[6];
    m[0] = new Dragon(0); m[3] = new Dragon(3);
    m[1] = new Zombie(1); m[4] = new Zombie(4);
    m[2] = new Goblin(2); m[5] = new Goblin(5);

    for (int i = 0; i < 6; ++i) {
        m[i]->set(p);
    }

    int aliveCount = 6;


    while (aliveCount > 1) {

        char next;
        for (int i = 0; i < 6; ++i)
        {
            if (m[i]->isAlive) {
                m[i]->walk(m);
                m[i]->info();
            }
        }
        p.print();

        aliveCount = 0;
        for (int i = 0; i < 6; ++i) {
            if (m[i]->isAlive) aliveCount++;
        }

        std::cout << "Alive: " << aliveCount << "\n";

        std::cin >> next;
    }


    return 0;
}