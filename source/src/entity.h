enum                            // static entity types
{
    NOTUSED = 0,                // entity slot not in use in map (usually seen at deleted entities)
    LIGHT,                      // lightsource, attr1 = radius, attr2 = intensity (or attr2..4 = r-g-b)
    PLAYERSTART,                // attr1 = angle, attr2 = team
    I_CLIPS, I_AMMO, I_GRENADE, // attr1 = elevation
    I_HEALTH, I_HELMET, I_ARMOUR, I_AKIMBO,
    MAPMODEL,                   // attr1 = angle, attr2 = idx, attr3 = elevation, attr4 = texture, attr5 = pitch, attr6 = roll
    CARROT,                     // attr1 = tag, attr2 = type
    LADDER,                     // attr1 = height
    CTF_FLAG,                   // attr1 = angle, attr2 = red/blue
    SOUND,                      // attr1 = idx, attr2 = radius, attr3 = size, attr4 = volume
    CLIP,                       // attr1 = elevation, attr2 = xradius, attr3 = yradius, attr4 = height, attr6 = slope, attr7 = shape
    PLCLIP,                     // attr1 = elevation, attr2 = xradius, attr3 = yradius, attr4 = height, attr6 = slope, attr7 = shape
    DUMMYENT,                   // temporary entity without any function - will not be saved to map files, used to mark positions and for scripting
    MAXENTTYPES
};

extern short entwraparound[MAXENTTYPES][7];
extern uchar entscale[MAXENTTYPES][7];
#define ENTSCALE10 10
#define ENTSCALE5 5
#define VANTAGEDUMMY2 222

enum {MAP_IS_BAD, MAP_IS_EDITABLE, MAP_IS_GOOD};

extern const char *entnames[];
#define isitem(i) ((i) >= I_CLIPS && (i) <= I_AKIMBO)

struct persistent_entity        // map entity
{
    short x, y, z;              // cube aligned position
    short attr1;
    uchar type;                 // type is one of the above
    uchar attr2, attr3, attr4;
    short attr5;
    char attr6;
    unsigned char attr7;
    persistent_entity(short x, short y, short z, uchar type, short attr1, uchar attr2, uchar attr3, uchar attr4, short attr5, char attr6, unsigned char attr7) : x(x), y(y), z(z), attr1(attr1), type(type), attr2(attr2), attr3(attr3), attr4(attr4), attr5(attr5), attr6(attr6), attr7(attr7) {}
    persistent_entity(short x, short y, short z, uchar type, short attr1, uchar attr2, uchar attr3, uchar attr4) : x(x), y(y), z(z), attr1(attr1), type(type), attr2(attr2), attr3(attr3), attr4(attr4), attr5(0), attr6(0), attr7(0) {}
    persistent_entity() {}
};

struct entity : persistent_entity
{
    bool spawned;               //the dynamic states of a map entity
    int lastmillis;
    entity(short x, short y, short z, uchar type, short attr1, uchar attr2, uchar attr3, uchar attr4, short attr5, char attr6, unsigned char attr7) : persistent_entity(x, y, z, type, attr1, attr2, attr3, attr4, attr5, attr6, attr7), spawned(false) {}
    entity(short x, short y, short z, uchar type, short attr1, uchar attr2, uchar attr3, uchar attr4) : persistent_entity(x, y, z, type, attr1, attr2, attr3, attr4), spawned(false) {}
    entity() {}
    bool fitsmode(int gamemode) { return !m_noitems && isitem(type) && !(m_noitemsnade && type!=I_GRENADE) && !(m_pistol && type==I_AMMO); }
    void transformtype(int gamemode)
    {
        if(m_noitemsnade && type==I_CLIPS) type = I_GRENADE;
        else if(m_pistol && ( type==I_AMMO || type==I_GRENADE )) type = I_CLIPS;
    }
};

