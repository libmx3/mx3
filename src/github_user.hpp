#include <iosfwd>
#include <string>

namespace mx3 {
struct GithubUser {
using namespace std;
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
