#define CPPHTTPLIB_DISABLE_FILE_REQUEST_HANDLER
#define CPPHTTPLIB_DISABLE_FILE_SINK
#define _WIN32_WINNT 0x0A00

#include "Extension/httplib.h"
#include "Extension/Information.h"
#include "Extension/GSICreator.h"
#include "Extension/json.hpp"


#include <functional>
#include <queue>
#include <fstream>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>

using json = nlohmann::json;

using namespace std;

/*--------------------GSI SERVER CLASS---------------------*/
/*--------------------GSI SERVER CLASS---------------------*/
/*--------------------GSI SERVER CLASS---------------------*/

class GSTServer { 
protected:

/*----------------------------MUTEX-CONTROL--------------------------------*/
    std::mutex event_mutex;
    std::mutex players_mutex;
    std::mutex listeners_mutex;
    std::mutex state_mutex; 

/*----------------------------DATA-STORAGE---------------------------------*/
    std::unordered_map<std::string, Player> players;
    std::unordered_map<EventType, std::vector<std::function<void(const Player&, const std::string&)>>> bombListeners;
    std::unordered_map<EventType, std::vector<std::function<void(const Player&)>>> listeners;// мапа для зберігання колбеків для кожного типу івенту
private:

    httplib::Server svr; // створюємо сервер
    GameState state;
    GameState lastState;

    json DATA; // головні данні

    std::thread server_thread; // потік для роботи сервера, щоб не блокувати основний потік програми



    void updatePlayerInDB(const Player& newPlayer) {// метод для оновлення інформації 
        std::lock_guard<std::mutex> lock(players_mutex);
        players[newPlayer.steamid] = newPlayer;
}

    void parseAllData(const json& DATA, GameState& state) { //ГОЛОВНА ХУЙНЯ

    // ---------- PLAYER ----------
    if (DATA.contains("player")) {
        auto& pl = DATA["player"];

        state.player.name = pl.value("name", "");
        state.player.team = pl.value("team", "");
        state.player.steamid = pl.value("steamid", "");
        state.player.activity = pl.value("activity", "");

        if (pl.contains("state")) {
            auto& s = pl["state"];
            state.player.state.health = s.value("health", 0);
            state.player.state.armor = s.value("armor", 0);
            state.player.state.money = s.value("money", 0);
        }

        if (pl.contains("match_stats")) {
            auto& ms = pl["match_stats"];
            state.player.match_stats.kills = ms.value("kills", 0);
            state.player.match_stats.deaths = ms.value("deaths", 0);
            state.player.match_stats.assists = ms.value("assists", 0);
            state.player.match_stats.score = ms.value("score", 0);
        }

        if (pl.contains("weapons")) {
            state.player.weapons.clear(); // важливо

            for (auto& [key, w] : pl["weapons"].items()) {
                Weapon weapon;
                weapon.name = w.value("name", "");
                weapon.type = w.value("type", "");
                weapon.ammo_clip = w.value("ammo_clip", 0);

                state.player.weapons[key] = weapon;
            }
        }
        if (pl.contains("position")) {
            std::string posStr = pl["position"];

            std::istringstream iss(posStr);

            iss >> state.player.position.x
                >> state.player.position.y
                >> state.player.position.z;
}

       
    }
    // ---------- ROUND ----------
    if (DATA.contains("round")) {
        auto& r = DATA["round"];

        state.round.phase = r.value("phase", "");
        state.round.win_team = r.value("win_team", "");
        state.round.bomb.state = r.value("bomb", "");

    }

    // ---------- MAP ----------
    if (DATA.contains("map")) {
        auto& m = DATA["map"];

        state.map.name = m.value("name", "");
        state.map.phase = m.value("phase", "");
        state.map.round = m.value("round", 0);
        state.map.current_spectators = m.value("current_spectators", 0);

    }
    
}