enum { GUN_KNIFE = 0, GUN_PISTOL, GUN_CARBINE, GUN_SHOTGUN, GUN_SUBGUN, GUN_SNIPER, GUN_ASSAULT, GUN_GRENADE, GUN_AKIMBO, GUN_FLINTLOCK, GUN_HANDS, NUMGUNS };
#define valid_weapon(g) ((g) >= GUN_KNIFE && (g) < NUMGUNS)
#define reloadable_gun(g) ((g) != GUN_KNIFE && (g) != GUN_GRENADE && (g) != GUN_HANDS)

#define SGRAYS 21
#define SGDMGTOTAL 82

#define SGDMGBONUS 50
#define SGDMGDISTB 50

#define SGCCdmg 520
#define SGCCbase 0
#define SGCCrange 20

#define SGCMdmg 365
#define SGCMbase 15
#define SGCMrange 35

#define SGCOdmg 115
#define SGCObase 25
#define SGCOrange 45

#define SGMAXDMGABS 95
#define SGMAXDMGLOC 84
#define EXPDAMRAD 30

struct itemstat { int add, start, max, sound; };
extern itemstat ammostats[NUMGUNS];
extern itemstat powerupstats[I_ARMOUR-I_HEALTH+1];

struct guninfo { char modelname[23], title[42]; short sound, reload, reloadtime, attackdelay, damage, piercing, projspeed, part, spread, recoil, magsize, mdl_kick_rot, mdl_kick_back, recoilincrease, recoilbase, maxrecoil, recoilbackfade, pushfactor; bool isauto; };
extern guninfo guns[NUMGUNS];
extern const char *gunnames[];

static inline int reloadtime(int gun) { return guns[gun].reloadtime; }
static inline int attackdelay(int gun) { return guns[gun].attackdelay; }
static inline int magsize(int gun) { return guns[gun].magsize; }

/** roseta stone:
       0000,         0001,      0010,           0011,            0100,       0101,     0110 */
enum { TEAM_CLA = 0, TEAM_RVSF, TEAM_CLA_SPECT, TEAM_RVSF_SPECT, TEAM_SPECT, TEAM_NUM, TEAM_ANYACTIVE };
extern const char *teamnames[];
extern const char *teamnames_s[];
extern const char *killmessages[2][NUMGUNS];

#define isteam(a,b)   (m_teammode && (a) == (b))
#define team_opposite(o) (team_isvalid(o) && (o) < TEAM_SPECT ? (o) ^ 1 : TEAM_SPECT)
#define team_base(t) ((t) & 1)
#define team_basestring(t) ((t) == 1 ? teamnames[1] : ((t) == 0 ? teamnames[0] : "SPECT"))
#define team_isvalid(t) ((int(t)) >= 0 && (t) < TEAM_NUM)
#define team_isactive(t) ((t) == TEAM_CLA || (t) == TEAM_RVSF)
#define team_isspect(t) ((t) > TEAM_RVSF && (t) < TEAM_NUM)
#define team_group(t) ((t) == TEAM_SPECT ? TEAM_SPECT : team_base(t))
#define team_tospec(t) ((t) == TEAM_SPECT ? TEAM_SPECT : team_base(t) + TEAM_CLA_SPECT - TEAM_CLA)
// note: team_isactive and team_base can/should be used to check the limits for arrays of size '2'
static inline const char *team_string(int t, bool abbr = false) { const char **n = abbr ? teamnames_s : teamnames; return team_isvalid(t) ? n[t] : n[TEAM_NUM + 1]; }

enum { ENT_PLAYER = 0, ENT_BOT, ENT_CAMERA, ENT_BOUNCE };
enum { CS_ALIVE = 0, CS_DEAD, CS_SPAWNING, CS_LAGGED, CS_EDITING, CS_SPECTATE };
enum { CR_DEFAULT = 0, CR_MASTER, CR_ADMIN, CR_OWNER, CR_NUM };
enum { SM_NONE = 0, SM_DEATHCAM, SM_FOLLOW1ST, SM_FOLLOW3RD, SM_FOLLOW3RD_TRANSPARENT, SM_FLY, SM_OVERVIEW, SM_NUM };
enum { FPCN_VOID = -4, FPCN_DEATHCAM = -2, FPCN_FLY = -2, FPCN_OVERVIEW = -1 };

