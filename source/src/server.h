// server.h

#define gamemode sg->smode   // allows the gamemode macros to work with the server mode

#define SERVER_PROTOCOL_VERSION    (PROTOCOL_VERSION)    // server without any gameplay modification
//#define SERVER_PROTOCOL_VERSION   (-PROTOCOL_VERSION)  // server with gameplay modification but compatible to vanilla client (using /modconnect)
//#define SERVER_PROTOCOL_VERSION  (PROTOCOL_VERSION)    // server with incompatible protocol (change PROTOCOL_VERSION in file protocol.h to a negative number!)

#define valid_flag(f) (f >= 0 && f < 2)

enum { GE_NONE = 0, GE_SHOT, GE_EXPLODE, GE_HIT, GE_AKIMBO, GE_RELOAD, GE_SUICIDE, GE_PICKUP };
enum { ST_EMPTY, ST_LOCAL, ST_TCPIP };

extern int servmillis;
extern bool triggerpollrestart;

struct shotevent
{
    int type;
    int millis, id;
    int gun;
    float from[3], to[3];
};

struct explodeevent
{
    int type;
    int millis, id;
    int gun;
};

struct hitevent
{
    int type;
    int target;
    int lifesequence;
    union
    {
        int info;
        float dist;
    };
    float dir[3];
};

struct suicideevent
{
    int type;
};

struct pickupevent
{
    int type;
    int ent;
};

struct akimboevent
{
    int type;
    int millis, id;
};

struct reloadevent
{
    int type;
    int millis, id;
    int gun;
};

union gameevent
{
    int type;
    shotevent shot;
    explodeevent explode;
    hitevent hit;
    suicideevent suicide;
    pickupevent pickup;
    akimboevent akimbo;
    reloadevent reload;
};

template <int N>
struct projectilestate
{
    int projs[N];
    int numprojs;

    projectilestate() : numprojs(0) {}

    void reset() { numprojs = 0; }

    void add(int val)
    {
        if(numprojs>=N) numprojs = 0;
        projs[numprojs++] = val;
    }

    bool remove(int val)
    {
        loopi(numprojs) if(projs[i]==val)
        {
            projs[i] = projs[--numprojs];
            return true;
        }
        return false;
    }
};

static const int DEATHMILLIS = 300;

struct clientstate : playerstate
{
    vec o;
    int state;
    int lastdeath, lastspawn, spawn, lifesequence, lastclaction;
    bool forced;
    int lastshot;
    projectilestate<8> grenades;
    int akimbomillis;
    bool scoped;
    int flagscore, frags, teamkills, deaths, shotdamage, damage, events, lastdisc, reconnections;
    int suicides, friendlyfire, enemyfire, goodflags, antiflags; // match only vita replacement

    clientstate() : state(CS_DEAD) {}

    bool isalive(int gamemillis)
    {
        return state==CS_ALIVE || (state==CS_DEAD && gamemillis - lastdeath <= DEATHMILLIS);
    }

    bool waitexpired(int gamemillis)
    {
        int wait = gamemillis - lastshot;
        loopi(NUMGUNS) if(wait < gunwait[i]) return false;
        return true;
    }

    void reset()
    {
        state = CS_DEAD;
        lifesequence = -1;
        grenades.reset();
        akimbomillis = 0;
        scoped = forced = false;
        flagscore = frags = teamkills = deaths = shotdamage = damage = events = lastdisc = reconnections = 0;
        lastdeath = lastclaction = 0;
        suicides = friendlyfire = enemyfire = goodflags = antiflags = 0;
        respawn();
    }

    void respawn()
    {
        playerstate::respawn();
        o = vec(-1e10f, -1e10f, -1e10f);
        lastspawn = -1;
        spawn = 0;
        lastshot = 0;
        akimbomillis = 0;
        scoped = false;
    }
};

struct savedscore
{
    string name;
    uint ip;
    int frags, flagscore, deaths, teamkills, shotdamage, damage, team, events, lastdisc, reconnections;
    bool valid, forced;

    void reset()
    {
        // to avoid 2 connections with the same score... this can disrupt some laggers that eventually produces 2 connections (but it is rare)
        frags = flagscore = deaths = teamkills = shotdamage = damage = events = lastdisc = reconnections = 0;
    }

    void save(clientstate &cs, int t)
    {
        frags = cs.frags;
        flagscore = cs.flagscore;
        deaths = cs.deaths;
        teamkills = cs.teamkills;
        shotdamage = cs.shotdamage;
        damage = cs.damage;
        forced = cs.forced;
        events = cs.events;
        lastdisc = cs.lastdisc;
        reconnections = cs.reconnections;
        team = t;
        valid = true;
    }

