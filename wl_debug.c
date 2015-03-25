// WL_DEBUG.C

#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include "wl_def.h"



/*
=============================================================================

                                                 LOCAL CONSTANTS

=============================================================================
*/

#define VIEWTILEX       (viewwidth/16)
#define VIEWTILEY       (viewheight/16)

/*
=============================================================================

                                                 GLOBAL VARIABLES

=============================================================================
*/

#ifdef DEBUGKEYS

int DebugKeys (void);


// from WL_DRAW.C

void ScalePost();
void SimpleScaleShape (int xcenter, int shapenum, unsigned height);

/*
=============================================================================

                                                 LOCAL VARIABLES

=============================================================================
*/

int     maporgx;
int     maporgy;


void ViewMap (void);

//===========================================================================

/*
==================
=
= CountObjects
=
==================
*/

void CountObjects (void)
{
    int     i,total,count,active,inactive,doors;
    objtype *obj;

    CenterWindow (17,7);
    active = inactive = count = doors = 0;

    US_Print ("Total statics :");
    total = (int)(laststatobj-&statobjlist[0]);
    US_PrintUnsigned (total);

    char str[60];
    sprintf(str,"\nlaststatobj=%.8X",(int32_t)(uintptr_t)laststatobj);
    US_Print(str);

    US_Print ("\nIn use statics:");
    for (i=0;i<total;i++)
    {
        if (statobjlist[i].shapenum != -1)
            count++;
        else
            doors++;        //debug
    }
    US_PrintUnsigned (count);

    US_Print ("\nDoors         :");
    US_PrintUnsigned (doornum);

    for (obj=player->next;obj;obj=obj->next)
    {
        if (obj->active)
            active++;
        else
            inactive++;
    }

    US_Print ("\nTotal actors  :");
    US_PrintUnsigned (active+inactive);

    US_Print ("\nActive actors :");
    US_PrintUnsigned (active);

    VW_UpdateScreen();
    IN_Ack ();
}


//===========================================================================

/*
===================
=
= PictureGrabber
=
===================
*/
void PictureGrabber (void)
{
}


//===========================================================================

/*
===================
=
= BasicOverhead
=
===================
*/

void BasicOverhead (void)
{
    int x, y, z, offx, offy;

    z = 128/MAPSIZE; // zoom scale
    offx = 320/2;
    offy = (160-MAPSIZE*z)/2;

#ifdef MAPBORDER
    int temp = viewsize;
    NewViewSize(16);
    DrawPlayBorder();
#endif

    // right side (raw)

    for(x=0;x<MAPSIZE;x++)
        for(y=0;y<MAPSIZE;y++)
            VWB_Bar(x*z+offx, y*z+offy,z,z,(unsigned)(uintptr_t)actorat[x][y]);

    // left side (filtered)

    uintptr_t tile;
    int color;
    offx -= 128;

    for(x=0;x<MAPSIZE;x++)
    {
        for(y=0;y<MAPSIZE;y++)
        {
            tile = (uintptr_t)actorat[x][y];
            if (ISPOINTER(tile) && ((objtype *)tile)->flags&FL_SHOOTABLE) color = 72;  // enemy
            else if (!tile || ISPOINTER(tile))
            {
                if (spotvis[x][y]) color = 111;  // visable
                else color = 0;  // nothing
            }
            else if (MAPSPOT(x,y,1) == PUSHABLETILE) color = 171;  // pushwall
            else if (tile == 64) color = 158; // solid obj
            else if (tile < 128) color = 154;  // walls
            else if (tile < 256) color = 146;  // doors

            VWB_Bar(x*z+offx, y*z+offy,z,z,color);
        }
    }

    VWB_Bar(player->tilex*z+offx,player->tiley*z+offy,z,z,15); // player

    // resize the border to match

    VW_UpdateScreen();
    IN_Ack();

#ifdef MAPBORDER
    NewViewSize(temp);
    DrawPlayBorder();
#endif
}


//===========================================================================

/*
================
=
= ShapeTest
=
================
*/

void ShapeTest (void)
{
    //TODO
}


//===========================================================================