class physent
{
public:
    vec o, vel;                         // origin, velocity
    vec deltapos, newpos;                       // movement interpolation
    float yaw, pitch, roll;             // used as vec in one place
    float pitchvel;
    float maxspeed;                     // cubes per second, 24 for player
    int timeinair;                      // used for fake gravity
    float radius, eyeheight, maxeyeheight, aboveeye;  // bounding box size
    bool inwater;
    bool onfloor, onladder, jumpnext, jumpd, crouching, crouchedinair, trycrouch, sprinting, trysprinting, cancollide, stuck, scoping;
    int lastjump;
    float lastjumpheight;
    int lastsplash;
    char move, strafe;
    uchar state, type;
    float eyeheightvel;
    int last_pos;

    physent() : o(0, 0, 0), deltapos(0, 0, 0), newpos(0, 0, 0), yaw(270), pitch(0), roll(0), pitchvel(0),
            crouching(false), crouchedinair(false), trycrouch(false), sprinting(false), trysprinting(false), cancollide(true), stuck(false), scoping(false), lastjump(0), lastjumpheight(200), lastsplash(0), state(CS_ALIVE), last_pos(0)
    {
        reset();
    }
    virtual ~physent() {}

    void resetinterp()
    {
        newpos = o;
        newpos.z -= eyeheight;
        deltapos = vec(0, 0, 0);
    }

    void reset()
    {
        vel.x = vel.y = vel.z = eyeheightvel = 0.0f;
        move = strafe = 0;
        timeinair = lastjump = lastsplash = 0;
        onfloor = onladder = inwater = jumpnext = jumpd = crouching = crouchedinair = trycrouch = sprinting = trysprinting = stuck = false;
        last_pos = 0;
    }

    virtual void oncollision() {}
    virtual void onmoved(const vec &dist) {}
};

class dynent : public physent                 // animated ent
{
public:
    bool k_left, k_right, k_up, k_down;         // see input code

    animstate prev[2], current[2];              // md2's need only [0], md3's need both for the lower&upper model
    int lastanimswitchtime[2];
    void *lastmodel[2];
    int lastrendered;

    void stopmoving()
    {
        k_left = k_right = k_up = k_down = jumpnext = false;
        move = strafe = 0;
    }

    void resetanim()
    {
        loopi(2)
        {
            prev[i].reset();
            current[i].reset();
            lastanimswitchtime[i] = -1;
            lastmodel[i] = NULL;
        }
        lastrendered = 0;
    }

    void reset()
    {
        physent::reset();
        stopmoving();
    }

    dynent() { reset(); resetanim(); }
    virtual ~dynent() {}
};

#define MAXNAMELEN 15

class bounceent;

#define POSHIST_SIZE 7

struct poshist
{
    int nextupdate, curpos, numpos;
    vec pos[POSHIST_SIZE];

    poshist() : nextupdate(0) { reset(); }

    const int size() const { return numpos; }

    void reset()
    {
        curpos = 0;
        numpos = 0;
    }

    void addpos(const vec &o)
    {
        pos[curpos] = o;
        curpos++;
        if(curpos>=POSHIST_SIZE) curpos = 0;
        if(numpos<POSHIST_SIZE) numpos++;
    }

    const vec &getpos(int i) const
    {
        i = curpos-1-i;
        if(i < 0) i += POSHIST_SIZE;
        return pos[i];
    }

    void update(const vec &o, int lastmillis)
    {
        if(lastmillis<nextupdate) return;
        if(o.dist(pos[0]) >= 4.0f) addpos(o);
        nextupdate = lastmillis + 100;
    }
};

class playerstate
{
public:
    int health, armour;
    int primary, nextprimary;
    int secondary, nextsecondary;
    int gunselect;
    bool akimbo;
    int ammo[NUMGUNS], mag[NUMGUNS], gunwait[NUMGUNS];
    int pstatshots[NUMGUNS], pstatdamage[NUMGUNS];

    playerstate() : armour(0), primary(GUN_ASSAULT), nextprimary(GUN_ASSAULT), secondary(GUN_PISTOL), nextsecondary(GUN_PISTOL), akimbo(false) {}
    virtual ~playerstate() {}

