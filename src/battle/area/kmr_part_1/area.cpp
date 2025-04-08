#include "area.hpp"
#include "stage/smb_bt00.hpp"

#include "battle/common/actor/false_bowser_and_axe_switch.hpp"
#include "battle/common/actor/false_bowser_and_axe_switch.inc.cpp"

using namespace battle::actor;

namespace battle::area::kmr_part_1 {

Vec3i FalseBowserPos = { 55, 0, 0 };
Vec3i AxeSwitchPos = { 179, 19, 0 };

Formation Formation_FalseBowser = { // Formation 0
    ACTOR_BY_POS(FalseBowser, FalseBowserPos, 10),
    ACTOR_BY_POS(AxeSwitch, AxeSwitchPos, 9),
};

BattleList Battles = {
    BATTLE(Formation_FalseBowser, CastleBridge, "False Bowser"),
    {},
};

StageList Stages = {
    STAGE("Castle Bride", CastleBridge), // Stage 0
    {},
};

}; // namespace battle::area::kmr_part_1