    void restore(clientstate &cs)
    {
        cs.frags = frags;
        cs.flagscore = flagscore;
        cs.deaths = deaths;
        cs.teamkills = teamkills;
        cs.shotdamage = shotdamage;
        cs.damage = damage;
        cs.forced = forced;
        cs.events = events;
        cs.lastdisc = lastdisc;
        cs.reconnections = reconnections;
        reset();
    }
};

#define VITANAMEHISTLEN 4
#define VITAIPHISTLEN 4
#define VITACCHISTLEN 4
#define VITACOMMENTLEN 31
#define VITACLANLEN 7

struct vita_s
{
    char namehist[VITANAMEHISTLEN][MAXNAMELEN + 1];
    enet_uint32 iphist[VITAIPHISTLEN];
    char privatecomment[VITACOMMENTLEN + 1], publiccomment[VITACOMMENTLEN + 1]; // private comments can be seen by admins and owners only, public comments can be seen by everyone
    char clan[VITACLANLEN + 1];
    char cchist[VITACCHISTLEN * 2];
    int vs[VS_NUM];
    void addname(const char *name);
    void addip(enet_uint32 ip);
    void addcc(const char *cc);
};

struct vitakey_s { const vita_s *v; const uchar32 *k; };

struct client                   // server side version of "dynent" type
{
    int type;
    int clientnum, clientsequence;
    ENetPeer *peer;
    string hostname;
    enet_uint32 ip, ip_censored;
    int connectclock;
    char country[4];
    uchar32 pubkey;
    char pubkeyhex[68];
    vita_s *vita;
    string servinforesponse;
    string name;
    int team;
    int maxroll, maxrolleffect, ffov, scopefov;
    char lang[3];
    int ping;
    int skin[2];
    int vote;
    int role;
    int connectmillis, ldt, spj;
    int mute, spam, lastvc; // server side voice comm spam control
    int acversion, acbuildtype;
    bool isauthed; // for passworded servers
    bool needsauth;
    bool haswelcome;
    bool isonrightmap, loggedwrongmap, freshgame;
    bool timesync;
    bool ispaused;
    int overflow;
    int gameoffset, lastevent, lastvotecall;
    int lastprofileupdate, fastprofileupdates;
    int demoflags;
    clientstate state;
    vector<gameevent> events;
    vector<uchar> position, messages;
    string lastsaytext;
    int saychars, lastsay, spamcount, badspeech, badmillis;
    int at3_score, at3_lastforce;
    bool at3_dontmove;
    int spawnindex;
    int spawnperm, spawnpermsent;
    bool autospawn;
    int salt;
    string pwd;
    int spectcn;
    int mapcollisions, farpickups;
    enet_uint32 bottomRTT;
    int lag;
    int nvotes;
    int ffire, wn, f, t, yaw, pitch;

    gameevent &addevent()
    {
        static gameevent dummy;
        if(events.length()>100) return dummy;
        return events.add();
    }

    void mapchange(bool getmap = false)
    {
        state.reset();
        events.setsize(0);
        overflow = 0;
        timesync = false;
        isonrightmap = false;
        spawnperm = SP_WRONGMAP;
        spawnpermsent = servmillis;
        autospawn = false;
        if(!getmap)
        {
            loggedwrongmap = false;
            freshgame = true;         // permission to spawn at once
        }
        lastevent = 0;
        at3_lastforce = 0;
        mapcollisions = farpickups = 0;
        lag = 0;
        ldt = spj = 0;
        ffire = 0;
        f = yaw = pitch = t = 0;
        ispaused = 0;
    }

    void reset()
    {
        name[0] = pwd[0] = country[0] = lang[0] = demoflags = 0;
        ip = ip_censored = 0;
        pubkeyhex[0] = '\0';
        vita = NULL;
        bottomRTT = ping = 9999;
        team = TEAM_SPECT;
        maxroll = maxrolleffect = ffov = scopefov = 0;
        state.state = CS_SPECTATE;
        loopi(2) skin[i] = 0;
        position.setsize(0);
        messages.setsize(0);
        isauthed = haswelcome = false;
        role = CR_DEFAULT;
        lastvotecall = 0;
        lastprofileupdate = fastprofileupdates = 0;
        vote = VOTE_NEUTRAL;
        lastsaytext[0] = '\0';
        saychars = 0;
        spawnindex = -1;
        mapchange();
        freshgame = false;         // don't spawn into running games
        mute = spam = lastvc = badspeech = badmillis = nvotes = 0;
        ispaused = 0;
    }

    void zap()
    {
        type = ST_EMPTY;
        role = CR_DEFAULT;
        isauthed = haswelcome = false;
    }

    bool checkvitadate(int n)
    {
        extern int servclock;
        return vita && (vita->vs[n] == 1 || (servclock - vita->vs[n]) < 0);
    }

    void incrementvitacounter(int vitastat, int amount)
    {
        if (!vita) return;
        vita->vs[vitastat] += amount;
    }
};

