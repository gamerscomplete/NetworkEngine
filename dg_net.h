#ifndef DG_NET
#define DG_NET

#include <postgresql/libpq-fe.h>
#include <iostream>
#include <algorithm>
#include "utils.h"
#include <sstream>
#include <sys/stat.h>
#include "utilities.h"
//#include <sstream>

class dg_net {
    PGconn          *conn;
    PGresult        *res;
    int             rec_count;
    int             row;
    int             col;
//    char s[300];
    int unitySock;
    int num, serverfd, clientfd, playerID;
    Vector3 playerPosition;
    std::vector<std::string> Splitter(std::string del, std::string message);
    std::string DoLogin(std::string login);
    std::string status;
    bool InstantiatePlayer();
    void PassInputs(std::string inputs);
    bool SendServerMessage(std::string message);

public:
    void SetFD(int fd);
    dg_net();
    std::string process_message(std::string message);
    std::string GetMOTD();
    void Logout();
};

void dg_net::SetFD(int fd) {
    unitySock = fd;
}

dg_net::dg_net() {
    status = "DISC";
    conn = PQconnectdb("dbname=darkgate_unity host=database user=darkgate password=dizarkgat3");
    if (PQstatus(conn) == CONNECTION_BAD) {
        std::cout << "We were unable to connect to the database";
//        goto FINISH;
    }
}

bool dg_net::SendServerMessage(std::string message) {
    int bytecount;
    if(bytecount = send(unitySock, message.c_str(),  strlen(message.c_str()), 0) == -1) {
        std::cout << "Could not send server message\n";
        fprintf(stderr, "Error sending data %d\n", errno);
        return false;
    } else {
        return true;
    }
}

std::string dg_net::GetMOTD() {
    std::string blah;
    std::string welcome_message;

    res = PQexec(conn, "SELECT message FROM motd order by id desc limit 1");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cout << "Message of the day not found";
    }

    rec_count = PQntuples(res);

    if(rec_count != 1) {
        std::cout << "No MOTD found\n";
        return "";
    }

    blah = PQgetvalue(res, 0, 0);
    welcome_message = "MOTD|" + blah;
    PQclear(res);
    return welcome_message;
}

void dg_net::Logout() {
    PQfinish(conn);
}

std::string dg_net::process_message(std::string message) {

/*    if(message[message.size() - 1] == '\n') {
        std::cout << "Last character is a carriage return\n";
        message.erase(message.size() - 1);
    } else {
        std::cout << "last character: \"" << message[message.size() - 1] << "\"\n";
    }
*/

//    message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());
    std::cout << "Processing message from client: \"" << trim(message) << "\"\n";
    std::vector<std::string> tempMessage = Splitter("|", message);
    std::string returnMessage;

    if(tempMessage.size() == 0) {
        std::cout << "Command not found.\n";
        return "Failed|Command not found";
    }

    if(tempMessage[0] == "LOGIN") {
        std::cout << "Login\n";
        return DoLogin(tempMessage[1]);
    } else if(tempMessage[0] == "INPUTS") {
        std::cout << "Inputs\n";
        PassInputs(tempMessage[1]);
    } else if(tempMessage[0] == "PING") {
        std::cout << "Ping\n";
        return "PONG|" + tempMessage[1];
    } else if(tempMessage[0] == "FOLD") {
        std::cout << "Fold\n";
        return "FOLD|true";
    }

    std::cout << "Made it past process_message\n";
    return "";
}

std::vector<std::string> dg_net::Splitter(std::string del, std::string str) {
    std::vector<std::string> returnString;
    int pos = str.find(del);
    if(pos == -1) {
//        std::cout << "Did not find delimiter in string \"" << str << "\"\n";
        return returnString;
    }
//    std::cout << "Looking for \"" << del << "\" in \"" << str << "\" Position: " << pos << "\n";
//    std::cout << "Substring block 1: \"" << str.substr(0, pos) << "\"\n";
    returnString.push_back(str.substr(0, pos));
    returnString.push_back(str.substr(pos + 1));
    return returnString;
}

std::string dg_net::DoLogin(std::string login) {
    if(status == "CONNECTED") {
        std::cout << "Player already connected\n";
        return "LOGIN|Player already connected";
    }
    std::vector<std::string> splitResults = Splitter(":", login);

    if(splitResults.size() != 2) {
        std::cout << "Incorrect login format \"" << login << "\"\n";
        return "LOGIN|Incorrect format";
    }

    std::string query = "SELECT id, x, y, z FROM players WHERE username='" + splitResults[0] + "' and password='" + splitResults[1] + "'";

    res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cout << "Query failed: " << query << "\n";
        return "LOGIN|Failed";
    }

    rec_count = PQntuples(res);
    std::cout << "Record count: " << rec_count << "\n";

    if(rec_count != 1) {
        std::cout << "Login failed no rows found\n";
        return "LOGIN|Credentials incorrect";
    } else {
        std::cout << "ID returned: \"" << PQgetvalue(res, 0, 0) << "\"\n";
    }

    std::string loginreturn = PQgetvalue(res, 0, 0);
    Vector3 position;
    position.x = atof(PQgetvalue(res, 0, 1));
    position.y = atof(PQgetvalue(res, 0, 2));
    position.z = atof(PQgetvalue(res, 0, 3));

    playerPosition = position;

    std::cout << "Login position: " << stringify(position) << "\n";

    std::stringstream convert(loginreturn);
    int Result;
    if (!(convert >> Result)) {
        std::cout << "Conversion to int failed for player\n";
        Result = 0;
    }

    playerID = Result;

    if(!InstantiatePlayer()) {
        std::cout << "Could not instantiate player\n";
        return "LOGIN|Could not instantiate player";
    }
    PQclear(res);
    status = "CONNECTED";
    return "LOGIN|SUCCESS";
}

bool dg_net::InstantiatePlayer() {
    std::cout << "Instantiating player\n";
    std::string tempMessage = "INSTANTIATE|" + stringify(playerID) + ":" + stringify(playerPosition);
    if(SendServerMessage(tempMessage)) {
        return true;
    } else {
        return false;
    }
    //get player position, rotation
    //call into master server thread for unity game server to instantiate a player
    //if instantiate is succesful then return true, otherwise return false
}

void dg_net::PassInputs(std::string inputs) {
    std::cout << "Passing inputs\n";
    std::string temp = "INPUTS|" + stringify(playerID) + ":" + inputs;
    SendServerMessage(temp);
    //Take inputs string and pass them to master unity thread
}

#endif
