/*
 * Copyright (C) 2011 Atlantiss <http://www.atlantiss.pl>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
 /*
 Made By: Jenova     
 Project: Atlantiss Core  
 SDName: boss_setesh
 SD%Complete: 90%
 SDComment: Tested and Fixed most bugs
 SDCategory: Halls Of Origination
 
 Known Bugs:
 1. Seed Of Chaos doesnt give buff
 2. Chaos Portal needs modelid
 3. Sentinel needs DB fixes
 4. Missing Choas Blast

 TODO:
 1. Check Timers
 */
                             
#include "ScriptMgr.h"
#include "ScriptedCreature.h" 
#include "ScriptPCH.h"
#include "halls_of_origination.h"
enum Spells
{
    //Setesh
    SPELL_CHAOS_BOLT         = 77069,
    SPELL_REIGN_OF_CHAOS     = 77023,
    SPELL_CHAOS_BLAST        = 76681, //76676
    SPELL_SEED_OF_CHAOS      = 76870,
    SPELL_SUMMON_CHAOS_SEED  = 76888,
    
    //Sentinel
    SPELL_VOID_BARRIER       = 63710,
    SPELL_CHARGED_FISTS      = 77238,

    //Seeker
    SPELL_ANTIMAGIC_PRISON   = 76903,

    //Reign Of Chaos
    SPELL_REIGN_OF_CHAOS_DMG = 77026

};

enum Texts
{
    SAY_AGGRO = 0,
    SAY_CHAOS = 1,
    SAY_KILL_1 = 2,
    SAY_KILL_2 = 2,
    SAY_DEATH = 3
};

enum Gameobjects
{

};

enum NPCs
{
    NPC_VOID_SENTINEL       = 41208,
    NPC_VOID_SEEKER         = 41371,
    NPC_VOID_WURM           = 41374,
    NPC_CHAOS_PORTAL        = 41055
};

enum Events
{
    EVENT_CHAOS_BOLT           = 1,
    EVENT_REIGN_OF_CHAOS       = 2,
    EVENT_CHAOS_BLAST          = 3,
    EVENT_SUMMON_SEED_OF_CHAOS = 4,
    EVENT_SUMMON_CHAOS_PORTAL  = 5
};

class boss_setesh : public CreatureScript
{
    public:
        boss_setesh() : CreatureScript("boss_setesh") { }

        struct boss_seteshAI : public BossAI
        {
            boss_seteshAI(Creature* creature) : BossAI(creature, DATA_SETESH)
            {
                instance = me->GetInstanceScript();
            }

            InstanceScript* instance;

            void Reset()
            {
                if (instance)
                    instance->SetData(DATA_SETESH, NOT_STARTED);
            }

            void EnterCombat(Unit* /*who*/)
            {
                DoScriptText(SAY_AGGRO, me);

                if (instance)
                    instance->SetData(DATA_SETESH, IN_PROGRESS);

                events.ScheduleEvent(EVENT_CHAOS_BOLT, 10000);
                events.ScheduleEvent(EVENT_REIGN_OF_CHAOS, 15000);
                events.ScheduleEvent(EVENT_CHAOS_BLAST, 12000);
                events.ScheduleEvent(EVENT_SUMMON_SEED_OF_CHAOS, 20000);
                events.ScheduleEvent(EVENT_SUMMON_CHAOS_PORTAL, 20000);

                DoZoneInCombat();
            }

