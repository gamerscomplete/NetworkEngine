#ifndef DG_NET
#define DG_NET

#include <postgresql/libpq-fe.h>
#include <iostream>
#include <algorithm>
#include "utils.h"

class dg_net {
    PGconn          *conn;
    PGresult        *res;
    int             rec_count;
    int             row;
    int             col;
    std::vector<std::string> Splitter(std::string del, std::string message);
    std::string DoLogin(std::string login);

public:
    dg_net();
    std::string process_message(std::string message);
    std::string GetMOTD();    
    void Logout();
};

dg_net::dg_net() {
    conn = PQconnectdb("dbname=darkgate_unity host=database user=darkgate password=dizarkgat3");
    if (PQstatus(conn) == CONNECTION_BAD) {
        std::cout << "We were unable to connect to the database";
//        goto FINISH;
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
//        handleInputs(tempMessage[1]);
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
    std::vector<std::string> splitResults = Splitter(":", login);

    if(splitResults.size() != 2) {
        std::cout << "Incorrect login format \"" << login << "\"\n";
        return "LOGIN|Incorrect format";
    }

    std::string query = "SELECT count(*)  FROM players WHERE username='" + splitResults[0] + "' and password='" + splitResults[1] + "'";

    res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cout << "Query failed: " << query << "\n";
        return "LOGIN|Failed";
    }

    rec_count = PQntuples(res);
    std::cout << "Record count: " << rec_count << "\n";

    std::string loginreturn = PQgetvalue(res, 0, 0);
    if(loginreturn == "0") {
        std::cout << "Login failed\n";
        return "LOGIN|Credentials incorrect";
    } else {
        std::cout << "Get vals returned: \"" << loginreturn << "\"\n";
    }
    PQclear(res);
    return "LOGIN|SUCCESS";
}


#endif