/*
================
=
= DebugKeys
=
================
*/

int DebugKeys (void)
{
    boolean esc;
    int level;

    if (Keyboard[sc_B])             // B = border color
    {
        CenterWindow(20,3);
        PrintY+=6;
        US_Print(" Border color (0-56): ");
        VW_UpdateScreen();
        esc = !US_LineInput (px,py,str,NULL,true,2,0);
        if (!esc)
        {
            level = atoi (str);
            if (level>=0 && level<=99)
            {
                if (level<30) level += 31;
                else
                {
                    if (level > 56) level=31;
                    else level -= 26;
                }

                bordercol=level*4+3;

                if (bordercol == VIEWCOLOR)
                    DrawStatusBorder(bordercol);

                DrawPlayBorder();

                return 0;
            }
        }
        return 1;
    }
    if (Keyboard[sc_C])             // C = count objects
    {
        CountObjects();
        return 1;
    }
    if (Keyboard[sc_D])             // D = Darkone's FPS counter
    {
        CenterWindow (22,2);
        if (fpscounter)
            US_PrintCentered ("Darkone's FPS Counter OFF");
        else
            US_PrintCentered ("Darkone's FPS Counter ON");
        VW_UpdateScreen();
        IN_Ack();
        fpscounter ^= 1;
        return 1;
    }
    if (Keyboard[sc_E])             // E = quit level
        playstate = ex_completed;

    if (Keyboard[sc_F])             // F = facing spot
    {
        char str[60];
        CenterWindow (14,6);
        US_Print ("x:");     US_PrintUnsigned (player->x);
        US_Print (" (");     US_PrintUnsigned (player->x%65536);
        US_Print (")\ny:");  US_PrintUnsigned (player->y);
        US_Print (" (");     US_PrintUnsigned (player->y%65536);
        US_Print (")\nA:");  US_PrintUnsigned (player->angle);
        US_Print (" X:");    US_PrintUnsigned (player->tilex);
        US_Print (" Y:");    US_PrintUnsigned (player->tiley);
        US_Print ("\n1:");   US_PrintUnsigned (tilemap[player->tilex][player->tiley]);
        sprintf(str," 2:%.8X",(unsigned)(uintptr_t)actorat[player->tilex][player->tiley]); US_Print(str);
        US_Print ("\nf 1:"); US_PrintUnsigned (player->areanumber);
        US_Print (" 2:");    US_PrintUnsigned (MAPSPOT(player->tilex,player->tiley,1));
        US_Print (" 3:");
        if ((unsigned)(uintptr_t)actorat[player->tilex][player->tiley] < 256)
            US_PrintUnsigned (spotvis[player->tilex][player->tiley]);
        else
            US_PrintUnsigned (actorat[player->tilex][player->tiley]->flags);
        VW_UpdateScreen();
        IN_Ack();
        return 1;
    }

    if (Keyboard[sc_G])             // G = god mode
    {
        CenterWindow (12,2);
        if (godmode == 0)
            US_PrintCentered ("God mode ON");
        else if (godmode == 1)
            US_PrintCentered ("God (no flash)");
        else if (godmode == 2)
            US_PrintCentered ("God mode OFF");

        VW_UpdateScreen();
        IN_Ack();
        if (godmode != 2)
            godmode++;
        else
            godmode = 0;
        return 1;
    }
    if (Keyboard[sc_H])             // H = hurt self
    {
        IN_ClearKeysDown ();
        TakeDamage (16,NULL);
    }
    else if (Keyboard[sc_I])        // I = item cheat
    {
        CenterWindow (12,3);
        US_PrintCentered ("Free items!");
        VW_UpdateScreen();
        GivePoints (100000);
        HealSelf (99);
        if (gamestate.bestweapon<wp_chaingun)
            GiveWeapon (gamestate.bestweapon+1);
        gamestate.ammo += 50;
        if (gamestate.ammo > 99)
            gamestate.ammo = 99;
        DrawAmmo ();
        IN_Ack ();
        return 1;
    }
    else if (Keyboard[sc_K])        // K = give keys
    {
        CenterWindow(16,3);
        PrintY+=6;
        US_Print("  Give Key (1-4): ");
        VW_UpdateScreen();
        esc = !US_LineInput (px,py,str,NULL,true,1,0);
        if (!esc)
        {
            level = atoi (str);
            if (level>0 && level<5)
                GiveKey(level-1);
        }
        return 1;
    }
    else if (Keyboard[sc_L])        // L = level ratios
    {
        byte x,start,end=LRpack;

        if (end == 8)   // wolf3d
        {
            CenterWindow(17,10);
            start = 0;
        }
        else            // sod
        {
            CenterWindow(17,12);
            start = 0; end = 10;
        }
again:
        for(x=start;x<end;x++)
        {
            US_PrintUnsigned(x+1);
            US_Print(" ");
            US_PrintUnsigned(LevelRatios[x].time/60);
            US_Print(":");
            if (LevelRatios[x].time%60 < 10)
                US_Print("0");
            US_PrintUnsigned(LevelRatios[x].time%60);
            US_Print(" ");
            US_PrintUnsigned(LevelRatios[x].kill);
            US_Print("% ");
            US_PrintUnsigned(LevelRatios[x].secret);
            US_Print("% ");
            US_PrintUnsigned(LevelRatios[x].treasure);
            US_Print("%\n");
        }
        VW_UpdateScreen();
        IN_Ack();
        if (end == 10 && gamestate.mapon > 9)
        {
            start = 10; end = 20;
            CenterWindow(17,12);
            goto again;
        }

        return 1;
    }
    else if (Keyboard[sc_N])        // N = no clip
    {
        noclip^=1;
        CenterWindow (18,3);
        if (noclip)
            US_PrintCentered ("No clipping ON");
        else
            US_PrintCentered ("No clipping OFF");
        VW_UpdateScreen();
        IN_Ack ();
        return 1;
    }
    else if (Keyboard[sc_O])        // O = basic overhead
    {
        BasicOverhead();
        return 1;
    }
    else if(Keyboard[sc_P])         // P = Ripper's picture grabber
    {
        PictureGrabber();
        return 1;
    }
    else if (Keyboard[sc_Q])        // Q = fast quit
        Quit (NULL);
    else if (Keyboard[sc_S])        // S = slow motion
    {
        CenterWindow(30,3);
        PrintY+=6;
        US_Print(" Slow Motion steps (default 14): ");
        VW_UpdateScreen();
        esc = !US_LineInput (px,py,str,NULL,true,2,0);
        if (!esc)
        {
            level = atoi (str);
            if (level>=0 && level<=50)
                singlestep = level;
        }
        return 1;
    }
    else if (Keyboard[sc_T])        // T = shape test
    {
        ShapeTest ();
        return 1;
    }
    else if (Keyboard[sc_V])        // V = extra VBLs
    {
        CenterWindow(30,3);
        PrintY+=6;
        US_Print("  Add how many extra VBLs(0-8): ");
        VW_UpdateScreen();
        esc = !US_LineInput (px,py,str,NULL,true,1,0);
        if (!esc)
        {
            level = atoi (str);
            if (level>=0 && level<=8)
                extravbls = level;
        }
        return 1;
    }
    else if (Keyboard[sc_W])        // W = warp to level
    {
        CenterWindow(26,3);
        PrintY+=6;
#ifndef SPEAR
        US_Print("  Warp to which level(1-10): ");
#else
        US_Print("  Warp to which level(1-21): ");
#endif
        VW_UpdateScreen();
        esc = !US_LineInput (px,py,str,NULL,true,2,0);
        if (!esc)
        {
            level = atoi (str);
#ifndef SPEAR
            if (level>0 && level<11)
#else
            if (level>0 && level<22)
#endif
            {
                gamestate.mapon = level-1;
                playstate = ex_warped;
            }
        }
        return 1;
    }
    else if (Keyboard[sc_X])        // X = item cheat
    {
        CenterWindow (12,3);
        US_PrintCentered ("Extra stuff!");
        VW_UpdateScreen();
        // DEBUG: put stuff here
        IN_Ack ();
        return 1;
    }


    return 0;
}


#endif
