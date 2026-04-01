# CS2 Game State Integration (GSI) Library

A lightweight C++ library for parsing and retrieving real-time game state data from Counter-Strike 2 (CS2) through the Game State Integration API.

## Overview

This library provides an easy-to-use interface to integrate with Counter-Strike 2's Game State Integration API. It allows you to:

- **Receive real-time game data** from CS2 (player stats, positions, weapons, bomb state, etc.)
- **Parse JSON game state** automatically
- **Set up event-driven callbacks** for in-game events (kills, deaths, bomb actions, etc.)
- **Filter events** by specific players or conditions
- **Manage multi-threaded server** safely with built-in mutex controls

## Features

**Real-time Game Data Parsing**
- Player information (name, team, SteamID, activity)
- Player state (health, armor, money)
- Match statistics (kills, deaths, assists, score)
- Weapon inventory with ammo counts
- Player position coordinates
- Bomb state and location
- All players' data in a single update
- Round and map information

**Event-Driven Architecture**
- Subscribe to in-game events (onKill, etc.)
- Custom callback functions for game events
- Optional player filtering

**Thread-Safe Operations**
- Built-in mutex protection for concurrent access
- Dedicated server thread (non-blocking)
- Safe multi-threaded player data access

**Automatic Configuration**
- Auto-detection of CS2 installation directory
- Automatic config file generation
- Configurable HTTP server port

## Library Structure

```
GSI_SERVER_CS2_C++/
├── GSI_LIB.h                          # Main GSI server class                 
├── GSICreator.cpp                     # Config generation implementation
├── EXAMPLE_OF_CODE.cpp                # Usage example
├── README.md                          # This documentation
└── Extension/
    ├── httplib.h                      # HTTP server library (cpp-httplib)
    ├── json.hpp                       # JSON parsing library (nlohmann/json)
    ├── Information.h                  # Data structure definitions
    ├── gamestate_integration_test.cfg # config file for parsing data
    └── GSICreator.h                   # File for Creating config in cs2 folder
```

**File Descriptions:**
- `GSI_LIB.h` - Main library containing the `GSTServer` class and game state management
- `GSICreator.h/cpp` - Utilities for auto-detecting CS2 installation and creating config files
- `Information.h` - All data structures (Player, Weapon, Map, Bomb, etc.)
- `EXAMPLE_OF_CODE.cpp` - Complete example demonstrating library usage
- `gamestate_integration_test.cfg` - GSI configuration that works with Counter-Strike 2
- `Extension/httplib.h` - Lightweight HTTP server for receiving GSI data
- `Extension/json.hpp` - Modern JSON library for parsing game state

## Data Structures

### Player Info

#### `State`
Player's current in-game state.
- `int health` - Current health (0-100)
- `int armor` - Armor points (0-100)
- `int money` - Current cash
- `int flashed` - Flashed duration in ms
- `int smoked` - Smoke effect duration in ms
- `int burning` - Burning duration in ms
- `int equip_value` - Total equipment value
- `int round_kills` - Kills in current round
- `int round_killhs` - Headshots in current round
- `int round_totaldmg` - Total damage dealt in round
- `bool helmet` - Whether player has helmet

#### `MatchStats`
Player's match statistics.
- `int kills` - Total match kills
- `int assists` - Total assists
- `int deaths` - Total deaths
- `int mvps` - MVP awards
- `int score` - Current score

#### `Weapon`
Weapon information.
- `std::string name` - Weapon name (e.g., "ak47")
- `std::string type` - Weapon type (e.g., "Rifle")
- `int ammo_clip` - Ammunition in magazine
- `int ammo_reserve` - Ammunition in reserve
- `std::string state` - Weapon state (e.g., "active", "holstered")

#### `Position`
3D coordinates in the game world.
- `std::string x` - X coordinate
- `std::string y` - Y coordinate
- `std::string z` - Z coordinate (height)

#### `Player`
Main player data structure containing all player information.
- `std::string name` - Player nickname
- `std::string steamid` - Steam ID
- `std::string team` - Team assignment ("T" or "CT")
- `std::string activity` - Current activity (e.g., "playing", "dead")
- `int observer_slot` - Observer slot number (if spectating)
- `State state` - Player's current state
- `MatchStats match_stats` - Match statistics
- `std::map<std::string, Weapon> weapons` - Equipped weapons map
- `Position position` - Player's 3D position
- `std::string current_weapon` - Currently held weapon

### Game World

#### `Team`
Team information.
- `int score` - Team's round score
- `std::string name` - Team name
- `int consecutive_round_losses` - Consecutive lost rounds
- `int timeouts_remaining` - Available timeouts
- `int matches_won_this_series` - Series wins

#### `Map`
Map and round information.
- `std::string name` - Map name (e.g., "de_mirage")
- `std::string mode` - Game mode (e.g., "competitive")
- `std::string phase` - Current phase (e.g., "live", "intermission")
- `int round` - Current round number
- `int current_spectators` - Number of spectators
- `Team team_ct` - Counter-Terrorist team info
- `Team team_t` - Terrorist team info

#### `PhaseCountdowns`
Phase timing information.
- `std::string phase` - Phase name
- `double phase_ends_in` - Seconds until phase ends

#### `BombSite` (Enum)
Bomb site enumeration.
- `A` - Bomb site A
- `B` - Bomb site B
- `Unknown` - Unknown site