    void resetstats() { loopi(NUMGUNS) pstatshots[i] = pstatdamage[i] = 0; }

    itemstat *itemstats(int type)
    {
        switch(type)
        {
            case I_CLIPS:   return &ammostats[GUN_SNIPER];
            case I_AMMO:    return &ammostats[primary];
            case I_GRENADE: return &ammostats[GUN_GRENADE];
            case I_AKIMBO:  return &ammostats[GUN_AKIMBO];
            case I_HEALTH:
            case I_HELMET:
            case I_ARMOUR:
                return &powerupstats[type - I_HEALTH];
            default:
                return NULL;
        }
    }

    bool canpickup(int type)
    {
        switch(type)
        {
            case I_CLIPS: return ammo[akimbo && ammo[GUN_AKIMBO]<ammostats[GUN_AKIMBO].max ? GUN_AKIMBO : GUN_SNIPER]<ammostats[akimbo && ammo[GUN_AKIMBO]<ammostats[GUN_AKIMBO].max ? GUN_AKIMBO : GUN_SNIPER].max;
            case I_AMMO: return ammo[primary]<ammostats[primary].max;
            case I_GRENADE: return mag[GUN_GRENADE]<ammostats[GUN_GRENADE].max;
            case I_HEALTH: return health<powerupstats[type-I_HEALTH].max;
            case I_HELMET:
            case I_ARMOUR: return armour<powerupstats[type-I_HEALTH].max;
            case I_AKIMBO: return !akimbo;
            default: return false;
        }
    }

    void additem(itemstat &is, int &v)
    {
        v += is.add;
        if(v > is.max) v = is.max;
    }

    void pickup(int type)
    {
        switch(type)
        {
            case I_CLIPS:
                additem(ammostats[GUN_SNIPER], ammo[GUN_SNIPER]);
                additem(ammostats[GUN_AKIMBO], ammo[GUN_AKIMBO]);
                break;
            case I_AMMO: additem(ammostats[primary], ammo[primary]); break;
            case I_GRENADE: additem(ammostats[GUN_GRENADE], mag[GUN_GRENADE]); break;
            case I_HEALTH: additem(powerupstats[type-I_HEALTH], health); break;
            case I_HELMET:
            case I_ARMOUR:
                additem(powerupstats[type-I_HEALTH], armour); break;
            case I_AKIMBO:
                akimbo = true;
                mag[GUN_AKIMBO] = guns[GUN_AKIMBO].magsize;
                additem(ammostats[GUN_AKIMBO], ammo[GUN_AKIMBO]);
                break;
        }
    }

    void respawn()
    {
        health = 100;
        armour = 25;
        gunselect = GUN_SNIPER;
        akimbo = false;
        loopi(NUMGUNS) ammo[i] = mag[i] = gunwait[i] = 0;
        ammo[GUN_KNIFE] = mag[GUN_KNIFE] = 1;
    }

    virtual void spawnstate(int gamemode)
    {
        if(m_pistol) primary = GUN_PISTOL;
        else if(m_osok) primary = GUN_SNIPER;
        else if(m_lss) primary = GUN_KNIFE;
        else primary = nextprimary;

        // Set secondary weapon based on game mode
        if(m_lss) secondary = GUN_KNIFE;
        else if(m_osok) secondary = GUN_KNIFE; // OSOK only has sniper + knife
        else secondary = nextsecondary;

        if(!m_nopistol)
        {
            ammo[GUN_PISTOL] = ammostats[GUN_PISTOL].start-magsize(GUN_PISTOL);
            mag[GUN_PISTOL] = magsize(GUN_PISTOL);
        }

        if(!m_noprimary)
        {
            ammo[primary] = ammostats[primary].start-magsize(primary);
            mag[primary] = magsize(primary);
        }

        // Give ammo to secondary weapon (if different from primary)
        if(secondary != primary && secondary != GUN_KNIFE)
        {
            if(secondary == GUN_PISTOL && !m_nopistol)
            {
                // Pistol ammo already set above
            }
            else if(!m_noprimary)
            {
                ammo[secondary] = ammostats[secondary].start-magsize(secondary);
                mag[secondary] = magsize(secondary);
            }
        }

        gunselect = primary;

        // Give grenades based on ammostats (like other weapons)
        //ammo[GUN_GRENADE] = ammostats[GUN_GRENADE].start - magsize(GUN_GRENADE);
        //mag[GUN_GRENADE] = magsize(GUN_GRENADE);

        if(m_osok) health = 1;
        if(m_lms) // Survivor && Team-Survivor : 2010nov19
        {
            health = 100;
            armour = 100;
            // LMS mode uses same grenade count as other modes
        }
        
        // Bot Slow TDM: limit all magazines to 1 bullet, enable grenades
        if(m_botslowtdm)
        {
            loopi(NUMGUNS) if(mag[i] > 1) mag[i] = 1;
            ammo[GUN_GRENADE] = 2;
        }
    }