    void handleGameState(const json& DATA) {// метод для обробки геймстейту, парсинг даних і визначення, які івенти сталися

    GameState newState = state; // важливо

    parseAllData(DATA, newState);

    bool killEvent = false;
    bool deathEvent = false;
    bool assistEvent = false;
    bool startRoundEvent = false;
    bool endRoundEvent = false;
    bool onPlantEvent = false;
    bool onDefuseEvent = false;
    
/*----------------------------EVENT-HANDLING--------------------------------*/
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        bool bombStateChanged = (state.round.bomb.state != newState.round.bomb.state);

        killEvent = (newState.player.match_stats.kills > state.player.match_stats.kills);
        deathEvent = (newState.player.match_stats.deaths > state.player.match_stats.deaths);
        assistEvent = (newState.player.match_stats.assists > state.player.match_stats.assists);

        startRoundEvent = (state.round.phase != "live" && newState.round.phase == "live");
        endRoundEvent   = (state.round.phase != "over" && newState.round.phase == "over");

        onDefuseEvent = bombStateChanged && newState.round.bomb.state == "defused";

        onPlantEvent = bombStateChanged && newState.round.bomb.state == "planted";

        lastState = state;
        state = newState;
    }

    updatePlayerInDB(state.player);
/*----------------------------EMIT-EVENTS--------------------------------*/
    if (killEvent) emit(EventType::Kill, state.player);
    if (deathEvent) emit(EventType::Death, state.player);
    if (assistEvent) emit(EventType::Assist, state.player);
    if (startRoundEvent) emit(EventType::StartRound, state.player);
    if (endRoundEvent) emit(EventType::EndRound, state.player);
    if (onPlantEvent) emitBomb(EventType::Plantbomb, state.player, state.round.bomb.site);
    if (onDefuseEvent) emitBomb(EventType::Defusebomb, state.player, state.round.bomb.site);
}


public:

void emitBomb(EventType type, const Player& player, BombSite site) { //виклик бомб колбеків
    std::lock_guard<std::mutex> lock(listeners_mutex);

    auto it = bombListeners.find(type); 
    if (it != bombListeners.end()) {
        for (auto& callback : it->second) {
            callback(player, site == BombSite::A ? "A" : "B"); 
        }
    }

}
void emit(EventType type, const Player& player) { // виклик колбеків для звичайних івентів
    std::vector<std::function<void(const Player&)>> copy;

    {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        auto it = listeners.find(type);
        if (it != listeners.end()) {
            copy = it->second;
        }
    }

    for (auto& callback : copy) {
        callback(player); 
    }
}
/*----------------------------EVENT-SUBSCRIPTION--------------------------------*/
template<typename Func>
void on(EventType type, Func&& callback) {// метод для підписки на івенти, приймає тип івенту і колбек, який буде викликатися при цьому івенті
    std::lock_guard<std::mutex> lock(listeners_mutex);
    listeners[type].push_back(std::forward<Func>(callback));
}
template<typename Func>
void on_forbomb(EventType type, Func&& callback) {
    std::lock_guard<std::mutex> lock(listeners_mutex);
    bombListeners[type].push_back(std::forward<Func>(callback));
}
template<typename Func>
void onKill(Func&& callback, Filters filters) {

    on(EventType::Kill,
        [cb = std::forward<Func>(callback), filters](const Player& p) mutable {

            if (!filters.ActorId || p.steamid == *filters.ActorId) {
                cb(p);
            }
        }
    );
}
template<typename Func> //кажу, що ондеат це тип дед і приймає функцію, яка буде викликана при настанні події дед
void onDeath(Func&& callback,Filters filters) {
    on(EventType::Death, [cb = std::forward<Func>(callback), filters = std::forward<Filters>(filters)](const Player& p) mutable {
        if (!filters.ActorId.has_value() || p.steamid == filters.ActorId.value()) {
            cb(p);
        }
    });
}
template<typename Func> //кажу, що онассист це тип ассист і приймає функцію, яка буде викликана при настанні події ассист
void onAssist(Func&& callback,Filters filters) {
    on(EventType::Assist, [cb = std::forward<Func>(callback), filters = std::forward<Filters>(filters)](const Player& p) mutable {
        if (!filters.ActorId.has_value() || p.steamid == filters.ActorId.value()) {
            cb(p);
        }
    });
}
template<typename Func> // загальний шаблон для підписки на івенти, приймає тип івенту і колбек, який буде викликатися при цьому івенті
void onStartRound(Func&& callback) {
    on(EventType::StartRound, std::forward<Func>(callback));
}
template<typename Func>
void onEndRound(Func&& callback) {
    on(EventType::EndRound, std::forward<Func>(callback));
}
template<typename Func>
void onPlantbomb(Func&& callback) {
    on_forbomb(EventType::Plantbomb, std::forward<Func>(callback));
    
}
template<typename Func>
void onDefusebomb(Func&& callback) {
    on_forbomb(EventType::Defusebomb, std::forward<Func>(callback));
    
}

