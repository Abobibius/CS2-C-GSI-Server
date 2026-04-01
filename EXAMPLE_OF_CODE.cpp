
/* MUST HAVE LIBRARIES */
#include "Extension/httplib.h"
#include "Extension/Information.h"
#include "Extension/GSICreator.h"
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
    server.onKill([&server](const Player &p) { //Example of callback function for kill event, that will be called every time when player with id 76561197968303909 will get a kill

    /*callbackfuctions*/
    cout<< "Kill event: " << p.name << " got a kill! Total kills: " << p.match_stats.kills << "\n";
    /*callbackfuctions*/

}, filters); /* + filters*/

    console.run(); //console run
    return 0;   
}

// ДОДАТИ КРЧ КАСТОМНІ ФІЛЬТРИ ЯКІ МОЖНА САМОМУ ПИСАТИ + ВИПРАВИТИ СИСТЕМУ, ЩОБ МОЖНА БУЛО ДЕКІЛЬКА ФІЛЬТРІВ ДОДАВАТИ
