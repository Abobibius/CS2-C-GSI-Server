#pragma once

#include <string>
#include <map>

struct State {
    int health = 0;
    int armor = 0;
    int money = 0;
    int flashed = 0;
    int smoked = 0;
    int burning = 0;
    int equip_value = 0;
    int round_kills = 0;
    int round_killhs = 0;
    int round_totaldmg = 0;
    bool helmet = false;
};
struct PhaseCountdowns {
    std::string phase;
    double phase_ends_in = 0.0;
};
struct Team {
    int score = 0;
    std::string name;
    int consecutive_round_losses = 0;
    int timeouts_remaining = 0;
    int matches_won_this_series = 0;
};
struct Map {
    std::string name;
    std::string mode;
    std::string phase; 
    int round = 0;

    int current_spectators = 0;

    Team team_ct;
    Team team_t;
};
struct MatchStats {
    int kills = 0;
    int assists = 0;
    int deaths = 0;
    int mvps = 0;
    int score = 0;
};

struct Weapon {
    std::string name;
    std::string type;
    int ammo_clip = 0;
    int ammo_reserve = 0;
    std::string state;
};
struct Position {
    std::string x;
    std::string y;
    std::string z;
};
struct Player {
    std::string name;
    std::string steamid;
    std::string team;
    std::string activity; 
    int observer_slot = 0;
    State state;
    MatchStats match_stats;
    std::map<std::string, Weapon> weapons;
    Position position;
    std::string current_weapon;
};
enum class BombSite {
    A,
    B,
    Unknown
};

struct Bomb {
    std::string state;
    std::string player;
    BombSite site;

    int countdown = 0;
};

struct Round {
    std::string phase;
    std::string win_team;
    Bomb bomb;
};

struct Provider {
    std::string name;
    int appid = 0;
    std::string steamid;
    long long timestamp = 0;
    int version = 0;
};
struct GameState {
    Player player;
    Map map;
    Round round;
    Provider provider;
    PhaseCountdowns phase_countdowns;
};
enum class EventType {
    Kill,
    Death,
    Assist,
    StartRound,
    EndRound,
    Plantbomb,
    Defusebomb
};
struct Filters{
    std::optional<std::string> ActorId;    
};
struct Event {
    std::string type;
    std::string player;
    
};

/*--------------------Console Structure ---------------------*/

struct ParsedCommand {
        std::string cmd;
        std::string arg;
    };