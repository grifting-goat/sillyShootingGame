// renderhud.cpp: HUD rendering

#include "cube.h"

void drawicon(Texture *tex, float x, float y, float s, int col, int row, float ts)
{
    if(tex && tex->xs == tex->ys) quad(tex->id, x, y, s, ts*col, ts*row, ts);
}

inline void turn_on_transparency(int alpha = 255)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(255, 255, 255, alpha);
}

void drawequipicon(float x, float y, int col, int row)
{
    static Texture *tex = NULL;
    if(!tex) tex = textureload("packages/misc/items.png", 3);
    if(tex)
    {
        turn_on_transparency();
        drawicon(tex, x, y, 120, col, row, 1/4.0f);
        glDisable(GL_BLEND);
    }
}

VARP(radarentsize, 4, 12, 64);

void drawradaricon(float x, float y, float s, int col, int row)
{
    static Texture *tex = NULL;
    if(!tex) tex = textureload("packages/misc/radaricons.png", 3);
    if(tex)
    {
        glEnable(GL_BLEND);
        drawicon(tex, x, y, s, col, row, 1/4.0f);
        glDisable(GL_BLEND);
    }
}

void drawctficon(float x, float y, float s, int col, int row, float ts, int alpha)
{
    static Texture *ctftex = NULL, *htftex = NULL, *ktftex = NULL;
    if(!ctftex) ctftex = textureload("packages/misc/ctficons.png", 3);
    if(!htftex) htftex = textureload("packages/misc/htficons.png", 3);
    if(!ktftex) ktftex = textureload("packages/misc/ktficons.png", 3);
    glColor4ub(255, 255, 255, alpha);
    if(m_htf)
    {
        if(htftex) drawicon(htftex, x, y, s, col, row, ts);
    }
    else if(m_ktf)
    {
        if(ktftex) drawicon(ktftex, x, y, s, col, row, ts);
    }
    else
    {
        if(ctftex) drawicon(ctftex, x, y, s, col, row, ts);
    }
}

VARP(votealpha, 0, 255, 255);
void drawvoteicon(float x, float y, int col, int row, bool noblend)
{
    static Texture *tex = NULL;
    if(!tex) tex = textureload("packages/misc/voteicons.png", 3);
    if(tex)
    {
        if(noblend) glDisable(GL_BLEND);
        else turn_on_transparency(votealpha); // if(transparency && !noblend)
        drawicon(tex, x, y, 240, col, row, 1/2.0f);
        if(noblend) glEnable(GL_BLEND);
    }
}

VARP(crosshairsize, 0, 15, 50);
VARP(showstats, 0, 1, 2);
VARP(crosshairfx, 0, 1, 3);
VARP(crosshairteamsign, 0, 1, 1);

// Headshot crosshair effect tracking
extern int lastheadshot;
VARP(headshotcrosshairfx, 0, 1, 1); // Enable/disable headshot crosshair effects
VARP(hideradar, 0, 0, 1);
VARP(hidecompass, 0, 0, 1);
VARP(hideteam, 0, 0, 1);
VARP(hideteamscorehud, 0, 0, 1);
VARP(flagscorehudtransparency, 0, 2, 2);
VARP(hideeditinfopanel, 0, 0, 1);
VARP(hidevote, 0, 0, 2);
VARP(hidehudmsgs, 0, 0, 1);
VARP(hidehudequipment, 0, 0, 1);
VARP(hideconsole, 0, 0, 1);
VARP(hidespecthud, 0, 0, 1);
VAR(showmap, 0, 0, 1);
VARP(editinfopanelmillis, 5, 80, 2000);

void drawscope(bool preload)
{
    static Texture *scopetex = NULL;
    if(!scopetex) scopetex = textureload("packages/misc/scope.png", 3);
    if(preload) return;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, scopetex->id);
    glColor3ub(255, 255, 255);

    // figure out the bounds of the scope given the desired aspect ratio
    int scopecenterfix = 2; // center the 512x512 scope image
    float sz = min(VIRTW, VIRTH),
          x1 = VIRTW/2 - sz/2 + scopecenterfix,
          x2 = VIRTW/2 + sz/2,
          y1 = VIRTH/2 - sz/2,
          y2 = VIRTH/2 + sz/2,
          border = (512 - 64*2)/512.0f;

    // draw center viewport
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f, 0.5f);
    glVertex2f(x1 + 0.5f*sz, y1 + 0.5f*sz);
    loopi(8+1)
    {
        float c = 0.5f*(1 + border*cosf(i*PI2/8.0f)), s = 0.5f*(1 + border*sinf(i*PI2/8.0f));
        glTexCoord2f(c, s);
        glVertex2f(x1 + c*sz, y1 + s*sz);
    }
    glEnd();

    glDisable(GL_BLEND);

    // draw outer scope
    glBegin(GL_TRIANGLE_STRIP);
    loopi(8+1)
    {
        float c = 0.5f*(1 + border*cosf(i*PI2/8.0f)), s = 0.5f*(1 + border*sinf(i*PI2/8.0f));
        glTexCoord2f(c, s);
        glVertex2f(x1 + c*sz, y1 + s*sz);
        c = c < 0.4f ? 0 : (c > 0.6f ? 1 : 0.5f);
        s = s < 0.4f ? 0 : (s > 0.6f ? 1 : 0.5f);
        glTexCoord2f(c, s);
        glVertex2f(x1 + c*sz, y1 + s*sz);
    }
    glEnd();

    // fill unused space with border texels
    if(x1 > 0 || x2 < VIRTW || y1 > 0 || y2 < VIRTH)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2f(0,  0);
        glTexCoord2f(0, 0); glVertex2f(x1, y1);
        glTexCoord2f(0, 1); glVertex2f(0,  VIRTH);
        glTexCoord2f(0, 1); glVertex2f(x1, y2);

        glTexCoord2f(1, 1); glVertex2f(VIRTW, VIRTH);
        glTexCoord2f(1, 1); glVertex2f(x2, y2);
        glTexCoord2f(1, 0); glVertex2f(VIRTW, 0);
        glTexCoord2f(1, 0); glVertex2f(x2, y1);

        glTexCoord2f(0, 0); glVertex2f(0,  0);
        glTexCoord2f(0, 0); glVertex2f(x1, y1);
        glEnd();
    }

    glEnable(GL_BLEND);
}

const char *crosshairnames[CROSSHAIR_NUM + 1];  // filled in main.cpp
Texture *crosshairs[CROSSHAIR_NUM] = { NULL }; // weapon specific crosshairs

Texture *loadcrosshairtexture(const char *c)
{
    defformatstring(p)("packages/crosshairs/%s", behindpath(c));
    Texture *crosshair = textureload(p, 3);
    if(crosshair==notexture) crosshair = textureload("packages/crosshairs/default.png", 3);
    return crosshair;
}

void loadcrosshair(const char *type, const char *filename)
{
    int index = CROSSHAIR_DEFAULT;
    if(!*filename)
    {   // special form: loadcrosshair filename   // short for "loadcrosshair default filename"
        if(strcasecmp(type, "reset")) filename = type;
        else
        { // "loadcrosshair reset" does exactly that
            filename = "default.png";
            crosshairs[CROSSHAIR_TEAMMATE] = loadcrosshairtexture("teammate.png");
            crosshairs[CROSSHAIR_SCOPE] = loadcrosshairtexture("red_dot.png");
        }
    }
    else if(strchr(type, '.'))
    {   // old syntax "loadcrosshair filename type", remove this in 2020
        const char *oldcrosshairnames[CROSSHAIR_NUM + 1] = { "default", "teammate", "scope", "knife", "pistol", "carbine", "shotgun", "smg", "sniper", "ar", "grenades", "akimbo", "" };
        index = getlistindex(filename, oldcrosshairnames, false, 0);
        if(index > 2) index -= 3;
        else index += NUMGUNS;
        filename = type;
    }
    else
    {   // new syntax with proper gun names
        index = getlistindex(type, crosshairnames, false, CROSSHAIR_DEFAULT);
    }
    if(index == CROSSHAIR_DEFAULT)
    {
        loopi(CROSSHAIR_NUM) if(i != CROSSHAIR_TEAMMATE && i != CROSSHAIR_SCOPE) crosshairs[i] = loadcrosshairtexture(filename);
    }
    else
    {
        crosshairs[index] = loadcrosshairtexture(filename);
    }
}

