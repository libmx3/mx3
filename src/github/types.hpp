#pragma once
#include "stl.hpp"

namespace github {

// lets put all the "entities" of the github api in here
struct User {
    string login;
    string avatarURL;
    string gravatarID;
    string url;
    string htmlURL;
    string followersURL;
    string followingURL;
    string gistsURL;
    string starredURL;
    string subscriptionsURL;
    string organizationsURL;
    string reposURL;
    string eventsURL;
    string receivedEventsURL;
    string accountType;
    bool siteAdmin;
};

}