    // just subtract damage here, can set death, etc. later in code calling this
    int dodamage(int damage, int gun)
    {
        guninfo gi = guns[gun];
        if(damage == INT_MAX)
        {
            damage = health;
            armour = health = 0;
            return damage;
        }

        // 4-level armour - tiered approach: 16%, 33%, 37%, 41%
        // Please update ./ac_website/htdocs/docs/introduction.html if this changes.
        int armoursection = 0;
        int ad = damage;
        if(armour > 25) armoursection = 1;
        if(armour > 50) armoursection = 2;
        if(armour > 75) armoursection = 3;
        switch(armoursection)
        {
            case 0: ad = (int) (16.0f/25.0f * armour); break;             // 16
            case 1: ad = (int) (17.0f/25.0f * armour) - 1; break;         // 33
            case 2: ad = (int) (4.0f/25.0f * armour) + 25; break;         // 37
            case 3: ad = (int) (4.0f/25.0f * armour) + 25; break;         // 41
            default: break;
        }

        //ra - reduced armor
        //rd - reduced damage
        int ra = (int) (ad * damage/100.0f);
        int rd = ra-(ra*(gi.piercing/100.0f)); //Who cares about rounding errors anyways?

        armour -= ra;
        damage -= rd;

        health -= damage;
        return damage;
    }
};

#ifndef STANDALONE
#define HEADSIZE 0.4f

#define ROLLMOVMAX 20
#define ROLLMOVDEF 0
#define ROLLEFFMAX 30
#define ROLLEFFDEF 0

class playerent : public dynent, public playerstate
{
private:
    int curskin, nextskin[2];
public:
    int clientnum, lastupdate, plag, ping;
    enet_uint32 address;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int frags, flagscore, deaths, tks;
    int lastaction, lastmove, lastpain, lastvoicecom, lastdeath;
    int clientrole;
    bool attacking;
    string name;
    int team;
    int weaponchanging;
    int nextweapon; // weapon we switch to
    int spectatemode, followplayercn;
    int eardamagemillis;
    float maxroll, maxrolleffect, movroll, effroll;  // roll added by movement and damage
    int ffov, scopefov;
    bool allowmove() { return (state!=CS_DEAD && state!=CS_SPECTATE) || spectatemode==SM_FLY; }

    weapon *weapons[NUMGUNS];
    weapon *prevweaponsel, *weaponsel, *nextweaponsel, *primweap, *nextprimweap, *secweap, *nextsecweap, *lastattackweapon;
    weapon *weapon_before_sprint; // weapon to re-equip after sprint ends
    weapon *pending_weapon_switch; // weapon to switch to after sprint ends

    poshist history; // Previous stored locations of this player

    const char *skin_noteam, *skin_cla, *skin_rvsf;

    float deltayaw, deltapitch, newyaw, newpitch;
    int smoothmillis;

    vec head;

    bool ignored, muted;
    bool nocorpse;