/*----------------------------SERVER-SETUP--------------------------------*/
bool startServer(int port, bool AccessControlAllowOrigin = false) {
    try{
        svr.Post("/", [AccessControlAllowOrigin,this](const httplib::Request& req, httplib::Response& res) { // запит на данні і відправка
    
    try {
    DATA = json::parse(req.body); 

    handleGameState(DATA); // обробка геймстейту, парсинг
    } catch (const std::exception& e) {
        std::cerr << "JSON error: " << e.what() << std::endl;
        
    }
    if (AccessControlAllowOrigin) {
        res.set_header("Access-Control-Allow-Origin", "*");
    }
    res.set_content("OK", "text/plain");
});

    std::cout << "Server running on local port " << port << "...\n";
    server_thread = std::thread([this, port]() { // запускаємо сервер у окремому потоці
        svr.listen("0.0.0.0", port);
    }); 
    
    return true;

}catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << std::endl;
    return false;
}}
/*----------------------------DATA-ACCESSORS--------------------------------*/
std::string getALLInfo() { 
    std::lock_guard<std::mutex> lock(state_mutex);
    GameState snapshot = state;

    json j;

    j["player"]["name"] = snapshot.player.name;
    j["player"]["steamid"] = snapshot.player.steamid;
    j["player"]["team"] = snapshot.player.team;

    j["player"]["state"]["health"] = snapshot.player.state.health;
    j["player"]["state"]["armor"] = snapshot.player.state.armor;
    j["player"]["state"]["money"] = snapshot.player.state.money;

    j["player"]["match_stats"]["kills"] = snapshot.player.match_stats.kills;
    j["player"]["match_stats"]["assists"] = snapshot.player.match_stats.assists;
    j["player"]["match_stats"]["deaths"] = snapshot.player.match_stats.deaths;
    j["player"]["match_stats"]["mvps"] = snapshot.player.match_stats.mvps;
    j["player"]["match_stats"]["score"] = snapshot.player.match_stats.score;

    j["player"]["position"] = (snapshot.player.position.x) + " " +
                                (snapshot.player.position.y) + " " +
                                (snapshot.player.position.z);

     for (const auto& [key, weapon] : snapshot.player.weapons) {
        j["player"]["weapons"][key]["name"] = weapon.name;
        j["player"]["weapons"][key]["type"] = weapon.type;
        j["player"]["weapons"][key]["ammo_clip"] = weapon.ammo_clip;
    }

    j["round"]["phase"] = snapshot.round.phase;
    j["round"]["win_team"] = snapshot.round.win_team;

    j["map"]["name"] = snapshot.map.name;
    j["map"]["phase"] = snapshot.map.phase;
    j["map"]["round"] = snapshot.map.round;

    j["bomb"]["state"] = snapshot.round.bomb.state;
    j["bomb"]["player"] = snapshot.round.bomb.player;   

    j["map"]["current_spectators"] = snapshot.map.current_spectators;   

    return j.dump(4);
}