#### `Bomb`
Bomb state information.
- `std::string state` - Bomb state (e.g., "planted", "carried", "defused")
- `std::string player` - SteamID of bomb carrier or planter
- `BombSite site` - Bomb site (A, B, or Unknown)
- `int countdown` - Detonation countdown in seconds

#### `Round`
Round information.
- `std::string phase` - Round phase (e.g., "live", "over")
- `std::string win_team` - Winning team (e.g., "CT" or "T")
- `Bomb bomb` - Bomb state in this round

### System & Events

#### `Provider`
Game provider information.
- `std::string name` - Provider name (usually "CS2")
- `int appid` - Application ID
- `std::string steamid` - Server SteamID
- `long long timestamp` - Unix timestamp
- `int version` - Protocol version

#### `GameState`
Complete game state snapshot.
- `Player player` - Local player information
- `Map map` - Map and round data
- `Round round` - Current round information
- `Provider provider` - Provider/server information
- `PhaseCountdowns phase_countdowns` - Phase timing

#### `EventType` (Enum)
Available game events.
- `Kill` - Player kill event
- `Death` - Player death event
- `Assist` - Player assist event
- `StartRound` - Round started
- `EndRound` - Round ended
- `Plantbomb` - Bomb planted
- `Defusebomb` - Bomb defused

#### `Filters`
Event filtering options.
- `std::optional<std::string> ActorId` - Filter by specific SteamID (empty = all players)

## Installation & Setup

### Prerequisites
- C++17 or later
- Windows (currently configured for Windows)
- Counter-Strike 2 installed
- Visual Studio or compatible C++ compiler

### Getting Started

1. **Clone/Copy the library** to your project directory

2. **Include the main header:**
   ```cpp
   #include "GSI_LIB.h"
 
3. **Initialize the server:**
   ```cpp
   GSTServer server;
   
   // Auto-configure CS2
   std::optional<std::filesystem::path> path = GSCreator::findCS2CFG();
   if (path) {
       GSCreator::createConfig(path.value());
   }
   
   // Start server on port 3000
   if (!server.startServer(3000)) {
       std::cerr << "Failed to start server" << std::endl;
       return 1;
   }
   ```

## Usage Example
```cpp
/* MUST HAVE LIBRARIES */
#include "Extension/httplib.h"
#include "Extension/Information.h"
#include "GSICreator.h"
#include "GSI_LIB.h"
/* MUST HAVE LIBRARIES */

using namespace std;

int main() {

    GSTServer server;
    ConsoleInterface console(server);

    std::optional<std::filesystem::path> path = GSCreator::findCS2CFG(); //Searching for game folder

    if (path) {
        GSCreator::createConfig(path.value()); // GENERATION OF CONFIG FILE
    }

    if (!server.startServer(3000)) {
        std::cerr << "Failed to start the server." << std::endl;   //Example of error handling
        return 1;
    };
    /*declaration of Filters*/
    Filters filters;
    filters.ActorId = "76561197968303909"; //some id of player, that should be listened, if empty - all players will be listened

    server.onKill([&server](const Player& p) {

    /*callbackfuctions*/
    cout<< "Kill event: " << p.name << " (" << p.steamid << ") now has " << p.match_stats.kills << " kills.\n";
    /*callbackfuctions*/

}, filters); /* + filters*/

    console.run(); //console run
    return 0;   
}




## Configuration

The library automatically generates a GSI config file in the CS2 directory. The configuration includes:

```
"GST"
{
    "uri"                   "http://127.0.0.1:3000"
    "timeout"               "5.0"
    "buffer"                "0.1"
    "throttle"              "0.1"
    "heartbeat"             "30.0"
    
    "data"
    {
        "provider"          "1"
        "map"               "1"
        "round"             "1"
        "player_id"         "1"
        "player_state"      "1"
        "player_match_stats"    "1"
        "player_weapons"    "1"
        "phase_countdowns"  "1"
        "bomb"              "1"
        "allplayers"        "1"
        "allplayers_weapons"    "1"
        "player_position"   "1"
    }
}
```

#### `Filters`
Optional filtering for events.

**Members:**
- `std::string ActorId` - Filter by specific SteamID (empty = all players)

#### `GameState`
Contains the current and previous game state.

**Members:**
- `Player player` - Local player data
- `std::vector<Player> allPlayers` - All players in match
- `// + other game state fields`

## Thread Safety

All internal operations are automatically protected.

## Troubleshooting

### Server won't start
- Check if the specified port is already in use
- Ensure Windows firewall allows the connection
- Verify CS2 is installed and the config was generated

### No data received
- Ensure CS2 is running and a game is active
- Check if the config file was created in the CS2 directory
- Verify the URI in the config matches your server address
- Try connecting to http://127.0.0.1:3000 from a browser

### Multiple same events triggered
- Throttle rate in the config may be too low
- Consider adding event debouncing in callbacks

## Performance Considerations

- The server runs on a separate thread, avoiding blocking
- Player data is stored in an unordered_map for O(1) access
- JSON parsing is done efficiently using nlohmann/json
- Mutex locks are kept minimal to avoid contention

## License

This library is provided as-is for integration with Counter-Strike 2.

## Contributing

For issues, suggestions, or improvements, please contribute or report issues.

---