    playerent() : curskin(0), clientnum(-1), lastupdate(0), plag(0), ping(0), address(0), lifesequence(0), frags(0), flagscore(0), deaths(0), tks(0), lastpain(0), lastvoicecom(0), lastdeath(0), clientrole(CR_DEFAULT),
                  team(TEAM_SPECT), spectatemode(SM_NONE), followplayercn(FPCN_VOID), eardamagemillis(0), maxroll(ROLLMOVDEF), maxrolleffect(ROLLEFFDEF), movroll(0), effroll(0), ffov(0), scopefov(0),
                  prevweaponsel(NULL), weaponsel(NULL), nextweaponsel(NULL), primweap(NULL), nextprimweap(NULL), secweap(NULL), nextsecweap(NULL), lastattackweapon(NULL), weapon_before_sprint(NULL), pending_weapon_switch(NULL),
                  smoothmillis(-1),
                  head(-1, -1, -1), ignored(false), muted(false), nocorpse(false)
    {
        type = ENT_PLAYER;
        name[0] = 0;
        maxeyeheight = 4.5f;
        aboveeye = 0.7f;
        radius = 1.1f;
        maxspeed = 16.0f;
        skin_noteam = skin_cla = skin_rvsf = NULL;
        loopi(2) nextskin[i] = 0;
        respawn();
    }

    virtual ~playerent()
    {
        extern void removebounceents(playerent *owner);
        extern void removedynlights(physent *owner);
        extern void zapplayerflags(playerent *owner);
        extern void cleanplayervotes(playerent *owner);
        extern physent *camera1;
        extern void spectatemode(int mode);
        removebounceents(this);
        audiomgr.detachsounds(this);
        removedynlights(this);
        zapplayerflags(this);
        cleanplayervotes(this);
        if(this==camera1) spectatemode(SM_FOLLOW1ST);
    }

    void damageroll(float damage)
    {
        damage *= effroll > 0.001f || (effroll > -0.001f && rnd(2)) ? 2.0f : -2.0f; // give player a kick
        effroll = clamp(effroll + damage, -maxrolleffect, maxrolleffect);
    }

    void hitpush(int damage, const vec &dir, playerent *actor, int gun)
    {
        if(!valid_weapon(gun)) return;
        vec push(dir);
        push.mul(damage/250.0f*guns[gun].pushfactor + guns[gun].pushfactor/2.1f);
        push.z += (damage/250.0f*guns[gun].pushfactor + guns[gun].pushfactor/2.9f); // add some vertical push
        vel.add(push);
        extern int lastmillis;
        if(gun==GUN_GRENADE && damage > 50) eardamagemillis = lastmillis+damage*100;
    }

    void resetspec()
    {
        spectatemode = SM_NONE;
        followplayercn = FPCN_VOID;
    }

    void respawn()
    {
        dynent::reset();
        playerstate::respawn();
        history.reset();
        if(weaponsel) weaponsel->reset();
        lastaction = 0;
        lastattackweapon = NULL;
        attacking = false;
        extern int lastmillis;
        weaponchanging = lastmillis - weapons[gunselect]->weaponchangetime/2; // 2011jan16:ft: for a little no-shoot after spawn
        resetspec();
        eardamagemillis = 0;
        eyeheight = maxeyeheight;
        curskin = nextskin[team_base(team)];
        nocorpse = false;
        extern void *damageindicatorplayer;
        if(damageindicatorplayer == this)
        {
            extern int damagedirections[8];
            loopi(8) damagedirections[i] = lastmillis;
        }
    }

    void spawnstate(int gamemode)
    {
        playerstate::spawnstate(gamemode);
        prevweaponsel = weaponsel = weapons[gunselect];
        primweap = weapons[primary];
        nextprimweap = weapons[nextprimary];
        secweap = weapons[secondary];
        nextsecweap = weapons[nextsecondary];
        curskin = nextskin[team_base(team)];
    }

    void selectweapon(int w, bool other = false)
    {
        if(!valid_weapon(w)) return;
        if(weaponsel) prevweaponsel = weaponsel;
        weaponsel = weapons[(gunselect = w)];
        if(!prevweaponsel) prevweaponsel = weaponsel;
        if(other && (prevweaponsel != weaponsel)) weaponswitch(weaponsel, true);
    }