int getHealth(const std::string playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);
    auto player = players.find(playerId);
    if (player != players.end()) {
        return player->second.state.health;
    }
    return 0;
}
int getArmor(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.state.armor;
    }
    return 0;
}
int getMoney(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.state.money;
    }
    return 0;
}
int getKills(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.match_stats.kills;
    }
    return 0;
}
int getAssists(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.match_stats.assists;
    }
    return 0;
}
int getDeaths(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.match_stats.deaths;
    }
    return 0;
}
int getMVPs(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.match_stats.mvps;
    }
    return 0;
}
int getScore(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.match_stats.score;
    }
    return 0;
}
std::string getPlayerName(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.name;
    }
    return "";
}
std::string getPlayerTeam(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.team;
    }
    return "";
}
std::string getPlayerActivity(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.activity;
    }
    return "";
}
std::string getPlayerCurrentWeapon(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        return it->second.current_weapon;
    }
    return "";
}
Position getPlayerPosition(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        const auto& pos = it->second.position;
        return Position {pos.x, pos.y, pos.z};
    }
    return Position {"-1", "-1", "-1"};
}
std::string getPlayerPositionAsString(const std::string& playerId) {
    std::lock_guard<std::mutex> lock(players_mutex);

    auto it = players.find(playerId);
    if (it != players.end()) {
        const auto& pos = it->second.position;
        return pos.x + " " + pos.y + " " + pos.z;
    }
    return "-1 -1 -1";
}
json getJsonOfData(){
    return DATA;
}
GameState getGameState() {
    std::lock_guard<std::mutex> lock(state_mutex);
    return state; 
}
std::optional<Player> getPlayerById(const std::string& id) {
    std::lock_guard<std::mutex> lock(players_mutex);
    auto it = players.find(id);
    if (it != players.end()) {
        return it->second;
    }
    return std::nullopt;
}
std::vector<std::string> getAllPlayerIds() {// геттер для отримання всіх айді гравців
    std::lock_guard<std::mutex> lock(players_mutex);
    std::vector<std::string> ids;
    for (auto& [id, _] : players) {
        ids.push_back(id);
    }
    return ids;
}

std::vector<Player> getAllPlayers() {// геттер для отримання всіх гравців у вигляді вектора з структури Player
    std::lock_guard<std::mutex> lock(players_mutex);
    std::vector<Player> result;
    for (auto& [_, p] : players) {
        result.push_back(p);
    }
    return result;
}
void clearTestPlayer(const std::string& id) {//чисто піддебаг хуйня
    std::lock_guard<std::mutex> lock(players_mutex);
    players.erase(id);
}
/*-------------------------------Breaking------------------------------------*/
void stopServer() { // зупиняємо сервер
    cout<<"Stopping server...\n";
    svr.stop();
    if (server_thread.joinable()) {
        server_thread.join(); // чекаємо завершення потока сервера
    }
}
~GSTServer() {
    stopServer(); //при знищенні стопимо сервер
}};


/*----------------------------------------------CONSOLE CLASS-------------------------------------------------------------*/
/*----------------------------------------------CONSOLE CLASS-------------------------------------------------------------*/
/*----------------------------------------------CONSOLE CLASS-------------------------------------------------------------*/

class ConsoleInterface {
private:
    GSTServer& server;

    std::unordered_map<std::string, std::function<void(std::string)>> commands;

    ParsedCommand parseCommand(std::string input) {// метод для парсингу команди з аргументом
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);

        input.erase(0, input.find_first_not_of(" \t\r\n"));
        input.erase(input.find_last_not_of(" \t\r\n") + 1);

        size_t spacePos = input.find(' ');

        if (spacePos == std::string::npos)
            return {input, ""};

        return {input.substr(0, spacePos), input.substr(spacePos + 1)};
    }