COMMAND(loadcrosshair, "ss");

// Function to trigger headshot crosshair effect
void triggerheadshoteffect()
{
    lastheadshot = lastmillis;
}

// Test command for headshot effect
void testheadshoteffect()
{
    triggerheadshoteffect();
}

// Test command for non-lethal headshot effect  
void testnonlethalheadshot()
{
    extern int lastheadshotflash;
    lastheadshotflash = lastmillis;
}

// Test command for kill effect
void testkilleffect()
{
    extern int lastkillflash;
    lastkillflash = lastmillis;
}

COMMAND(testheadshoteffect, "");
COMMAND(testnonlethalheadshot, "");
COMMAND(testkilleffect, "");

void drawcrosshair(playerent *p, int n, color *c, float size)
{
    if (cleanedit && editmode) return;
    Texture *crosshair = crosshairs[n];
    if(!crosshair)
    {
        crosshair = crosshairs[CROSSHAIR_DEFAULT];
        if(!crosshair) crosshair = crosshairs[CROSSHAIR_DEFAULT] = loadcrosshairtexture("default.png");
    }

    if(crosshair->bpp==32) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    else glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, crosshair->id);
    glColor3ub(255,255,255);
    
    extern int lastdamageflash;
    extern int lastdamageamount;
    extern int lastheadshotflash;
    extern int lastkillflash;
    float rotation = 0.0f;
    float sizeMult = 1.0f;
    bool isHeadshotEffect = false;
    bool isNonLethalHeadshotEffect = false;
    bool isKillEffect = false;
    
    // Check for headshot effect first (takes priority)
    if(headshotcrosshairfx && lastmillis - lastheadshot < 800)
    {
        float progress = (lastmillis - lastheadshot) / 800.0f; // 0.0 to 1.0 over 800ms
        float easedProgress = 1.0f - (progress * progress); // Ease-out effect
        bool isFullAuto = p && p->weaponsel && guns[p->weaponsel->type].isauto;
        
        isHeadshotEffect = true;
        
        if(isFullAuto)
        {
            // Full-auto weapons get enhanced spinning effect
            rotation = easedProgress * 1080.0f; // Three full rotations for full-auto
            
            // More dramatic pulsing for full-auto
            float pulsePhase = sin(progress * 12.0f * PI) * easedProgress; // More pulses
            sizeMult = 1.0f + (1.2f * easedProgress) + (0.5f * pulsePhase); // Bigger effect
        }
        else
        {
            // Standard effect for semi-auto weapons
            rotation = easedProgress * 720.0f; // Two full rotations that slow down
            
            // Standard pulsing size effect - starts big, shrinks to normal
            float pulsePhase = sin(progress * 8.0f * PI) * easedProgress; // Multiple pulses
            sizeMult = 1.0f + (0.8f * easedProgress) + (0.3f * pulsePhase);
        }
        
        // Color will be set below to gold/red
    }
    // Check for non-lethal headshot effect (takes priority over regular damage flash)
    else if(headshotcrosshairfx && lastmillis - lastheadshotflash < 300)
    {
        float progress = (lastmillis - lastheadshotflash) / 300.0f; // 0.0 to 1.0 over 300ms
        float easedProgress = 1.0f - (progress * progress); // Ease-out effect
        
        isNonLethalHeadshotEffect = true;
        
        // Moderate spinning effect for non-lethal headshots
        rotation = easedProgress * 180.0f; // Half rotation
        
        // Subtle pulsing effect
        float pulsePhase = sin(progress * 4.0f * PI) * easedProgress;
        sizeMult = 1.0f + (0.3f * easedProgress) + (0.1f * pulsePhase);
    }
    // Check for regular kill effect (red version of headshot kill animation)
    else if(headshotcrosshairfx && lastmillis - lastkillflash < 800)
    {
        float progress = (lastmillis - lastkillflash) / 800.0f; // 0.0 to 1.0 over 800ms
        float easedProgress = 1.0f - (progress * progress); // Ease-out effect
        bool isFullAuto = p && p->weaponsel && guns[p->weaponsel->type].isauto;
        
        isKillEffect = true;
        
        if(isFullAuto)
        {
            // Full-auto weapons get enhanced spinning effect (same as headshot kills)
            rotation = easedProgress * 1080.0f; // Three full rotations for full-auto
            
            // More dramatic pulsing for full-auto
            float pulsePhase = sin(progress * 12.0f * PI) * easedProgress; // More pulses
            sizeMult = 1.0f + (1.2f * easedProgress) + (0.5f * pulsePhase); // Bigger effect
        }
        else
        {
            // Standard effect for semi-auto weapons (same as headshot kills)
            rotation = easedProgress * 720.0f; // Two full rotations that slow down
            
            // Standard pulsing size effect - starts big, shrinks to normal
            float pulsePhase = sin(progress * 8.0f * PI) * easedProgress; // Multiple pulses
            sizeMult = 1.0f + (0.8f * easedProgress) + (0.3f * pulsePhase);
        }
    }
    // Check if we should apply damage effects - only for automatic weapons and not during any effect
    else if(lastmillis - lastdamageflash < 200 && p && p->weaponsel && guns[p->weaponsel->type].isauto && 
            !isHeadshotEffect && !isNonLethalHeadshotEffect && !isKillEffect)
    {
        float progress = (lastmillis - lastdamageflash) / 200.0f; // 0.0 to 1.0
        rotation = 0; // (1.0f - progress) * 360.0f; // Full rotation that slows down
        
        // Scale size based on damage: 10 damage = 1.1x, 50 damage = 1.5x, 100+ damage = 2.0x
        float damageScale = 1.0f + (lastdamageamount / 100.0f); // 1.0 to 2.0 range
        if(damageScale > 2.0f) damageScale = 2.0f; // Cap at 2x size
        sizeMult = 1.0f + (1.0f - progress) * (damageScale - 1.0f); // Shrinks back to normal
    }
    sizeMult = (isHeadshotEffect || isNonLethalHeadshotEffect || isKillEffect) ? sizeMult : 1.0f; //hardcode back to 1.0f for now except effects
    
    // Set colors - SIMPLIFIED VERSION
    if(isKillEffect)
    {
        // Regular kill: simple red color
        glColor3f(1.0f, 0.0f, 0.0f); // Bright red
    }
    else if(isNonLethalHeadshotEffect)
    {
        // Non-lethal headshot: simple gold color
        glColor3f(1.0f, 0.8f, 0.0f); // Gold
    }
    else if(isHeadshotEffect)
    {
        // Lethal headshot: simple gold color
        glColor3f(1.0f, 0.8f, 0.0f); // Gold
    }
    else if(c) glColor3f(c->r, c->g, c->b);
    else if(crosshairfx==1 || crosshairfx==2 || n==CROSSHAIR_TEAMMATE)
    {
        /*if(n==CROSSHAIR_TEAMMATE) glColor3ub(255, 0, 0);
        else if(!m_osok)
        {
            //if(p->health<=25) glColor3ub(255,0,0);
            //else if(p->health<=50) glColor3ub(255,128,0);
        }*/
    }
    else
    {
        // Default white color when no effects are active
        glColor3ub(255, 255, 255);
    }
    float s = size>0 ? size : (float)crosshairsize;
    float chsize = s * sizeMult * ((p == player1 && p->weaponsel && p->weaponsel->type==GUN_ASSAULT && p->weaponsel->shots > 30) && (crosshairfx==1 || crosshairfx==3) ? 1.4f : 1.0f);
    
    // Apply rotation if needed
    if(rotation != 0.0f)
    {
        glPushMatrix();
        glTranslatef(VIRTW/2, VIRTH/2, 0);
        glRotatef(rotation, 0, 0, 1);
        glTranslatef(-VIRTW/2, -VIRTH/2, 0);
    }
    
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2f(VIRTW/2 - chsize, VIRTH/2 - chsize);
    glTexCoord2f(1, 0); glVertex2f(VIRTW/2 + chsize, VIRTH/2 - chsize);
    glTexCoord2f(0, 1); glVertex2f(VIRTW/2 - chsize, VIRTH/2 + chsize);
    glTexCoord2f(1, 1); glVertex2f(VIRTW/2 + chsize, VIRTH/2 + chsize);
    glEnd();
    
    if(rotation != 0.0f)
    {
        glPopMatrix();
    }
}

