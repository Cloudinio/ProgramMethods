#ifndef DATA_H
#define DATA_H

/// @file Файл data.h: файл, в котором реализована функция парсинга csv файла, сгенерированного generator.py.
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "player.h"

/// @brief функция, которая преобразует строковое представление position в перечисленное из Positions
/// @param s 
/// @return определенная position.
Positions ParsePosition(const string& s) {
    if (s == "goalkeeper") return goalkeeper;
    if (s == "defender") return defender;
    if (s == "halfdefender") return halfdefender;
    return attacker;
}

/// @brief Функция чтения данных из csv файла. 
/// Эта функция читает файл, парсит строки и преобразует их в объекты Player, которые добавляются в вектор.
/// @param filename 
/// @return Вектор players. 
vector<Player> ReadPlayersFromCSV(const string& filename) {
    /// Создание объекта для работы с файлами.
    ifstream file(filename);
    vector<Player> players;

    /// Проверка, открылся ли файл.
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << filename << "\n";
        return players;
    }

    string line;

    /// Чтение заголовка.
    getline(file, line);

    /// Чтение оставшихся строк, каждая строка представляет одного игрока.
    while (getline(file, line)) {
        /// Создание объекта stringstream для дальнейшего разделения строки.
        stringstream ss(line);
        string country, fullName, clubName, posStr, gamesStr, goalsStr;

        getline(ss, country, ',');
        getline(ss, fullName, ',');
        getline(ss, clubName, ',');
        getline(ss, posStr, ',');
        getline(ss, gamesStr, ',');
        getline(ss, goalsStr, ',');

        /// Преобразуем строковое представление позиции в перечисление.
        Positions pos = ParsePosition(posStr);
        
        /// Преобразование строк в числа для количества матчей и голов.
        int games = stoi(gamesStr);
        int goals = stoi(goalsStr);

        /// Добавление нового объекта Player в вектор.
        players.emplace_back(country, fullName, clubName, pos, games, goals);
    }

    ///Возвращаем вектор с объектами Player
    return players;
}

#endif DATA_H