struct ban
{
    ENetAddress address;
    int millis, type;
};

struct worldstate
{
    enet_uint32 uses;
    vector<uchar> positions, messages;
};

struct server_entity            // server side version of "entity" type
{
    int type;
    bool spawned, legalpickup, twice;
    int spawntime;
    short x, y;
};

struct sflaginfo
{
    int state;
    int actor_cn;
    float pos[3];
    int lastupdate;
    int stolentime;
    short x, y;          // flag entity location
};

struct clientidentity
{
    uint ip;
    int clientnum;
};

void startgame(const char *newname, int newmode, int newtime = -1, bool notify = true);
void disconnect_client(int n, int reason = -1);
void sendiplist(int receiver, int cn = -1);
int clienthasflag(int cn);
bool refillteams(bool now = false, int ftr = FTR_AUTOTEAM);
void changeclientrole(int client, int role, char *pwd = NULL, bool force=false);
int findmappath(const char *mapname, char *filename = NULL);
int calcscores();
void recordpacket(int chan, void *data, int len);
void senddisconnectedscores(int cn);
void process(ENetPacket *packet, int sender, int chan);
void welcomepacket(packetbuf &p, int n);
void sendwelcome(client *cl, int chan = 1);
void sendpacket(int n, int chan, ENetPacket *packet, int exclude = -1, bool demopacket = false);
int numclients();
bool updateclientteam(int cln, int newteam, int ftr);
void forcedeath(client *cl);
void sendf(int cn, int chan, const char *format, ...);

extern bool isdedicated;

const char *modeinfologfilename = NULL;

const char *messagenames[SV_NUM] =
{
    "SV_SERVINFO", "SV_SERVINFO_RESPONSE", "SV_SERVINFO_CONTD", "SV_WELCOME", "SV_INITCLIENT", "SV_POS", "SV_POSC", "SV_POSC2", "SV_POSC3", "SV_POSC4", "SV_POSN",
    "SV_TEXT", "SV_TEAMTEXT", "SV_TEXTME", "SV_TEAMTEXTME", "SV_TEXTPRIVATE",
    "SV_SOUND", "SV_VOICECOM", "SV_VOICECOMTEAM", "SV_CDIS",
    "SV_SHOOT", "SV_EXPLODE", "SV_SUICIDE", "SV_AKIMBO", "SV_RELOAD",
    "SV_GIBDIED", "SV_DIED", "SV_GIBDAMAGE", "SV_DAMAGE", "SV_HITPUSH", "SV_SHOTFX", "SV_THROWNADE",
    "SV_TRYSPAWN", "SV_SPAWNSTATE", "SV_SPAWN", "SV_SPAWNDENY", "SV_FORCEDEATH", "SV_RESUME",
    "SV_DISCSCORES", "SV_TIMEUP", "SV_EDITENT", "SV_ITEMACC",
    "SV_MAPCHANGE", "SV_ITEMSPAWN", "SV_ITEMPICKUP",
    "SV_PING", "SV_PONG", "SV_CLIENTPING",
    "SV_EDITMODE", "SV_EDITXY", "SV_EDITARCH", "SV_EDITBLOCK", "SV_EDITD", "SV_EDITE", "SV_NEWMAP",
    "SV_SENDMAP", "SV_RECVMAP", "SV_REMOVEMAP",
    "SV_SERVMSG", "SV_SERVMSGVERB", "SV_ITEMLIST", "SV_WEAPCHANGE", "SV_PRIMARYWEAP", "SV_SECONDARYWEAP",
    "SV_FLAGACTION", "SV_FLAGINFO", "SV_FLAGMSG", "SV_FLAGCNT",
    "SV_ARENAWIN",
    "SV_SETADMIN", "SV_SERVOPINFO",
    "SV_CALLVOTE", "SV_CALLVOTESUC", "SV_CALLVOTEERR", "SV_VOTE", "SV_VOTERESULT",
    "SV_SETTEAM", "SV_TEAMDENY", "SV_SERVERMODE",
    "SV_IPLIST", "SV_SPECTCN",
    "SV_LISTDEMOS", "SV_SENDDEMOLIST", "SV_GETDEMO", "SV_SENDDEMO", "SV_DEMOPLAYBACK",
    "SV_CONNECT",
    "SV_SWITCHNAME", "SV_SWITCHSKIN", "SV_SWITCHTEAM",
    "SV_CLIENT",
    "SV_EXTENSION",
    "SV_MAPIDENT", "SV_DEMOCHECKSUM", "SV_DEMOSIGNATURE",
    "SV_PAUSEMODE"
};

const char *entnames[] =
{
    "none?",
    "light", "playerstart", "pistol", "ammobox","grenades",
    "health", "helmet", "armour", "akimbo",
    "mapmodel", "trigger", "ladder", "ctf-flag", "sound", "clip", "plclip", "dummy", ""
};