VARP(hidedamageindicator, 0, 0, 1);
VARP(damageindicatorsize, 0, 200, 10000);
VARP(damageindicatordist, 0, 500, 10000);
VARP(damageindicatortime, 1, 1000, 10000);
VARP(damageindicatoralpha, 1, 50, 100);
int damagedirections[8] = {0};
void *damageindicatorplayer = NULL;

void updatedmgindicator(playerent *p, vec &attack)
{
    if(hidedamageindicator || !damageindicatorsize) return;
    vec base_d = p->o;
    base_d.sub(attack);
    damagedirections[(int(742.5f - p->yaw - base_d.anglexy()) / 45) & 0x7] = lastmillis + damageindicatortime;
    damageindicatorplayer = p;
}

void drawdmgindicator(playerent *p)
{
    if(!damageindicatorsize) return;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    float size = (float)damageindicatorsize;
    loopi(8)
    {
        if(!damagedirections[i] || damagedirections[i] < lastmillis) continue;
        float t = damageindicatorsize/(float)(damagedirections[i]-lastmillis);
        glPushMatrix();
        glColor4f(0.5f, 0.0f, 0.0f, damageindicatoralpha/100.0f);
        glTranslatef(VIRTW/2, VIRTH/2, 0);
        glRotatef(i*45, 0, 0, 1);
        glTranslatef(0, (float)-damageindicatordist, 0);
        glScalef(max(0.0f, 1.0f-t), max(0.0f, 1.0f-t), 0);

        glBegin(GL_TRIANGLES);
        glVertex3f(size/2.0f, size/2.0f, 0.0f);
        glVertex3f(-size/2.0f, size/2.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glEnd();
        glPopMatrix();
    }
    glEnable(GL_TEXTURE_2D);
}

VARP(hidektfindicator, 0, 0, 1);
VARP(ktfindicatoralpha, 1, 70, 100);

void drawktfindicator(playerent *p)
{
    if(hidektfindicator || !m_ktf) return;
    vec flagpos(-1.0f, -1.0f, 0.0f);
    loopi(2)
    {
        flaginfo &f = flaginfos[i];
        if(f.state == CTFF_INBASE) flagpos = f.pos;
        else if(f.state == CTFF_STOLEN && f.actor && f.actor != p) flagpos = f.actor->o;
    }
    if(flagpos.x > 0 && flagpos.y > 0)
    {
        flagpos.sub(p->o);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glPushMatrix();
        glColor4f(0.7f, 0.7f, 0.0f, ktfindicatoralpha / 100.0f);
        glTranslatef(VIRTW/2, VIRTH/2, 0);
        glRotatef(180.0f - p->yaw - flagpos.anglexy(), 0, 0, 1);
        glTranslatef(0, -200.0f, 0); // dist
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(20.0f, 50.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 40.0f, 0.0f);
        glVertex3f(-20.0f, 50.0f, 0.0f);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }
}

extern int oldfashionedgunstats;

int getprevweaponsel(playerent *p) // get previous weapon or, if we don't know it, get the primary
{
    return p->prevweaponsel->type == GUN_ASSAULT && p->primweap->type != GUN_ASSAULT ? p->primweap->type : p->prevweaponsel->type;
}

void drawequipicons(playerent *p)
{
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 0.2f+(sinf(lastmillis/100.0f)+1.0f)/2.0f);

    // health & armor
    if(p->armour) drawequipicon(HUDPOS_ARMOUR*2, 1650, (p->armour-1)/25, 2);
    drawequipicon(HUDPOS_HEALTH*2, 1650, 2, 3);
    if(p->mag[GUN_GRENADE]) drawequipicon(oldfashionedgunstats ? (HUDPOS_GRENADE + (((float)screenw / (float)screenh > 1.5f) ? 75 : 25)) * 2 : HUDPOS_GRENADE*2, 1650, 3, 1);

    // weapons
    int c = p->weaponsel->type != GUN_GRENADE ? p->weaponsel->type : getprevweaponsel(p), r = 0;
    if(c==GUN_AKIMBO) c = GUN_PISTOL; // same icon for akimb & pistol
    if(c>3) { c -= 4; r = 1; }

    if(p->weaponsel && valid_weapon(p->weaponsel->type)) drawequipicon(HUDPOS_WEAPON*2, 1650, c, r);
    glEnable(GL_BLEND);
}

void drawradarent(float x, float y, float yaw, int col, int row, float iconsize, bool pulse, const char *label = NULL, ...) PRINTFARGS(8, 9);

void drawradarent(float x, float y, float yaw, int col, int row, float iconsize, bool pulse, const char *label, ...)
{
    glPushMatrix();
    if(pulse) glColor4f(1.0f, 1.0f, 1.0f, 0.2f+(sinf(lastmillis/30.0f)+1.0f)/2.0f);
    else glColor4f(1, 1, 1, 1);
    glTranslatef(x, y, 0);
    glRotatef(yaw, 0, 0, 1);
    drawradaricon(-iconsize/2.0f, -iconsize/2.0f, iconsize, col, row);
    glPopMatrix();
    if(label && showmap)
    {
        glPushMatrix();
        glEnable(GL_BLEND);
        glTranslatef(iconsize/2, iconsize/2, 0);
        glScalef(1/2.0f, 1/2.0f, 1/2.0f);
        defvformatstring(lbl, label, label);
        draw_text(lbl, (int)(x*2), (int)(y*2));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);
        glPopMatrix();
    }
}

struct hudline : cline
{
    int type;

    hudline() : type(HUDMSG_INFO) {}
};

struct hudmessages : consolebuffer<hudline>
{
    hudmessages() : consolebuffer<hudline>(20) {}

    void addline(const char *sf)
    {
        if(conlines.length() && conlines[0].type&HUDMSG_OVERWRITE)
        {
            conlines[0].millis = totalmillis;
            conlines[0].type = HUDMSG_INFO;
            copystring(conlines[0].line, sf);
        }
        else consolebuffer<hudline>::addline(sf, totalmillis);
    }
    void editline(int type, const char *sf)
    {
        if(conlines.length() && ((conlines[0].type&HUDMSG_TYPE)==(type&HUDMSG_TYPE) || conlines[0].type&HUDMSG_OVERWRITE))
        {
            conlines[0].millis = totalmillis;
            conlines[0].type = type;
            copystring(conlines[0].line, sf);
        }
        else consolebuffer<hudline>::addline(sf, totalmillis).type = type;
    }
    void render()
    {
        if(!conlines.length()) return;
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, VIRTW*0.9f, VIRTH*0.9f, 0, -1, 1);
        int dispmillis = arenaintermission ? 6000 : 3000;
        loopi(min(conlines.length(), 3)) if(totalmillis-conlines[i].millis<dispmillis)
        {
            cline &c = conlines[i];
            int tw = text_width(c.line);
            draw_text(c.line, int(tw > VIRTW*0.9f ? 0 : (VIRTW*0.9f-tw)/2), int(((VIRTH*0.9f)/4*3)+FONTH*i+pow((totalmillis-c.millis)/(float)dispmillis, 4)*VIRTH*0.9f/4.0f));
        }
        glPopMatrix();
    }
};

hudmessages hudmsgs;

void hudoutf(const char *s, ...)
{
    defvformatstring(sf, s, s);
    hudmsgs.addline(sf);
    conoutf("%s", sf);
}

void hudonlyf(const char *s, ...)
{
    defvformatstring(sf, s, s);
    hudmsgs.addline(sf);
}

void hudeditf(int type, const char *s, ...)
{
    defvformatstring(sf, s, s);
    hudmsgs.editline(type, sf);
}