/*--------------------------------------Console Commands--------------------------------------*/
    void registerCommands() {

        commands["gethealth"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: gethealth <id>\n";
                return;
            }
            std::cout << server.getHealth(id) << " HP\n";
        };

        commands["getarmor"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getarmor <id>\n";
                return;
            }
            std::cout << "Armor: " << server.getArmor(id) << "\n";
        };

        commands["getkills"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getkills <id>\n";
                return;
            }
            std::cout << "Kills: " << server.getKills(id) << "\n";
        };

        commands["getassists"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getassists <id>\n";
                return;
            }
            std::cout << "Assists: " << server.getAssists(id) << "\n";
        };

        commands["getdeaths"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getdeaths <id>\n";
                return;
            }
            std::cout << "Deaths: " << server.getDeaths(id) << "\n";
        };

        commands["getmvps"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getmvps <id>\n";
                return;
            }
            std::cout << "MVPs: " << server.getMVPs(id) << "\n";
        };

        commands["getscore"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getscore <id>\n";
                return;
            }
            std::cout << "Score: " << server.getScore(id) << "\n";
        };

        commands["getallinfo"] = [&](std::string) {
            std::cout << server.getALLInfo() << "\n";
        };

        commands["getjson"] = [&](std::string) {
            std::cout << server.getJsonOfData().dump(4) << "\n";
        };

        commands["getallplayersids"] = [&](std::string) {
            auto ids = server.getAllPlayerIds();
            for (const auto& id : ids) {
                std::cout << id << "\n";
            }
        };

        commands["getplayer"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getplayer <id>\n";
                return;
            }

            auto p = server.getPlayerById(id);
            if (p) {
                std::cout << p->name << " (" << p->team << ")\n";
            } else {
                std::cout << "Player not found\n";
            }
        };

        commands["getallplayers"] = [&](std::string) {
            auto players = server.getAllPlayers();

            if (players.empty()) {
                std::cout << "No players found\n";
                return;
            }

            std::cout << "=== PLAYERS LIST ===\n";
            for (const auto& p : players) {
                std::cout << "ID: " << p.steamid
                          << " | Name: " << p.name << "\n";
            }
            std::cout << "====================\n";
        };
        commands["getposition"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getposition <id>\n";
                return;
            }
            std::cout << "Position: " << server.getPlayerPositionAsString(id) << "\n";
        };
        commands["getcurrentweapon"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getcurrentweapon <id>\n";
                return;
            }
            std::cout << "Current Weapon: " << server.getPlayerCurrentWeapon(id) << "\n";
        };
        commands["getteam"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getteam <id>\n";
                return;
            }
            std::cout << "Team: " << server.getPlayerTeam(id) << "\n";
        };
        commands["getactivity"] = [&](std::string id) {
            if (id.empty()) {
                std::cout << "Usage: getactivity <id>\n";
                return;
            }
            std::cout << "Activity: " << server.getPlayerActivity(id) << "\n";
        };

        commands["emitkill"] = [&](std::string) {
            Player p;
            p.steamid = "123";
            p.name = "Test";
            p.match_stats.kills = server.getKills(p.steamid) + 1;

            server.emit(EventType::Kill, p);
        };

        commands["emitassist"] = [&](std::string) {
            Player p;
            p.steamid = "123";
            p.name = "Test";
            p.match_stats.assists = server.getAssists(p.steamid) + 1;

            server.emit(EventType::Assist, p);
        };

        commands["emitdeath"] = [&](std::string) {
            Player p;
            p.steamid = "123";
            p.name = "Test";
            p.match_stats.deaths = server.getDeaths(p.steamid) + 1;

            server.emit(EventType::Death, p);
        };

        commands["emitstartround"] = [&](std::string) {
            Player p;
            p.steamid = "123";
            p.name = "Test";

            server.emit(EventType::StartRound, p);
        };

        commands["emitendround"] = [&](std::string) {
            Player p;
            p.steamid = "123";
            p.name = "Test";

            server.emit(EventType::EndRound, p);
        };

        commands["emitplant"] = [&](std::string) {
            Player p;
            p.steamid = "123";
            p.name = "Test";

            server.emitBomb(EventType::Plantbomb, p, BombSite::A);
        };

        commands["emitdefuse"] = [&](std::string) {
            Player p;
            p.steamid = "123";
            p.name = "Test";

            server.emitBomb(EventType::Defusebomb, p, BombSite::A);
        };

        commands["cleartest"] = [&](std::string) {
            server.clearTestPlayer("123");
            std::cout << "Test data cleared\n";
        };

        commands["help"] = [&](std::string) {
            std::cout << "Available commands:\n";
            for (const auto& pair : commands) {
                std::cout << "  " << pair.first << "\n";
            }
        };

        commands["exit"] = [&](std::string) {
            server.stopServer();
            std::exit(0);
        };
    }
/*--------------------------------------Console RUN INTERFACE--------------------------------------*/
public:
    ConsoleInterface(GSTServer& srv) : server(srv) {};
    void run() {
        registerCommands();
        while (true) {
            std::string input;
            std::getline(std::cin, input);

            auto pc = parseCommand(input);

            auto it = commands.find(pc.cmd);
            if (it != commands.end()) {
                it->second(pc.arg);
            } else {
                std::cout << "Unknown command\n";
            }
        }
    }
/*--------------------------------------Console Break--------------------------------------*/
~ConsoleInterface() {
        server.stopServer();
        std::exit(0);
    }
};