// entity attribute scaling and wraparound definitions for mapformat 10
short entwraparound[MAXENTTYPES][7] =
{ // limit angles to 0..359 degree and mapsound slot numbers to 0..255
    {    0,    0,    0,    0,    0,    0,    0 },  // deleted
    {    0,    0,    0,    0,    0,    0,    0 },  // light
    { 3600,    0,    0,    0,    0,    0,    0 },  // playerstart
    {    0,    0,    0,    0,    0,    0,    0 },  // pistol
    {    0,    0,    0,    0,    0,    0,    0 },  // ammobox
    {    0,    0,    0,    0,    0,    0,    0 },  // grenades
    {    0,    0,    0,    0,    0,    0,    0 },  // health
    {    0,    0,    0,    0,    0,    0,    0 },  // helmet
    {    0,    0,    0,    0,    0,    0,    0 },  // armour
    {    0,    0,    0,    0,    0,    0,    0 },  // akimbo
    { -3600,   0,    0,    0, -3600,   0,    0 },  // mapmodel
    {    0,    0,    0,    0,    0,    0,    0 },  // trigger
    {    0,    0,    0,    0,    0,    0,    0 },  // ladder
    { -3600,   0,    0,    0,    0,    0,    0 },  // ctf-flag
    {  256,    0,    0,    0,    0,    0,    0 },  // sound
    {    0,    0,    0,    0,    0,    0,    4 },  // clip
    {    0,    0,    0,    0,    0,    0,    4 },  // plclip
    {    0,    0,    0,    0,    0,    0,    0 }   // dummy
};

uchar entscale[MAXENTTYPES][7] =
{ // (no zeros allowed here!)
    {  1,  1,  1,  1,  1,  1,  1 },  // deleted
    {  1,  1,  1,  1,  1,  1,  1 },  // light
    { 10,  1,  1,  1,  1,  1,  1 },  // playerstart
    { 10,  1,  1,  1,  1,  1,  1 },  // pistol
    { 10,  1,  1,  1,  1,  1,  1 },  // ammobox
    { 10,  1,  1,  1,  1,  1,  1 },  // grenades
    { 10,  1,  1,  1,  1,  1,  1 },  // health
    { 10,  1,  1,  1,  1,  1,  1 },  // helmet
    { 10,  1,  1,  1,  1,  1,  1 },  // armour
    { 10,  1,  1,  1,  1,  1,  1 },  // akimbo
    { 10,  1,  5,  1, 10,  1,  1 },  // mapmodel
    {  1,  1,  1,  1,  1,  1,  1 },  // trigger
    {  1,  1,  1,  1,  1,  1,  1 },  // ladder
    { 10,  1,  1,  1,  1,  1,  1 },  // ctf-flag
    {  1,  1,  1,  1,  1,  1,  1 },  // sound
    { 10,  5,  5,  5,  1, 10,  1 },  // clip
    { 10,  5,  5,  5,  1, 10,  1 },  // plclip
    {  1,  1,  1,  1,  1,  1,  1 }   // dummy
};

// see entity.h:61: struct itemstat { int add, start, max, sound; };
// Please update ./ac_website/htdocs/docs/introduction.html if these figures change.
itemstat ammostats[NUMGUNS] =
{
    {  1,  1,   1,  S_ITEMAMMO  },   // knife dummy
    { 20, 260, 100,  S_ITEMAMMO  },   // pistol
    { 15, 230,  30,  S_ITEMAMMO  },   // carbine
    { 14, 228,  21,  S_ITEMAMMO  },   // shotgun
    { 60, 590,  90,  S_ITEMAMMO  },   // subgun
    { 10, 220,  15,  S_ITEMAMMO  },   // sniper
    { 40, 560,  60,  S_ITEMAMMO  },   // assault
    {  1,  5,   5,  S_ITEMAMMO  },   // grenade
    {140,  0, 140,  S_ITEMAKIMBO},   // akimbo
    { 20, 260, 100,  S_ITEMAMMO  }    // flintlock
};

itemstat powerupstats[I_ARMOUR-I_HEALTH+1] =
{
    {100, 0, 100, S_ITEMHEALTH}, // 0 health - max health set to 1000
    {50, 0, 100, S_ITEMHELMET}, // 1 helmet
    {100, 0, 100, S_ITEMARMOUR}, // 2 armour
};