bool insideradar(const vec &centerpos, float radius, const vec &o)
{
    if(showmap) return !o.reject(centerpos, radius);
    return o.distxy(centerpos)<=radius;
}

bool isattacking(playerent *p) { return lastmillis-p->lastaction < 500; }

vec getradarpos()
{
    float radarviewsize = VIRTH/6;
    float overlaysize = radarviewsize*4.0f/3.25f;
    return vec(VIRTW-10-VIRTH/28-overlaysize, 10+VIRTH/52, 0);
}

VARP(showmapbackdrop, 0, 0, 2);
VARP(showmapbackdroptransparency, 0, 75, 100);
VARP(radarheight, 5, 150, 500);

void drawradar_showmap(playerent *p, int w, int h)
{
    float minimapviewsize = 3*min(VIRTW,VIRTH)/4; //minimap default size
    float halfviewsize = minimapviewsize/2.0f;
    float iconsize = radarentsize/0.2f;
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    bool spect3rd = p->spectatemode > SM_FOLLOW1ST && p->spectatemode <= SM_FOLLOW3RD_TRANSPARENT;
    playerent *d = spect3rd ? players[p->followplayercn] : p;
    int p_baseteam = p->team == TEAM_SPECT && spect3rd ? team_base(players[p->followplayercn]->team) : team_base(p->team);
    extern GLuint minimaptex;
    vec centerpos(VIRTW/2 , VIRTH/2, 0.0f);
    if(showmapbackdrop)
    {
        glDisable(GL_TEXTURE_2D);
        if(showmapbackdrop==2) glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_COLOR);
        loopi(2)
        {
            int cg = i?(showmapbackdrop==2?((int)(255*(100-showmapbackdroptransparency)/100.0f)):0):(showmapbackdrop==2?((int)(255*(100-showmapbackdroptransparency)/100.0f)):64);
            int co = i?0:4;
            glColor3ub(cg, cg, cg);
            glBegin(GL_QUADS);
            glVertex2f( centerpos.x - halfviewsize - co, centerpos.y + halfviewsize + co);
            glVertex2f( centerpos.x + halfviewsize + co, centerpos.y + halfviewsize + co);
            glVertex2f( centerpos.x + halfviewsize + co, centerpos.y - halfviewsize - co);
            glVertex2f( centerpos.x - halfviewsize - co, centerpos.y - halfviewsize - co);
            glEnd();
        }
        glColor3ub(255,255,255);
        glEnable(GL_TEXTURE_2D);
    }
    glTranslatef(centerpos.x - halfviewsize, centerpos.y - halfviewsize , 0);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    quad(minimaptex, 0, 0, minimapviewsize, 0.0f, 0.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    float gdim = max(clmapdims.xspan, clmapdims.yspan); //no border
    float offd = fabs((clmapdims.yspan - clmapdims.xspan) / 2.0f);
    if(!gdim) { gdim = ssize/2.0f; offd = 0; }
    float coordtrans = minimapviewsize / gdim;
    float offx = gdim == clmapdims.yspan ? offd : 0;
    float offy = gdim == clmapdims.xspan ? offd : 0;

    vec mdd = vec(clmapdims.x1 - offx, clmapdims.y1 - offy, 0);
    vec ppv = vec(p->o).sub(mdd).mul(coordtrans);

    if(!(p->isspectating() && spect3rd)) drawradarent(ppv.x, ppv.y, p->yaw, (p->state==CS_ALIVE || p->state==CS_EDITING) ? (isattacking(p) ? 2 : 0) : 1, 2, iconsize, isattacking(p), "%s", colorname(p)); // local player
    loopv(players) // other players
    {
        playerent *pl = players[i];
        if(!pl || pl == p || !team_isactive(pl->team)) continue;
        if(OUTBORD(pl->o.x, pl->o.y)) continue;
        int pl_baseteam = team_base(pl->team);
        if(p->team < TEAM_SPECT && ((m_teammode && !isteam(p_baseteam, pl_baseteam)) || (!m_teammode && !(spect3rd && d == pl)))) continue;
        if(p->team == TEAM_SPECT && !(spect3rd && (isteam(p_baseteam, pl_baseteam) || d == pl))) continue;
        vec rtmp = vec(pl->o).sub(mdd).mul(coordtrans);
        drawradarent(rtmp.x, rtmp.y, pl->yaw, pl->state==CS_ALIVE ? (isattacking(pl) ? 2 : 0) : 1, spect3rd && d == pl ? 2 : pl_baseteam, iconsize, isattacking(pl), "%s", colorname(pl));
    }
    if(m_flags_)
    {
        glColor4f(1.0f, 1.0f, 1.0f, (sinf(lastmillis / 100.0f) + 1.0f) / 2.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        loopi(2) // flag items
        {
            flaginfo &f = flaginfos[i];
            entity *e = f.flagent;
            bool hasflagent = e && e->x != -1 && e->y != -1; // no base for flagentitydummies (HTF on maps without bases)
            if(hasflagent)
            {
                vec pos = vec(e->x, e->y, 0).sub(mdd).mul(coordtrans);
                drawradarent(pos.x, pos.y, 0, m_ktf ? 2 : f.team, 3, iconsize, false); // draw bases
            }
            if((f.state == CTFF_INBASE && hasflagent) || f.state == CTFF_DROPPED)
            {
                vec fltxoff = vec(8, -8, 0);
                vec cpos = vec(f.pos.x, f.pos.y, f.pos.z).sub(mdd).mul(coordtrans).add(fltxoff);
                float flgoff=fabs((radarentsize*2.1f)-8);
                drawradarent(cpos.x+flgoff, cpos.y-flgoff, 0, 3, m_ktf ? 2 : f.team, iconsize, false); // draw on entity pos or whereever dropped
            }
            if(f.state == CTFF_STOLEN)
            {
                if(m_teammode && !m_ktf && player1->team == TEAM_SPECT && (p->spectatemode == SM_DEATHCAM || p->spectatemode > SM_FOLLOW3RD_TRANSPARENT)) continue;
                float d2c = 1.6f * radarentsize/16.0f;
                vec apos(d2c, -d2c, 0);
                if(f.actor)
                {
                    apos.add(f.actor->o);
                    bool tm = i != p_baseteam;
                    if(m_htf) tm = !tm;
                    else if(m_ktf) tm = true;
                    if(tm)
                    {
                        apos.sub(mdd).mul(coordtrans);
                        drawradarent(apos.x, apos.y, 0, 3, m_ktf ? 2 : f.team, iconsize, true); // draw near flag thief
                    }
                }
            }
        }
    }
    glEnable(GL_BLEND);
    glPopMatrix();
}

void drawradar_vicinity(playerent *p, int w, int h)
{
    bool spect3rd = p->spectatemode > SM_FOLLOW1ST && p->spectatemode <= SM_FOLLOW3RD_TRANSPARENT;
    playerent *d = spect3rd ? players[p->followplayercn] : p;
    int p_baseteam = p->team == TEAM_SPECT && spect3rd ? team_base(players[p->followplayercn]->team) : team_base(p->team);
    extern GLuint minimaptex;
    int gdim = max(clmapdims.xspan, clmapdims.yspan);
    float radarviewsize = min(VIRTW,VIRTH)/5;
    float halfviewsize = radarviewsize/2.0f;
    float iconsize = radarentsize/0.4f;
    float scaleh = radarheight/(2.0f*gdim);
    float scaled = radarviewsize/float(radarheight);
    float offd = fabs((clmapdims.yspan - clmapdims.xspan) / 2.0f);
    if(gdim < 1) { gdim = ssize/2; offd = 0; }
    float offx = gdim == clmapdims.yspan ? offd : 0;
    float offy = gdim == clmapdims.xspan ? offd : 0;
    vec rtr = vec(clmapdims.x1 - offx, clmapdims.y1 - offy, 0);
    vec rsd = vec(clmapdims.xm, clmapdims.ym, 0);
    float d2s = radarheight * radarheight / 4.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    vec centerpos(VIRTW-halfviewsize-72, halfviewsize+64, 0);
    glTranslatef(centerpos.x, centerpos.y, 0);
    glRotatef(-camera1->yaw, 0, 0, 1);
    glTranslatef(-halfviewsize, -halfviewsize, 0);
    vec d4rc = vec(d->o).sub(rsd).normalize().mul(0);
    vec usecenter = vec(d->o).sub(rtr).sub(d4rc);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    circle(minimaptex, halfviewsize, halfviewsize, halfviewsize, usecenter.x/(float)gdim, usecenter.y/(float)gdim, scaleh, 31); //Draw mimimaptext as radar background
    glTranslatef(halfviewsize, halfviewsize, 0);

    if(!(p->isspectating() && spect3rd)) drawradarent(0, 0, p->yaw, (p->state==CS_ALIVE || p->state==CS_EDITING) ? (isattacking(p) ? 2 : 0) : 1, 2, iconsize, isattacking(p), "%s", colorname(p)); // local player
    loopv(players) // other players
    {
        playerent *pl = players[i];
        if(!pl || pl == p || !team_isactive(pl->team)) continue;
        if(OUTBORD(pl->o.x, pl->o.y)) continue;
        int pl_baseteam = team_base(pl->team);
        if(p->team < TEAM_SPECT && ((m_teammode && !isteam(p_baseteam, pl_baseteam)) || (!m_teammode && !(spect3rd && d == pl)))) continue;
        if(p->team == TEAM_SPECT && !(spect3rd && (isteam(p_baseteam, pl_baseteam) || d == pl))) continue;
        vec rtmp = vec(pl->o).sub(d->o);
        bool isok = rtmp.sqrxy() < d2s;
        if(isok)
        {
            rtmp.mul(scaled);
            drawradarent(rtmp.x, rtmp.y, pl->yaw, pl->state==CS_ALIVE ? (isattacking(pl) ? 2 : 0) : 1, spect3rd && d == pl ? 2 : pl_baseteam, iconsize, isattacking(pl), "%s", colorname(pl));
        }
    }
    if(m_flags_)
    {
        glColor4f(1.0f, 1.0f, 1.0f, (sinf(lastmillis / 100.0f) + 1.0f) / 2.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        float d2c = 1.6f * radarentsize/16.0f;
        loopi(2) // flag items
        {
            flaginfo &f = flaginfos[i];
            entity *e = f.flagent;
            bool hasflagent = e && e->x != -1 && e->y != -1; // no base for flagentitydummies (HTF on maps without bases)
            if(hasflagent)
            {
                vec pos = vec(e->x, e->y, 0).sub(d->o);
                if(pos.sqrxy() < d2s)
                {
                    pos.mul(scaled);
                    drawradarent(pos.x, pos.y, 0, m_ktf ? 2 : f.team, 3, iconsize, false); // draw bases [circle doesn't need rotating]
                }
            }
            if((f.state == CTFF_INBASE && hasflagent) || f.state == CTFF_DROPPED)
            {
                vec cpos = vec(f.pos.x, f.pos.y, f.pos.z).sub(d->o);
                if(cpos.sqrxy() < d2s)
                {
                    cpos.mul(scaled);
                    float flgoff=radarentsize/0.68f;
                    float ryaw=(camera1->yaw-45)*RAD;
                    float offx=flgoff*cosf(-ryaw);
                    float offy=flgoff*sinf(-ryaw);
                    drawradarent(cpos.x+offx, cpos.y-offy, camera1->yaw, 3, m_ktf ? 2 : f.team, iconsize, false); // draw flag on entity pos or whereever dropped
                }
            }
            if(f.state == CTFF_STOLEN)
            {
                if(m_teammode && !m_ktf && player1->team == TEAM_SPECT && (p->spectatemode == SM_DEATHCAM || p->spectatemode > SM_FOLLOW3RD_TRANSPARENT)) continue;
                vec apos(d2c, -d2c, 0);
                if(f.actor)
                {
                    apos.add(f.actor->o);
                    bool tm = i != p_baseteam;
                    if(m_htf) tm = !tm;
                    else if(m_ktf) tm = true;
                    if(tm)
                    {
                        apos.sub(d->o);
                        if(apos.sqrxy() < d2s)
                        {
                            apos.mul(scaled);
                            drawradarent(apos.x, apos.y, camera1->yaw, 3, m_ktf ? 2 : f.team, iconsize, true); // draw near flag thief
                        }
                    }
                }
            }
        }
    }
    glEnable(GL_BLEND);
    glPopMatrix();
    // eye candy:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1, 1, 1);
    static Texture *bordertex = NULL;
    if(!bordertex) bordertex = textureload("packages/misc/compass-base.png", 3);
    quad(bordertex->id, centerpos.x-halfviewsize-16, centerpos.y-halfviewsize-16, radarviewsize+32, 0, 0, 1, 1);
    if(!hidecompass)
    {
        static Texture *compasstex = NULL;
        if(!compasstex) compasstex = textureload("packages/misc/compass-rose.png", 3);
        glPushMatrix();
        glTranslatef(centerpos.x, centerpos.y, 0);
        glRotatef(-camera1->yaw, 0, 0, 1);
        quad(compasstex->id, -halfviewsize-8, -halfviewsize-8, radarviewsize+16, 0, 0, 1, 1);
        glPopMatrix();
    }
}

void drawradar(playerent *p, int w, int h)
{
    if(showmap) drawradar_showmap(p,w,h);
    else drawradar_vicinity(p,w,h);
}

void drawteamicons(int w, int h, bool spect)
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1, 1, 1);
    static Texture *icons = NULL;
    if(!icons) icons = textureload("packages/misc/teamicons.png", 3);
    if(player1->team < TEAM_SPECT || spect) quad(icons->id, VIRTW-VIRTH/12-10, 10, VIRTH/12, team_base(spect ? players[player1->followplayercn]->team : player1->team) ? 0.5f : 0, 0, 0.49f, 1.0f);
}

