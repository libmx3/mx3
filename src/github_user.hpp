#include <iosfwd>
#include <string>

namespace mx3 {
struct GithubUser {
    std::string login;
    std::string avatarURL;
    std::string gravatarID;
    std::string url;
    std::string htmlURL;
    std::string followersURL;
    std::string followingURL;
    std::string gistsURL;
    std::string starredURL;
    std::string subscriptionsURL;
    std::string organizationsURL;
    std::string reposURL;
    std::string eventsURL;
    std::string receivedEventsURL;
    std::string type;
    bool siteAdmin;
};
}
