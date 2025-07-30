#pragma once

namespace battle::actor {

enum BossCommon {
    // Train Heist Formation Actors
    ACTOR_GREEN_BANDIT                  = ACTOR_ENEMY0,
    ACTOR_BUZZY_BEETLE                  = ACTOR_ENEMY1,
    ACTOR_BRIGADER_BONES                = ACTOR_ENEMY2,
    ACTOR_YELLOW_BANDIT                 = ACTOR_ENEMY1,
    ACTOR_GIANT_CHOMP                   = ACTOR_ENEMY2,
    ACTOR_YELLOW_HAMMER_BRO             = ACTOR_ENEMY3,
    ACTOR_BLACK_BANDIT                  = ACTOR_ENEMY0,
    ACTOR_CRATE                         = ACTOR_ENEMY2,
    ACTOR_DYANMITE_CRATE                = ACTOR_ENEMY3,
    ACTOR_SHY_GUY_RIDER_1               = ACTOR_ENEMY4,
    ACTOR_SHY_GUY_RIDER_2               = ACTOR_ENEMY5,
    ACTOR_RED_BANDIT                    = ACTOR_ENEMY1,
    ACTOR_PYRO_GUY                      = ACTOR_ENEMY2,
    ACTOR_BOB_OMB                       = ACTOR_ENEMY0,
    ACTOR_KOOPA_THE_KID                = ACTOR_ENEMY0,
    ACTOR_KOOPA_GANG                    = ACTOR_ENEMY6,
    GREEN_ACTOR                         = ACTOR_ENEMY2,
    YELLOW_ACTOR                        = ACTOR_ENEMY3,
    BLACK_ACTOR                         = ACTOR_ENEMY4,
    RED_ACTOR                           = ACTOR_ENEMY5,
    ACTOR_STAGE_GUY                     = ACTOR_ENEMY7,
    // ACTOR_GREEN_HAMMER_BRO              = ACTOR_ENEMY6, // Unused in Train Heist

    // // Green Phase Actors
    // ACTOR_GREEN_BANDIT                  = ACTOR_ENEMY0,
    // ACTOR_BUZZY_BEETLE                  = ACTOR_ENEMY1,
    // ACTOR_BRIGADER_BONES                = ACTOR_ENEMY2,
    // // Yellow Phase Actors
    // ACTOR_YELLOW_BANDIT                 = ACTOR_ENEMY0,
    // ACTOR_GIANT_CHOMP                   = ACTOR_ENEMY1,
    // ACTOR_YELLOW_HAMMER_BRO             = ACTOR_ENEMY2,
    // // Black Phase Actors
    // ACTOR_BLACK_BANDIT                  = ACTOR_ENEMY0,
    // ACTOR_CRATE                         = ACTOR_ENEMY1,
    // ACTOR_DYANMITE_CRATE                = ACTOR_ENEMY2,
    // ACTOR_SHY_GUY_RIDER_1               = ACTOR_ENEMY3,
    // ACTOR_SHY_GUY_RIDER_2               = ACTOR_ENEMY4,
    // // Red Phase Actors
    // ACTOR_RED_BANDIT                    = ACTOR_ENEMY0,
    // ACTOR_PYRO_GUY                      = ACTOR_ENEMY1,
    // ACTOR_BOB_OMB                       = ACTOR_ENEMY2,
    // // Bowser Phase Actors
    // ACTOR_KOOPA_THE_KID                = ACTOR_ENEMY0,
    // ACTOR_KOOPA_GANG                    = ACTOR_ENEMY1,
    // ACTOR_GREEN_HAMMER_BRO              = ACTOR_ENEMY2,

    // Scene Vars
    AVAR_Scene_BeginBattle              = 0,
    AVAL_Scene_GreenPhase               = 0,
    AVAL_Scene_YellowPhase              = 1,
    AVAL_Scene_BlackPhase               = 2,
    AVAL_Scene_RedPhase                 = 3,
    AVAL_Scene_BowserPhase              = 4,
    AVAL_Scene_Defeat                   = 5,

    // Phase Actor Vars
    AVAR_Phase                          = 1,
    AVAL_IntroPhase                     = 0,
    AVAL_GreenPhase                     = 1,
    AVAL_YellowPhase                    = 2,
    AVAL_BlackPhase                     = 3,
    AVAL_RedPhase                       = 4,
    AVAL_BowserPhase                    = 5,

    // Green Phase Actor Vars
    AVAR_GreenPhase_CannonAttacks       = 2,
    AVAL_GreenPhase_SlowCannonAttack    = 0,
    AVAL_GreenPhase_FastCannonAttack    = 1,
    AVAR_GreenPhase_BrigaderCommand     = 3,
    AVAR_GreenPhase_ActorsSpawned       = 4,

    // Yellow Phase Actor Vars
    AVAR_YellowPhase_ActorsSpawned      = 2,

    // Black Phase Actor Vars
    AVAR_BlackPhase_RidersDefeated      = 2,
    AVAR_BlackPhase_ActorsSpawned       = 3,

    // Red Phase Actor Vars
    AVAR_RedPhase_PyroDefeated          = 2,
    AVAR_RedPhase_RandomAttack          = 3,
    AVAL_RedPhase_LitBombAttack         = 0,
    AVAL_RedPhase_LitBobombSummon       = 1,
    AVAL_RedPhase_UnlitBombAttack       = 2,
    AVAL_RedPhase_UnlitBobombSummon     = 3,
    AVAL_RedPhase_PokeyAttack           = 4,
    AVAR_RedPhase_SummonedBobomb        = 4,
    AVAR_RedPhase_BobOmbIgnited         = 5,
    AVAR_RedPhase_ActorsSpawned         = 6,
    AVAR_RedPhase_RedDefeated           = 7,

    // Bowser Phase Actor Vars
    AVAR_BowserPhase_KoopaGangSpitAttack = 2,
};

}; // namespace battle::actor