int damageblendmillis = 0;
void *damageblendplayer = NULL;

VARFP(damagescreen, 0, 1, 1, { if(!damagescreen) damageblendmillis = 0; });
VARP(damagescreenfactor, 1, 7, 100);
VARP(damagescreenalpha, 1, 45, 100);
VARP(damagescreenfade, 0, 125, 1000);

void damageblend(int n, void *p)
{
    if(!damagescreen) return;
    if(lastmillis > damageblendmillis || damageblendplayer != p) damageblendmillis = lastmillis;
    damageblendmillis = min(damageblendmillis + n * damagescreenfactor, lastmillis + 3999);
    damageblendplayer = p;
}

inline char rangecolor(int val, const char *colors, int thres1, int thres2, int thres3)
{
    if(val < thres1) return colors[0];
    else if(val < thres2) return colors[1];
    else if(val < thres3) return colors[2];
    return colors[3];
}

string enginestateinfo = "";
COMMANDF(getEngineState, "", () { result(enginestateinfo); });

VARP(gametimedisplay,0,1,2);
VARP(dbgpos,0,0,1);
VARP(showtargetname,0,1,1);
VARP(showspeed, 0, 0, 1);
VAR(blankouthud, 0, 0, 10000); //for "clean" screenshot
VARP(overviewflags, 0, 1, 2); // SM_OVERVIEW flag rendering: 0:regular, 1:model-askew 2:radarents
string gtime;
int dimeditinfopanel = 255;


const char *ghoststrings[] =
{
  "none", "deathcam", "chase 1st", "chase 3rd [O]", "chase 3rd [A]", "fly", "overview", ""
};