            void KilledUnit(Unit* victim)
            {
                Talk(RAND(SAY_KILL_1, SAY_KILL_2));
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;
                
                events.Update(diff);

                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                            case EVENT_CHAOS_BOLT:                             
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, true))
                                    DoCast(target, SPELL_CHAOS_BOLT);
                                events.ScheduleEvent(EVENT_CHAOS_BOLT, 10000);
                                break;
                            case EVENT_REIGN_OF_CHAOS:
                                DoScriptText(SAY_CHAOS, me);
                                DoCast(me, SPELL_REIGN_OF_CHAOS);
                                events.ScheduleEvent(EVENT_REIGN_OF_CHAOS, urand(35000, 45000));
                                break;
                            /*case EVENT_CHAOS_BLAST:
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, true))
                                    DoCast(target, SPELL_CHAOS_BLAST);
                                events.ScheduleEvent(EVENT_CHAOS_BLAST, 15000);
                            break;*/
                            case EVENT_SUMMON_SEED_OF_CHAOS:
                                DoCast(SPELL_SUMMON_CHAOS_SEED);
                                events.ScheduleEvent(EVENT_SUMMON_SEED_OF_CHAOS, 25000);
                                break;
                            case EVENT_SUMMON_CHAOS_PORTAL:
                                if (Creature *chaos = me->SummonCreature(NPC_CHAOS_PORTAL, me->GetPositionX()+rand()%10, me->GetPositionY()+rand()%10, me->GetPositionZ()))
                                    chaos->SetVisible(true);
                                events.ScheduleEvent(EVENT_SUMMON_CHAOS_PORTAL, 30000);
                                break;
                            default:
                                break;  
                    }
                }

                DoMeleeAttackIfReady();
            }

            void JustDied(Unit* /*who*/)
            {
                DoScriptText(SAY_DEATH, me);

                if (instance)
                    instance->SetData(DATA_SETESH, DONE);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_seteshAI(creature);
        }
};

class mob_seed_of_chaos : public CreatureScript
{
public:
    mob_seed_of_chaos() : CreatureScript("mob_seed_of_chaos") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_seed_of_chaosAI(creature);
    }

    struct mob_seed_of_chaosAI : public Scripted_NoMovementAI
    {
        mob_seed_of_chaosAI(Creature* creature) : Scripted_NoMovementAI(creature), Summons(me)
        {
            m_pInstance = (InstanceScript*)creature->GetInstanceScript();
            Reset();
        }

        InstanceScript* m_pInstance;

        SummonList Summons;
        uint32 checkTimer;

        void Reset()
        {   
            DoCast(me, 76865);
            me->SetReactState(REACT_AGGRESSIVE);
            me->setFaction(14);
            me->StopMoving();
            checkTimer = 1000;
            
        }

        void UpdateAI(uint32 const diff)
        {
            if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, true))
                if (me->IsWithinMeleeRange(target))
                {
                    me->AddAura(SPELL_SEED_OF_CHAOS, target);
                    me->DisappearAndDie();
                }
        }

        void JustDied(Unit* /*killer*/)
        {
            // used to despawn corpse immediately
            me->DespawnOrUnsummon();
        }
    };

};

