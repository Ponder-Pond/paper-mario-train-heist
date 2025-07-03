#include "area.hpp"
#include "stage/trn_bt00.hpp"

#include "battle/common/newactor/train_heist_actors.hpp"

#include "battle/common/newactor/green_koopa_bandit.inc.cpp"
#include "battle/common/newactor/brigader_bones.inc.cpp"
#include "battle/common/newactor/yellow_koopa_bandit.inc.cpp"
#include "battle/common/newactor/giant_chain_chomp.inc.cpp"
#include "battle/common/newactor/hammer_bros.inc.cpp"
#include "battle/common/newactor/black_koopa_bandit.inc.cpp"
#include "battle/common/newactor/crate.inc.cpp"
#include "battle/common/newactor/dyanmite_crate.inc.cpp"
#include "battle/common/newactor/shy_guy_rider.inc.cpp"
#include "battle/common/newactor/red_koopa_bandit.inc.cpp"
#include "battle/common/newactor/pyro_guy.inc.cpp"
#include "battle/common/newactor/koopa_the_kid.inc.cpp"
#include "battle/common/newactor/koopa_gang.inc.cpp"

using namespace battle::actor;

namespace battle::area::kmr_part_1 {

Vec3i GreenPos = { 70, 25, 20 };
Vec3i BuzzyBeetlePos = { 100, 25, 20 };
Vec3i BrigaderBonesPos = { 23, 0, 30 };
// Vec3i YellowPos = { 105, 0, 20 };
// Vec3i GiantChompPos = { 25, 0, 21 };
// Vec3i YellowHammerBroPos = { 145, 0, 20 };
// Vec3i BlackPos = { 115, 10, 20 };
// Vec3i CratePos = { 15, 0, 20 };
// Vec3i DyanmitePos = { 55, 0, 20 };
// Vec3i Rider1Pos = { 45, -25, -50 };
// Vec3i Rider2Pos = { -25, -25, -50};
// Vec3i RedPos = { 115, 22, 10 };
// Vec3i PyroGuyPos = { 150, 54, 10 };
Vec3i KoopaTheKidPos = { 100, 35, 20 };
Vec3i KoopaGangPos = { 60, 0, 20 };
Vec3i GreenHammerBroPos = { 20, 0, 20 };



// Vec3i KoopaTheKidTestingPos = { 130, 35, 20 };
// Vec3i KoopaGangTestingPos = { 60, 0, 20 };
// Vec3i HammerBroTestingPos = { 15, 0, 20 };
// Vec3i HowitzerHalPos = { -5, 0, 25 };
// Vec3i YellowPos = { 105, 0, 10 };
// Vec3i GiantChompPos = { 25, 0, 10 };
// Vec3i GreenHammerBroPos = { 145, 0, 10 };
// Vec3i BlackPos = { 140, 10, 20 };
// Vec3i CratePos = { 15, 0, 20 };
// Vec3i DyanmitePos = { 55, 0, 20 };
// Vec3i Rider1Pos = { 45, -25, -50 };
// Vec3i Rider2Pos = { -25, -25, -50 };


Vec3i YellowPos = { NPC_DISPOSE_LOCATION };
Vec3i GiantChompPos = { NPC_DISPOSE_LOCATION };
Vec3i YellowHammerBroPos = { NPC_DISPOSE_LOCATION };
Vec3i BlackPos = { NPC_DISPOSE_LOCATION };
Vec3i CratePos = { NPC_DISPOSE_LOCATION };
Vec3i DyanmitePos = { NPC_DISPOSE_LOCATION };
Vec3i Rider1Pos = { NPC_DISPOSE_LOCATION };
Vec3i Rider2Pos = { NPC_DISPOSE_LOCATION };
Vec3i RedPos = { NPC_DISPOSE_LOCATION };
Vec3i PyroGuyPos = { NPC_DISPOSE_LOCATION };
// Vec3i KoopaTheKidPos = { NPC_DISPOSE_LOCATION };
// Vec3i KoopaGangPos = { NPC_DISPOSE_LOCATION };
// Vec3i GreenHammerBroPos = { NPC_DISPOSE_LOCATION };


// [BTL_POS_GROUND_A] { 5, 0, -20 },
// [BTL_POS_GROUND_B] { 45, 0, -5 },
// [BTL_POS_GROUND_C] { 85, 0, 10 },
// [BTL_POS_GROUND_D] { 125, 0, 25 },

Formation Formation_TrainHeist = {
    ACTOR_BY_POS(GreenBanditKoopa, GreenPos, 8),
    ACTOR_BY_POS(BuzzyBeetle, BuzzyBeetlePos, 9),
    ACTOR_BY_POS(BrigaderBones, BrigaderBonesPos, 10),
    // ACTOR_BY_POS(YellowBanditKoopa, YellowPos, 8),
    // ACTOR_BY_POS(GiantChainChomp, GiantChompPos, 10),
    // ACTOR_BY_POS(YellowHammerBro, YellowHammerBroPos, 9),
    // ACTOR_BY_POS(BlackBanditKoopa, BlackPos, 8),
    // ACTOR_BY_POS(Crate, CratePos, 10),
    // ACTOR_BY_POS(DyanmiteCrate, DyanmitePos, 10),
    // ACTOR_BY_POS(ShyGuyRider, Rider1Pos, 9),
    // ACTOR_BY_POS(ShyGuyRider, Rider2Pos, 10),
    // ACTOR_BY_POS(RedBanditKoopa, RedPos, 10),
    // ACTOR_BY_POS(PyroGuy, PyroGuyPos, 9),
    // ACTOR_BY_POS(KoopaTheKid, KoopaTheKidPos, 8),
    // ACTOR_BY_POS(KoopaGang, KoopaGangPos, 9),
    // ACTOR_BY_POS(GreenHammerBro, GreenHammerBroPos, 10),
};

Formation Formation_GreenPhase = {
    ACTOR_BY_POS(GreenBanditKoopa, GreenPos, 8),
    ACTOR_BY_POS(BuzzyBeetle, BuzzyBeetlePos, 9),
    ACTOR_BY_POS(BrigaderBones, BrigaderBonesPos, 10),
};

Formation Formation_YellowPhase = {
    ACTOR_BY_POS(YellowBanditKoopa, YellowPos, 8),
    ACTOR_BY_POS(GiantChainChomp, GiantChompPos, 10),
    ACTOR_BY_POS(YellowHammerBro, YellowHammerBroPos, 9),
};

Formation Formation_BlackPhase = {
    ACTOR_BY_POS(BlackBanditKoopa, BlackPos, 8),
    ACTOR_BY_POS(Crate, CratePos, 10),
    ACTOR_BY_POS(DyanmiteCrate, DyanmitePos, 10),
    ACTOR_BY_POS(ShyGuyRider, Rider1Pos, 9),
    ACTOR_BY_POS(ShyGuyRider, Rider2Pos, 10),
};

Formation Formation_RedPhase = {
    ACTOR_BY_POS(RedBanditKoopa, RedPos, 10),
    ACTOR_BY_POS(PyroGuy, PyroGuyPos, 9),
};

Formation Formation_BowserPhase = {
    ACTOR_BY_POS(KoopaTheKid, KoopaTheKidPos, 8),
    ACTOR_BY_POS(KoopaGang, KoopaGangPos, 9),
    ACTOR_BY_POS(GreenHammerBro, GreenHammerBroPos, 10),
};

BattleList Battles = {
    BATTLE(Formation_TrainHeist, TrainHeist, "Train Heist"), // Battle 0
    BATTLE(Formation_GreenPhase, TrainHeist, "Train Heist Green Phase"), // Battle 1
    BATTLE(Formation_YellowPhase, TrainHeist, "Train Heist Yellow Phase"), // Battle 2
    BATTLE(Formation_BlackPhase, TrainHeist, "Train Heist Black Phase"), // Battle 3
    BATTLE(Formation_RedPhase, TrainHeist, "Train Heist Red Phase"), // Battle 4
    BATTLE(Formation_BowserPhase, TrainHeist, "Train Heist Bowser Phase"), // Battle 5
    {},
};

StageList Stages = {
    STAGE("Train Heist", TrainHeist), // Stage 0
    {},
};

}; // namespace battle::area::kmr_part_1