void gl_drawhud(int w, int h, int curfps, int nquads, int curvert, bool underwater, int elapsed)
{
    if(blankouthud > 0) { blankouthud -= elapsed; return; }
    else blankouthud = 0;
    playerent *p = camera1->type<ENT_CAMERA ? (playerent *)camera1 : player1;
    bool spectating = player1->isspectating();
    bool is_spect = (player1->spectatemode >= SM_FOLLOW1ST && player1->spectatemode <= SM_FOLLOW3RD_TRANSPARENT &&
        players.inrange(player1->followplayercn) && players[player1->followplayercn]);

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, VIRTW, VIRTH, 0, -1, 1);
    glEnable(GL_BLEND);

    if(underwater)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4ub(hdr.watercolor[0], hdr.watercolor[1], hdr.watercolor[2], 102);

        glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(0, 0);
        glVertex2f(VIRTW, 0);
        glVertex2f(0, VIRTH);
        glVertex2f(VIRTW, VIRTH);
        glEnd();
    }

    if(lastmillis < damageblendmillis && damageblendplayer == p)
    {
        static Texture *damagetex = NULL;
        if(!damagetex) damagetex = textureload("packages/misc/damage.png", 3);

        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, damagetex->id);
        float fade = damagescreenalpha/100.0f;
        if(damageblendmillis - lastmillis < damagescreenfade)
            fade *= float(damageblendmillis - lastmillis)/damagescreenfade;
        glColor4f(fade, fade, fade, fade);

        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(1, 0); glVertex2f(VIRTW, 0);
        glTexCoord2f(0, 1); glVertex2f(0, VIRTH);
        glTexCoord2f(1, 1); glVertex2f(VIRTW, VIRTH);
        glEnd();
    }

    glEnable(GL_TEXTURE_2D);

    playerent *targetplayer = playerincrosshair();
    bool menu = menuvisible();
    bool command = getcurcommand(NULL) ? true : false;
    bool reloading = lastmillis < p->weaponsel->reloading + p->weaponsel->info.reloadtime;
    if(p->state==CS_ALIVE || p->state==CS_EDITING)
    {
        bool drawteamwarning = crosshairteamsign && targetplayer && isteam(targetplayer->team, p->team) && targetplayer->state==CS_ALIVE;
        if(!reloading) p->weaponsel->renderaimhelp(drawteamwarning);
        if(!editmode && !showmap) drawktfindicator(p);
    }

    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // without below condition it was always set in drawdmgindicator()
    if(damageindicatorplayer == p || (is_spect && getclient(player1->followplayercn) == damageindicatorplayer)) drawdmgindicator(p);

    if(p->state==CS_ALIVE && !hidehudequipment) drawequipicons(p);

    if(!editmode)
    {
        if((!hideradar || showmap) && player1->spectatemode <= SM_FLY) drawradar(p, w, h); // EDITMINIMAP: radar/showmap in editmode will be out-of-date
        glMatrixMode(GL_MODELVIEW);
        if(!hideteam && m_teammode) drawteamicons(w, h, is_spect);
        glMatrixMode(GL_PROJECTION);
    }

    char *infostr = editinfo();
    int commandh = HUDPOS_Y_BOTTOMLEFT + FONTH;
    if(command) commandh -= rendercommand(-1, HUDPOS_Y_BOTTOMLEFT, VIRTW - FONTH); // dryrun to get height
    else if(infostr)
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, VIRTW * 2, VIRTH * 2, 0, -1, 1);
        glScalef(1.0, 1.0, 1.0); //set scale
        draw_text(infostr, 48, VIRTH * 2 - 3 * FONTH);
        glPopMatrix();
    }
    else if(targetplayer && showtargetname) 
    {
        defformatstring(tpinfo)("%s [cn:%d]", colorname(targetplayer), targetplayer->clientnum);
        draw_text(tpinfo, HUDPOS_X_BOTTOMLEFT, HUDPOS_Y_BOTTOMLEFT);
    }
    glLoadIdentity();
    glOrtho(0, VIRTW*2, VIRTH*2, 0, -1, 1);
    extern void r_accuracy(int h);
    extern void *scoremenu;
    extern gmenu *curmenu;
    if(!is_spect && !editmode && !watchingdemo && !command && curmenu == scoremenu) r_accuracy(commandh);
    if(!hideconsole) renderconsole();
    formatstring(enginestateinfo)("%d %d %d %d %d", curfps, lod_factor(), nquads, curvert, xtraverts);

    string ltime, text;
    const char *ltimeformat = getalias("wallclockformat");
    bool wallclock = ltimeformat && *ltimeformat;
    //wallclockformat beginning with "U" shows UTC/GMT time
    if(wallclock) filtertext(ltime, timestring(*ltimeformat != 'U', ltimeformat + int(*ltimeformat == 'U')), FTXT_TOLOWER);

    if(showstats)
    {
        if(showstats==2 && !dbgpos)
        {
            int left = (VIRTW-225-10)*2, top = (VIRTH*7/8)*2, ttll = VIRTW*2 - 3*FONTH/2, lf = lod_factor();
            blendbox(left - 24, top - 24, VIRTW*2 - 72, VIRTH*2 - 48, true, -1);

            draw_text("fps", left - (text_width("fps") + FONTH/2), top);
            draw_text("lod", left - (text_width("lod") + FONTH/2), top + 80);
            draw_text("wqd", left - (text_width("wqd") + FONTH/2), top + 160);
            draw_text("wvt", left - (text_width("wvt") + FONTH/2), top + 240);
            draw_text("evt", left - (text_width("evt") + FONTH/2), top + 320);

            formatstring(text)("\f%c%d", rangecolor(curfps, "xwvu", 30, 100, 150), curfps);             draw_text(text, ttll - text_width(text), top);
            formatstring(text)("\f%c%d", rangecolor(lf, "uvwx", 199, 299, 399), lf);                    draw_text(text, ttll - text_width(text), top + 80);
            formatstring(text)("\f%c%d", rangecolor(nquads, "uvwx", 3999, 5999, 7999), nquads);         draw_text(text, ttll - text_width(text), top + 160);
            formatstring(text)("\f%c%d", rangecolor(curvert, "uvwx", 3999, 5999, 7999), curvert);       draw_text(text, ttll - text_width(text), top + 240);
            formatstring(text)("\f%c%d", rangecolor(xtraverts, "uvwx", 3999, 5999, 7999), xtraverts);   draw_text(text, ttll - text_width(text), top + 320);

            if(wallclock) draw_text(ltime, ttll - text_width(ltime), top - 90);
            if(unsavededits) draw_text("U", ttll - text_width("U"), top - 90 - (wallclock ? 2*FONTH/2 : 0));
        }
        else
        {
            if(dbgpos)
            {
                pushfont("mono");
                formatstring(text)("%05.2f YAW", camera1->yaw);     draw_text(text, VIRTW*2 - ( text_width(text) + FONTH ), VIRTH*2 - 17*FONTH/2);
                formatstring(text)("%05.2f PIT", camera1->pitch);   draw_text(text, VIRTW*2 - ( text_width(text) + FONTH ), VIRTH*2 - 15*FONTH/2);
                formatstring(text)("%05.2f X  ", camera1->o.x);     draw_text(text, VIRTW*2 - ( text_width(text) + FONTH ), VIRTH*2 - 13*FONTH/2);
                formatstring(text)("%05.2f Y  ", camera1->o.y);     draw_text(text, VIRTW*2 - ( text_width(text) + FONTH ), VIRTH*2 - 11*FONTH/2);
                formatstring(text)("%05.2f Z  ", camera1->o.z);     draw_text(text, VIRTW*2 - ( text_width(text) + FONTH ), VIRTH*2 - 9*FONTH/2);
                popfont();
            }
            defformatstring(c_val)("fps %d", curfps);         draw_text(c_val, VIRTW*2 - ( text_width(c_val) + FONTH ), VIRTH*2 - 3*FONTH/2);

            if(wallclock) draw_text(ltime, VIRTW*2 - text_width(ltime) - FONTH, VIRTH*2 - 5*FONTH/2);
            if(unsavededits) draw_text("U", VIRTW*2 - text_width("U") - FONTH, VIRTH*2 - (wallclock ? 7 : 5)*FONTH/2);
        }
    }
    else if(wallclock) draw_text(ltime, VIRTW*2 - text_width(ltime) - FONTH, VIRTH*2 - 3*FONTH/2);

    if(editmode && !hideeditinfopanel)
    {
        static int lasteditip = 0;
        static char *editip = NULL;
        if(!lasteditip || totalmillis - lasteditip > editinfopanelmillis)
        { // update edit info panel
            lasteditip = totalmillis;
            const char *editipfunc = getalias("updateeditinfopanel");
            DELSTRING(editip);
            editip = executeret(editipfunc);
        }
        if(editip && dimeditinfopanel)
        {
            int w, h;
            text_bounds(editip, w, h, -1);
            draw_text(editip, VIRTW*2 - w - FONTH, VIRTH*13/8 - h, 255, 255, 255, dimeditinfopanel);
        }
        dimeditinfopanel += elapsed;
        if(dimeditinfopanel > 255) dimeditinfopanel = 255;
    }

    if(!intermission && lastgametimeupdate!=0)
    {
        int cssec = (gametimecurrent+(lastmillis-lastgametimeupdate))/1000;
        int gtsec = cssec%60;
        int gtmin = cssec/60;
        if(gametimedisplay == 1)
        {
            int gtmax = gametimemaximum/60000;
            gtmin = gtmax - gtmin;
            if(gtsec!=0)
            {
                gtmin -= 1;
                gtsec = 60 - gtsec;
            }
        }
        formatstring(gtime)("%02d:%02d", gtmin, gtsec);
        if(gametimedisplay) draw_text(gtime, (VIRTW-225-10)*2 - (text_width(gtime)/2 + FONTH/2), 20);
    }

    if(hidevote < 2 && multiplayer(NULL))
    {
        extern votedisplayinfo *curvote;

        if(curvote && curvote->millis >= totalmillis && !(hidevote == 1 && curvote->localplayervoted && curvote->result == VOTE_NEUTRAL))
        {
            const int left = 2 * HUDPOS_X_BOTTOMLEFT, top = VIRTH;
            defformatstring(str)("%s called a vote:", curvote->owner ? colorname(curvote->owner) : "");
            draw_text(str, left, top + 240, 255, 255, 255, votealpha);
            draw_text(curvote->desc, left, top + 320, 255, 255, 255, votealpha);
            draw_text("----", left, top + 400, 255, 255, 255, votealpha);
            formatstring(str)("%d yes vs. %d no", curvote->stats[VOTE_YES], curvote->stats[VOTE_NO]);
            draw_text(str, left, top + 480, 255, 255, 255, votealpha);

            switch(curvote->result)
            {
                case VOTE_NEUTRAL:
                    drawvoteicon(left, top, 0, 0, false);
                    if(!curvote->localplayervoted)
                        draw_text("\f3press F1/F2 to vote yes or no", left, top+560, 255, 255, 255, votealpha);
                    break;
                default:
                    drawvoteicon(left, top, (curvote->result-1)&1, 1, false);
                    formatstring(str)("\f3vote %s", curvote->result == VOTE_YES ? "PASSED" : "FAILED");
                    draw_text(str, left, top + 560, 255, 255, 255, votealpha);
                    break;
            }
        }
    }
    //else draw_textf("%c%d here F1/F2 will be praised during a vote", 20*2, VIRTH+560, '\f', 0); // see position (left/top) setting in block above

    if(menu) rendermenu();
    else if(command)
	{
		int offsetx = 40 + ((editmode && (showeditingsettings >= 2 || keepshowingeditingsettingstill)) ? ((showeditingsettings == 2 && keepshowingeditingsettingstill == 0) ? 2*VIRTW/32 : VIRTW/4) : 0);
		renderdoc(offsetx, VIRTH, max(commandh*2 - VIRTH, 0));
	}

    if(!hidespecthud && !menu && p->state==CS_DEAD && p->spectatemode<=SM_DEATHCAM)
    {
        glLoadIdentity();
        glOrtho(0, VIRTW*3/2, VIRTH*3/2, 0, -1, 1);
        const int left = (VIRTW)*3/2, top = (VIRTH*3/2)*3/4;
        draw_textf("SPACE to change view", left - (text_width("SCROLL to change player") + FONTH/2), top);
        if(multiplayer(NULL) || watchingdemo) draw_textf("SCROLL to change player", left - (text_width("SCROLL to change player") + FONTH/2), top+80);
    }

    if(editmode)
    {
        extern void renderhudtexturepreviews();
        extern void rendereditingsettings();
        renderhudtexturepreviews();
        rendereditingsettings();
    }

    if(ispaused)
    {
        glLoadIdentity();
        const char* matchpaused = "MATCH PAUSED";
        const double matchpausedfontfactor = 3 / 2.0f;
        glOrtho(0, VIRTW * matchpausedfontfactor, VIRTH * matchpausedfontfactor, 0, -1, 1);
        const int left = (VIRTW * matchpausedfontfactor) / 2 - text_width(matchpaused) / 2;
        const int top = (VIRTH * matchpausedfontfactor) * 5 / 6;
        draw_text(matchpaused, left, top);
    }

    /* * /
    glLoadIdentity();
    glOrtho(0, VIRTW*3/2, VIRTH*3/2, 0, -1, 1);
    const int tbMSGleft = (VIRTW*3/2)*5/6;
    const int tbMSGtop = (VIRTH*3/2)*7/8;
    draw_textf("!TEST BUILD!", tbMSGleft, tbMSGtop);
    / * */

    if(showspeed && !menu)
    {
        glLoadIdentity();
        glPushMatrix();
        glOrtho(0, VIRTW, VIRTH, 0, -1, 1);
        glScalef(0.8, 0.8, 1);
        draw_textf("Speed: %.2f", VIRTW/2, VIRTH, p->vel.magnitudexy());
        glPopMatrix();
    }

    if(!hidespecthud && spectating && player1->spectatemode!=SM_DEATHCAM)
    {
        glLoadIdentity();
        glOrtho(0, VIRTW*2, VIRTH*2, 0, -1, 1);
        int virtposx = (VIRTW*2)/40;
        int virtposy = (VIRTH*2)/10;
        const char *specttext = "GHOST";
        if(player1->team == TEAM_SPECT) specttext = "GHOST";
        else if(player1->team == TEAM_CLA_SPECT) specttext = "[CLA]";
        else if(player1->team == TEAM_RVSF_SPECT) specttext = "[RVSF]";
        draw_text(specttext, virtposx, virtposy*7);

        extern bool smoverviewflyforbidden();
        int lastvalid = smoverviewflyforbidden() ? SM_FLY : SM_OVERVIEW;
        int smprev = player1->spectatemode > SM_FOLLOW1ST ? (player1->spectatemode - 1) : lastvalid;
        int smnext = player1->spectatemode < lastvalid ? (player1->spectatemode + 1) : SM_FOLLOW1ST;
        int gtoffx = 250; // static offset /hoping/ to be longer than the longest(!font?) specttext
        draw_textf("\fZ%s", virtposx + gtoffx, virtposy*7 - FONTH, ghoststrings[smprev]);
        draw_textf("\fY%s", virtposx + gtoffx, virtposy*7, ghoststrings[player1->spectatemode]);
        draw_textf("\fZ%s", virtposx + gtoffx, virtposy*7 + FONTH, ghoststrings[smnext]);

        if(is_spect)
        {
            defformatstring(name)("Player %s [cn:%d]", colorname(players[player1->followplayercn]), player1->followplayercn);
            draw_text(name, virtposx, virtposy*8);
        }
    }

    if(p->state == CS_ALIVE || (p->state == CS_DEAD && p->spectatemode == SM_DEATHCAM))
    {
        glLoadIdentity();
        glOrtho(0, VIRTW/2, VIRTH/2, 0, -1, 1);

        if(p->state == CS_ALIVE && !hidehudequipment)
        {
            pushfont("huddigits");
            draw_textf("%d", HUDPOS_HEALTH + HUDPOS_NUMBERSPACING, 823, p->health);
            if(p->armour) draw_textf("%d", HUDPOS_ARMOUR + HUDPOS_NUMBERSPACING, 823, p->armour);
            if(p->weaponsel && valid_weapon(p->weaponsel->type))
            {
                glMatrixMode(GL_MODELVIEW);
                if (p->weaponsel->type!=GUN_GRENADE) p->weaponsel->renderstats();
                else if (p->prevweaponsel->type==GUN_AKIMBO || p->prevweaponsel->type==GUN_PISTOL) p->weapons[p->akimbo ? GUN_AKIMBO : GUN_PISTOL]->renderstats();
                else p->weapons[getprevweaponsel(p)]->renderstats();
                if(p->mag[GUN_GRENADE]) p->weapons[GUN_GRENADE]->renderstats();
                glMatrixMode(GL_PROJECTION);
            }
            popfont();
        }

        if((m_flags_ || m_teammode) && !hideteamscorehud)
        {
            glLoadIdentity();
            glOrtho(0, VIRTW, VIRTH, 0, -1, 1);
            glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
            turn_on_transparency(255);
            int scores[4], offs = m_flags_ ? 0 : 2;
            calcteamscores(scores);
            const char *cc = scores[offs] > 99 || scores[offs + 1] > 99 ? "31" : "55";

            if(!offs || scores[2] || scores[3]) loopi(2) // flag state or frag-counter
            {
                bool inbase = flaginfos[i].state == CTFF_INBASE || offs;
                drawctficon(i*120+VIRTW/4.0f*3.0f, 1650, 120, i + (inbase || flagscorehudtransparency == 2 ? 0 : 2), offs / 2, 1/4.0f, inbase || !flagscorehudtransparency ? 255 : 100);
                if(m_teammode)
                {
                    defformatstring(count)("\f%c%d", cc[i], scores[i + offs]);
                    int countwidth = text_width(count);
                    draw_text(count, i * 120 + VIRTW / 4.0f * 3.0f + (countwidth > 114 ? (i ? 3 : (120 - countwidth - 3)) : (60 - countwidth / 2)), 1590);
                }
            }
            if(!offs)
            {
                // big flag-stolen icon
                int ft = 0;
                if((flaginfos[0].state==CTFF_STOLEN && flaginfos[0].actor == p && flaginfos[0].ack) ||
                   (flaginfos[1].state==CTFF_STOLEN && flaginfos[1].actor == p && flaginfos[1].ack && ++ft))
                {
                    drawctficon(VIRTW-225-10, VIRTH*5/8, 225, ft, 1, 1/2.0f, (sinf(lastmillis/100.0f)+1.0f) *128);
                }
            }
        }
    }

    if(!hidehudmsgs) hudmsgs.render();

    glLoadIdentity();
    glOrtho(0, VIRTW, VIRTH, 0, -1, 1);
    if(m_flags_ && player1->spectatemode==SM_OVERVIEW && overviewflags==2)
    {
        float iconsize = radarentsize/0.2f;
        vec coordtrans = vec(VIRTW/(1.0f*clmapdims.xspan+4), VIRTH/(1.0f*clmapdims.yspan+4), 1); // compare orthd for SM_OVERVIEW in setperspective()
        vec coordcenter = vec(clmapdims.x1+clmapdims.xspan/2,clmapdims.y1+clmapdims.yspan/2,0);
        bool axisxoff = clmapdims.xspan < clmapdims.yspan; // the axis of the not-gdim coordspan needs fixing
        float axisscale = (1000.0f / (axisxoff ? (clmapdims.yspan/(1.0f*clmapdims.xspan)) : (clmapdims.xspan/(1.0f*clmapdims.yspan))))/1000.0f; // not used if !axisdiff
        if(axisxoff)
        {
            coordtrans.x *= axisscale;
        }else{
            coordtrans.y *= axisscale;
        }
        glColor4f(1.0f, 1.0f, 1.0f, (sinf(lastmillis / 100.0f) + 1.0f) / 2.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        loopi(2) // flag items
        {
            flaginfo &f = flaginfos[i];
            entity *e = f.flagent;
            bool hasflagent = e && e->x != -1 && e->y != -1; // no base for flagentitydummies (HTF on maps without bases)
            if(hasflagent)
            {
                vec pos = vec((e->x-coordcenter.x)*coordtrans.x, (e->y-coordcenter.y)*coordtrans.y, 0);
                drawradarent(pos.x+VIRTW/2, pos.y+VIRTH/2, 0, m_ktf ? 2 : f.team, 3, iconsize, false); // draw bases
            }
            if((f.state == CTFF_INBASE && hasflagent) || f.state == CTFF_DROPPED)
            {
                vec cpos = vec((f.pos.x-coordcenter.x)*coordtrans.x, (f.pos.y-coordcenter.y)*coordtrans.y, f.pos.z);
                float flgoff=fabs(radarentsize*2.1f);
                drawradarent(cpos.x+flgoff+VIRTW/2, cpos.y-flgoff+VIRTH/2, 0, 3, m_ktf ? 2 : f.team, iconsize, false); // draw on entity pos or whereever dropped
            }
            if(f.state == CTFF_STOLEN)
            {
                float d2c = 1.6f * radarentsize/16.0f;
                vec apos(d2c, -d2c, 0);
                if(f.actor)
                {
                    apos.add(f.actor->o).sub(coordcenter);
                    bool tm = i != team_base(player1->team);
                    if(m_htf) tm = !tm;
                    else if(m_ktf) tm = true;
                    if(tm)
                    {
                        apos.x *= coordtrans.x;
                        apos.y *= coordtrans.y;
                        drawradarent(apos.x+VIRTW/2, apos.y+VIRTH/2, 0, 3, m_ktf ? 2 : f.team, iconsize, true); // draw near flag thief
                    }
                }
            }
        }
    }
    if(command)
    {
        glEnable(GL_BLEND);
        rendercommand(HUDPOS_X_BOTTOMLEFT, HUDPOS_Y_BOTTOMLEFT, VIRTW - FONTH);
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
}


Texture *startscreen = NULL;

void loadingscreen(const char *fmt, ...)
{
    if(!startscreen) startscreen = textureload("packages/misc/startscreen.png", 3);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, VIRTW, VIRTH, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0, 0, 0, 1);
    glColor3f(1, 1, 1);

    loopi(fmt ? 1 : 2)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        quad(startscreen->id, (VIRTW-VIRTH)/2, 0, VIRTH, 0, 0, 1);
        if(fmt)
        {
            glEnable(GL_BLEND);
            defvformatstring(str, fmt, fmt);
            int w = text_width(str);
            draw_text(str, w>=VIRTW ? 0 : (VIRTW-w)/2, VIRTH*3/4);
            glDisable(GL_BLEND);
        }
        SDL_GL_SwapWindow(screen);
    }

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
}