guninfo guns[NUMGUNS] =
{
    // Please update ./ac_website/htdocs/docs/introduction.html if these figures change.
    //mKR: mdl_kick_rot && mKB: mdl_kick_back
    //reI: recoilincrease && reB: recoilbase && reM: maxrecoil && reF: recoilbackfade
    //pFX: pushfactor
    //modelname                 sound                reloadtime        damage    projspeed  spread     magsize    mKB      reB          reF        isauto
    //             title                      reload       attackdelay      piercing     part     recoil       mKR     reI        reM        pFX
    { "knife",   "Knife",        S_KNIFE,   S_NULL,     0,      500,    90, 100,     0,   0,  1,    0,   0,    0,  0,   0,   0,    0,    0,   1,   true },
    { "pistol",  "Pistol",       S_PISTOL,  S_RPISTOL,  2020,   320,    25,   19,     0,   0,  0,   50,   14,   6,  5,   6,  35,   58,   125,  1,   false },
    { "carbine", "TMP-M&A CB",   S_CARBINE, S_RCARBINE, 2020,   650,    60,  50,     0,   0,  0,   0,   8,   4,  4,  10,  60,   60,   150,  2,   false },
    { "shotgun", "V-19 CS",      S_SHOTGUN, S_RSHOTGUN, 2560,  800,    0,    0,     0,   0,  0,   130,   6,   9,  9,  10, 140,  140,   125,  4,   false },   // CAUTION dmg only sane for server!
    { "subgun",  "A-ARD/10 SMG", S_SUBGUN,  S_RSUBGUN,  3000,   85,     12,   0,     0,   0,  0,   20,   40,   1,  2,   5,  25,   50,   188,  1,   true  },
    { "sniper",  "AD-81 SR",     S_SNIPER,  S_RSNIPER,  3320,   900,   86,  40,      0,   0,  45,   80,   5,   4,  4,  10,  85,   85,   100,  2,   false },
    { "assault", "MTP-57 AR",    S_ASSAULT, S_RASSAULT, 2560,   150,    23,   20,     0,   0,  0,   10,   30,   0,  2,   3,  25,   50,   115,  1,   true  },
    { "grenade", "Grenades",     S_NULL,    S_NULL,     1000,   650,    50,  0,    20,   6,  0,    0,   1,    3,  1,   0,   0,    0,    0,   5,   false },
    { "pistol",  "Akimbo",       S_PISTOL,  S_RAKIMBO,  1000,   65,     18,   0,     0,   0,  0,   20,   40,   6,  5,   4,  15,   25,   115,  1,   true  },
    { "flintlock",  "Flintlock",    S_PISTOL,  S_RPISTOL,  2020,   320,    90,   20,     0,   0,  0,   400,   1,    6,  5,   6,  35,   58,   125,  6,   false },
};

const char *gunnames[NUMGUNS + 1];


const char *teamnames[] = {"CLA", "RVSF", "CLA-SPECT", "RVSF-SPECT", "SPECTATOR", "", "void"};
const char *teamnames_s[] = {"CLA", "RVSF", "CSPC", "RSPC", "SPEC", "", "void"};

const char *rolenames[CR_NUM + 1] = { "unarmed", "master", "admin", "owner", "" };

const char *killmessages[2][NUMGUNS] =
{
    { "",        "dispatched", "jorked",     "peppered",   "killed",     "penetrated", "shredded",      "", "busted", "disassembled" },
    { "london'd", "executed",  "ejorkulated","splattered", "anihilated",   "headshot",  "domed","exploded", "busted all over", "jfk'd"}
};

