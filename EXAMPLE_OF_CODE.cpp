
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
    filters.ActorId = "76561197968303909"; //some id of player, that should be listened, if empty - all players will be listened

    server.onKill([&server](const Player& p) {

    /*callbackfuctions*/
    cout<< "Kill event: " << p.name << " (" << p.steamid << ") now has " << p.match_stats.kills << " kills.\n";
    /*callbackfuctions*/

}, filters); /* + filters*/

    console.run(); //console run
    return 0;   
}