static void bar(float bar, int o, float r, float g, float b)
{
    int side = 2*FONTH;
    float x1 = side, x2 = bar*(VIRTW*1.2f-2*side)+side;
    float y1 = o*FONTH;
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_TRIANGLE_STRIP);
    loopk(10)
    {
       float c = 1.2f*cosf(PI/2 + k/9.0f*PI), s = 1 + 1.2f*sinf(PI/2 + k/9.0f*PI);
       glVertex2f(x2 - c*FONTH, y1 + s*FONTH);
       glVertex2f(x1 + c*FONTH, y1 + s*FONTH);
    }
    glEnd();

    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_STRIP);
    loopk(10)
    {
       float c = cosf(PI/2 + k/9.0f*PI), s = 1 + sinf(PI/2 + k/9.0f*PI);
       glVertex2f(x2 - c*FONTH, y1 + s*FONTH);
       glVertex2f(x1 + c*FONTH, y1 + s*FONTH);
    }
    glEnd();
}

void show_out_of_renderloop_progress(float bar1, const char *text1, float bar2, const char *text2)   // also used during loading
{
    c2skeepalive();

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, VIRTW*1.2f, VIRTH*1.2f, 0, -1, 1);

    glLineWidth(3);

    if(text1)
    {
        bar(1, 1, 0.1f, 0.1f, 0.1f);
        if(bar1>0) bar(bar1, 1, 0.2f, 0.2f, 0.2f);
    }

    if(bar2>0)
    {
        bar(1, 3, 0.1f, 0.1f, 0.1f);
        bar(bar2, 3, 0.2f, 0.2f, 0.2f);
    }

    glLineWidth(1);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    if(text1) draw_text(text1, 2*FONTH, 1*FONTH + FONTH/2);
    if(bar2>0) draw_text(text2, 2*FONTH, 3*FONTH + FONTH/2);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    SDL_GL_SwapWindow(screen);
}
