#ifndef PLAYER_H
#define PLAYER_H

/// @file Файл Player.h: файл, в котором написана реализация класса Player и перегрузка операторов сравнения.
#include <string>
#include <iostream>

using namespace std;

/// Enum'ка, в которой перечислены позиции футболистов.
enum Positions{goalkeeper, defender, halfdefender, attacker};

/**
 * Класс для представления игрока.
 * Включает страну, ФИО, клуб, позицию, количество матчей и голов за сборную.
 */
class Player {
private:
    string country;
    string fullName;
    string clubName;
    Positions position;
    int counterGames;
    int counterGoals;

public:
    /**
     * Конструктор для инициализации объекта Player.
     * @param country Страна игрока.
     * @param fullName Полное имя игрока.
     * @param clubName Название клуба игрока.
     * @param position Позиция игрока на поле.
     * @param counterGames Количество сыгранных матчей за сборную.
     * @param counterGoals Количество голов за сборную (или пропущенных для вратаря).
     */
    Player(string country, string fullName, string clubName, Positions position, int counterGames, int counterGoals){
        this->country = country;
        this->fullName = fullName;
        this->clubName = clubName;
        this->position = position;
        this->counterGames = counterGames;
        this->counterGoals = counterGoals;
    }

    /**
     * Пустой конструктор для создания объекта Player без инициализации.
     * Используется в случае, если требуется создать пустой объект.
     */
    Player() {}

    /// Геттеры.
    string GetCountry() const { return country; }

    string GetFullName() const { return fullName; }

    string GetClubName() const { return clubName; }

    Positions GetPosition() const { return position; }

    int GetCounterGames() const { return counterGames; }

    int GetCounterGoals() const { return counterGoals; }
};

/**
 * Перегрузка оператора <.
 * Сначала сравнивается количество матчей. Если они равны, то сравниваются имена игроков.
 * Если и имена равны, то сравниваются количество голов (вратарям — количество пропущенных голов).
 * @param a Первый игрок.
 * @param b Второй игрок.
 * @return true, если игрок a "меньше" игрока b по сравнению по данным правилам.
 */
bool operator < (const Player& a, const Player& b) {
    if (a.GetCounterGames() == b.GetCounterGames()) {
        if (a.GetFullName() == b.GetFullName()) {
            return a.GetCounterGoals() > b.GetCounterGoals();
        }
        else {
            return a.GetFullName() < b.GetFullName();
        }
    }
    else {
        return a.GetCounterGames() < b.GetCounterGames();
    }
}

/**
 * Перегрузка оператора >
 * Использует логическое отрицание от оператора <.
 * @param a Первый игрок.
 * @param b Второй игрок.
 * @return true, если игрок a "больше" игрока b.
 */
bool operator > (const Player& a, const Player& b) {
    return b < a;
}

/**
 * Перегрузка оператора <=
 * Использует логическое отрицание от оператора >.
 * @param a Первый игрок.
 * @param b Второй игрок.
 * @return true, если игрок a "меньше или равен" игроку b.
 */
bool operator <= (const Player& a, const Player& b) {
    return !(a > b);
}

/**
 * Перегрузка оператора >=
 * Использует логическое отрицание от оператора <.
 * @param a Первый игрок.
 * @param b Второй игрок.
 * @return true, если игрок a "больше или равен" игроку b.
 */
bool operator >= (const Player& a, const Player& b) {
    return !(a < b);
}

/**
 * Перегрузка оператора ==
 * Игроки считаются равными, если они не меньше и не больше друг друга.
 * @param a Первый игрок.
 * @param b Второй игрок.
 * @return true, если игроки равны по всем критериям.
 */
bool operator == (const Player& a, const Player& b) {
    return !(a < b) && !(a > b);
}

#endif