#define C(x) (1<<(SC_##x))
soundcfgitem soundcfg[S_NULL] =
{ //  name                      desc                    vol, loop, rad, key,                 flags
    { "player/jump",            "Jump",                    80, 0,  0, S_JUMP,                   C(MOVEMENT)      }, // 0
    { "player/step",            "Soft landing",            90, 0, 24, S_SOFTLAND,               C(MOVEMENT)      }, // 1
    { "player/land",            "Hard landing",            95, 0, 24, S_HARDLAND,               C(MOVEMENT)      }, // 2
    { "weapon/ric1",            "Ricochet air 1",           0, 0,  0, S_BULLETAIR1,             C(BULLET)        }, // 3
    { "weapon/ric2",            "Ricochet air 2",           0, 0,  0, S_BULLETAIR2,             C(BULLET)        }, // 4
    { "weapon/ric3",            "Ricochet hit",             0, 0,  0, S_BULLETHIT,              C(BULLET)        }, // 5
    { "weapon/waterimpact",     "Bullet (water impact)",    0, 0,  0, S_BULLETWATERHIT,         C(BULLET)        }, // 6
    { "weapon/knife",           "Knife",                    0, 0,  0, S_KNIFE,                  C(WEAPON)        }, // 7
    { "weapon/usp",             "Pistol",                   0, 0,  0, S_PISTOL,                 C(WEAPON)        }, // 8
    { "weapon/pistol_reload",   "Pistol reloading",         0, 0,  0, S_RPISTOL,                C(WEAPON)        }, // 9
    { "weapon/carbine",         "Carbine",                  0, 0,  0, S_CARBINE,                C(WEAPON)        }, // 10
    { "weapon/carbine_reload",  "Carbine reloading",        0, 0,  0, S_RCARBINE,               C(WEAPON)        }, // 11
    { "weapon/shotgun",         "Shotgun",                  0, 0,  0, S_SHOTGUN,                C(WEAPON)        }, // 12
    { "weapon/shotgun_reload",  "Shotgun reloading",        0, 0,  0, S_RSHOTGUN,               C(WEAPON)        }, // 13
    { "weapon/sub",             "Submachine gun",           0, 0,  0, S_SUBGUN,                 C(WEAPON)        }, // 14
    { "weapon/sub_reload",      "Submachine gun reloading", 0, 0,  0, S_RSUBGUN,                C(WEAPON)        }, // 15
    { "weapon/sniper",          "Sniper",                   0, 0,  0, S_SNIPER,                 C(WEAPON)        }, // 16
    { "weapon/sniper_reload",   "Sniper reloading",         0, 0,  0, S_RSNIPER,                C(WEAPON)        }, // 17
    { "weapon/auto",            "Assault rifle",            0, 0,  0, S_ASSAULT,                C(WEAPON)        }, // 18
    { "weapon/auto_reload",     "Assault rifle reloading",  0, 0,  0, S_RASSAULT,               C(WEAPON)        }, // 19
    { "misc/pickup_ammo_clip",  "Ammo pickup",              0, 0,  0, S_ITEMAMMO,               C(PICKUP)        }, // 20
    { "misc/pickup_health",     "Health pickup",            0, 0,  0, S_ITEMHEALTH,             C(PICKUP)        }, // 21
    { "misc/pickup_armour",     "Armour pickup",            0, 0,  0, S_ITEMARMOUR,             C(PICKUP)        }, // 22
    { "misc/tick9",             "Akimbo pickup",            0, 0,  0, S_ITEMAKIMBO,             C(PICKUP)        }, // 23
    { "weapon/clip_empty",      "Empty clip",               0, 0,  0, S_NOAMMO,                 C(WEAPON)        }, // 24
    { "misc/tick11",            "Akimbo finished",          0, 0,  0, S_AKIMBOOUT,              C(PICKUP)        }, // 25
    { "player/pain1",           "Pain 1",                   0, 0,  0, S_PAIN1,                  C(PAIN)          }, // 26
    { "player/pain2",           "Pain 2",                   0, 0,  0, S_PAIN2,                  C(PAIN)          }, // 27
    { "player/pain3",           "Pain 3",                   0, 0,  0, S_PAIN3,                  C(PAIN)          }, // 28
    { "player/pain4",           "Pain 4",                   0, 0,  0, S_PAIN4,                  C(PAIN)          }, // 29
    { "player/pain5",           "Pain 5",                   0, 0,  0, S_PAIN5,                  C(PAIN)          }, // 30
    { "player/pain6",           "Pain 6",                   0, 0,  0, S_PAIN6,                  C(PAIN)          }, // 31
    { "player/die1",            "Die 1",                    0, 0,  0, S_DIE1,                   C(PAIN)          }, // 32
    { "player/die2",            "Die 2",                    0, 0,  0, S_DIE2,                   C(PAIN)          }, // 33
    { "weapon/grenade_exp",     "Grenade explosion",        0, 0,  0, S_FEXPLODE,               C(BULLET)        }, // 34
    { "player/splash1",         "Splash 1",                 0, 0,  0, S_SPLASH1,                C(MOVEMENT)      }, // 35
    { "player/splash2",         "Splash 2",                 0, 0,  0, S_SPLASH2,                C(MOVEMENT)      }, // 36
    { "ctf/flagdrop",           "Flag drop",                0, 0,  0, S_FLAGDROP,               C(OTHER)         }, // 37
    { "ctf/flagpickup",         "Flag pickup",              0, 0,  0, S_FLAGPICKUP,             C(OTHER)         }, // 38
    { "ctf/flagreturn",         "Flag return",              0, 0,  0, S_FLAGRETURN,             C(OTHER)         }, // 39
    { "ctf/flagscore",          "Flag score",               0, 0,  0, S_FLAGSCORE,              C(OTHER)         }, // 40
    { "weapon/grenade_pull",    "Grenade pull",             0, 0,  0, S_GRENADEPULL,            C(WEAPON)        }, // 41
    { "weapon/grenade_throw",   "Grenade throw",            0, 0,  0, S_GRENADETHROW,           C(WEAPON)        }, // 42
    { "weapon/grenade_bounce1", "Grenade bounce 1",         0, 0,  0, S_GRENADEBOUNCE1,         C(WEAPON)        }, // 43
    { "weapon/grenade_bounce2", "Grenade bounce 2",         0, 0,  0, S_GRENADEBOUNCE2,         C(WEAPON)        }, // 44
    { "weapon/pistol_akreload", "Akimbo reload",            0, 0,  0, S_RAKIMBO,                C(WEAPON)        }, // 45
    { "misc/change_gun",        "Change weapon",            0, 0,  0, S_GUNCHANGE,              C(WEAPON)        }, // 46
    { "misc/impact_t",          "Hit sound",                0, 0,  0, S_HITSOUND,               C(BULLET)        }, // 47
    { "player/gib",             "Gib sounds",               0, 0,  0, S_GIB,                    C(PAIN)          }, // 48
    { "misc/headshot",          "Headshot",                10, 0,  0, S_HEADSHOT,               C(OTHER)         }, // 49
    { "vote/vote_call",         "Call vote",                0, 0,  0, S_CALLVOTE,               C(OTHER)         }, // 50
    { "vote/vote_pass",         "Pass vote",                0, 0,  0, S_VOTEPASS,               C(OTHER)         }, // 51
    { "vote/vote_fail",         "Fail vote",                0, 0,  0, S_VOTEFAIL,               C(OTHER)         }, // 52
    { "player/footsteps",       "Footsteps",               40, 1, 20, S_FOOTSTEPS,              C(MOVEMENT)      }, // 53
    { "player/crouch",          "Crouch",                  40, 1, 10, S_FOOTSTEPSCROUCH,        C(MOVEMENT)      }, // 54
    { "player/watersteps",      "Water footsteps",         40, 1, 20, S_WATERFOOTSTEPS,         C(MOVEMENT)      }, // 55
    { "player/watercrouch",     "Water crouching",         40, 1, 10, S_WATERFOOTSTEPSCROUCH,   C(MOVEMENT)      }, // 56
    { "player/crouch_in",       "Crouch-in",               85, 0, 14, S_CROUCH,                 C(MOVEMENT)      }, // 57
    { "player/crouch_out",      "Crouch-out",              75, 0, 14, S_UNCROUCH,               C(MOVEMENT)      }, // 58
    { "misc/menu_click1",       "Menu select",              0, 0,  0, S_MENUSELECT,             C(OTHER)         }, // 59
    { "misc/menu_click2",       "Menu enter",               0, 0,  0, S_MENUENTER,              C(OTHER)         }, // 60
    { "player/underwater",      "Underwater",               0, 0,  0, S_UNDERWATER,             C(MOVEMENT)      }, // 61
    { "misc/tinnitus",          "Tinnitus",                 2, 0,  0, S_TINNITUS,               C(OWNPAIN)       }, // 62
    { "voicecom/affirmative",   "Affirmative",              0, 0,  0, S_AFFIRMATIVE,            C(VOICECOM)|C(TEAM)                                 }, // 63
    { "voicecom/allrightsir",   "All-right sir",            0, 0,  0, S_ALLRIGHTSIR,            C(VOICECOM)|C(TEAM)                                 }, // 64
    { "voicecom/comeonmove",    "Come on, move",            0, 0,  0, S_COMEONMOVE,             C(VOICECOM)|C(TEAM)          |C(FFA)                }, // 65
    { "voicecom/cominginwiththeflag", "Coming in with the flag", 0,0,0, S_COMINGINWITHTHEFLAG,  C(VOICECOM)|C(TEAM)                 |C(FLAGONLY)    }, // 66
    { "voicecom/coverme",       "Cover me",                 0, 0,  0, S_COVERME,                C(VOICECOM)|C(TEAM)                                 }, // 67
    { "voicecom/defendtheflag", "Defend the flag",          0, 0,  0, S_DEFENDTHEFLAG,          C(VOICECOM)|C(TEAM)                 |C(FLAGONLY)    }, // 68
    { "voicecom/enemydown",     "Enemy down",               0, 0,  0, S_ENEMYDOWN,              C(VOICECOM)|C(TEAM)          |C(FFA)                }, // 69
    { "voicecom/gogetemboys",   "Go get 'em boys!",         0, 0,  0, S_GOGETEMBOYS,            C(VOICECOM)|C(TEAM)                                 }, // 70
    { "voicecom/goodjobteam",   "Good job team",            0, 0,  0, S_GOODJOBTEAM,            C(VOICECOM)|C(TEAM)                                 }, // 71
    { "voicecom/igotone",       "I got one!",               0, 0,  0, S_IGOTONE,                C(VOICECOM)|C(TEAM)          |C(FFA)                }, // 72
    { "voicecom/imadecontact",  "I made contact",           0, 0,  0, S_IMADECONTACT,           C(VOICECOM)|C(TEAM)                                 }, // 73
    { "voicecom/imattacking",   "I'm attacking",            0, 0,  0, S_IMATTACKING,            C(VOICECOM)|C(TEAM)                                 }, // 74
    { "voicecom/imondefense",   "I'm on defense",           0, 0,  0, S_IMONDEFENSE,            C(VOICECOM)|C(TEAM)                                 }, // 75
    { "voicecom/imonyourteamman", "I'm on your team",       0, 0,  0, S_IMONYOURTEAMMAN,        C(VOICECOM)|C(TEAM)                                 }, // 76
    { "voicecom/negative",      "Negative",                 0, 0,  0, S_NEGATIVE,               C(VOICECOM)|C(TEAM)                                 }, // 77
    { "voicecom/nocando",       "No can do",                0, 0,  0, S_NOCANDO,                C(VOICECOM)|C(TEAM)                                 }, // 78
    { "voicecom/recovertheflag", "Recover the flag",        0, 0,  0, S_RECOVERTHEFLAG,         C(VOICECOM)|C(TEAM)                 |C(FLAGONLY)    }, // 79
    { "voicecom/sorry",         "Sorry",                    0, 0,  0, S_SORRY,                  C(VOICECOM)|C(TEAM)          |C(FFA)                }, // 80
    { "voicecom/spreadout",     "Spread out",               0, 0,  0, S_SPREADOUT,              C(VOICECOM)|C(TEAM)                                 }, // 81
    { "voicecom/stayhere",      "Stay here",                0, 0,  0, S_STAYHERE,               C(VOICECOM)|C(TEAM)                                 }, // 82
    { "voicecom/staytogether",  "Stay together",            0, 0,  0, S_STAYTOGETHER,           C(VOICECOM)|C(TEAM)                                 }, // 83
    { "voicecom/theresnowaysir", "There's no way sir",      0, 0,  0, S_THERESNOWAYSIR,         C(VOICECOM)|C(TEAM)                                 }, // 84
    { "voicecom/wedidit",       "We did it!",               0, 0,  0, S_WEDIDIT,                C(VOICECOM)|C(TEAM)                                 }, // 85
    { "voicecom/yes",           "Yes",                      0, 0,  0, S_YES,                    C(VOICECOM)|C(TEAM)                                 }, // 86
    { "voicecom/onthemove_1",   "Under way",                0, 0,  0, S_ONTHEMOVE1,             C(VOICECOM)|C(TEAM)                                 }, // 87
    { "voicecom/onthemove_2",   "On the move",              0, 0,  0, S_ONTHEMOVE2,             C(VOICECOM)|C(TEAM)                                 }, // 88
    { "voicecom/igotyourback",  "Got your back",            0, 0,  0, S_GOTURBACK,              C(VOICECOM)|C(TEAM)                                 }, // 89
    { "voicecom/igotyoucovered", "Got you covered",         0, 0,  0, S_GOTUCOVERED,            C(VOICECOM)|C(TEAM)                                 }, // 90
    { "voicecom/inposition_1",  "In position",              0, 0,  0, S_INPOSITION1,            C(VOICECOM)|C(TEAM)                                 }, // 91
    { "voicecom/inposition_2",  "In position now",          0, 0,  0, S_INPOSITION2,            C(VOICECOM)|C(TEAM)                                 }, // 92
    { "voicecom/reportin",      "Report in!",               0, 0,  0, S_REPORTIN,               C(VOICECOM)|C(TEAM)                                 }, // 93
    { "voicecom/niceshot",      "Nice shot",                0, 0,  0, S_NICESHOT,               C(VOICECOM)|C(TEAM)|C(PUBLIC)                       }, // 94
    { "voicecom/thanks_1",      "Thanks",                   0, 0,  0, S_THANKS1,                C(VOICECOM)|C(TEAM)|C(PUBLIC)                       }, // 95
    { "voicecom/thanks_2",      "Thanks, man",              0, 0,  0, S_THANKS2,                C(VOICECOM)|C(TEAM)|C(PUBLIC)                       }, // 96
    { "voicecom/awesome_1",     "Awesome (1)",              0, 0,  0, S_AWESOME1,               C(VOICECOM)|C(TEAM)|C(PUBLIC)                       }, // 97
    { "voicecom/awesome_2",     "Awesome (2)",              0, 0,  0, S_AWESOME2,               C(VOICECOM)|C(TEAM)|C(PUBLIC)                       }, // 98
    { "misc/pickup_helmet",     "Helmet pickup",            0, 0,  0, S_ITEMHELMET,             C(PICKUP)        }, // 99
    { "player/heartbeat",       "Heartbeat",                0, 0,  0, S_HEARTBEAT,              C(OWNPAIN)       }, // 100
    { "ktf/flagscore",          "KTF score",                0, 0,  0, S_KTFSCORE,               C(OTHER)         }, // 101
    { "misc/camera",            "Screenshot",               0, 0,  0, S_CAMERA,                 C(OTHER)         }  // 102
};
#undef C
