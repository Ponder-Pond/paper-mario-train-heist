#include "battle/battle.h"

static Vec3i green_koopa_bandit_pos = { 90, 25, 20 };
static Vec3i green_buzzy_beetle_pos = { 120, 25, 20 };
static Vec3i brigader_bones_pos = { 60, 0, 40 };
static Vec3i yellow_koopa_bandit_pos = { 85, 0, 15 };
static Vec3i giant_chomp_pos = { 25, 0, 20 };
static Vec3i yellow_hammer_bro_pos = { 125, 0, 10 };
static Vec3i black_koopa_bandit_pos = { 115, 10, 20 };
static Vec3i crate_pos = { 15, 0, 20 };
static Vec3i dyanmite_pos = { 55, 0, 20 };
static Vec3i rider1_pos = { 80, -25, -52 };
static Vec3i rider2_pos = { 40, -25, -50 };
static Vec3i red_koopa_bandit_pos = { 115, 22, 10 };
static Vec3i red_pyro_guy_pos = { 150, 54, 10 };
static Vec3i koopa_the_kid_pos = { 105, 45, 0 };
static Vec3i koopa_gang_pos = { 30, 0, 20 };
static Vec3i green_tower_pos = { 30, 0, 5 };
static Vec3i yellow_tower_pos = { 60, 0, 10 };
static Vec3i black_tower_pos = { 90, 0, 15 };
static Vec3i red_tower_pos = { 120, 0, 20 };
// static Vec3i green_hammer_bro_pos = { 0, 0, 0 };
static Vec3i calamity_kammy_pos = { 60, 0, 0 };

// static Vec3i koopa_the_kid_testing_pos = { 130, 35, 20 };
// static Vec3i koopa_gang_testing_pos = { 60, 0, 20 };
// static Vec3i hammer_bro_testing_pos = { 15, 0, 20 };
// static Vec3i howitzer_hal_pos = { -5, 0, 25 };
// static Vec3i yellow_testing_pos = { 105, 0, 10 };
// static Vec3i giant_chomp_testing_pos = { 25, 0, 10 };
// static Vec3i green_hammer_bro_pos = { 145, 0, 10 };
// static Vec3i black_koopa_bandit_pos = { 140, 10, 20 };
// static Vec3i crate_pos = { 15, 0, 20 };
// static Vec3i dyanmite_pos = { 55, 0, 20 };
// static Vec3i rider1_pos = { 45, -25, -50 };
// static Vec3i rider2_pos = { -25, -25, -50 };

// static Vec3i yellow_koopa_bandit_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i giant_chomp_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i yellow_hammer_bro_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i black_koopa_bandit_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i crate_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i dyanmite_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i rider1_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i rider2_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i red_koopa_bandit_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i pyro_guy_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i koopa_the_kid_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i koopa_gang_pos = { NPC_DISPOSE_LOCATION };
// static Vec3i green_hammer_bro_pos = { NPC_DISPOSE_LOCATION };


// [BTL_POS_GROUND_A] { 5, 0, -20 },
// [BTL_POS_GROUND_B] { 45, 0, -5 },
// [BTL_POS_GROUND_C] { 85, 0, 10 },
// [BTL_POS_GROUND_D] { 125, 0, 25 },