class mob_choas_portal : public CreatureScript
{
public:
    mob_choas_portal() : CreatureScript("mob_choas_portal") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_choas_portalAI(creature);
    }

    struct mob_choas_portalAI : public Scripted_NoMovementAI
    {
        mob_choas_portalAI(Creature* creature) : Scripted_NoMovementAI(creature), Summons(me)
        {
            m_pInstance = (InstanceScript*)creature->GetInstanceScript();
            Reset();
        }

        InstanceScript* m_pInstance;

        SummonList Summons;
        uint32 SummonTimer;
        uint32 CastTimer;

        void Reset()
        {
            me->SetVisible(true);
            me->SetReactState(REACT_PASSIVE);

            if (!IsHeroic())
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            else
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            
            CastTimer = 1000;
            SummonTimer = 5000;
            Summons.DespawnAll();
        }
        
        void Summon(uint8 summon)
        {
            if (summon == 0)
            {
                me->SummonCreature(NPC_VOID_SEEKER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                me->SummonCreature(NPC_VOID_WURM, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
            }
            else
                me->SummonCreature(NPC_VOID_SENTINEL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
        }

        void UpdateAI(uint32 const diff)
        {
            if (SummonTimer <= diff)
            {
                uint8 summon = urand(0,1);
                Summon(summon);
                if(!IsHeroic())
                    me->DespawnOrUnsummon();
                else
                    SummonTimer = 15000;
            } else SummonTimer -= diff;

            if (CastTimer <= diff)
            {
                DoCast(me, 77607);
                CastTimer = 2000;
            } else CastTimer -= diff;
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {

        }

        void JustSummoned(Creature* summoned)
        {
            Summons.Summon(summoned);
            // makes immediate corpse despawn of summoned Felflame Infernals
            summoned->SetCorpseDelay(0);
        }

        void JustDied(Unit* /*killer*/)
        {
            // used to despawn corpse immediately
            me->DespawnOrUnsummon();
        }
    };

};

class mob_void_sentinel : public CreatureScript
{
public:
    mob_void_sentinel() : CreatureScript("mob_void_sentinel") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_void_sentinelAI(creature);
    }

    struct mob_void_sentinelAI : public ScriptedAI
    {
        mob_void_sentinelAI(Creature* creature) : ScriptedAI(creature)
        {
            m_pInstance = (InstanceScript*)creature->GetInstanceScript();
            Reset();
        }

        InstanceScript* m_pInstance;
        uint32 BarrierTimer;
        uint32 FistTimer;

        void Reset()
        {
            BarrierTimer = 10000;
            FistTimer = 3000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;
                
            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            if (BarrierTimer <= diff)
            {
                DoCast(SPELL_VOID_BARRIER);
                BarrierTimer = 25000;
            } else BarrierTimer -= diff;

            if (FistTimer <= diff)
            {
                DoCast(SPELL_CHARGED_FISTS);
                FistTimer = 25000;
            } else FistTimer -= diff;
        }

        void JustDied(Unit* /*killer*/)
        {
            // used to despawn corpse immediately
            me->DespawnOrUnsummon();
        }
    };

};

class mob_void_seeker : public CreatureScript
{
public:
    mob_void_seeker() : CreatureScript("mob_void_seeker") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_void_seekerAI(creature);
    }

    struct mob_void_seekerAI : public ScriptedAI
    {
        mob_void_seekerAI(Creature* creature) : ScriptedAI(creature)
        {
            m_pInstance = (InstanceScript*)creature->GetInstanceScript();
            Reset();
        }

        InstanceScript* m_pInstance;
        uint32 PrisonTimer;

        void Reset()
        {
            PrisonTimer = 5000;
        }

         void UpdateAI(uint32 const diff)
         {
                if (!UpdateVictim())
                    return;
                
                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

            if (PrisonTimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, true))
                    DoCast(target, SPELL_ANTIMAGIC_PRISON);
                PrisonTimer = 25000;
            } else PrisonTimer -= diff;
        }

        void JustDied(Unit* /*killer*/)
        {
            // used to despawn corpse immediately
            me->DespawnOrUnsummon();
        }
    };

};

class mob_reign_of_chaos : public CreatureScript
{
public:
    mob_reign_of_chaos() : CreatureScript("mob_reign_of_chaos") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_reign_of_chaosAI(creature);
    }

    struct mob_reign_of_chaosAI : public Scripted_NoMovementAI
    {
        mob_reign_of_chaosAI(Creature* creature) : Scripted_NoMovementAI(creature), Summons(me)
        {
            m_pInstance = (InstanceScript*)creature->GetInstanceScript();
            Reset();
        }

        InstanceScript* m_pInstance;
        SummonList Summons;
        uint32 DespawnTimer;

        void Reset()
        {   DespawnTimer = 6000;
            DoCast(me, SPELL_REIGN_OF_CHAOS_DMG);
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(uint32 const diff)
        {
            if (DespawnTimer<= diff)
            {
                me->ForcedDespawn();
            } else DespawnTimer -= diff;

        }

        void JustDied(Unit* /*killer*/)
        {
            // used to despawn corpse immediately
            me->DespawnOrUnsummon();
        }
    };

};

void AddSC_boss_setesh()
{
    new boss_setesh;
    new mob_choas_portal;
    new mob_seed_of_chaos;
    new mob_void_sentinel;
    new mob_void_seeker;
    new mob_reign_of_chaos;
}