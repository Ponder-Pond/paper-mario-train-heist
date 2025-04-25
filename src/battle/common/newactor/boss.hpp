#pragma once

namespace battle::actor {

enum BossCommon {
    // Train Heist Formation Actors
    ACTOR_GREEN_BANDIT                  = ACTOR_ENEMY0,
    ACTOR_BUZZY_BEETLE                  = ACTOR_ENEMY1,
    ACTOR_BRIGADER_BONES                = ACTOR_ENEMY2,
    ACTOR_YELLOW_BANDIT                 = ACTOR_ENEMY1,
    ACTOR_GIANT_CHOMP                   = ACTOR_ENEMY2,
    ACTOR_YELLOW_HAMMER_BRO                = ACTOR_ENEMY3,
    ACTOR_BLACK_BANDIT                  = ACTOR_ENEMY0,
    ACTOR_CRATE                         = ACTOR_ENEMY1,
    ACTOR_DYANMITE_CRATE                = ACTOR_ENEMY3,
    ACTOR_SHY_GUY_RIDER_1               = ACTOR_ENEMY4,
    ACTOR_SHY_GUY_RIDER_2               = ACTOR_ENEMY5,
    ACTOR_RED_BANDIT                    = ACTOR_ENEMY1,
    ACTOR_PYRO_GUY                      = ACTOR_ENEMY2,
    ACTOR_BOB_OMB                       = ACTOR_ENEMY0,
    ACTOR_BOWSER_THE_KID                = ACTOR_ENEMY0,
    ACTOR_KOOPA_GANG                    = ACTOR_ENEMY1,
    ACTOR_GREEN_HAMMER_BRO                    = ACTOR_ENEMY2,
    // // Green Phase Actors
    // ACTOR_GREEN_BANDIT                  = ACTOR_ENEMY0,
    // ACTOR_BUZZY_BEETLE                  = ACTOR_ENEMY1,
    // ACTOR_BRIGADER_BONES                = ACTOR_ENEMY2,
    // // Yellow Phase Actors
    // ACTOR_YELLOW_BANDIT                 = ACTOR_ENEMY0,
    // ACTOR_GIANT_CHOMP                   = ACTOR_ENEMY1,
    // ACTOR_YELLOW_HAMMER_BRO                = ACTOR_ENEMY2,
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
    // Bowser Phase Actors
    // ACTOR_BOWSER_THE_KID                = ACTOR_ENEMY0,
    // ACTOR_KOOPA_GANG                    = ACTOR_ENEMY1,
    // ACTOR_GREEN_HAMMER_BRO                    = ACTOR_ENEMY2,
    // Phase Actor Vars
    AVAR_Phase                          = 0,
    AVAL_IntroPhase                     = 0,
    AVAL_GreenPhase                     = 1,
    AVAL_YellowPhase                    = 2,
    AVAL_BlackPhase                     = 3,
    AVAL_RedPhase                       = 4,
    AVAL_BowserPhase                    = 5,
    // Green Phase Actor Vars
    AVAR_GreenPhase_CannonAttacks       = 1,
    AVAL_GreenPhase_SlowCannonAttack    = 0,
    AVAL_GreenPhase_FastCannonAttack    = 1,
    AVAR_GreenPhase_BrigaderCommand     = 2,
    AVAR_GreenPhase_ActorsSpawned       = 3,
    // Yellow Phase Actor Vars
    AVAR_YellowPhase_ActorsSpawned      = 1,
    // Black Phase Actor Vars
    AVAR_BlackPhase_RidersDefeated      = 1,
    AVAR_BlackPhase_ActorsSpawned       = 2,
    // Red Phase Actor Vars
    AVAR_RedPhase_PyroDefeated          = 1,
    AVAR_RedPhase_RandomAttack          = 2,
    AVAL_RedPhase_LitBombAttack         = 0,
    AVAL_RedPhase_LitBobombSummon       = 1,
    AVAL_RedPhase_UnlitBombAttack       = 2,
    AVAL_RedPhase_UnlitBobombSummon     = 3,
    AVAL_RedPhase_PokeyAttack           = 4,
    AVAR_RedPhase_SummonedBobomb        = 3,
    AVAR_RedPhase_BobOmbIgnited         = 4,
    AVAR_RedPhase_ActorsSpawned         = 5,

    // Bowser Phase Actor Vars
    AVAR_BowserPhase_KoopaGangSpitAttack = 1,
};

}; // namespace battle::actor