    void setprimary(int w) { if(!valid_weapon(w)) return; primweap = weapons[(primary = w)]; }
    void setnextprimary(int w) { nextprimweap = weapons[(nextprimary = w)]; }
    void setsecondary(int w) { if(!valid_weapon(w)) return; secweap = weapons[(secondary = w)]; }
    void setnextsecondary(int w) { nextsecweap = weapons[(nextsecondary = w)]; }
    bool isspectating() { return state==CS_SPECTATE || (state==CS_DEAD && spectatemode > SM_NONE); }
    void weaponswitch(weapon *w, bool sound = true)
    {
        if(!w) return;
        extern int lastmillis;
        // Cancel reload on current weapon when switching
        if(weaponsel->reloading) 
        {
            weaponsel->reloading = 0;
            // Restore original mag count if reload was in progress
            if(weaponsel->mag_before_reload >= 0)
            {
                int bullets_to_restore = weaponsel->mag - weaponsel->mag_before_reload;
                weaponsel->mag = weaponsel->mag_before_reload;
                weaponsel->ammo += bullets_to_restore;
                weaponsel->mag_before_reload = -1;
            }
        }
        weaponsel->ondeselecting();
        weaponchanging = lastmillis;
        nextweaponsel = w;
        w->onselecting(sound);
    }
    int skin(int t = -1) { return nextskin[team_base(t < 0 ? team : t)]; }
    void setskin(int t, int s)
    {
        const int maxskin[2] = { 4, 6 };
        t = team_base(t < 0 ? team : t);
        nextskin[t] = (s & 63) % maxskin[t];
    }
    void startmap()
    {
        frags = flagscore = deaths = lifesequence = tks = 0;
    }
};



class CBot;

class botent : public playerent
{
public:
    // Added by Rick
    CBot *pBot; // Only used if this is a bot, points to the bot class if we are the host,
                // for other clients its NULL
    // End add by Rick

    playerent *enemy;                      // monster wants to kill this entity
    // Added by Rick: targetpitch
    float targetpitch;                    // monster wants to look in this direction
    // End add
    float targetyaw;                    // monster wants to look in this direction

    botent() : pBot(NULL), enemy(NULL) { type = ENT_BOT; }
    ~botent() { }

    int deaths() { return lifesequence; }
};
#endif //#ifndef STANDALONE

// flag-mode entities

enum { CTFF_INBASE = 0, CTFF_STOLEN, CTFF_DROPPED, CTFF_IDLE };

struct flaginfo
{
    int team;
    entity *flagent;
    int actor_cn;
    playerent *actor;
    vec pos;
    int state; // one of CTFF_*
    bool ack;
    flaginfo() : flagent(0), actor(0), state(CTFF_INBASE), ack(false) {}
};

// nades, gibs

enum { BT_NONE, BT_NADE, BT_GIB };

class bounceent : public physent
{
public:
    int millis, timetolive, bouncetype; // see enum above
    float rotspeed;
    bool plclipped;
    playerent *owner;

    bounceent() : bouncetype(BT_NONE), rotspeed(1.0f), plclipped(false), owner(NULL)
    {
        type = ENT_BOUNCE;
        maxspeed = 40;
        radius = 0.2f;
        eyeheight = maxeyeheight = 0.3f;
        aboveeye = 0.0f;
    }

    virtual ~bounceent() {}

    bool isalive(int lastmillis) { return lastmillis - millis < timetolive; }
    virtual void destroy() {}
    virtual bool applyphysics() { return true; }
};

struct hitmsg
{
    int target, lifesequence, info;
    ivec dir;
};

class grenadeent : public bounceent
{
public:
    bool local;
    int nadestate;
    float distsincebounce;
    grenadeent(playerent *owner, int millis = 0);
    ~grenadeent();
    void activate(const vec &from, const vec &to);
    void _throw(const vec &from, const vec &vel);
    void explode();
    void splash();
    virtual void destroy();
    virtual bool applyphysics();
    void moveoutsidebbox(const vec &direction, playerent *boundingbox);
    void oncollision();
    void onmoved(const vec &dist);
};