static Formation train_heist = {
    OVL_ACTOR_BY_POS("green_bandit_koopa", green_koopa_bandit_pos, 8),
    OVL_ACTOR_BY_POS("green_buzzy_beetle", green_buzzy_beetle_pos, 9),
    OVL_ACTOR_BY_POS("brigader_bones", brigader_bones_pos, 10),
    // OVL_ACTOR_BY_POS("yellow_bandit_koopa", yellow_koopa_bandit_pos, 8),
    // OVL_ACTOR_BY_POS("giant_chain_chomp", giant_chomp_pos, 10),
    // OVL_ACTOR_BY_POS("yellow_hammer_bro", yellow_hammer_bro_pos, 9),
    // OVL_ACTOR_BY_POS("black_bandit_koopa", black_koopa_bandit_pos, 8),
    // OVL_ACTOR_BY_POS("crate", crate_pos, 10),
    // OVL_ACTOR_BY_POS("dyanmite_crate", dyanmite_pos, 10),
    // OVL_ACTOR_BY_POS("shy_guy_rider", rider1_pos, 9),
    // OVL_ACTOR_BY_POS("shy_guy_rider", rider2_pos, 10),
    // OVL_ACTOR_BY_POS("red_bandit_koopa", red_koopa_bandit_pos, 10),
    // OVL_ACTOR_BY_POS("red_pyro_guy", pyro_guy_pos, 9),
    // OVL_ACTOR_BY_POS("koopa_the_kid", koopa_the_kid_pos, 8),
    // OVL_ACTOR_BY_POS("koopa_gang", koopa_gang_pos, 9),
    // OVL_ACTOR_BY_POS("green_hammer_bro", green_hammer_bro_pos, 10),
};

static Formation green_phase = {
    OVL_ACTOR_BY_POS("green_bandit_koopa", green_koopa_bandit_pos, 8),
    OVL_ACTOR_BY_POS("green_buzzy_beetle", green_buzzy_beetle_pos, 9),
    OVL_ACTOR_BY_POS("brigader_bones", brigader_bones_pos, 10),
};

static Formation yellow_phase = {
    OVL_ACTOR_BY_POS("yellow_bandit_koopa", yellow_koopa_bandit_pos, 8),
    OVL_ACTOR_BY_POS("giant_chain_chomp", giant_chomp_pos, 10),
    OVL_ACTOR_BY_POS("yellow_hammer_bro", yellow_hammer_bro_pos, 9),
};

static Formation black_phase = {
    OVL_ACTOR_BY_POS("black_bandit_koopa", black_koopa_bandit_pos, 8),
    OVL_ACTOR_BY_POS("crate", crate_pos, 10),
    OVL_ACTOR_BY_POS("dyanmite_crate", dyanmite_pos, 10),
    OVL_ACTOR_BY_POS("shy_guy_rider", rider1_pos, 9),
    OVL_ACTOR_BY_POS("shy_guy_rider", rider2_pos, 10),
};

static Formation red_phase = {
    OVL_ACTOR_BY_POS("red_bandit_koopa", red_koopa_bandit_pos, 10),
    OVL_ACTOR_BY_POS("red_pyro_guy", red_pyro_guy_pos, 9),
};

static Formation bowser_phase = {
    OVL_ACTOR_BY_POS("koopa_the_kid", koopa_the_kid_pos, 9),
    OVL_ACTOR_BY_POS("koopa_gang", koopa_gang_pos, 10),
    OVL_ACTOR_BY_POS("tower_green_bandit", green_tower_pos, 10),
    OVL_ACTOR_BY_POS("tower_yellow_bandit", yellow_tower_pos, 10),
    OVL_ACTOR_BY_POS("tower_black_bandit", black_tower_pos, 10),
    OVL_ACTOR_BY_POS("tower_red_bandit", red_tower_pos, 10),
    // ACTOR_BY_POS(GreenHammerBro, GreenHammerBroPos, 10),
};

static Formation calamity_kammy = {
    OVL_ACTOR_BY_POS("calamity_kammy", calamity_kammy_pos, 10),
};

static BattleList Formations = {
    BATTLE(train_heist, "trn_00", "Train Heist"), // Battle 0
    BATTLE(green_phase, "trn_00", "Train Heist Green Phase"), // Battle 1
    BATTLE(yellow_phase, "trn_00", "Train Heist Yellow Phase"), // Battle 2
    BATTLE(black_phase, "trn_00", "Train Heist Black Phase"), // Battle 3
    BATTLE(red_phase, "trn_00", "Train Heist Red Phase"), // Battle 4
    BATTLE(bowser_phase, "trn_00", "Train Heist Bowser Phase"), // Battle 5
    BATTLE(calamity_kammy, "trn_00", "Calamity Kammy"), // Battle 6
    {},
};

OVL_DEF_BATTLE_AREA(Formations);
